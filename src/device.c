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

#include <errno.h>
#include "mdr/errors.h"

typedef struct subscription subscription_t;

struct subscription
{
    void (*user_result_callback)();
    void* user_data;

    void* handle;

    subscription_t* next;
};

struct mdr_device
{
    mdr_packetconn_t* conn;

    subscription_t* subscriptions, *subscriptions_tail;

    mdr_device_supported_functions_t supported_functions;
};

mdr_device_t* mdr_device_new_from_packetconn(mdr_packetconn_t* conn)
{
    mdr_device_t* device = malloc(sizeof(mdr_device_t));

    if (device == NULL) return NULL;

    device->conn = conn;

    device->subscriptions = device->subscriptions_tail = NULL;

    memset(&device->supported_functions, 0,
            sizeof(mdr_device_supported_functions_t));

    return device;
}

mdr_device_t* mdr_device_new_from_sock(int sock)
{
    mdr_packetconn_t* conn = mdr_packetconn_new_from_sock(sock);

    if (conn == NULL) return NULL;

    mdr_device_t* device = mdr_device_new_from_packetconn(conn);

    if (device == NULL)
    {
        mdr_packetconn_free(conn);
        return NULL;
    }

    device->conn = conn;

    return device;
}

void mdr_device_free(mdr_device_t* device)
{
    subscription_t* next;
    for (subscription_t* subscription = device->subscriptions;
         subscription != NULL;
         subscription = next)
    {
        mdr_packetconn_remove_subscription(
                device->conn,
                subscription->handle);
        next = subscription->next;
        free(subscription);
    }

    mdr_packetconn_free(device->conn);
    free(device);
}

void mdr_device_close(mdr_device_t* device)
{
    subscription_t* next;
    for (subscription_t* subscription = device->subscriptions;
         subscription != NULL;
         subscription = next)
    {
        mdr_packetconn_remove_subscription(
                device->conn,
                subscription->handle);
        next = subscription->next;
        free(subscription);
    }

    mdr_packetconn_close(device->conn);
    free(device);
}

mdr_poll_info mdr_device_poll_info(mdr_device_t* device)
{
    return mdr_packetconn_poll_info(device->conn);
}

int mdr_device_process(mdr_device_t* device)
{
    return mdr_packetconn_process(device->conn);
}

int mdr_device_process_by_availability(mdr_device_t* device,
                                       bool readable,
                                       bool writable)
{
    return mdr_packetconn_process_by_availability(device->conn,
                                                  readable,
                                                  writable);
}

mdr_device_supported_functions_t
        mdr_device_get_supported_functions(mdr_device_t* device)
{
    return device->supported_functions;
}

typedef struct
{
    mdr_device_t* device;
    void (*user_result_callback)();
    void (*user_error_callback)(void* user_data);
    void* user_data;
} callback_data_t;

static void success_callback_passthrough(mdr_packet_t* packet,
                                         void* user_data)
{
    callback_data_t* callback_data = user_data;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(callback_data->user_data);
    }

    free(user_data);
}

static void error_callback_passthrough(void* user_data)
{
    callback_data_t* callback_data = user_data;

    if (callback_data->user_error_callback != NULL)
    {
        callback_data->user_error_callback(callback_data->user_data);
    }

    free(user_data);
}

static void mdr_device_make_request(
        mdr_device_t* device,
        mdr_packet_t* request_packet,
        mdr_packetconn_reply_specifier_t reply_specifier,
        void (*device_result_callback)(mdr_packet_t*, void*),
        void (*user_result_callback)(),
        void (*user_error_callback)(void*),
        void* user_data)
{
    callback_data_t* callback_data = malloc(sizeof(callback_data_t));
    if (callback_data == NULL)
    {
        if (user_error_callback != NULL) user_error_callback(user_data);
        return;
    }

    callback_data->device = device;
    callback_data->user_result_callback = user_result_callback;
    callback_data->user_error_callback = user_error_callback;
    callback_data->user_data = user_data;

    mdr_packetconn_make_request(
            device->conn,
            request_packet,
            reply_specifier,
            device_result_callback,
            error_callback_passthrough,
            callback_data);
}

static void* mdr_device_add_subscription(
        mdr_device_t* device,
        mdr_packetconn_reply_specifier_t reply_specifier,
        void (*device_result_callback)(mdr_packet_t*, void*),
        void (*user_result_callback)(),
        void* user_data)
{
    subscription_t* subscription = malloc(sizeof(subscription_t));
    if (subscription == NULL)
    {
        return NULL;
    }

    subscription->user_result_callback = user_result_callback;
    subscription->user_data = user_data;

    void* handle = mdr_packetconn_subscribe(
            device->conn,
            reply_specifier,
            device_result_callback,
            subscription);

    if (handle == NULL)
    {
        free(subscription);
        return NULL;
    }

    subscription->handle = handle;

    subscription->next = NULL;

    if (device->subscriptions_tail == NULL)
    {
        device->subscriptions = subscription;
        device->subscriptions_tail = subscription;
    }
    else
    {
        device->subscriptions_tail->next = subscription;
        device->subscriptions_tail = subscription;
    }

    return subscription;
}

static void mdr_device_init_result_supported_function(mdr_packet_t* packet,
                                                      void* user_data)
{
    callback_data_t* callback_data = (callback_data_t*) user_data;

    mdr_device_t* device = callback_data->device;

    for (int i = 0;
         i < packet->data.connect_ret_support_function.num_function_types;
         i++)
    {
        switch (packet->data.connect_ret_support_function.function_types[i])
        {
            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_POWER_OFF:
                device->supported_functions.power_off = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_BATTERY_LEVEL:
                device->supported_functions.battery = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_LEFT_RIGHT_BATTERY_LEVEL:
                device->supported_functions.left_right_battery = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_CRADLE_BATTERY_LEVEL:
                device->supported_functions.cradle_battery = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_NOISE_CANCELLING:
                device->supported_functions.noise_cancelling = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_AMBIENT_SOUND_MODE:
                device->supported_functions.ambient_sound_mode = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_NOISE_CANCELLING_AND_AMBIENT_SOUND_MODE:
                device->supported_functions.noise_cancelling = true;
                device->supported_functions.ambient_sound_mode = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_LEFT_RIGHT_CONNECTION_STATUS:
                device->supported_functions.left_right_connection_status = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_PRESET_EQ:
                device->supported_functions.eq = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                device->supported_functions.eq_non_customizable = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_AUTO_POWER_OFF:
                device->supported_functions.auto_power_off = true;
                break;

            case MDR_PACKET_SUPPORT_FUNCTION_TYPE_PLAYBACK_CONTROLLER:
                device->supported_functions.playback_controller = true;

            default:
                break;
        }
    }

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(callback_data->user_data);
    }

    free(user_data);
}

static void mdr_device_init_result_protocol_info(mdr_packet_t* packet,
                                                 void* user_data)
{
    callback_data_t* callback_data = (callback_data_t*) user_data;

    mdr_device_t* device = callback_data->device;

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION;
    request_packet.data.connect_get_support_function.fixed_value = 0;

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION,
                .only_ack = false,
            },
            mdr_device_init_result_supported_function,
            callback_data->user_result_callback,
            callback_data->user_error_callback,
            callback_data->user_data);

    free(callback_data);
}

void mdr_device_init(mdr_device_t* device,
                     void (*success)(void* user_data),
                     void (*error)(void* user_data),
                     void* user_data)
{
    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_CONNECT_GET_PROTOCOL_INFO;
    request_packet.data.connect_get_protocol_info.fixed_value = 0;

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_CONNECT_RET_PROTOCOL_INFO,
                .only_ack = false,
            },
            mdr_device_init_result_protocol_info,
            success,
            error,
            user_data);
}

void mdr_device_remove_subscription(mdr_device_t* device,
                                    void* handle)
{
    subscription_t* prev = NULL;
    subscription_t* next;
    for (subscription_t* subscription = device->subscriptions;
         subscription != NULL;
         prev = subscription, subscription = next)
    {
        next = subscription->next;

        if (subscription == handle)
        {
            mdr_packetconn_remove_subscription(device->conn,
                                               subscription->handle);

            if (prev == NULL)
            {
                device->subscriptions = subscription->next;
            }
            else
            {
                prev->next = subscription->next;
            }
            if (subscription->next == NULL)
            {
                device->subscriptions_tail = prev;
            }

            free(subscription);

            break;
        }
    }
}

static void mdr_device_get_model_name_result(mdr_packet_t* packet,
                                             void* user_data)
{
    callback_data_t* callback_data = user_data;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
            packet->data.connect_ret_device_info.model_name.len,
            packet->data.connect_ret_device_info.model_name.string,
            callback_data->user_data);
    }

    free(callback_data);
}

void mdr_device_get_model_name(
        mdr_device_t* device,
        void (*result)(uint8_t length, const uint8_t* name, void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    mdr_packet_t packet = {
        .type = MDR_PACKET_CONNECT_GET_DEVICE_INFO,
        .data = {
            .connect_get_device_info = {
                .inquired_type = MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME
            }
        }
    };

    mdr_device_make_request(
            device,
            &packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_CONNECT_RET_DEVICE_INFO,
                .extra = MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME,
                .only_ack = false
            },
            mdr_device_get_model_name_result,
            (void (*)()) result,
            error,
            user_data);
}

static void mdr_device_get_fw_version_result(mdr_packet_t* packet,
                                             void* user_data)
{
    callback_data_t* callback_data = user_data;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
            packet->data.connect_ret_device_info.fw_version.len,
            packet->data.connect_ret_device_info.fw_version.string,
            callback_data->user_data);
    }

    free(callback_data);
}

void mdr_device_get_fw_version(
        mdr_device_t* device,
        void (*result)(uint8_t length, const uint8_t* name, void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    mdr_packet_t packet = {
        .type = MDR_PACKET_CONNECT_GET_DEVICE_INFO,
        .data = {
            .connect_get_device_info = {
                .inquired_type = MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION
            }
        }
    };

    mdr_device_make_request(
            device,
            &packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_CONNECT_RET_DEVICE_INFO,
                .extra = MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION,
                .only_ack = false
            },
            mdr_device_get_fw_version_result,
            (void (*)()) result,
            error,
            user_data);
}

static void mdr_device_get_series_and_color_result(mdr_packet_t* packet,
                                                   void* user_data)
{
    callback_data_t* callback_data = user_data;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
            packet->data.connect_ret_device_info.series_and_color.series,
            packet->data.connect_ret_device_info.series_and_color.color,
            callback_data->user_data);
    }

    free(callback_data);
}

void mdr_device_get_series_and_color(
        mdr_device_t* device,
        void (*result)(mdr_packet_device_info_model_series_t,
                       mdr_packet_device_info_model_color_t,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    mdr_packet_t request_packet = {
        .type = MDR_PACKET_CONNECT_GET_DEVICE_INFO,
        .data = {
            .connect_get_device_info = {
                .inquired_type = MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR
            }
        }
    };

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_CONNECT_RET_DEVICE_INFO,
                .extra = MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR,
                .only_ack = false
            },
            mdr_device_get_series_and_color_result,
            (void (*)()) result,
            error,
            user_data);
}

int mdr_device_power_off(
        mdr_device_t* device,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.power_off)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet = {
        .type = MDR_PACKET_COMMON_SET_POWER_OFF,
        .data = {
            .common_set_power_off = {
                .inquired_type
                    = MDR_PACKET_COMMON_POWER_OFF_INQUIRED_TYPE_FIXED_VALUE,
                .setting_value
                    = MDR_PACKET_COMMON_POWER_OFF_SETTING_VALUE_USER_POWER_OFF,
            },
        },
    };

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .only_ack = true,
            },
            success_callback_passthrough,
            (void (*)()) success,
            error,
            user_data);

    return 0;
}

static void mdr_device_get_battery_level_result(mdr_packet_t* packet,
                                                void* user_data)
{
    callback_data_t* callback_data = user_data;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
            packet->data.common_ret_battery_level.battery.level,
            packet->data.common_ret_battery_level.battery.charging,
            callback_data->user_data);
    }

    free(callback_data);
}

int mdr_device_get_battery_level(
        mdr_device_t* device,
        void (*result)(uint8_t level, bool charging, void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.battery)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t packet = {
        .type = MDR_PACKET_COMMON_GET_BATTERY_LEVEL,
        .data = {
            .common_get_battery_level = {
                .inquired_type = MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY
            }
        }
    };

    mdr_device_make_request(
            device,
            &packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_COMMON_RET_BATTERY_LEVEL,
                .extra = MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY,
                .only_ack = false
            },
            mdr_device_get_battery_level_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

static void mdr_device_subscribe_battery_level_update(
        mdr_packet_t* packet,
        void* user_data)
{
    subscription_t* subscription = user_data;

    if (subscription->user_result_callback != NULL)
    {
        subscription->user_result_callback(
                packet->data.common_ntfy_battery_level.battery.level,
                packet->data.common_ntfy_battery_level.battery.charging,
                subscription->user_data);
    }
}

void* mdr_device_subscribe_battery_level(
        mdr_device_t* device,
        void (*update)(uint8_t level, bool charging, void* user_data),
        void* user_data)
{
    if (!device->supported_functions.battery)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return NULL;
    }

    return mdr_device_add_subscription(
            device,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL,
                .extra = MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY,
                .only_ack = false
            },
            mdr_device_subscribe_battery_level_update,
            (void (*)()) update,
            user_data);
}

static void mdr_device_get_left_right_battery_level_result(
        mdr_packet_t* packet,
        void* user_data)
{
    callback_data_t* callback_data = user_data;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
            packet->data.common_ret_battery_level.left_right_battery.left.level,
            packet->data.common_ret_battery_level.left_right_battery.left.charging,
            packet->data.common_ret_battery_level.left_right_battery.right.level,
            packet->data.common_ret_battery_level.left_right_battery.right.charging,
            callback_data->user_data);
    }

    free(callback_data);
}

int mdr_device_get_left_right_battery_level(
        mdr_device_t* device,
        void (*result)(uint8_t left_level,
                       bool left_charging,
                       uint8_t right_level,
                       bool right_charging,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.left_right_battery)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t packet = {
        .type = MDR_PACKET_COMMON_GET_BATTERY_LEVEL,
        .data = {
            .common_get_battery_level = {
                .inquired_type = MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY
            }
        }
    };

    mdr_device_make_request(
            device,
            &packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_COMMON_RET_BATTERY_LEVEL,
                .extra = MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY,
                .only_ack = false
            },
            mdr_device_get_left_right_battery_level_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

static void mdr_device_subscribe_left_right_battery_level_update(
        mdr_packet_t* packet,
        void* user_data)
{
    subscription_t* subscription = user_data;

    if (subscription->user_result_callback != NULL)
    {
        subscription->user_result_callback(
                packet->data.common_ntfy_battery_level.left_right_battery.left.level,
                packet->data.common_ntfy_battery_level.left_right_battery.left.charging,
                packet->data.common_ntfy_battery_level.left_right_battery.right.level,
                packet->data.common_ntfy_battery_level.left_right_battery.right.charging,
                subscription->user_data);
    }
}

void* mdr_device_subscribe_left_right_battery_level(
        mdr_device_t* device,
        void (*update)(uint8_t left_level,
                       bool left_charging,
                       uint8_t right_level,
                       bool right_charging,
                       void* user_data),
        void* user_data)
{
    if (!device->supported_functions.left_right_battery)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return NULL;
    }

    return mdr_device_add_subscription(
            device,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL,
                .extra = MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY,
                .only_ack = false
            },
            mdr_device_subscribe_left_right_battery_level_update,
            (void (*)()) update,
            user_data);
}

static void mdr_device_get_cradle_battery_level_result(mdr_packet_t* packet,
                                                       void* user_data)
{
    callback_data_t* callback_data = user_data;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
            packet->data.common_ret_battery_level.cradle_battery.level,
            packet->data.common_ret_battery_level.cradle_battery.charging,
            callback_data->user_data);
    }

    free(callback_data);
}

int mdr_device_get_cradle_battery_level(
        mdr_device_t* device,
        void (*result)(uint8_t level, bool charging, void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.cradle_battery)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t packet = {
        .type = MDR_PACKET_COMMON_GET_BATTERY_LEVEL,
        .data = {
            .common_get_battery_level = {
                .inquired_type = MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY
            }
        }
    };

    mdr_device_make_request(
            device,
            &packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_COMMON_RET_BATTERY_LEVEL,
                .extra = MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY,
                .only_ack = false
            },
            mdr_device_get_cradle_battery_level_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

static void mdr_device_subscribe_cradle_battery_level_update(
        mdr_packet_t* packet,
        void* user_data)
{
    subscription_t* subscription = user_data;

    if (subscription->user_result_callback != NULL)
    {
        subscription->user_result_callback(
                packet->data.common_ntfy_battery_level.cradle_battery.level,
                packet->data.common_ntfy_battery_level.cradle_battery.charging,
                subscription->user_data);
    }
}

void* mdr_device_subscribe_cradle_battery_level(
        mdr_device_t* device,
        void (*update)(uint8_t level, bool charging, void* user_data),
        void* user_data)
{
    if (!device->supported_functions.cradle_battery)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return NULL;
    }

    return mdr_device_add_subscription(
            device,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL,
                .extra = MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY,
                .only_ack = false
            },
            mdr_device_subscribe_cradle_battery_level_update,
            (void (*)()) update,
            user_data);
}

static void mdr_device_get_left_right_connection_status_result(
        mdr_packet_t* packet,
        void* user_data)
{
    callback_data_t* callback_data = user_data;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
            packet->data.common_ret_connection_status.left_right.left_status
                == MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_CONNECTED,
            packet->data.common_ret_connection_status.left_right.right_status
                == MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_CONNECTED,
            callback_data->user_data);
    }

    free(callback_data);
}

int mdr_device_get_left_right_connection_status(
        mdr_device_t* device,
        void (*result)(bool left_connected,
                       bool right_connected,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.left_right_connection_status)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t packet = {
        .type = MDR_PACKET_COMMON_GET_CONNECTION_STATUS,
        .data = {
            .common_get_connection_status = {
                .inquired_type = MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT,
            },
        },
    };

    mdr_device_make_request(
            device,
            &packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_COMMON_RET_CONNECTION_STATUS,
                .extra = MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT,
                .only_ack = false
            },
            mdr_device_get_left_right_connection_status_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

static void mdr_device_subscribe_left_right_connection_status_update(
        mdr_packet_t* packet,
        void* user_data)
{
    subscription_t* subscription = user_data;

    if (subscription->user_result_callback != NULL)
    {
        subscription->user_result_callback(
                packet->data.common_ntfy_connection_status.left_right.left_status
                    == MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_CONNECTED,
                packet->data.common_ntfy_connection_status.left_right.right_status
                    == MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_CONNECTED,
                subscription->user_data);
    }
}

void* mdr_device_subscribe_left_right_connection_status(
        mdr_device_t* device,
        void (*update)(bool left_connected,
                       bool right_connected,
                       void* user_data),
        void* user_data)
{
    if (!device->supported_functions.left_right_connection_status)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return NULL;
    }

    return mdr_device_add_subscription(
            device,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS,
                .extra = MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT,
                .only_ack = false
            },
            mdr_device_subscribe_left_right_connection_status_update,
            (void (*)()) update,
            user_data);
}

static void mdr_device_get_noise_cancelling_enabled_result(
        mdr_packet_t* packet,
        void* callback_data_ptr)
{
    callback_data_t* callback_data = (callback_data_t*) callback_data_ptr;

    if (callback_data->user_result_callback != NULL)
    {
        void* user_data = callback_data->user_data;

        if (packet->data.ncasm_ret_param.inquired_type
                == MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING)
        {
            callback_data->user_result_callback(
                    packet->data.ncasm_ret_param.noise_cancelling.nc_setting_value
                        == MDR_PACKET_NCASM_NC_SETTING_VALUE_ON,
                    user_data);
        }
        else if (packet->data.ncasm_ret_param.inquired_type
                == MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM)
        {
            callback_data->user_result_callback(
                    packet->data.ncasm_ret_param.noise_cancelling_asm.ncasm_effect
                        == MDR_PACKET_NCASM_NCASM_EFFECT_ON,
                    user_data);
        }
    }

    free(callback_data);
}

int mdr_device_get_noise_cancelling_enabled(
        mdr_device_t* device,
        void (*result)(bool enabled, void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.noise_cancelling)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_NCASM_GET_PARAM;

    if (device->supported_functions.ambient_sound_mode)
    {
        request_packet.data.ncasm_get_param.inquired_type
            = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM;
    }
    else
    {
        request_packet.data.ncasm_get_param.inquired_type
            = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING;
    }

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_NCASM_RET_PARAM,
                .extra = request_packet.data.ncasm_get_param.inquired_type,
                .only_ack = false,
            },
            mdr_device_get_noise_cancelling_enabled_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

static void mdr_device_subscribe_noise_cancelling_enabled_update(
        mdr_packet_t* packet,
        void* user_data)
{
    subscription_t* subscription = user_data;

    if (subscription->user_result_callback != NULL)
    {
        if (packet->data.ncasm_ntfy_param.inquired_type
                == MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING)
        {
            subscription->user_result_callback(
                    packet->data.ncasm_ntfy_param.noise_cancelling.nc_setting_value
                        == MDR_PACKET_NCASM_NC_SETTING_VALUE_ON,
                    subscription->user_data);
        }
        else if (packet->data.ncasm_ntfy_param.inquired_type
                == MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM)
        {
            subscription->user_result_callback(
                    packet->data.ncasm_ntfy_param.noise_cancelling_asm.ncasm_effect
                        == MDR_PACKET_NCASM_NCASM_EFFECT_ON,
                    subscription->user_data);
        }
    }
}

void* mdr_device_subscribe_noise_cancelling_enabled(
        mdr_device_t* device,
        void (*update)(bool enabled, void* user_data),
        void* user_data)
{
    if (!device->supported_functions.noise_cancelling)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return NULL;
    }

    uint8_t inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING;

    if (device->supported_functions.ambient_sound_mode)
    {
        inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM;
    }

    return mdr_device_add_subscription(
            device,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_NCASM_NTFY_PARAM,
                .extra = inquired_type,
                .only_ack = false
            },
            mdr_device_subscribe_noise_cancelling_enabled_update,
            (void (*)()) update,
            user_data);
}

static void mdr_device_get_ambient_sound_mode_settings_result(
        mdr_packet_t* packet,
        void* callback_data_ptr)
{
    callback_data_t* callback_data = (callback_data_t*) callback_data_ptr;

    if (callback_data->user_result_callback != NULL)
    {
        void* user_data = callback_data->user_data;

        if (packet->data.ncasm_ret_param.inquired_type
                == MDR_PACKET_NCASM_INQUIRED_TYPE_ASM)
        {
            callback_data->user_result_callback(
                    packet->data.ncasm_ret_param.ambient_sound_mode.asm_amount,
                    packet->data.ncasm_ret_param.ambient_sound_mode.asm_id
                        == MDR_PACKET_NCASM_ASM_ID_VOICE,
                    user_data);
        }
        else if (packet->data.ncasm_ret_param.inquired_type
                == MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM)
        {
            uint8_t amount = 0;

            if (packet->data.ncasm_ret_param.noise_cancelling_asm.ncasm_effect
                        == MDR_PACKET_NCASM_NCASM_EFFECT_ON)
            {
                amount = packet->data.ncasm_ret_param.noise_cancelling_asm
                    .asm_amount;
            }

            callback_data->user_result_callback(
                    amount,
                    packet->data.ncasm_ret_param.noise_cancelling_asm.asm_id
                        == MDR_PACKET_NCASM_ASM_ID_VOICE,
                    user_data);
        }
    }

    free(callback_data);
}

int mdr_device_get_ambient_sound_mode_settings(
        mdr_device_t* device,
        void (*result)(uint8_t level, bool voice, void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.ambient_sound_mode)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_NCASM_GET_PARAM;

    if (device->supported_functions.noise_cancelling)
    {
        request_packet.data.ncasm_get_param.inquired_type
            = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM;
    }
    else
    {
        request_packet.data.ncasm_get_param.inquired_type
            = MDR_PACKET_NCASM_INQUIRED_TYPE_ASM;
    }

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_NCASM_RET_PARAM,
                .extra = request_packet.data.ncasm_get_param.inquired_type,
                .only_ack = false,
            },
            mdr_device_get_ambient_sound_mode_settings_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

static void mdr_device_subscribe_ambient_sound_mode_settings_update(
        mdr_packet_t* packet,
        void* user_data)
{
    subscription_t* subscription = user_data;

    if (subscription->user_result_callback != NULL)
    {
        if (packet->data.ncasm_ntfy_param.inquired_type
                == MDR_PACKET_NCASM_INQUIRED_TYPE_ASM)
        {
            subscription->user_result_callback(
                    packet->data.ncasm_ntfy_param.ambient_sound_mode.asm_amount,
                    packet->data.ncasm_ntfy_param.ambient_sound_mode.asm_id
                        == MDR_PACKET_NCASM_ASM_ID_VOICE,
                    subscription->user_data);
        }
        else if (packet->data.ncasm_ntfy_param.inquired_type
                == MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM)
        {
            uint8_t amount = 0;

            if (packet->data.ncasm_ntfy_param.noise_cancelling_asm.ncasm_effect
                        == MDR_PACKET_NCASM_NCASM_EFFECT_ON)
            {
                amount = packet->data.ncasm_ntfy_param.noise_cancelling_asm
                    .asm_amount;
            }

            subscription->user_result_callback(
                    amount,
                    packet->data.ncasm_ntfy_param.noise_cancelling_asm.asm_id
                        == MDR_PACKET_NCASM_ASM_ID_VOICE,
                    subscription->user_data);
        }
    }
}

void* mdr_device_subscribe_ambient_sound_mode_settings(
        mdr_device_t* device,
        void (*update)(uint8_t amount, bool voice, void* user_data),
        void* user_data)
{
    if (!device->supported_functions.ambient_sound_mode)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return NULL;
    }

    uint8_t inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_ASM;

    if (device->supported_functions.noise_cancelling)
    {
        inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM;
    }

    return mdr_device_add_subscription(
            device,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_NCASM_NTFY_PARAM,
                .extra = inquired_type,
                .only_ack = false
            },
            mdr_device_subscribe_ambient_sound_mode_settings_update,
            (void (*)()) update,
            user_data);
}

int mdr_device_disable_ncasm(
        mdr_device_t* device,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.noise_cancelling)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_NCASM_SET_PARAM;

    if (device->supported_functions.ambient_sound_mode)
    {
        request_packet.data = (mdr_packet_data_t){
            .ncasm_set_param = {
                .inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM,
                .noise_cancelling_asm = {
                    .ncasm_effect = MDR_PACKET_NCASM_NCASM_EFFECT_OFF,
                    .ncasm_setting_type = MDR_PACKET_NCASM_NCASM_SETTING_TYPE_DUAL_SINGLE_OFF,
                    .ncasm_amount = 2,
                    .asm_setting_type = MDR_PACKET_NCASM_ASM_SETTING_TYPE_LEVEL_ADJUSTMENT,
                    .asm_id = MDR_PACKET_NCASM_ASM_ID_NORMAL,
                    .asm_amount = 0
                }
            }
        };
    }
    else
    {
        request_packet.data = (mdr_packet_data_t){
            .ncasm_set_param = {
                .inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING,
                .noise_cancelling = {
                    .nc_setting_type = MDR_PACKET_NCASM_NC_SETTING_TYPE_ON_OFF,
                    .nc_setting_value = MDR_PACKET_NCASM_NC_SETTING_VALUE_OFF
                }
            }
        };
    }

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .only_ack = true
            },
            success_callback_passthrough,
            (void (*)()) success,
            error,
            user_data);

    return 0;
}

int mdr_device_enable_noise_cancelling(
        mdr_device_t* device,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.noise_cancelling)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_NCASM_SET_PARAM;

    if (device->supported_functions.ambient_sound_mode)
    {
        request_packet.data = (mdr_packet_data_t){
            .ncasm_set_param = {
                .inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM,
                .noise_cancelling_asm = {
                    .ncasm_effect = MDR_PACKET_NCASM_NCASM_EFFECT_ON,
                    .ncasm_setting_type = MDR_PACKET_NCASM_NCASM_SETTING_TYPE_DUAL_SINGLE_OFF,
                    .ncasm_amount = 2,
                    .asm_setting_type = MDR_PACKET_NCASM_ASM_SETTING_TYPE_LEVEL_ADJUSTMENT,
                    .asm_id = MDR_PACKET_NCASM_ASM_ID_NORMAL,
                    .asm_amount = 0
                }
            }
        };
    }
    else
    {
        request_packet.data = (mdr_packet_data_t){
            .ncasm_set_param = {
                .inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING,
                .noise_cancelling = {
                    .nc_setting_type = MDR_PACKET_NCASM_NC_SETTING_TYPE_ON_OFF,
                    .nc_setting_value = MDR_PACKET_NCASM_NC_SETTING_VALUE_ON
                }
            }
        };
    }

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .only_ack = true
            },
            success_callback_passthrough,
            (void (*)()) success,
            error,
            user_data);

    return 0;
}

int mdr_device_enable_ambient_sound_mode(
        mdr_device_t* device,
        uint8_t level,
        bool voice,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.ambient_sound_mode)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_NCASM_SET_PARAM;

    if (device->supported_functions.noise_cancelling)
    {
        request_packet.data = (mdr_packet_data_t){
            .ncasm_set_param = {
                .inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM,
                .noise_cancelling_asm = {
                    .ncasm_effect = MDR_PACKET_NCASM_NCASM_EFFECT_ON,
                    .ncasm_setting_type = MDR_PACKET_NCASM_NCASM_SETTING_TYPE_DUAL_SINGLE_OFF,
                    .ncasm_amount = 0,
                    .asm_setting_type = MDR_PACKET_NCASM_ASM_SETTING_TYPE_LEVEL_ADJUSTMENT,
                    .asm_id = voice ? MDR_PACKET_NCASM_ASM_ID_VOICE
                                    : MDR_PACKET_NCASM_ASM_ID_NORMAL,
                    .asm_amount = level
                }
            }
        };
    }
    else
    {
        request_packet.data = (mdr_packet_data_t){
            .ncasm_set_param = {
                .inquired_type = MDR_PACKET_NCASM_INQUIRED_TYPE_ASM,
                .ambient_sound_mode = {
                    .ncasm_effect = MDR_PACKET_NCASM_NCASM_EFFECT_ON,
                    .asm_setting_type = MDR_PACKET_NCASM_ASM_SETTING_TYPE_LEVEL_ADJUSTMENT,
                    .asm_id = voice ? MDR_PACKET_NCASM_ASM_ID_VOICE
                                    : MDR_PACKET_NCASM_ASM_ID_NORMAL,
                    .asm_amount = level,
                },
            },
        };
    }

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .only_ack = true
            },
            success_callback_passthrough,
            (void (*)()) success,
            error,
            user_data);

    return 0;
}

void mdr_device_get_eq_capabilities_result(
        mdr_packet_t* packet,
        void* callback_data_ptr)
{
    callback_data_t* callback_data = (callback_data_t*) callback_data_ptr;

    if (callback_data->user_result_callback != NULL)
    {
        uint8_t num_presets = packet->data.eqebb_ret_capability.eq.num_presets;
        mdr_packet_eqebb_eq_preset_id_t* presets = malloc(num_presets);

        if (presets == NULL)
        {
            error_callback_passthrough(callback_data_ptr);
            return;
        }

        for (int i = 0; i < num_presets; i++)
        {
            presets[i] = packet->data.eqebb_ret_capability.eq.presets[i]
                .preset_id;
        }

        callback_data->user_result_callback(
                packet->data.eqebb_ret_capability.eq.band_count,
                packet->data.eqebb_ret_capability.eq.level_steps,
                num_presets,
                presets,
                callback_data->user_data);

        free(presets);
    }

    free(callback_data);
}

int mdr_device_get_eq_capabilities(
        mdr_device_t* device,
        void (*result)(uint8_t band_count,
                       uint8_t level_steps,
                       uint8_t num_presets,
                       mdr_packet_eqebb_eq_preset_id_t* presets,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.eq
            && !device->supported_functions.eq_non_customizable)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_EQEBB_GET_CAPABILITY;

    request_packet.data = (mdr_packet_data_t){
        .eqebb_get_capability = {
            .inquired_type = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
            .display_language = MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_UNDEFINED_LANGUAGE,
        },
    };

    if (device->supported_functions.eq_non_customizable)
    {
        request_packet.data.eqebb_get_capability.inquired_type
            = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE;
    }

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_EQEBB_RET_CAPABILITY,
                .extra = request_packet.data.eqebb_get_capability.inquired_type,
                .only_ack = false,
            },
            mdr_device_get_eq_capabilities_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

void mdr_device_get_eq_preset_and_levels_result(
        mdr_packet_t* packet,
        void* callback_data_ptr)
{
    callback_data_t* callback_data = (callback_data_t*) callback_data_ptr;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
                packet->data.eqebb_ret_param.eq.preset_id,
                packet->data.eqebb_ret_param.eq.num_levels,
                packet->data.eqebb_ret_param.eq.levels,
                callback_data->user_data);
    }

    free(callback_data);
}

int mdr_device_get_eq_preset_and_levels(
        mdr_device_t* device,
        void (*result)(mdr_packet_eqebb_eq_preset_id_t preset,
                       uint8_t num_levels,
                       uint8_t* levels,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.eq
            && !device->supported_functions.eq_non_customizable)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_EQEBB_GET_PARAM;

    request_packet.data = (mdr_packet_data_t){
        .eqebb_get_param = {
            .inquired_type = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
        },
    };

    if (device->supported_functions.eq_non_customizable)
    {
        request_packet.data.eqebb_get_param.inquired_type
            = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE;
    }

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_EQEBB_RET_PARAM,
                .extra = request_packet.data.eqebb_ret_param.inquired_type,
                .only_ack = false,
            },
            mdr_device_get_eq_preset_and_levels_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

static void mdr_device_subscribe_eq_preset_and_levels_update(
        mdr_packet_t* packet,
        void* user_data)
{
    subscription_t* subscription = user_data;

    if (subscription->user_result_callback != NULL)
    {
        subscription->user_result_callback(
                packet->data.eqebb_ntfy_param.eq.preset_id,
                packet->data.eqebb_ntfy_param.eq.num_levels,
                packet->data.eqebb_ntfy_param.eq.levels,
                subscription->user_data);
    }
}

void* mdr_device_subscribe_eq_preset_and_levels(
        mdr_device_t* device,
        void (*update)(mdr_packet_eqebb_eq_preset_id_t preset_id,
                       uint8_t num_levels,
                       uint8_t* levels,
                       void* user_data),
        void* user_data)
{
    if (!device->supported_functions.eq
            && !device->supported_functions.eq_non_customizable)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return NULL;
    }

    uint8_t inquired_type = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ;

    if (device->supported_functions.eq_non_customizable)
    {
        inquired_type = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE;
    }

    return mdr_device_add_subscription(
            device,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_EQEBB_NTFY_PARAM,
                .extra = inquired_type,
                .only_ack = false
            },
            mdr_device_subscribe_eq_preset_and_levels_update,
            (void (*)()) update,
            user_data);
}

int mdr_device_set_eq_preset(
        mdr_device_t* device,
        mdr_packet_eqebb_eq_preset_id_t preset_id,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.eq)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_EQEBB_SET_PARAM;

    request_packet.data = (mdr_packet_data_t){
        .eqebb_set_param = {
            .inquired_type = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
            .eq = {
                .preset_id = preset_id,
                .num_levels = 0,
                .levels = NULL,
            },
        },
    };

    if (device->supported_functions.eq_non_customizable)
    {
        request_packet.data.eqebb_set_param.inquired_type
            = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE;
    }

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .only_ack = true,
            },
            success_callback_passthrough,
            (void (*)()) success,
            error,
            user_data);

    return 0;
}

int mdr_device_set_eq_levels(
        mdr_device_t* device,
        uint8_t num_levels,
        uint8_t* levels,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.eq)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_EQEBB_SET_PARAM;

    request_packet.data = (mdr_packet_data_t){
        .eqebb_set_param = {
            .inquired_type = MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
            .eq = {
                .preset_id = MDR_PACKET_EQEBB_EQ_PRESET_ID_UNSPECIFIED,
                .num_levels = num_levels,
                .levels = levels,
            },
        },
    };

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .only_ack = true
            },
            success_callback_passthrough,
            (void (*)()) success,
            error,
            user_data);

    return 0;
}

void mdr_device_setting_get_auto_power_off_timeouts_result(
        mdr_packet_t* packet,
        void* callback_data_ptr)
{
    callback_data_t* callback_data = (callback_data_t*) callback_data_ptr;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
                packet->data.system_ret_capability.auto_power_off.element_id_count,
                packet->data.system_ret_capability.auto_power_off.element_ids,
                callback_data->user_data);
    }

    free(callback_data);
}

int mdr_device_setting_get_auto_power_off_timeouts(
        mdr_device_t* device,
        void (*result)(uint8_t timeout_count,
                       mdr_packet_system_auto_power_off_element_id_t* timeouts,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.auto_power_off)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_SYSTEM_GET_CAPABILITY;

    request_packet.data = (mdr_packet_data_t){
        .system_get_param = {
            .inquired_type = MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF,
        },
    };

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_SYSTEM_RET_CAPABILITY,
                .extra = request_packet.data.system_get_param.inquired_type,
                .only_ack = false,
            },
            mdr_device_setting_get_auto_power_off_timeouts_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

void mdr_device_setting_get_auto_power_off_result(
        mdr_packet_t* packet,
        void* callback_data_ptr)
{
    callback_data_t* callback_data = (callback_data_t*) callback_data_ptr;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
                packet->data.system_ret_param.auto_power_off.element_id
                    != MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_DISABLE,
                packet->data.system_ret_param.auto_power_off
                    .select_time_element_id,
                callback_data->user_data);
    }

    free(callback_data);
}

int mdr_device_setting_get_auto_power_off(
        mdr_device_t* device,
        void (*result)(bool enabled,
                       mdr_packet_system_auto_power_off_element_id_t time,
                       void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.auto_power_off)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_SYSTEM_GET_PARAM;

    request_packet.data = (mdr_packet_data_t){
        .system_get_param = {
            .inquired_type = MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF,
        },
    };

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_SYSTEM_RET_PARAM,
                .extra = request_packet.data.system_get_param.inquired_type,
                .only_ack = false,
            },
            mdr_device_setting_get_auto_power_off_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

void mdr_device_setting_subscribe_auto_power_off_update(
        mdr_packet_t* packet,
        void* user_data)
{
    subscription_t* subscription = user_data;

    if (subscription->user_result_callback != NULL)
    {
        subscription->user_result_callback(
                packet->data.system_ntfy_param.auto_power_off.element_id
                    != MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_DISABLE,
                packet->data.system_ntfy_param.auto_power_off
                    .select_time_element_id,
                subscription->user_data);
    }
}

void* mdr_device_setting_subscribe_auto_power_off(
        mdr_device_t* device,
        void (*update)(bool enabled,
                       mdr_packet_system_auto_power_off_element_id_t time,
                       void* user_data),
        void* user_data)
{
    if (!device->supported_functions.auto_power_off)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return NULL;
    }

    return mdr_device_add_subscription(
            device,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_SYSTEM_NTFY_PARAM,
                .extra = MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF,
                .only_ack = false,
            },
            mdr_device_setting_subscribe_auto_power_off_update,
            (void (*)()) update,
            user_data);
}

int mdr_device_setting_disable_auto_power_off(
        mdr_device_t* device,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.auto_power_off)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_SYSTEM_SET_PARAM;

    request_packet.data = (mdr_packet_data_t){
        .system_set_param = {
            .inquired_type = MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF,
            .auto_power_off = {
                .parameter_type = MDR_PACKET_SYSTEM_AUTO_POWER_OFF_PARAMETER_TYPE_ACTIVE_AND_SELECT_TIME_ID,
                .element_id = MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_DISABLE,
                .select_time_element_id
                    = MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_DISABLE,
            },
        },
    };

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .only_ack = true
            },
            success_callback_passthrough,
            (void (*)()) success,
            error,
            user_data);

    return 0;
}

int mdr_device_setting_enable_auto_power_off(
        mdr_device_t* device,
        mdr_packet_system_auto_power_off_element_id_t time,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.auto_power_off)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_SYSTEM_SET_PARAM;

    request_packet.data = (mdr_packet_data_t){
        .system_set_param = {
            .inquired_type = MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF,
            .auto_power_off = {
                .parameter_type = MDR_PACKET_SYSTEM_AUTO_POWER_OFF_PARAMETER_TYPE_ACTIVE_AND_SELECT_TIME_ID,
                .element_id = MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_WHEN_REMOVED_FROM_EARS,
                .select_time_element_id = time,
            },
        },
    };

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .only_ack = true
            },
            success_callback_passthrough,
            (void (*)()) success,
            error,
            user_data);

    return 0;
}

static void mdr_device_playback_get_volume_result(
        mdr_packet_t* packet,
        void* callback_data_ptr)
{
    callback_data_t* callback_data = (callback_data_t*) callback_data_ptr;

    if (callback_data->user_result_callback != NULL)
    {
        callback_data->user_result_callback(
                packet->data.play_ret_param.volume,
                callback_data->user_data);
    }

    free(callback_data);
}

int mdr_device_playback_get_volume(
        mdr_device_t* device,
        void (*result)(uint8_t volume, void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.playback_controller)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_PLAY_GET_PARAM;
    request_packet.data = (mdr_packet_data_t){
        .play_get_param = {
            .inquired_type = MDR_PACKET_PLAY_INQUIRED_TYPE_PLAYBACK_CONTROLLER,
            .detailed_data_type = MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME,
        },
    };

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_PLAY_RET_PARAM,
                .extra = MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME,
                .only_ack = false,
            },
            mdr_device_playback_get_volume_result,
            (void (*)()) result,
            error,
            user_data);

    return 0;
}

static void mdr_device_playback_subscribe_volume_update(
        mdr_packet_t* packet,
        void* user_data)
{
    subscription_t* subscription = user_data;

    if (subscription->user_result_callback != NULL)
    {
        subscription->user_result_callback(
                packet->data.play_ntfy_param.volume,
                subscription->user_data);
    }
}

void* mdr_device_playback_subscribe_volume(
        mdr_device_t* device,
        void (*update)(uint8_t volume, void* user_data),
        void* user_data)
{
    if (!device->supported_functions.playback_controller)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return NULL;
    }

    return mdr_device_add_subscription(
            device,
            (mdr_packetconn_reply_specifier_t){
                .packet_type = MDR_PACKET_PLAY_NTFY_PARAM,
                .extra = MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME,
                .only_ack = false
            },
            mdr_device_playback_subscribe_volume_update,
            (void (*)()) update,
            user_data);
}

int mdr_device_playback_set_volume(
        mdr_device_t* device,
        uint8_t volume,
        void (*success)(void* user_data),
        void (*error)(void* user_data),
        void* user_data)
{
    if (!device->supported_functions.playback_controller)
    {
        errno = MDR_E_NOT_SUPPORTED;
        return -1;
    }

    mdr_packet_t request_packet;
    request_packet.type = MDR_PACKET_PLAY_SET_PARAM;
    request_packet.data = (mdr_packet_data_t){
        .play_set_param = {
            .inquired_type = MDR_PACKET_PLAY_INQUIRED_TYPE_PLAYBACK_CONTROLLER,
            .detailed_data_type = MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME,
            .volume = volume,
        },
    };

    mdr_device_make_request(
            device,
            &request_packet,
            (mdr_packetconn_reply_specifier_t){
                .only_ack = true,
            },
            success_callback_passthrough,
            (void (*)()) success,
            error,
            user_data);

    return 0;
}
