/*
 * libmdr - MDR protocol library
 *
 *  Copyright (C) 2021 Andreas Olofsson
 *
 *
 * This file is part of libmdr.
 *
 * libmdr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libmdr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libmdr. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mdr/packetconn.h"

#include "mdr/errors.h"

#include <time.h>
#include <errno.h>

typedef struct
{
    mdr_packetconn_result_callback result;
    mdr_packetconn_error_callback error;
    void* user_data;
}
callbacks_t;

typedef struct request request_t;

struct request
{
    mdr_frame_t*                     frame;
    struct timespec                  timeout;
    int                              attempts;
    bool                             acked;
    callbacks_t                      callbacks;
    mdr_packetconn_reply_specifier_t expected_reply;

    request_t* next;
};

typedef struct subscription subscription_t;

struct subscription
{
    callbacks_t                      callbacks;
    mdr_packetconn_reply_specifier_t specifier;

    subscription_t* next;
};

struct mdr_packetconn
{
    mdr_frameconn_t* fconn;

    uint8_t next_sequence_id;

    request_t*      request, *request_queue_tail;
    subscription_t* subscription, *subscription_list_tail;
};

/*
 * Number of times to try sending a packet before giving up.
 */
#define PACKET_MAX_TRIES 3

/*
 * Time to wait before re-sending an un-ACKd packet (0.5 s).
 */
static const struct timespec packet_ack_timeout = {
    .tv_sec = 1,
    .tv_nsec = 0,
};

/*
 * Time to wait after a request being ACKd to consider the reply lost (1.0s).
 */
static struct timespec packet_reply_timeout = {
    .tv_sec = 1,
    .tv_nsec = 0,
};

static struct timespec timespec_add(struct timespec, struct timespec);
static struct timespec timespec_sub(struct timespec, struct timespec);
static int timespec_compare(struct timespec, struct timespec);

mdr_packetconn_t* mdr_packetconn_new_from_sock(int sock)
{
    mdr_frameconn_t* fconn = mdr_frameconn_new(sock);
    if (fconn == NULL) return NULL;

    mdr_packetconn_t* pconn = mdr_packetconn_new_from_frameconn(fconn);
    if (pconn == NULL)
    {
        mdr_frameconn_free(fconn);
        return NULL;
    }

    return pconn;
}

mdr_packetconn_t* mdr_packetconn_new_from_frameconn(mdr_frameconn_t* fconn)
{
    mdr_packetconn_t* conn = malloc(sizeof(mdr_packetconn_t));
    if (conn == NULL) return NULL;

    conn->fconn = fconn;
    conn->next_sequence_id = 0;

    conn->request = conn->request_queue_tail = NULL;
    conn->subscription = conn->subscription_list_tail = NULL;

    return conn;
}

static void mdr_packetconn_free_self(mdr_packetconn_t* conn)
{
    {
        request_t* next = NULL;
        for (request_t* request = conn->request;
             request != NULL;
             request = next)
        {
            free(request->frame);

            if (request->callbacks.error != NULL)
            {
                errno = MDR_E_CLOSED;
                request->callbacks.error(request->callbacks.user_data);
            }

            next = request->next;
            free(request);
        }
    }

    {
        subscription_t* next = NULL;
        for (subscription_t* subscription = conn->subscription;
             subscription != NULL;
             subscription = next)
        {
            next = subscription->next;
            free(subscription);
        }

    }

    free(conn);
}

void mdr_packetconn_close(mdr_packetconn_t* conn)
{
    mdr_frameconn_close(conn->fconn);

    mdr_packetconn_free_self(conn);
}

void mdr_packetconn_free(mdr_packetconn_t* conn)
{
    mdr_frameconn_free(conn->fconn);

    mdr_packetconn_free_self(conn);
}

mdr_poll_info mdr_packetconn_poll_info(mdr_packetconn_t* conn)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    mdr_poll_info poll_info;

    poll_info.fd = mdr_frameconn_get_socket(conn->fconn);

    poll_info.write = false;
    if (mdr_frameconn_waiting_write(conn->fconn))
    {
        poll_info.write = true;
    }
    else if (conn->request != NULL)
    {
        if (conn->request->attempts == 0
                || timespec_compare(now, conn->request->timeout) > 0)
        {
            poll_info.write = true;
        }
    }

    poll_info.timeout = -1;
    if (conn->request != NULL && conn->request->attempts != 0)
    {
        struct timespec timeout = timespec_sub(conn->request->timeout, now);

        if (timeout.tv_sec < 0)
        {
            poll_info.timeout = 0;
        }
        else
        {
            poll_info.timeout = timeout.tv_sec * 1000
                    + timeout.tv_nsec / 1000000;
        }
    }
    
    return poll_info;
}

int mdr_packetconn_process(mdr_packetconn_t* conn)
{
    return mdr_packetconn_process_by_availability(conn, true, true);
}

static bool reply_specifier_matches(
    mdr_packetconn_reply_specifier_t reply_spec,
    mdr_packet_t* packet)
{
    if (packet->type != reply_spec.packet_type)
        return false;

    switch (reply_spec.packet_type)
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
            return true;

        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
            return true;
        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
            return reply_spec.extra
                    == packet->data.connect_ret_device_info.inquired_type;

        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
            return true;

        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
            return true;

        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
            return reply_spec.extra
                    == packet->data.common_ret_battery_level.inquired_type;

        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
            return reply_spec.extra
                    == packet->data.common_ntfy_battery_level.inquired_type;

        case MDR_PACKET_COMMON_SET_POWER_OFF:
            return true;

        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
            return true;

        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
            return reply_spec.extra
                    == packet->data.common_ret_connection_status.inquired_type;

        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
            return reply_spec.extra
                    == packet->data.common_ntfy_connection_status.inquired_type;

        case MDR_PACKET_EQEBB_GET_CAPABILITY:
            return true;

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
            return reply_spec.extra
                    == packet->data.eqebb_ret_capability.inquired_type;

        case MDR_PACKET_EQEBB_GET_PARAM:
            return true;

        case MDR_PACKET_EQEBB_RET_PARAM:
            return reply_spec.extra
                    == packet->data.eqebb_ret_param.inquired_type;

        case MDR_PACKET_EQEBB_SET_PARAM:
            return reply_spec.extra
                    == packet->data.eqebb_set_param.inquired_type;

        case MDR_PACKET_EQEBB_NTFY_PARAM:
            return reply_spec.extra
                    == packet->data.eqebb_ntfy_param.inquired_type;

        case MDR_PACKET_NCASM_GET_PARAM:
            return true;

        case MDR_PACKET_NCASM_RET_PARAM:
            return reply_spec.extra
                    == packet->data.ncasm_ret_param.inquired_type;

        case MDR_PACKET_NCASM_SET_PARAM:
            return reply_spec.extra
                    == packet->data.ncasm_set_param.inquired_type;

        case MDR_PACKET_NCASM_NTFY_PARAM:
            return reply_spec.extra
                    == packet->data.ncasm_ntfy_param.inquired_type;

        case MDR_PACKET_PLAY_GET_PARAM:
            return true;

        case MDR_PACKET_PLAY_RET_PARAM:
            return reply_spec.extra
                    == packet->data.play_ret_param.detailed_data_type;

        case MDR_PACKET_PLAY_SET_PARAM:
            return reply_spec.extra
                    == packet->data.play_set_param.detailed_data_type;

        case MDR_PACKET_PLAY_NTFY_PARAM:
            return reply_spec.extra
                    == packet->data.play_ntfy_param.detailed_data_type;

        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
            return true;

        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
            return reply_spec.extra
                    == packet->data.system_ret_capability.inquired_type;

        case MDR_PACKET_SYSTEM_GET_PARAM:
            return true;

        case MDR_PACKET_SYSTEM_RET_PARAM:
            return reply_spec.extra
                    == packet->data.system_ret_param.inquired_type;

        case MDR_PACKET_SYSTEM_SET_PARAM:
            return reply_spec.extra
                    == packet->data.system_set_param.inquired_type;

        case MDR_PACKET_SYSTEM_NTFY_PARAM:
            return reply_spec.extra
                    == packet->data.system_ntfy_param.inquired_type;
    }

    return false;
}

/*
 * Frees the current frame and callbacks, dequeues a frame and sets it as the current frame.
 *
 * Does not call any frame callback.
 */
static void advance_frame_queue(mdr_packetconn_t* conn)
{
    if (conn->request == NULL)
        return;

    free(conn->request->frame);
    request_t* next = conn->request->next;

    free(conn->request);

    conn->request = next;

    if (conn->request == NULL)
    {
        conn->request_queue_tail = NULL;
    }
    else
    {
        conn->request->frame->sequence_id = conn->next_sequence_id;
        *mdr_frame_checksum(conn->request->frame)
                = mdr_frame_compute_checksum(conn->request->frame);
        conn->next_sequence_id = !conn->next_sequence_id;
    }
}

int mdr_packetconn_process_by_availability(mdr_packetconn_t* conn,
                                            bool readable,
                                            bool writable)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    if (writable)
    {
        if (mdr_frameconn_flush_write(conn->fconn) < 0)
        {
            if (!(errno == EAGAIN || errno == EWOULDBLOCK))
            {
                return -1;
            }
        }
    }

    if (conn->request != NULL)
    {
        if (conn->request->attempts == 0)
        {
            if (writable)
            {
                if (mdr_frameconn_write_frame(
                            conn->fconn,
                            conn->request->frame) < 0)
                {
                    if (!(errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        return -1;
                    }
                }
                else
                {
                    conn->request->attempts++;
                    conn->request->timeout
                            = timespec_add(now, packet_ack_timeout);
                }
            }
        }
        else if (timespec_compare(now, conn->request->timeout) > 0)
        {
            if (conn->request->acked
                    || conn->request->attempts >= PACKET_MAX_TRIES)
            {
                mdr_packetconn_error_callback error_callback
                        = conn->request->callbacks.error;
                void* user_data = conn->request->callbacks.user_data;

                if (!conn->request->acked)
                    conn->next_sequence_id = !conn->next_sequence_id;

                if (error_callback != NULL)
                {
                    if (conn->request->acked)
                        errno = MDR_E_NO_REPLY;
                    else
                        errno = MDR_E_NO_ACK;
                    error_callback(user_data);
                }

                advance_frame_queue(conn);
            }
            else
            {
                if (mdr_frameconn_write_frame(
                            conn->fconn,
                            conn->request->frame) < 0)
                {
                    if (!(errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        return -1;
                    }
                }
                else
                {
                    conn->request->attempts++;
                    conn->request->timeout
                            = timespec_add(now, packet_ack_timeout);
                }
            }
        }
    }

    if (readable)
    {
        mdr_frame_t* frame = mdr_frameconn_read_frame(conn->fconn);

        if (frame == NULL)
        {
            if (!(errno = EAGAIN || errno == EWOULDBLOCK))
            {
                return -1;
            }
        }
        else
        {
            if (frame->data_type == MDR_FRAME_DATA_TYPE_ACK)
            {
                if (conn->request != NULL
                        && !conn->request->acked
                        && frame->sequence_id
                            == 1-conn->request->frame->sequence_id)
                {
                    if (conn->request->expected_reply.only_ack)
                    {
                        mdr_packetconn_result_callback result_callback
                                = conn->request->callbacks.result;
                        void* user_data = conn->request->callbacks.user_data;

                        advance_frame_queue(conn);

                        if (result_callback != NULL)
                        {
                            result_callback(NULL, user_data);
                        }
                    }
                    else
                    {
                        conn->request->acked = true;
                        conn->request->timeout
                                = timespec_add(now, packet_reply_timeout);
                    }

                    free(frame);
                }
                else
                {
#ifdef __DEBUG
                    printf("Unexpected ACK (seq ID %d)\n", frame->sequence_id);
                    if (conn->request != NULL)
                    {
                        printf("Current request has seq ID %d",
                               conn->request->frame->sequence_id);
                        if (conn->request->frame->payload_length > 0)
                        {

                            printf(" (type 0x%02x)\n",
                                   mdr_frame_payload(conn->request->frame)[0]);
                        }
                        else
                        {
                            printf(" (invalid)\n");
                        }
                    }
                    else
                    {
                        printf("No Current request\n");
                    }
#endif
                    // Unexpected ACK
                    free(frame);
                }
            }
            else if (frame->data_type == MDR_FRAME_DATA_TYPE_DATA_MDR)
            {
                mdr_frame_t ack_frame;
                ack_frame.data_type = MDR_FRAME_DATA_TYPE_ACK;
                ack_frame.sequence_id = !frame->sequence_id;
                ack_frame.payload_length = 0;
                *mdr_frame_checksum(&ack_frame)
                        = mdr_frame_compute_checksum(&ack_frame);
                
                mdr_frameconn_write_frame(conn->fconn, &ack_frame);
                // Ignore send error, if the send fails the device will send it
                // again and it'll be ACK'd then.

                mdr_packet_t* packet = mdr_packet_from_frame(frame);
                if (packet == NULL)
                {
#ifdef __DEBUG
                    printf("Failed to parse packet from frame: %d", errno);
                    if (frame->payload_length > 0)
                    {
                        printf(" (id 0x%02x)\n", mdr_frame_payload(frame)[0]);
                    }
                    else
                    {
                        printf(" (empty)\n");
                    }
#endif

                    free(frame);
                    return -1;
                }

                free(frame);

                if (conn->request != NULL
                        && reply_specifier_matches(
                            conn->request->expected_reply, packet))
                {
                    mdr_packetconn_result_callback result_callback
                            = conn->request->callbacks.result;
                    void* user_data = conn->request->callbacks.user_data;

                    advance_frame_queue(conn);

                    if (result_callback != NULL)
                    {
                        result_callback(packet, user_data);
                    }
                }
                else
                {
#ifdef __DEBUG
                    bool subscription_matched = false;
#endif
                    for (subscription_t* subscription = conn->subscription;
                         subscription != NULL;
                         subscription = subscription->next)
                    {
                        if (reply_specifier_matches(
                                subscription->specifier, packet))
                        {
#ifdef __DEBUG
                            subscription_matched = true;
#endif
                            if (subscription->callbacks.result != NULL)
                            {
                                subscription->callbacks.result(
                                        packet,
                                        subscription->callbacks.user_data);
                            }
                        }
                    }

#ifdef __DEBUG
                    if (!subscription_matched)
                    {
                        printf("Got unexpected packet (type %02x)\n",
                               packet->type);
                        if (conn->request != NULL)
                        {
                            printf("Expected %02x (%02x)\n",
                                   conn->request->expected_reply.packet_type,
                                   conn->request->expected_reply.extra);
                        }
                        else
                        {
                            printf("No Current request\n");
                        }
                    }
#endif
                }

                mdr_packet_free(packet);
            }
            else
            {
                // Unknown/unsupported data type
                free(frame);
            }
        }
    }

    return 0;
}

void* mdr_packetconn_make_request(
        mdr_packetconn_t* conn,
        mdr_packet_t* packet,
        mdr_packetconn_reply_specifier_t reply_spec,
        mdr_packetconn_result_callback result_callback,
        mdr_packetconn_error_callback error_callback,
        void* user_data)
{
    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    if (frame == NULL) return NULL;

    request_t* request = malloc(sizeof(request_t));
    if (request == NULL)
    {
        free(frame);
        return NULL;
    }

    request->frame = frame;
    request->attempts = 0;
    request->acked = false;
    request->callbacks.result = result_callback;
    request->callbacks.error = error_callback;
    request->callbacks.user_data = user_data;
    request->expected_reply = reply_spec;
    request->next = NULL;

    if (conn->request == NULL)
    {
        conn->request = conn->request_queue_tail = request;

        request->frame->sequence_id = conn->next_sequence_id;
        *mdr_frame_checksum(request->frame)
                = mdr_frame_compute_checksum(request->frame);
        conn->next_sequence_id = !conn->next_sequence_id;
        
        return request;
    } else {
        conn->request_queue_tail->next = request;
        conn->request_queue_tail = request;

        return request;
    }
}

void* mdr_packetconn_subscribe(
        mdr_packetconn_t* conn,
        mdr_packetconn_reply_specifier_t reply_spec,
        mdr_packetconn_result_callback callback,
        void* user_data)
{
    subscription_t* subscription = malloc(sizeof(subscription_t));
    if (subscription == NULL)
        return NULL;

    subscription->callbacks.result = callback;
    subscription->callbacks.error = NULL;
    subscription->callbacks.user_data = user_data;
    subscription->specifier = reply_spec;
    subscription->next = NULL;

    if (conn->subscription == NULL)
    {
        conn->subscription = conn->subscription_list_tail = subscription;
    }
    else
    {
        conn->subscription_list_tail->next = subscription;
        conn->subscription_list_tail = subscription;
    }

    return subscription;
}

void* mdr_packetconn_wait_for_result(mdr_packetconn_t* conn, void* handle);

void mdr_packetconn_remove_subscription(mdr_packetconn_t* conn, void* handle)
{
    subscription_t* prev = NULL;
    for (subscription_t* subscription = conn->subscription;
         subscription != NULL;
         prev = subscription, subscription = subscription->next)
    {
        if (subscription == handle)
        {
            if (subscription->next == NULL)
            {
                conn->subscription_list_tail = prev;
            }

            if (prev != NULL)
            {
                prev->next = subscription->next;
            }
            else
            {
                conn->subscription = subscription->next;
            }

            free(subscription);
            break;
        }
    }
}

static struct timespec timespec_add(struct timespec a, struct timespec b)
{
    struct timespec result;

    result.tv_sec = a.tv_sec + b.tv_sec;
    result.tv_nsec = a.tv_nsec + b.tv_nsec;

    if (result.tv_nsec > 1000000000)
    {
        result.tv_sec += 1;
        result.tv_nsec -= 1000000000;
    }

    return result;
}

static struct timespec timespec_sub(struct timespec a, struct timespec b)
{
    struct timespec result;

    result.tv_sec = a.tv_sec - b.tv_sec;
    result.tv_nsec = a.tv_nsec - b.tv_nsec;

    if (result.tv_nsec < 0)
    {
        result.tv_sec -= 1;
        result.tv_nsec += 1000000000;
    }

    return result;
}

static int timespec_compare(struct timespec a, struct timespec b)
{
    if (a.tv_sec < b.tv_sec) return -1;
    else if (a.tv_sec > b.tv_sec) return 1;

    if (a.tv_nsec < b.tv_nsec) return -1;
    else if (a.tv_nsec > b.tv_nsec) return 1;
    else return 0;
}
