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

#include "mdr/device.h"

#include "mdr/errors.h"

#include <stdbool.h>
#include <time.h>
#include <errno.h>

/*
 * Time to wait before re-sending an un-ACKd packet.
 * In seconds and micro-seconds.
 */
// 0.5 s
static const struct timespec packet_ack_timeout = {
    .tv_sec = 0,
    .tv_nsec = 500 * 1000 * 1000,
};

/*
 * Number of times to try sending a packet before giving up.
 */
#define MDR_PACKET_MAX_TRIES 3

/*
 * Time to wait after a request being ACKd to consider the reply lost.
 * In seconds and micro-seconds.
 */
// 1.5 s
static struct timespec packet_reply_timeout = {
    .tv_sec = 1,
    .tv_nsec = 500 * 1000 * 1000,
};

typedef struct
{
    mdr_device_result_callback result_cb;
    mdr_device_error_callback error_cb;
    void* user_data;
}
callbacks_t;

typedef struct
{
    bool only_ack;
    mdr_packet_type_t packet_type;
    uint8_t extra;
}
reply_specifier_t;

typedef struct queued_frame queued_frame_t;

struct queued_frame
{
    mdr_frame_t* frame;
    callbacks_t* callbacks;
    reply_specifier_t expected_reply;
    queued_frame_t* next;
};

typedef struct notifier notifier_t;

struct notifier
{
    mdr_packet_type_t packet_type;
    uint8_t extra;
    callbacks_t* callbacks;
    notifier_t* next;
};

struct mdr_device
{
    mdr_connection_t* connection;

    uint8_t next_sequence_id;

    // The frame currently awaiting an ACK or reply.
    mdr_frame_t* frame;
    struct timespec frame_time;
    int frame_attempts;
    bool frame_acked;
    callbacks_t* frame_callbacks;
    reply_specifier_t frame_expected_reply;

    queued_frame_t* frame_queue, *frame_queue_tail;

    notifier_t* notifiers[0x100];
};

// timespec helpers
static struct timespec timespec_add(struct timespec, struct timespec);
static struct timespec timespec_sub(struct timespec, struct timespec);
static int timespec_compare(struct timespec, struct timespec);

mdr_device_t* mdr_device_new_from_connection(mdr_connection_t* connection)
{
    mdr_device_t* device = malloc(sizeof(mdr_device_t));
    if (device == NULL) return NULL;

    device->connection = connection;
    device->next_sequence_id = 0;

    device->frame = NULL;
    device->frame_queue = NULL;
    device->frame_queue_tail = NULL;

    memset(device->notifiers, 0, sizeof(notifier_t*) * 0x100);

    return device;
}

mdr_device_t* mdr_device_new_from_sock(int sock)
{
    mdr_connection_t* connection = mdr_connection_new(sock);
    if (connection == NULL) return NULL;

    mdr_device_t* device = mdr_device_new_from_connection(connection);
    if (device == NULL)
    {
        mdr_connection_free(connection);
        return NULL;
    }

    return device;
}

static void mdr_device_cancel_requests(mdr_device_t* device)
{
    if (device->frame != NULL)
    {
        callbacks_t* callbacks = device->frame_callbacks;

        if (callbacks->error_cb != NULL)
        {
            errno = MDR_E_CLOSED;
            callbacks->error_cb(callbacks->user_data);
        }

        free(device->frame_callbacks);
        free(device->frame);
    }

    queued_frame_t* queued_frame = device->frame_queue, *next;
    for (; queued_frame != NULL; queued_frame = next)
    {
        next = queued_frame->next;

        callbacks_t* callbacks = queued_frame->callbacks;

        if (callbacks->error_cb != NULL)
        {
            errno = MDR_E_CLOSED;
            callbacks->error_cb(callbacks->user_data);
        }

        free(callbacks);
        free(queued_frame->frame);
        free(queued_frame);
    }

    for (int notifier_type = 0; notifier_type < 0x100; notifier_type++)
    {
        notifier_t* notifier = device->notifiers[notifier_type], *next;

        for (; notifier != NULL; notifier = next)
        {
            next = notifier->next;

            free(notifier->callbacks);
            free(notifier);
        }
    }
}

void mdr_device_free(mdr_device_t* device)
{
    mdr_device_cancel_requests(device);

    mdr_connection_free(device->connection);
    free(device);
}

void mdr_device_close(mdr_device_t* device)
{
    mdr_device_cancel_requests(device);

    mdr_connection_close(device->connection);
    free(device);
}

bool mdr_device_waiting_read(mdr_device_t* device)
{
    return mdr_connection_waiting_read(device->connection);
}

bool mdr_device_waiting_write(mdr_device_t* device)
{
    return mdr_connection_waiting_write(device->connection)
        || device->frame != NULL;
}

int mdr_device_wait_timeout(mdr_device_t* device)
{
    if (device->frame == NULL) return -1;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    struct timespec timeout;

    if (device->frame_acked)
    {
        timeout = timespec_add(device->frame_time, packet_reply_timeout);
    }
    else
    {
        timeout = timespec_add(device->frame_time, packet_ack_timeout);
    }

    struct timespec diff = timespec_sub(timeout, now);
    time_t timeout_ms = diff.tv_sec * 1000 + diff.tv_nsec / 1000000;

    if (timeout_ms <= 0) return 0;
    else return timeout_ms;
}

static void* packet_get_data(mdr_packet_t* packet)
{
    switch (packet->type)
    {
        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
        {
            uint16_t* result = malloc(sizeof(uint16_t));
            if (result == NULL) return NULL;

            uint8_t high = packet->data.connect_ret_protocol_info.version_high;
            uint8_t low = packet->data.connect_ret_protocol_info.version_low;
            *result = ((uint16_t) high) << 8 | low;
            return result;
        }

        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
            switch (packet->data.connect_ret_device_info.inquired_type)
            {
                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME:
                {
                    mdr_packet_device_info_string_t* result =
                        malloc(sizeof(mdr_packet_device_info_string_t));
                    if (result == NULL) return NULL;

                    uint8_t* string = malloc(packet->data
                            .connect_ret_device_info.model_name.len + 1);
                    if (string == NULL)
                    {
                        free(result);
                        return NULL;
                    }

                    memcpy(string,
                            packet->data.connect_ret_device_info
                                .model_name.string,
                            packet->data.connect_ret_device_info
                                .model_name.len);
                    string[packet->data.connect_ret_device_info
                        .model_name.len] = 0;

                    result->len = packet->data.connect_ret_device_info
                                .model_name.len;
                    result->string = string;

                    return result;
                }

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION:
                {
                    mdr_packet_device_info_string_t* result =
                        malloc(sizeof(mdr_packet_device_info_string_t));
                    if (result == NULL) return NULL;

                    uint8_t* string = malloc(packet->data
                            .connect_ret_device_info.fw_version.len + 1);
                    if (string == NULL)
                    {
                        free(result);
                        return NULL;
                    }

                    memcpy(string,
                            packet->data.connect_ret_device_info
                                .fw_version.string,
                            packet->data.connect_ret_device_info
                                .fw_version.len);
                    string[packet->data.connect_ret_device_info
                        .fw_version.len] = 0;

                    result->len = packet->data.connect_ret_device_info
                                .fw_version.len;
                    result->string = string;

                    return result;
                }

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR:
                {
                    size_t len = sizeof(
                            mdr_packet_device_info_series_and_color_t);
                    mdr_packet_device_info_series_and_color_t* result =
                        malloc(len);
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.connect_ret_device_info
                               .series_and_color,
                           len);

                    return result;
                }

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE:
                {
                    mdr_packet_device_info_instruction_guide_t* result =
                        malloc(sizeof(
                            mdr_packet_device_info_instruction_guide_t));
                    if (result == NULL) return NULL;

                    mdr_packet_device_info_guidance_category_t* categories =
                        malloc(packet->data
                            .connect_ret_device_info.instruction_guide.count);
                    if (categories == NULL)
                    {
                        free(result);
                        return NULL;
                    }

                    memcpy(categories,
                            packet->data.connect_ret_device_info
                                .instruction_guide.guidance_categories,
                            packet->data.connect_ret_device_info
                                .instruction_guide.count);

                    result->count = packet->data.connect_ret_device_info
                                .instruction_guide.count;
                    result->guidance_categories = categories;

                    return result;
                }
            }

        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
        {
            mdr_packet_connect_ret_support_function_t* result =
                malloc(sizeof(mdr_packet_connect_ret_support_function_t));
            if (result == NULL) return NULL;

            mdr_packet_support_function_type_t* function_types =
                malloc(packet->data.connect_ret_support_function
                        .num_function_types);
            if (function_types == NULL)
            {
                free(result);
                return NULL;
            }

            memcpy(function_types,
                   packet->data.connect_ret_support_function.function_types,
                   packet->data.connect_ret_support_function
                       .num_function_types);

            result->fixed_value = 0;
            result->num_function_types =
                packet->data.connect_ret_support_function
                        .num_function_types;
            result->function_types = function_types;

            return result;
        }

        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
            switch (packet->data.common_ret_battery_level.inquired_type)
            {
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY:
                {
                    mdr_packet_battery_status_t* result =
                        malloc(sizeof(mdr_packet_battery_status_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.common_ret_battery_level
                               .battery,
                           sizeof(mdr_packet_battery_status_t));

                    return result;
                }

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY:
                {
                    mdr_packet_battery_status_left_right_t* result =
                        malloc(sizeof(mdr_packet_battery_status_left_right_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.common_ret_battery_level
                               .left_right_battery,
                           sizeof(mdr_packet_battery_status_left_right_t));

                    return result;
                }

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY:
                {
                    mdr_packet_battery_status_t* result =
                        malloc(sizeof(mdr_packet_battery_status_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.common_ret_battery_level
                               .cradle_battery,
                           sizeof(mdr_packet_battery_status_t));

                    return result;
                }
            }

        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
            switch (packet->data.common_ntfy_battery_level.inquired_type)
            {
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY:
                {
                    mdr_packet_battery_status_t* result =
                        malloc(sizeof(mdr_packet_battery_status_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.common_ntfy_battery_level
                               .battery,
                           sizeof(mdr_packet_battery_status_t));

                    return result;
                }

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY:
                {
                    mdr_packet_battery_status_left_right_t* result =
                        malloc(sizeof(mdr_packet_battery_status_left_right_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.common_ntfy_battery_level
                               .left_right_battery,
                           sizeof(mdr_packet_battery_status_left_right_t));

                    return result;
                }

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY:
                {
                    mdr_packet_battery_status_t* result =
                        malloc(sizeof(mdr_packet_battery_status_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.common_ntfy_battery_level
                               .cradle_battery,
                           sizeof(mdr_packet_battery_status_t));

                    return result;
                }
            }

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
            switch (packet->data.eqebb_ret_param.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                {
                    mdr_packet_eqebb_capability_eq_t* result =
                        malloc(sizeof(mdr_packet_eqebb_capability_eq_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.eqebb_ret_capability.eq,
                           sizeof(mdr_packet_eqebb_capability_eq_t));

                    result->presets = malloc(result->num_presets * sizeof(
                            mdr_packet_eqebb_capability_eq_preset_name_t));
                    if (result->presets == NULL)
                    {
                        free(result);
                        return NULL;
                    }

                    for (int i = 0; i < result->num_presets; i++)
                    {
                        memcpy(&result->presets[i],
                           &packet->data.eqebb_ret_capability.eq.presets[i],
                           sizeof(
                               mdr_packet_eqebb_capability_eq_preset_name_t));

                        result->presets[i].name =
                                malloc(result->presets[i].name_len + 1);
                        if (result->presets[i].name == NULL)
                        {
                            for (int j = 0; j < i; j++)
                            {
                                free(result->presets[j].name);
                                free(result->presets);
                                free(result);
                                return NULL;
                            }
                        }

                        memcpy(result->presets[i].name,
                               packet->data.eqebb_ret_capability.eq
                                   .presets[i].name,
                               result->presets[i].name_len);
                        result->presets[i].name[result->presets[i].name_len]
                            = 0;
                    }

                    return result;
                }

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                {
                    mdr_packet_eqebb_capability_ebb_t* result =
                        malloc(sizeof(mdr_packet_eqebb_capability_ebb_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.eqebb_ret_capability.ebb,
                           sizeof(mdr_packet_eqebb_capability_ebb_t));

                    return result;
                }
            }

        case MDR_PACKET_EQEBB_RET_PARAM:
            switch (packet->data.eqebb_ret_param.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                {
                    mdr_packet_eqebb_param_eq_t* result =
                        malloc(sizeof(mdr_packet_eqebb_param_eq_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.eqebb_ret_param.eq,
                           sizeof(mdr_packet_eqebb_param_eq_t));
                    
                    return result;
                }

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                {
                    uint8_t* result = malloc(sizeof(uint8_t));
                    if (result == NULL) return NULL;

                    *result = packet->data.eqebb_ret_param.ebb.level;
                    
                    return result;
                }
            }

        case MDR_PACKET_NCASM_RET_PARAM:
            switch (packet->data.ncasm_ret_param.inquired_type)
            {
                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING:
                {
                    mdr_packet_ncasm_param_noise_cancelling_t* result =
                        malloc(sizeof(mdr_packet_ncasm_param_noise_cancelling_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.ncasm_ret_param.noise_cancelling,
                           sizeof(mdr_packet_ncasm_param_noise_cancelling_t));

                    return result;
                }
                break;

                case MDR_PACKET_NCASM_INQUIRED_TYPE_ASM:
                {
                    mdr_packet_ncasm_param_asm_t* result =
                        malloc(sizeof(mdr_packet_ncasm_param_asm_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.ncasm_ret_param.ambient_sound_mode,
                           sizeof(mdr_packet_ncasm_param_asm_t));

                    return result;
                }
                break;

                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM:
                {
                    mdr_packet_ncasm_param_noise_cancelling_asm_t* result =
                        malloc(sizeof(mdr_packet_ncasm_param_noise_cancelling_asm_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.ncasm_ret_param.noise_cancelling_asm,
                           sizeof(mdr_packet_ncasm_param_noise_cancelling_asm_t));

                    return result;
                }
                break;
            }

        case MDR_PACKET_NCASM_NTFY_PARAM:
            switch (packet->data.ncasm_ntfy_param.inquired_type)
            {
                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING:
                {
                    mdr_packet_ncasm_param_noise_cancelling_t* result =
                        malloc(sizeof(mdr_packet_ncasm_param_noise_cancelling_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.ncasm_ntfy_param.noise_cancelling,
                           sizeof(mdr_packet_ncasm_param_noise_cancelling_t));

                    return result;
                }
                break;

                case MDR_PACKET_NCASM_INQUIRED_TYPE_ASM:
                {
                    mdr_packet_ncasm_param_asm_t* result =
                        malloc(sizeof(mdr_packet_ncasm_param_asm_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.ncasm_ntfy_param.ambient_sound_mode,
                           sizeof(mdr_packet_ncasm_param_asm_t));

                    return result;
                }
                break;

                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM:
                {
                    mdr_packet_ncasm_param_noise_cancelling_asm_t* result =
                        malloc(sizeof(mdr_packet_ncasm_param_noise_cancelling_asm_t));
                    if (result == NULL) return NULL;

                    memcpy(result,
                           &packet->data.ncasm_ntfy_param.noise_cancelling_asm,
                           sizeof(mdr_packet_ncasm_param_noise_cancelling_asm_t));

                    return result;
                }
                break;
            }

        default:
            errno = MDR_E_UNEXPECTED_PACKET;
            return NULL;
    }
}

static void mdr_device_dequeue_frame(mdr_device_t* device, struct timespec now)
{
    if (device->frame_queue != NULL)
    {
        queued_frame_t* queued_frame = device->frame_queue;

        device->frame_queue = queued_frame->next;
        if (device->frame_queue == NULL)
        {
            device->frame_queue_tail = NULL;
        }

        device->frame = queued_frame->frame;
        device->frame_callbacks = queued_frame->callbacks;
        device->frame_expected_reply =queued_frame->expected_reply;
        device->frame_attempts = 0;
        device->frame_time = now;
        device->frame_acked = false;

        device->frame->sequence_id = device->next_sequence_id;
        *mdr_frame_checksum(device->frame) =
            mdr_frame_compute_checksum(device->frame);
        device->next_sequence_id = !device->next_sequence_id;
    }
}

static bool packet_matches(mdr_packet_t* packet,
                           mdr_packet_type_t type,
                           uint8_t extra)
{
    if (packet->type != type) return false;

    switch (type)
    {
        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
            return true;

        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
            return extra == packet->data.connect_ret_device_info.inquired_type;

        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
            return extra == packet->data.common_ret_battery_level.inquired_type;

        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
            return extra == packet->data.common_ntfy_battery_level
                .inquired_type;

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
            return extra == packet->data.eqebb_ret_capability.inquired_type;

        case MDR_PACKET_EQEBB_RET_PARAM:
            return extra == packet->data.eqebb_ret_param.inquired_type;

        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
            return extra == packet->data.ncasm_ret_param.inquired_type;

        // Get/set packets are never expected replies.
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
        case MDR_PACKET_EQEBB_GET_CAPABILITY:
        case MDR_PACKET_EQEBB_GET_PARAM:
        case MDR_PACKET_EQEBB_SET_PARAM:
        case MDR_PACKET_NCASM_GET_PARAM:
        case MDR_PACKET_NCASM_SET_PARAM:
            return false;
    }

    return false;
}

int mdr_device_process(mdr_device_t* device)
{
    return mdr_device_process_by_availability(device, true, true);
}

int mdr_device_process_by_availability(mdr_device_t* device,
                                       bool readable,
                                       bool writable)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    if (readable)
    {
        mdr_frame_t* frame = mdr_connection_read_frame(device->connection);
        if (frame == NULL)
        {
            if (!(errno == EAGAIN || errno == EWOULDBLOCK))
            {
                return -1;
            }
        }
        else
        {
            if (frame->data_type == MDR_FRAME_DATA_TYPE_ACK)
            {
                if (device->frame != NULL)
                {
                    if (frame->sequence_id == !device->frame->sequence_id)
                    {
                        // Frame has been ACKd
                        clock_gettime(CLOCK_MONOTONIC,
                                      &device->frame_time);

                        device->frame_acked = true;
                        if (device->frame_expected_reply.only_ack)
                        {
                            callbacks_t* callbacks = device->frame_callbacks;

                            if (callbacks->result_cb != NULL)
                            {
                                callbacks->result_cb(NULL,
                                                     callbacks->user_data);
                            }

                            free(callbacks);
                            free(device->frame);
                            device->frame = NULL;

                            mdr_device_dequeue_frame(device, now);
                        }
                    }
#ifdef __DEBUG
                    else
                    {
                        fprintf(stderr,
                                "libmdr: "
                                "Received ACK with incorrect sequenceID\n");
                    }
#endif
                }

                free(frame);
            }
            else if (frame->data_type == MDR_FRAME_DATA_TYPE_DATA_MDR)
            {
                mdr_frame_t* ack_frame = malloc(MDR_FRAME_EMPTY_LEN);
                if (ack_frame == NULL)
                {
                    return -1;
                }

                ack_frame->data_type = MDR_FRAME_DATA_TYPE_ACK;
                ack_frame->sequence_id = !frame->sequence_id;
                ack_frame->payload_length = 0;
                *mdr_frame_checksum(ack_frame) =
                    mdr_frame_compute_checksum(ack_frame);

                if (mdr_connection_write_frame(device->connection,
                                               ack_frame) < 0)
                {
                    free(frame);
                    if (!(errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        return -1;
                    }

                }
                else
                {
                    mdr_packet_t* packet = mdr_packet_from_frame(frame);
                    free(frame);
                    if (packet == NULL)
                    {
                        return -1;
                    }

                    if (device->frame != NULL
                        && packet_matches(
                                packet,
                                device->frame_expected_reply.packet_type,
                                device->frame_expected_reply.extra))
                    {
                        errno = 0;
                        void* packet_data = packet_get_data(packet);
                        if (errno != 0)
                        {
                            mdr_packet_free(packet);
                            return -1;
                        }

                        callbacks_t* callbacks = device->frame_callbacks;

                        if (callbacks->result_cb != NULL)
                        {
                            callbacks->result_cb(packet_data,
                                                 callbacks->user_data);
                        }

                        free(callbacks);
                        free(device->frame);
                        device->frame = NULL;

                        mdr_device_dequeue_frame(device, now);
                    }
                    else
                    {
                        bool matched_notification = false;
                        void* packet_data;

                        notifier_t* notifier = device->notifiers[packet->type];

                        for (; notifier != NULL; notifier = notifier->next)
                        {
                            if (packet_matches(packet,
                                               notifier->packet_type,
                                               notifier->extra))
                            {
                                if (!matched_notification)
                                {
                                    packet_data = packet_get_data(packet);
                                }
                                matched_notification = true;

                                callbacks_t* callbacks = notifier->callbacks;

                                if (callbacks->result_cb != NULL)
                                {
                                    callbacks->result_cb(
                                            packet_data,
                                            callbacks->user_data);
                                }
                            }
                        }

#ifdef __DEBUG
                        if (!matched_notification)
                        {
                            fprintf(stderr,
                                    "libmdr: "
                                    "Received unexpected reply/notify packet "
                                    "(0x%02x).\n",
                                    packet->type);
                        }
#endif
                    }

                    mdr_packet_free(packet);
                }
            }
            else
            {
                // Unkown packet type
                free(frame);
            }
        }
    }

    if (writable)
    {
        if (mdr_connection_flush_write(device->connection) < 0)
        {
            if (!(errno == EAGAIN || errno == EWOULDBLOCK))
            {
                return -1;
            }
        }
    }

    // Call error on timeout, or send queued frame.
    if (device->frame != NULL)
    {
        if (device->frame_acked)
        {
            struct timespec timeout = timespec_add(device->frame_time,
                                                   packet_reply_timeout);

            if (timespec_compare(now, timeout) > 0)
            {
                // Reply timed out, call error callback and dequeue next frame.
                callbacks_t* callbacks = device->frame_callbacks;

                if (callbacks->error_cb != NULL)
                {
                    errno = MDR_E_NO_REPLY;
                    callbacks->error_cb(callbacks->user_data);
                }

                free(callbacks);
                free(device->frame);
                device->frame = NULL;

                mdr_device_dequeue_frame(device, now);
            }
        }

        if (!device->frame_acked) {
            struct timespec timeout = timespec_add(device->frame_time,
                                                   packet_ack_timeout);

            if (device->frame_attempts == 0
                    || timespec_compare(now, timeout) > 0)
            {
                if (device->frame_attempts >= MDR_PACKET_MAX_TRIES)
                {
                    // Frame timed out,
                    // call error callback and dequeue next frame.
                    callbacks_t* callbacks = device->frame_callbacks;

                    if (callbacks->error_cb != NULL)
                    {
                        errno = MDR_E_NO_ACK;;
                        callbacks->error_cb(callbacks->user_data);
                    }

                    free(callbacks);
                    free(device->frame);
                    device->frame = NULL;

                    device->next_sequence_id = !device->next_sequence_id;

                    mdr_device_dequeue_frame(device, now);
                }
                else if (writable)
                {
                    mdr_frame_t* send_frame = mdr_frame_dup(device->frame);
                    if (send_frame == NULL)
                    {
                        return -1;
                    }
                    if (mdr_connection_write_frame(device->connection,
                                                   send_frame) < 0)
                    {
                        if (!(errno == EAGAIN || errno == EWOULDBLOCK))
                        {
                            return -1;
                        }
                    }
                    else
                    {
                        // Resend complete, inc attempts and reset frame time.
                        device->frame_attempts++;
                        device->frame_time = now;
                    }
                }
            }
        }
    }

    return 0;
}

typedef struct
{
    bool got_result;
    void* result;
    int error;
} mdr_device_wait_for_result_user_data_t;

static void mdr_device_wait_for_result_return_callback(void* data,
                                                       void* user_data)
{
    ((mdr_device_wait_for_result_user_data_t*) user_data)->result = data;
    ((mdr_device_wait_for_result_user_data_t*) user_data)->got_result = true;
}

static void mdr_device_wait_for_result_error_callback(void* user_data)
{
    ((mdr_device_wait_for_result_user_data_t*) user_data)->error = errno;
}

void* mdr_device_wait_for_result(mdr_device_t* device, void* handle)
{
    if (handle == NULL) return NULL;

    mdr_device_wait_for_result_user_data_t wait_result;
    wait_result.got_result = false;
    wait_result.result = NULL;
    wait_result.error = 0;

    callbacks_t* callbacks = (callbacks_t*) handle;

    mdr_device_result_callback original_result_cb = callbacks->result_cb;
    mdr_device_error_callback original_error_cb = callbacks->error_cb;
    void* original_user_data = callbacks->user_data;

    callbacks->result_cb = mdr_device_wait_for_result_return_callback;
    callbacks->error_cb = mdr_device_wait_for_result_error_callback;
    callbacks->user_data = &wait_result;

    while (!wait_result.got_result && wait_result.error == 0)
    {
        if (mdr_device_process(device) < 0)
        {
            callbacks->result_cb = original_result_cb;
            callbacks->error_cb = original_error_cb;
            callbacks->user_data = original_user_data;

            return NULL;
        }
    }

    if (wait_result.got_result)
    {
        return wait_result.result;
    }
    else
    {
        errno = wait_result.error;
        return NULL;
    }
}

static void* mdr_device_make_request(
        mdr_device_t* device,
        mdr_frame_t* frame,
        reply_specifier_t expected_reply,
        mdr_device_result_callback result_callback,
        mdr_device_error_callback error_callback,
        void* user_data)
{
    if (mdr_device_process_by_availability(device, false, true) < 0)
    {
        free(frame);
        return NULL;
    }

    callbacks_t* callbacks = malloc(sizeof(callbacks_t));
    if (callbacks == NULL) return NULL;

    callbacks->result_cb = result_callback;
    callbacks->error_cb = error_callback;
    callbacks->user_data = user_data;

    if (device->frame == NULL)
    {
        frame->sequence_id = device->next_sequence_id;
        *mdr_frame_checksum(frame) =
            mdr_frame_compute_checksum(frame);

        mdr_frame_t* send_frame = mdr_frame_dup(frame);
        if (send_frame == NULL)
        {
            free(frame);
            free(callbacks);
            return NULL;
        }

        if (mdr_connection_write_frame(device->connection, send_frame) < 0)
        {
            free(frame);
            free(callbacks);
            return NULL;
        }
        device->next_sequence_id = !device->next_sequence_id;

        device->frame = frame;
        clock_gettime(CLOCK_MONOTONIC, &device->frame_time);
        device->frame_attempts = 1;
        device->frame_acked = false;
        device->frame_callbacks = callbacks;
        device->frame_expected_reply = expected_reply;
    }
    else
    {
        queued_frame_t* queued_frame = malloc(sizeof(queued_frame_t));
        if (queued_frame == NULL)
        {
            free(frame);
            free(callbacks);
            return NULL;
        }

        queued_frame->frame = frame;
        queued_frame->callbacks = callbacks;
        queued_frame->expected_reply = expected_reply;
        queued_frame->next = NULL;

        if (device->frame_queue_tail == NULL)
        {
            device->frame_queue = queued_frame;
            device->frame_queue_tail = queued_frame;
        }
        else
        {
            device->frame_queue_tail->next = queued_frame;
            device->frame_queue_tail = queued_frame;
        }
    }

    return callbacks;
}

static void* mdr_device_subscribe(
        mdr_device_t* device,
        mdr_packet_type_t type,
        uint8_t extra,
        mdr_device_result_callback callback,
        void* user_data)
{
    callbacks_t* callbacks = malloc(sizeof(callbacks_t));
    if (callbacks == NULL) return NULL;

    callbacks->result_cb = callback;
    callbacks->error_cb = NULL;
    callbacks->user_data = user_data;

    notifier_t* notifier = malloc(sizeof(notifier_t));
    if (notifier == NULL)
    {
        free(callbacks);
        return NULL;
    }

    notifier->packet_type = type;
    notifier->extra = extra;
    notifier->callbacks = callbacks;
    notifier->next = device->notifiers[type];

    device->notifiers[type] = notifier;

    return notifier;
}

void mdr_device_remove_subscription(mdr_device_t* device, void* handle)
{
    notifier_t* notifier = handle;

    notifier_t** notifier_list = &device->notifiers[notifier->packet_type];

    for (; *notifier_list != NULL; notifier_list = &(*notifier_list)->next)
    {
        if (*notifier_list == notifier)
        {
            *notifier_list = (*notifier_list)->next;
        }
    }

    free(notifier->callbacks);
    free(notifier);
}

void* mdr_device_get_protocol_version(
        mdr_device_t* device,
        mdr_device_result_callback result_callback,
        mdr_device_error_callback error_callback,
        void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_connect_get_protocol_info_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_CONNECT_GET_PROTOCOL_INFO;
    packet->data.connect_get_protocol_info.fixed_value = 0x00;

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = false,
        .packet_type = MDR_PACKET_CONNECT_RET_PROTOCOL_INFO,
        .extra = 0,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_get_device_info(mdr_device_t* device,
                                 mdr_packet_device_info_inquired_type_t type,
                                 mdr_device_result_callback result_callback,
                                 mdr_device_error_callback error_callback,
                                 void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_connect_get_device_info_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_CONNECT_GET_DEVICE_INFO;
    packet->data.connect_get_device_info.inquired_type = type;

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = false,
        .packet_type = MDR_PACKET_CONNECT_RET_DEVICE_INFO,
        .extra = type,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_get_support_functions(mdr_device_t* device,
                                       mdr_device_result_callback result_callback,
                                       mdr_device_error_callback error_callback,
                                       void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_connect_get_support_function_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION;
    packet->data.connect_get_support_function.fixed_value = 0x00;

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = false,
        .packet_type = MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION,
        .extra = 0,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_get_battery_level(mdr_device_t* device,
                                   mdr_packet_battery_inquired_type_t type,
                                   mdr_device_result_callback result_callback,
                                   mdr_device_error_callback error_callback,
                                   void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_common_get_battery_level_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_COMMON_GET_BATTERY_LEVEL;
    packet->data.common_get_battery_level.inquired_type = type;

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = false,
        .packet_type = MDR_PACKET_COMMON_RET_BATTERY_LEVEL,
        .extra = type,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_subscribe_battery_level(
        mdr_device_t* device,
        mdr_packet_battery_inquired_type_t type,
        mdr_device_result_callback callback,
        void* user_data)
{
    return mdr_device_subscribe(device,
                                MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL,
                                type,
                                callback,
                                user_data);
}

void* mdr_device_get_eqebb_capability(
        mdr_device_t* device,
        mdr_packet_eqebb_inquired_type_t type,
        mdr_packet_eqebb_display_language_t display_language,
        mdr_device_result_callback result_callback,
        mdr_device_error_callback error_callback,
        void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_eqebb_get_capability_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_EQEBB_GET_CAPABILITY;
    packet->data.eqebb_get_capability.inquired_type = type;
    packet->data.eqebb_get_capability.display_language = display_language;

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = false,
        .packet_type = MDR_PACKET_EQEBB_RET_CAPABILITY,
        .extra = type,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_get_eqebb_param(mdr_device_t* device,
                                 mdr_packet_eqebb_inquired_type_t type,
                                 mdr_device_result_callback result_callback,
                                 mdr_device_error_callback error_callback,
                                 void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_eqebb_get_param_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_EQEBB_GET_PARAM;
    packet->data.eqebb_get_param.inquired_type = type;

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = false,
        .packet_type = MDR_PACKET_EQEBB_RET_PARAM,
        .extra = type,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_set_eq_preset(mdr_device_t* device,
                               mdr_packet_eqebb_eq_preset_id_t preset_id,
                               mdr_device_result_callback result_callback,
                               mdr_device_error_callback error_callback,
                               void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_eqebb_set_param_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_EQEBB_SET_PARAM;
    packet->data.eqebb_set_param.inquired_type = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ;
    packet->data.eqebb_set_param.eq.preset_id = preset_id;
    packet->data.eqebb_set_param.eq.num_levels = 0;
    packet->data.eqebb_set_param.eq.levels = NULL;

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = true,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_set_eq_levels(mdr_device_t* device,
                               uint8_t num_levels,
                               uint8_t* levels,
                               mdr_device_result_callback result_callback,
                               mdr_device_error_callback error_callback,
                               void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_eqebb_set_param_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_EQEBB_SET_PARAM;
    packet->data.eqebb_set_param.inquired_type = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ;
    packet->data.eqebb_set_param.eq.preset_id =
        MDR_PACKET_EQEBB_EQ_PRESET_ID_UNSPECIFIED;
    packet->data.eqebb_set_param.eq.num_levels = num_levels;
    packet->data.eqebb_set_param.eq.levels = malloc(num_levels);
    memcpy(packet->data.eqebb_set_param.eq.levels,
           levels,
           num_levels);

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = true,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_get_ncasm_param(mdr_device_t* device,
                                 mdr_packet_ncasm_inquired_type_t type,
                                 mdr_device_result_callback result_callback,
                                 mdr_device_error_callback error_callback,
                                 void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_ncasm_get_param_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_NCASM_GET_PARAM;
    packet->data.ncasm_get_param.inquired_type = type;

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = false,
        .packet_type = MDR_PACKET_NCASM_RET_PARAM,
        .extra = type,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_ncasm_disable(mdr_device_t* device,
                               mdr_packet_ncasm_inquired_type_t type,
                               mdr_device_result_callback result_callback,
                               mdr_device_error_callback error_callback,
                               void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_ncasm_set_param_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_NCASM_SET_PARAM;
    packet->data.ncasm_set_param.inquired_type = type;
    switch (type)
    {
        case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING:
        {
            mdr_packet_ncasm_param_noise_cancelling_t param = {
                .nc_setting_type = MDR_PACKET_NCASM_NC_SETTING_TYPE_ON_OFF,
                .nc_setting_value = MDR_PACKET_NCASM_NC_SETTING_VALUE_OFF,
            };
            packet->data.ncasm_set_param.noise_cancelling = param;
        }
        break;

        case MDR_PACKET_NCASM_INQUIRED_TYPE_ASM:
        {
            mdr_packet_ncasm_param_asm_t param = {
                .ncasm_effect = MDR_PACKET_NCASM_NCASM_EFFECT_OFF,
                .asm_setting_type =
                    MDR_PACKET_NCASM_ASM_SETTING_TYPE_LEVEL_ADJUSTMENT,
                .asm_id = MDR_PACKET_NCASM_ASM_ID_NORMAL,
                .asm_amount = 0,
            };
            packet->data.ncasm_set_param.ambient_sound_mode = param;
        }
        break;

        case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM:
        {
            mdr_packet_ncasm_param_noise_cancelling_asm_t param = {
                .ncasm_effect = MDR_PACKET_NCASM_NCASM_EFFECT_OFF,
                .ncasm_setting_type =
                    MDR_PACKET_NCASM_NCASM_SETTING_TYPE_DUAL_SINGLE_OFF,
                .ncasm_amount = 2,
                .asm_setting_type = MDR_PACKET_NCASM_ASM_SETTING_TYPE_ON_OFF,
                .asm_id = MDR_PACKET_NCASM_ASM_ID_NORMAL,
                .asm_amount = 0,
            };
            packet->data.ncasm_set_param.noise_cancelling_asm = param;
        }
        break;
    }

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = true,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_ncasm_enable_nc(mdr_device_t* device,
                                 mdr_packet_ncasm_inquired_type_t type,
                                 mdr_device_result_callback result_callback,
                                 mdr_device_error_callback error_callback,
                                 void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_ncasm_set_param_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_NCASM_SET_PARAM;
    packet->data.ncasm_set_param.inquired_type = type;
    switch (type)
    {
        case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING:
        {
            mdr_packet_ncasm_param_noise_cancelling_t param = {
                .nc_setting_type = MDR_PACKET_NCASM_NC_SETTING_TYPE_ON_OFF,
                .nc_setting_value = MDR_PACKET_NCASM_NC_SETTING_VALUE_ON,
            };
            packet->data.ncasm_set_param.noise_cancelling = param;
        }
        break;

        case MDR_PACKET_NCASM_INQUIRED_TYPE_ASM:
            free(packet);
            errno = EINVAL;
            return NULL;

        case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM:
        {
            mdr_packet_ncasm_param_noise_cancelling_asm_t param = {
                .ncasm_effect = MDR_PACKET_NCASM_NCASM_EFFECT_ON,
                .ncasm_setting_type =
                    MDR_PACKET_NCASM_NCASM_SETTING_TYPE_DUAL_SINGLE_OFF,
                .ncasm_amount = 2,
                .asm_setting_type = MDR_PACKET_NCASM_ASM_SETTING_TYPE_ON_OFF,
                .asm_id = MDR_PACKET_NCASM_ASM_ID_NORMAL,
                .asm_amount = 0,
            };
            packet->data.ncasm_set_param.noise_cancelling_asm = param;
        }
        break;
    }

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = true,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;
}

void* mdr_device_ncasm_enable_asm(mdr_device_t* device,
                                  mdr_packet_ncasm_inquired_type_t type,
                                  bool voice,
                                  uint8_t amount,
                                  mdr_device_result_callback result_callback,
                                  mdr_device_error_callback error_callback,
                                  void* user_data)
{
    size_t len = 1 + sizeof(mdr_packet_ncasm_set_param_t);
    mdr_packet_t* packet = malloc(len);
    if (packet == NULL) return NULL;

    packet->type = MDR_PACKET_NCASM_SET_PARAM;
    packet->data.ncasm_set_param.inquired_type = type;
    switch (type)
    {
        case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING:
            free(packet);
            errno = EINVAL;
            return NULL;

        case MDR_PACKET_NCASM_INQUIRED_TYPE_ASM:
        {
            mdr_packet_ncasm_param_asm_t param = {
                .ncasm_effect = MDR_PACKET_NCASM_NCASM_EFFECT_ON,
                .asm_setting_type =
                    MDR_PACKET_NCASM_ASM_SETTING_TYPE_ON_OFF,
                .asm_id = voice ? MDR_PACKET_NCASM_ASM_ID_VOICE :
                                  MDR_PACKET_NCASM_ASM_ID_NORMAL,
                .asm_amount = amount,
            };
            packet->data.ncasm_set_param.ambient_sound_mode = param;
        }
        break;

        case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM:
        {
            mdr_packet_ncasm_param_noise_cancelling_asm_t param = {
                .ncasm_effect = MDR_PACKET_NCASM_NCASM_EFFECT_ON,
                .ncasm_setting_type =
                    MDR_PACKET_NCASM_NCASM_SETTING_TYPE_DUAL_SINGLE_OFF,
                .ncasm_amount = 0,
                .asm_setting_type =
                    MDR_PACKET_NCASM_ASM_SETTING_TYPE_LEVEL_ADJUSTMENT,
                .asm_id = voice ? MDR_PACKET_NCASM_ASM_ID_VOICE :
                                  MDR_PACKET_NCASM_ASM_ID_NORMAL,
                .asm_amount = amount,
            };
            packet->data.ncasm_set_param.noise_cancelling_asm = param;
        }
        break;
    }

    mdr_frame_t* frame = mdr_packet_to_frame(packet);
    free(packet);
    if (frame == NULL)
    {
        return NULL;
    }

    reply_specifier_t expected_reply = {
        .only_ack = true,
    };

    void* handle = mdr_device_make_request(device,
                                           frame,
                                           expected_reply,
                                           result_callback,
                                           error_callback,
                                           user_data);

    return handle;

}

void* mdr_device_ncasm_subscribe(mdr_device_t* device,
                                 mdr_packet_ncasm_inquired_type_t type,
                                 mdr_device_result_callback callback,
                                 void* user_data)
{
    return mdr_device_subscribe(device,
                                MDR_PACKET_NCASM_NTFY_PARAM,
                                type,
                                callback,
                                user_data);
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

