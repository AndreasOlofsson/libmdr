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

#ifndef __MDR_DEVICE_H__
#define __MDR_DEVICE_H__

#include "mdr/connection.h"
#include "mdr/packet.h"

#include <time.h>

/*
 * A managed connection to an MDR device.
 *
 * This provides a higher-level interface than `mdr_connection_t` by
 * converting between packets and frames and keeping track of what reply
 * correspond to which request. It also allows for subscriptions to
 * certain notifications.
 */
typedef struct mdr_device mdr_device_t;

/*
 * Creates a new `mdr_device_t` from an existing connection.
 *
 * Following a call to this function the `mdr_device_t` owns the connection
 * and will free it when the device is freed.
 *
 * Returns NULL and sets errno on error.
 */
mdr_device_t* mdr_device_new_from_connection(mdr_connection_t*);

/*
 * Creates a new `mdr_device_t` from a connected socket.
 *
 * Returns NULL and sets errno on error.
 */
mdr_device_t* mdr_device_new_from_sock(int sock);

/*
 * Free this device and any associated resources without closing the socket.
 *
 * Any in-progress request is cancelled by calling the error callback with
 * `errno` set to `MDR_E_CLOSED`.
 */
void mdr_device_free(mdr_device_t*);

/*
 * Free this device and any associated resources and close the socket.
 *
 * Like `mdr_device_free` this cancels all in-progress requests. 
 */
void mdr_device_close(mdr_device_t*);

/*
 * Checks if the device is currently waiting for its socket to become readable.
 *
 * This should be used if the program is using polling to
 * operate asynchronously.
 */
bool mdr_device_waiting_read(mdr_device_t*);

/*
 * Checks if the device is currently waiting for its socket to become writeable.
 *
 * This should be used if the program is using polling to
 * operate asynchronously.
 */
bool mdr_device_waiting_write(mdr_device_t*);

/*
 * Get the number of milliseconds before `mdr_device_process` should be called,
 * regardless on if the socket is readable or writeable.
 *
 * This should be used if the program is using polling to
 * operate asynchronously.
 */
int mdr_device_wait_timeout(mdr_device_t*);

/*
 * Process some data to/from the device and call any applicable callbacks.
 *
 * If the result is less than zero an error occurred and the errno will be set.
 * Note that the error may be EWOULDBLOCK or EAGAIN in which case it is safe
 * to continue calling `mdr_device_process` in a poll-loop.
 * Continuing if the error is ENOMEM is also safe but some request may fail
 * due to result structs not being able to be allocated.
 */
int mdr_device_process(mdr_device_t*);

/*
 * Same as mdr_device_process except only attempt to read/write if
 * `readable`/`writable` is true, respectively.
 */
int mdr_device_process_by_availability(mdr_device_t*,
                                       bool readable,
                                       bool writable);

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
void* mdr_device_wait_for_result(mdr_device_t*, void* handle);

/*
 * Removes a previously registered subscription (`mdr_device_subscribe`.. call)
 * using the handle that that function returned.
 */
void mdr_device_remove_subscription(mdr_device_t*, void* handle);

/*
 * Request Functions
 *
 * The following functions issue a request to the devie and registers
 * two callbacks; one to be called when a successful reply is received, and
 * one called if an error occurred while sending the request or
 * receiving the reply. 
 *
 * The `user_data` is not freed when the request is removed from
 * the mdr_device_t, it must be manually freed.
 * 
 * Any of the `result_callback` and `error_callback` arguments may be NULL, in
 * which case the callback won't be called.
 *
 * Subscription Functions
 *
 * The subscription functions function like requests except they only have a
 * result callback which may be called several times. The callback is also not
 * removed automatically, it must be unregistered using
 * `mdr_device_remove_subscription`, or by closing the mdr_device_t using
 * `mdr_device_free` or `mdr_device_close`.
 */

/*
 * Called after the sucessful completion of a request.
 *
 * The result pointer varies depending on the request.
 */
typedef void (*mdr_device_result_callback)(void* result, void* user_data);

/*
 * Called when a request results in an error.
 *
 * The error code will be set in `errno`.
 */
typedef void (*mdr_device_error_callback)(void* user_data);

/*
 * Get the protocol version the device uses.
 *
 * The `result` is a uint16_t.
 */
void* mdr_device_get_protocol_version(mdr_device_t*,
                                      mdr_device_result_callback,
                                      mdr_device_error_callback,
                                      void* user_data);

/*
 * Get an info property from the device.
 *
 * The `result` for inquired type
 *  - MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME
 *      is mdr_packet_device_info_string_t.
 *  - MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION
 *      is mdr_packet_device_info_string_t.
 *  - MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR
 *      is mdr_packet_device_info_series_and_color_t.
 *  - MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE;
 *      is mdr_packet_device_info_instruction_guide_t.
 */
void* mdr_device_get_device_info(mdr_device_t*,
                                 mdr_packet_device_info_inquired_type_t,
                                 mdr_device_result_callback,
                                 mdr_device_error_callback,
                                 void* user_data);

/*
 * Get a list of supported functions.
 *
 * The `result` is mdr_packet_ret_support_function_t.
 */
void* mdr_device_get_support_functions(mdr_device_t*,
                                       mdr_device_result_callback,
                                       mdr_device_error_callback,
                                       void* user_data);

/*
 * Get the device battery level and charging status.
 *
 * The inquired type must be available on the device, this can be checked
 * using `mdr_device_get_support_functions`.
 *
 * The `result` for inquired type
 *  - MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY
 *      is mdr_packet_battery_status_t.
 *  - MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY
 *      is mdr_packet_battery_status_left_right_t.
 *  - MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY
 *      is mdr_packet_battery_status_t.
 */
void* mdr_device_get_battery_level(mdr_device_t*,
                                   mdr_packet_battery_inquired_type_t,
                                   mdr_device_result_callback,
                                   mdr_device_error_callback,
                                   void* user_data);

/*
 * Subscribe to changes in battery level.
 *
 * The `result` data is the same as for `mdr_device_get_battery_level`.
 */
void* mdr_device_subscribe_battery_level(mdr_device_t*,
                                         mdr_packet_battery_inquired_type_t,
                                         mdr_device_result_callback,
                                         void* user_data);

/*
 * Get EQ/EBB capabilities.
 *
 * The `result` for inquired type
 *  - MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ
 *    or MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE
 *      is mdr_packet_eqebb_capability_eq_t.
 *  - MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB
 *      is mdr_packet_eqebb_capability_ebb_t.
 */
void* mdr_device_get_eqebb_capability(mdr_device_t*,
                                      mdr_packet_eqebb_inquired_type_t,
                                      mdr_packet_eqebb_display_language_t,
                                      mdr_device_result_callback,
                                      mdr_device_error_callback,
                                      void* user_data);

/*
 * Get the state of the EQ/EBB.
 *
 * The `result` for inquired type
 *  - MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ
 *    or MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE
 *      is mdr_packet_eqebb_param_eq_t.
 *  - MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB
 *      is mdr_packet_eqebb_param_ebb_t.
 */
void* mdr_device_get_eqebb_param(mdr_device_t*,
                                 mdr_packet_eqebb_inquired_type_t type,
                                 mdr_device_result_callback,
                                 mdr_device_error_callback,
                                 void* user_data);

/*
 * Set the active EQ preset.
 *
 * The `result` is always NULL.
 */
void* mdr_device_set_eq_preset(mdr_device_t* device,
                               mdr_packet_eqebb_eq_preset_id_t preset_id,
                               mdr_device_result_callback result_callback,
                               mdr_device_error_callback error_callback,
                               void* user_data);

/*
 * Set the EQ levels active preset.
 *
 * The `result` is always NULL.
 */
void* mdr_device_set_eq_levels(mdr_device_t* device,
                               uint8_t num_levels,
                               uint8_t* levels,
                               mdr_device_result_callback result_callback,
                               mdr_device_error_callback error_callback,
                               void* user_data);

/*
 * Get the state of noise cancelling and/or ambient sound mode.
 *
 * The inquired type must be available on the device, this can be checked
 * using `mdr_device_get_support_functions`.
 *
 * The `result` for inquired type
 *  - MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING
 *      is mdr_packet_ncasm_param_noise_cancelling_t.
 *  - MDR_PACKET_NCASM_INQUIRED_TYPE_ASM
 *      is mdr_packet_ncasm_param_asm_t.
 *  - MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM
 *      is mdr_packet_ncasm_param_noise_cancelling_asm_t.
 */
void* mdr_device_get_ncasm_param(mdr_device_t*,
                                 mdr_packet_ncasm_inquired_type_t type,
                                 mdr_device_result_callback,
                                 mdr_device_error_callback,
                                 void* user_data);

/*
 * Disable noise cancelling and/or ambient sound mode.
 *
 * The `result` is always NULL.
 */
void* mdr_device_ncasm_disable(mdr_device_t*,
                               mdr_packet_ncasm_inquired_type_t,
                               mdr_device_result_callback,
                               mdr_device_error_callback,
                               void* user_data);

/*
 * Enable noise cancelling mode.
 *
 * The inquired type must be available on the device, this can be checked
 * using `mdr_device_get_support_functions`.
 *
 * The `result` is always NULL.
 */
void* mdr_device_ncasm_enable_nc(mdr_device_t*,
                                 mdr_packet_ncasm_inquired_type_t,
                                 mdr_device_result_callback,
                                 mdr_device_error_callback,
                                 void* user_data);

/*
 * Enable ambient sound mode.
 *
 * The inquired type must be available on the device, this can be checked
 * using `mdr_device_get_support_functions`.
 *
 * The `result` is always NULL.
 */
void* mdr_device_ncasm_enable_asm(mdr_device_t*,
                                  mdr_packet_ncasm_inquired_type_t,
                                  bool voice,
                                  uint8_t amount,
                                  mdr_device_result_callback,
                                  mdr_device_error_callback,
                                  void* user_data);

/*
 * Subscribe to changes in noise cancelling and/or ambient sound mode.
 *
 * The inquired type must be available on the device, this can be checked
 * using `mdr_device_get_support_functions`.
 *
 * The `result` data is the same as for `mdr_device_get_ncasm_param`.
 */
void* mdr_device_ncasm_subscribe(mdr_device_t*,
                                 mdr_packet_ncasm_inquired_type_t,
                                 mdr_device_result_callback,
                                 void* user_data);

#endif /* __MDR_DEVICE_H__ */
