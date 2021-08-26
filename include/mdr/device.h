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

#include "mdr/packetconn.h"

typedef struct mdr_device mdr_device_t;

typedef struct mdr_device_supported_functions
{
    bool power_off;
    bool battery;
    bool left_right_battery;
    bool cradle_battery;
    bool noise_cancelling;
    bool ambient_sound_mode;
    bool left_right_connection_status;
    bool eq;
    bool eq_non_customizable;
    bool auto_power_off;
}
mdr_device_supported_functions_t;

/*
 * Creates a new `mdr_device_t` from an existing connection.
 *
 * Following a call to this function the `mdr_device_t` owns the connection
 * and will free it when the device is freed.
 */
mdr_device_t* mdr_device_new_from_packetconn(mdr_packetconn_t*);

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
 * Get data that should be used to poll if the underlying socket is
 * non-blocking.
 */
mdr_poll_info mdr_device_poll_info(mdr_device_t*);

/*
 * Process some data to/from the device and call any applicable callbacks.
 *
 * Returns -1 on error and sets errno.
 *
 * If the error is EWOULDBLOCK or EAGAIN it is safe to continue calling
 * `mdr_device_process`.
 * If the error is ENOMEM it is also safe to continue but some result callbacks
 * may not be called. An error callback will always be called if
 * a packet times out.
 * For any other error `mdr_device_close` should be called since the
 * connection cannot continue to be used.
 */
int mdr_device_process(mdr_device_t*);

/*
 * Same as `mdr_device_process` except only attempt to read/write if
 * `readable`/`writable` is true, respectively.
 */
int mdr_device_process_by_availability(mdr_device_t*,
                                       bool readable,
                                       bool writable);

/*
 * Get which functions are supported by this device.
 *
 * `mdr_device_init` must be called and completed before calling this function.
 */
mdr_device_supported_functions_t
        mdr_device_get_supported_functions(mdr_device_t*);

/*
 * Initialize the device and get its capabilities. This should be called
 * before any request is sent.
 */
void mdr_device_init(mdr_device_t*,
                     void (*success)(void* user_data),
                     void (*error)(void* user_data),
                     void* user_data);

/*
 * Remove a subscription for the device.
 */
void mdr_device_remove_subscription(mdr_device_t*,
                                    void* handle);

/*
 * Get the device's name.
 *
 * The returned name should not contain any NULL-characters but this is not
 * guaranteed, therefore length should be considered. The name is not
 * guaranteed to be NULL-teminated.
 */
void mdr_device_get_model_name(
        mdr_device_t*,
        void (*result)(uint8_t length, const uint8_t* name, void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Get the device's firmware version as a string.
 *
 * The returned version string should not contain any NULL-characters but this
 * is not guaranteed, therefore length should be considered.
 * The firmware version string is not guaranteed to be NULL-teminated.
 */
void mdr_device_get_fw_version(
        mdr_device_t*,
        void (*result)(uint8_t length, const uint8_t* version, void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Get the device's series and color.
 */
void mdr_device_get_series_and_color(
        mdr_device_t*,
        void (*result)(mdr_packet_device_info_model_series_t,
                       mdr_packet_device_info_model_color_t,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Turn the device off.
 *
 * This function can only be called if `power_off` is true in the device's
 * supported function. If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_power_off(
        mdr_device_t*,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Gets the device's battery level.
 *
 * This function can only be called if `battery` is true in the device's
 * supported function. If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_get_battery_level(
        mdr_device_t*,
        void (*result)(uint8_t level, bool charging, void* user_data),
        void (*error)(void* user_data),
        void* user_data);

void* mdr_device_subscribe_battery_level(
        mdr_device_t*,
        void (*update)(uint8_t level, bool charging, void* user_data),
        void* user_data);

/*
 * Gets the device's left-right battery level.
 *
 * This function can only be called if `left_right_battery` is true in the
 * device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_get_left_right_battery_level(
        mdr_device_t*,
        void (*result)(uint8_t left_level,
                       bool left_charging,
                       uint8_t right_level,
                       bool right_charging,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data);

void* mdr_device_subscribe_left_right_battery_level(
        mdr_device_t*,
        void (*update)(uint8_t left_level,
                       bool left_charging,
                       uint8_t right_level,
                       bool right_charging,
                       void* user_data),
        void* user_data);

/*
 * Gets the device's cradle battery level.
 *
 * This function can only be called if `cradle_battery` is true in the
 * device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_get_cradle_battery_level(
        mdr_device_t*,
        void (*result)(uint8_t level, bool charging, void* user_data),
        void (*error)(void* user_data),
        void* user_data);

void* mdr_device_subscribe_cradle_battery_level(
        mdr_device_t*,
        void (*update)(uint8_t level, bool charging, void* user_data),
        void* user_data);

/*
 * Gets the device's left-right connection status.
 *
 * This function can only be called if `left_right_connection_status` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_get_left_right_connection_status(
        mdr_device_t*,
        void (*result)(bool left_connected,
                       bool right_connected,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data);

void* mdr_device_subscribe_left_right_connection_status(
        mdr_device_t*,
        void (*update)(bool left_connected,
                       bool right_connected,
                       void* user_data),
        void* user_data);

/*
 * Check if noise cancelling mode is enbaled.
 *
 * This function can only be called if `noise_cancelling` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_get_noise_cancelling_enabled(
        mdr_device_t*,
        void (*result)(bool enabled, void* user_data),
        void (*error)(void* user_data),
        void* user_data);

void* mdr_device_subscribe_noise_cancelling_enabled(
        mdr_device_t*,
        void (*update)(bool enabled, void* user_data),
        void* user_data);

/*
 * Get ambient sound mode settings.
 *
 * This function can only be called if `ambient_sound_mode` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_get_ambient_sound_mode_settings(
        mdr_device_t*,
        void (*result)(uint8_t amount, bool voice, void* user_data),
        void (*error)(void* user_data),
        void* user_data);

void* mdr_device_subscribe_ambient_sound_mode_settings(
        mdr_device_t*,
        void (*update)(uint8_t amount, bool voice, void* user_data),
        void* user_data);

/*
 * Disable Noise Cancelling and Ambient Sound Mode.
 *
 * This function can only be called if
 * `noise_cancelling` or `ambient_sound_mode` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_disable_ncasm(
        mdr_device_t*,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Enable noise cancelling mode.
 *
 * This function can only be called if `noise_cancelling` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_enable_noise_cancelling(
        mdr_device_t*,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Enable ambient sound mode.
 *
 * This function can only be called if `ambient_sound_mode` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_enable_ambient_sound_mode(
        mdr_device_t*,
        uint8_t level,
        bool voice,
        void (*result)(void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Get the device's Equalizer capabilities.
 *
 * This function can only be called if `eq` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_get_eq_capabilities(
        mdr_device_t*,
        void (*result)(uint8_t band_count,
                       uint8_t level_steps,
                       uint8_t num_presets,
                       mdr_packet_eqebb_eq_preset_id_t* presets,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Get the device's current Equalizer settings.
 *
 * This function can only be called if `eq` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_get_eq_preset_and_levels(
        mdr_device_t*,
        void (*result)(mdr_packet_eqebb_eq_preset_id_t preset_id,
                       uint8_t num_levels,
                       uint8_t* levels,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data);

void* mdr_device_subscribe_eq_preset_and_levels(
        mdr_device_t*,
        void (*update)(mdr_packet_eqebb_eq_preset_id_t preset_id,
                       uint8_t num_levels,
                       uint8_t* levels,
                       void* user_data),
        void* user_data);

/*
 * Set the device's Equalizer preset.
 *
 * This function can only be called if `eq` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_set_eq_preset(
        mdr_device_t*,
        mdr_packet_eqebb_eq_preset_id_t preset_id,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Set the device's Equalizer levels.
 *
 * This function can only be called if `eq` is true
 * and `eq_non_customizable` is false in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_set_eq_levels(
        mdr_device_t*,
        uint8_t num_levels,
        uint8_t* levels,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Get the available timeouts for the device's auto power-off functionality.
 *
 * This function can only be called if `auto_power_off` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_setting_get_auto_power_off_timeouts(
        mdr_device_t*,
        void (*result)(uint8_t timeout_count,
                       mdr_packet_system_auto_power_off_element_id_t* timeouts,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Get the current auto power-off settings.
 *
 * This function can only be called if `auto_power_off` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_setting_get_auto_power_off(
        mdr_device_t*,
        void (*result)(bool enabled,
                       mdr_packet_system_auto_power_off_element_id_t time,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data);

void* mdr_device_setting_subscribe_auto_power_off(
        mdr_device_t*,
        void (*update)(bool enabled,
                       mdr_packet_system_auto_power_off_element_id_t time,
                       void* user_data),
        void* user_data);

/*
 * Disable auto power-off.
 *
 * This function can only be called if `auto_power_off` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_setting_disable_auto_power_off(
        mdr_device_t*,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data);

/*
 * Enable auto power-off with the specified timeout.
 *
 * This function can only be called if `auto_power_off` is true
 * in the device's supported function.
 * If is not supported, -1 is returned and errno is set to
 * `MDR_E_NOT_SUPPORTED`.
 *
 * Returns 0 if the request is successful and -1 on failure.
 */
int mdr_device_setting_enable_auto_power_off(
        mdr_device_t*,
        mdr_packet_system_auto_power_off_element_id_t time,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data);

#endif /* __MDR_DEVICE_H__ */
