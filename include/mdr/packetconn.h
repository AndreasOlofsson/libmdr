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

#ifndef __MDR_PACKETCONN_H__
#define __MDR_PACKETCONN_H__

#include "mdr/frameconn.h"
#include "mdr/packet.h"

/*
 * A packet-connection to an MDR socket.
 *
 * This is a somewhat low-level interface which ...
 *
 * See `mdr_frameconn_t` ...
 */
typedef struct mdr_packetconn mdr_packetconn_t;

/*
 * Create a packet-connection by wrapping an existing, connected socket.
 *
 * Returns NULL and sets errno on error.
 */
mdr_packetconn_t* mdr_packetconn_new_from_sock(int sock);

/*
 * Create a packet-connection by wrapping a `mdr_frameconn_t`.
 *
 * Returns NULL and sets errno on error.
 */
mdr_packetconn_t* mdr_packetconn_new_from_frameconn(mdr_frameconn_t*);

/*
 * Close a packet-connection and free any associcated resources.
 */
void mdr_packetconn_close(mdr_packetconn_t*);

/*
 * Free any resources allocated by the packet-connection
 * and the underlying frame-connection without closing the socket.
 */
void mdr_packetconn_free(mdr_packetconn_t*);

typedef struct
{
    int fd;
    bool write;
    int timeout;
}
mdr_poll_info;

/*
 * Get data that should be used to poll if the underlying socket is
 * non-blocking.
 */
mdr_poll_info mdr_packetconn_poll_info(mdr_packetconn_t*);

/*
 * Process some data to/from the connection and call any applicable callbacks.
 *
 * Returns -1 on error and sets errno.
 *
 * If the error is EWOULDBLOCK or EAGAIN it is safe to continue calling
 * `mdr_packetconn_process`.
 * If the error is ENOMEM it is also safe to continue but some result callbacks
 * may not be called. An error callback will always be called if
 * a packet times out.
 * For any other error `mdr_packetconn_close` should be called since the
 * connection cannot continue to be used.
 */
int mdr_packetconn_process(mdr_packetconn_t*);

/*
 * Same as `mdr_packetconn_process` except only attempt to read/write if
 * `readable`/`writable` is true, respectively.
 */
int mdr_packetconn_process_by_availability(mdr_packetconn_t*,
                                            bool readable,
                                            bool writable);

/*
 * Called after the sucessful completion of a request.
 */
typedef void (*mdr_packetconn_result_callback)(mdr_packet_t*, void* user_data);

/*
 * Called when a request results in an error.
 *
 * The error code will be set in `errno`.
 */
typedef void (*mdr_packetconn_error_callback)(void* user_data);

/*
 * A match pattern for reply packets.
 *
 * This is used to filter which reply counts as an expected reply.
 */
typedef struct
{
    // The reply only expects an ACK and no response packet,
    // if true all other fields will be ignored.
    //
    // This field is ignored for subscriptions.
    bool only_ack;
    // The `packet_type` to match against.
    mdr_packet_type_t packet_type;
    // An extra parameter to check which depends on the `packet_type`,
    // for most packets this will be the "inquired type".
    uint8_t extra;
}
mdr_packetconn_reply_specifier_t;

/*
 * Send a request and register a reply callback to be called on result or error.
 *
 * The reply pointer is a handle to the request which can be passed to
 * `mdr_packetconn_wait_for_result` to finish a call synchronously.
 */
void* mdr_packetconn_make_request(
        mdr_packetconn_t*,
        mdr_packet_t*,
        mdr_packetconn_reply_specifier_t,
        mdr_packetconn_result_callback result_callback,
        mdr_packetconn_error_callback error_callback,
        void* user_data);


/*
 * Subscribe to packets of a certain type and, for some types, extra type.
 * A callback is registered to be called when a matching packet is recieved.
 *
 * The reply pointer is a handle to the subscription which can be passed to
 * `mdr_packetconn_remove_subscription` to remove a subscription.
 * Subcriptions are automatically removed when `mdr_packetconn_close` or
 * `mdr_packetconn_free` is called.
 */
void* mdr_packetconn_subscribe(
        mdr_packetconn_t*,
        mdr_packetconn_reply_specifier_t,
        mdr_packetconn_result_callback callback,
        void* user_data);

/*
 * Wait for the completion of a previously made request.
 *
 * If the request is completed during the call to this function,
 * the provided callback is not called and the result (that would be
 * the `result` provided to the callback) is instead returned.
 *
 * If the request produces an error, `errno` will be set to a non-zero value
 * and NULL is returned instead.
 *
 * Requests where the expected reply is NULL must use `errno` to determine if
 * the request was successful.
 *
 * If the underlying socket is non-blocking, this function may return NULL
 * with errno set to EAGAIN or EWOULDBLOCK.
 */
void* mdr_packetconn_wait_for_result(mdr_packetconn_t*, void* handle);

/*
 * Removes a previously registered subscription (`mdr_device_subscribe`.. call)
 * using the handle that that function returned.
 */
void mdr_packetconn_remove_subscription(mdr_packetconn_t*, void* handle);

#endif /* __MDR_PACKETCONN_H__ */
