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

#include "mdr/packet.h"

#include "mdr/errors.h"

#include <string.h>
#include <errno.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

void mdr_packet_free(mdr_packet_t* packet)
{
    if (packet == NULL)
        return;

    switch (packet->type)
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
            break;

        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
            break;

        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
            switch (packet->data.connect_ret_device_info.inquired_type)
            {
                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME:
                    free(packet->data.connect_ret_device_info
                            .model_name.string);
                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION:
                    free(packet->data.connect_ret_device_info
                            .fw_version.string);
                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR:
                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE:
                    free(packet->data.connect_ret_device_info
                            .instruction_guide.guidance_categories);
                    break;
            }
            break;

        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
            free(packet->data.connect_ret_support_function.function_types);
            break;

        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
            break;

        case MDR_PACKET_COMMON_SET_POWER_OFF:
            break;

        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
            break;

        case MDR_PACKET_EQEBB_GET_CAPABILITY:
            break;

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
            switch (packet->data.eqebb_ret_capability.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    if (packet->data.eqebb_ret_capability.eq.presets != NULL)
                    {
                        for (int i = 0;
                             i < packet->data.eqebb_ret_capability.eq
                                    .num_presets;
                             i++)
                        {
                            free(packet->data.eqebb_ret_capability.eq.presets[i]
                                    .name);
                        }
                    }
                    free(packet->data.eqebb_ret_capability.eq.presets);
                    break;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    break;
            }
            break;

        case MDR_PACKET_EQEBB_GET_PARAM:
            break;

        case MDR_PACKET_EQEBB_RET_PARAM:
            switch (packet->data.eqebb_ret_param.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    free(packet->data.eqebb_ret_param.eq.levels);
                    break;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    break;
            }
            break;

        case MDR_PACKET_EQEBB_SET_PARAM:
            switch (packet->data.eqebb_set_param.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    free(packet->data.eqebb_set_param.eq.levels);
                    break;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    break;
            }
            break;

        case MDR_PACKET_EQEBB_NTFY_PARAM:
            switch (packet->data.eqebb_ntfy_param.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    free(packet->data.eqebb_ntfy_param.eq.levels);
                    break;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    break;
            }
            break;

        case MDR_PACKET_NCASM_GET_PARAM:
        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
            break;

        case MDR_PACKET_PLAY_GET_PARAM:
            break;

        case MDR_PACKET_PLAY_RET_PARAM:
            switch (packet->data.play_ret_param.detailed_data_type)
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    break;

                default:
                    free(packet->data.play_ret_param.string.data);
                    break;
            }
            break;

        case MDR_PACKET_PLAY_SET_PARAM:
            switch (packet->data.play_set_param.detailed_data_type)
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    break;

                default:
                    free(packet->data.play_set_param.string.data);
                    break;
            }
            break;

        case MDR_PACKET_PLAY_NTFY_PARAM:
            switch (packet->data.play_ntfy_param.detailed_data_type)
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    break;

                default:
                    free(packet->data.play_ntfy_param.string.data);
                    break;
            }
            break;

        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
            switch (packet->data.system_ret_capability.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    free(packet->data.system_ret_capability
                            .auto_power_off.element_ids);

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    if (packet->data.system_ret_capability.assignable_settings
                            .capability_keys != NULL)
                    {
                        for (int i = 0;
                             i < packet->data.system_ret_capability
                                     .assignable_settings.num_capability_keys;
                             i++)
                        {
                            if (packet->data.system_ret_capability
                                    .assignable_settings.capability_keys[i]
                                    .capability_presets != NULL)
                            {
                                for (int j = 0;
                                     j < packet->data.system_ret_capability
                                            .assignable_settings.capability_keys[i]
                                            .num_capability_presets;
                                     j++)
                                {
                                    free(packet->data.system_ret_capability
                                            .assignable_settings.capability_keys[i]
                                            .capability_presets[j]
                                            .capability_actions);
                                }
                            }

                            free(packet->data.system_ret_capability
                                    .assignable_settings.capability_keys[i]
                                    .capability_presets);
                        }
                    }

                    free(packet->data.system_ret_capability
                            .assignable_settings.capability_keys);
                    break;
            }
            break;

        case MDR_PACKET_SYSTEM_GET_PARAM:
            break;

        case MDR_PACKET_SYSTEM_RET_PARAM:
            switch (packet->data.system_ret_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    free(packet->data.system_ret_param
                            .assignable_settings.presets);
                    break;
            }
            break;

        case MDR_PACKET_SYSTEM_SET_PARAM:
            switch (packet->data.system_set_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    free(packet->data.system_set_param
                            .assignable_settings.presets);
                    break;
            }
            break;

        case MDR_PACKET_SYSTEM_NTFY_PARAM:
            switch (packet->data.system_ntfy_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    free(packet->data.system_ntfy_param
                            .assignable_settings.presets);
                    break;
            }
            break;
    }

    free(packet);
}

mdr_packet_t* mdr_packet_from_frame(mdr_frame_t* frame)
{
    if (frame->data_type != MDR_FRAME_DATA_TYPE_DATA_MDR)
    {
        errno = MDR_E_INVALID_PACKET;
        return NULL;
    }

    uint8_t* payload = mdr_frame_payload(frame);
    size_t payload_len = frame->payload_length;

#define INVALID_FRAME \
    { \
        errno = MDR_E_INVALID_FRAME; \
        return NULL; \
    }

#define ASSERT_LEN_AT_LEAST(n) \
    if (payload_len < n) INVALID_FRAME;

#define ASSERT_ITH_EQUALS(i, val) \
    if (payload[i] != val) INVALID_FRAME;

#define ASSERT_ITH_IN_RANGE(i, min, max) \
    if (!(min <= payload[i] && payload[i] <= max)) INVALID_FRAME;

#define ALLOC_PACKET(len) \
    packet = malloc(len); \
    if (packet == NULL) return NULL;

#define COPY_TO_PACKET(len) \
    memcpy(packet, payload, len);

#define READ_VARIABLE_LEN(result, offset, len, item) \
    ASSERT_LEN_AT_LEAST(offset + len * sizeof(item)); \
    result = malloc(len * sizeof(item)); \
    if (result == NULL) \
    { \
        free(packet); \
        return NULL; \
    } \
    memcpy(result, &payload[offset], len * sizeof(item));

#define RETURN_FIXED_LENGTH_PACKET(len) \
    ALLOC_PACKET(len); \
    COPY_TO_PACKET(len); \
    return packet;

    mdr_packet_t* packet = NULL;

    ASSERT_LEN_AT_LEAST(1);

    switch (payload[0])
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_EQUALS(1, 0);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_connect_get_protocol_info_t));

        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_EQUALS(1, 0);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_connect_ret_protocol_info_t));

        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME,
                MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_connect_get_device_info_t));

        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME,
                MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE);
            
            switch (payload[1])
            {
                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME:
                    ALLOC_PACKET(2 + sizeof(mdr_packet_device_info_string_t));
                    COPY_TO_PACKET(3);
                    packet->data.connect_ret_device_info.model_name.len =
                        min(128,
                            packet->data.connect_ret_device_info
                                .model_name.len);
                    READ_VARIABLE_LEN(
                        packet->data.connect_ret_device_info.model_name.string,
                        3,
                        packet->data.connect_ret_device_info.model_name.len,
                        uint8_t);
                    return packet;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION:
                    ALLOC_PACKET(2 + sizeof(mdr_packet_device_info_string_t));
                    COPY_TO_PACKET(3);
                    packet->data.connect_ret_device_info.fw_version.len =
                        min(128,
                            packet->data.connect_ret_device_info
                                .fw_version.len);
                    READ_VARIABLE_LEN(
                        packet->data.connect_ret_device_info.fw_version.string,
                        3,
                        packet->data.connect_ret_device_info.fw_version.len,
                        uint8_t);
                    return packet;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR:
                    RETURN_FIXED_LENGTH_PACKET(1 +
                            sizeof(mdr_packet_device_info_series_and_color_t));

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE:
                    ALLOC_PACKET(2 +
                            sizeof(mdr_packet_device_info_instruction_guide_t));
                    COPY_TO_PACKET(3);
                    READ_VARIABLE_LEN(
                        packet->data.connect_ret_device_info
                            .instruction_guide.guidance_categories,
                        3,
                        packet->data.connect_ret_device_info
                            .instruction_guide.count,
                        uint8_t);
                    return packet;
            }

        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_EQUALS(1, 0);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_connect_get_support_function_t));

        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_EQUALS(1, 0);

            ALLOC_PACKET(1 + sizeof(mdr_packet_connect_ret_support_function_t));
            COPY_TO_PACKET(3);
            READ_VARIABLE_LEN(
                    packet->data.connect_ret_support_function
                        .function_types,
                    3,
                    packet->data.connect_ret_support_function
                        .num_function_types,
                    mdr_packet_support_function_type_t);
            return packet;

        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY,
                    MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_common_get_battery_level_t));

        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY,
                    MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY);

            switch (payload[1])
            {
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_battery_status_t));

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_battery_status_left_right_t));

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_battery_status_t));
            }

        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY,
                    MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY);

            switch (payload[1])
            {
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_battery_status_t));

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_battery_status_left_right_t));

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_battery_status_t));
            }

        case MDR_PACKET_COMMON_SET_POWER_OFF:
            ASSERT_LEN_AT_LEAST(1
                    + sizeof(mdr_packet_common_set_power_off_t));
            ASSERT_ITH_EQUALS(1,
                    MDR_PACKET_COMMON_POWER_OFF_INQUIRED_TYPE_FIXED_VALUE);
            ASSERT_ITH_EQUALS(2,
                    MDR_PACKET_COMMON_POWER_OFF_SETTING_VALUE_USER_POWER_OFF);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_common_set_power_off_t));

        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT,
                    MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_common_get_connection_status_t));

        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT,
                    MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT);

            switch (payload[1])
            {
                case MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_connection_status_left_right_status_t));
            }

        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT,
                    MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT);

            switch (payload[1])
            {
                case MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_connection_status_left_right_status_t));
            }

        case MDR_PACKET_EQEBB_GET_CAPABILITY:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_eqebb_get_capability_t));

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE);

            switch (payload[1])
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                {
                    ALLOC_PACKET(2 + sizeof(mdr_packet_eqebb_capability_eq_t));
                    COPY_TO_PACKET(5);

                    uint8_t num_presets = payload[4];
                    mdr_packet_eqebb_capability_eq_preset_name_t* presets =
                        malloc(num_presets * sizeof(
                                mdr_packet_eqebb_capability_eq_preset_name_t));
                    if (presets == NULL)
                    {
                        free(packet);
                        return NULL;
                    }

                    size_t pos = 5;
                    
                    for (int i = 0; i < num_presets; i++)
                    {
                        ASSERT_LEN_AT_LEAST(pos + 2);

                        memcpy(&presets[i], &payload[pos], 2);
                        pos += 2;
                        
                        uint8_t name_len = presets[i].name_len;
                        uint8_t* name = malloc(name_len);
                        if (name == NULL)
                        {
                            for (int j = 0; j < i; j++)
                            {
                                free(presets[i].name);
                            }
                            free(presets);
                            free(packet);
                            return NULL;
                        }

                        memcpy(name, &payload[pos], name_len);
                        pos += name_len;

                        presets[i].name = name;
                    }

                    packet->data.eqebb_ret_capability.eq.presets = presets;

                    return packet;
                }

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_eqebb_capability_ebb_t));
            }

        case MDR_PACKET_EQEBB_GET_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_eqebb_get_param_t));

        case MDR_PACKET_EQEBB_RET_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE);

            switch (payload[1])
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    ALLOC_PACKET(2 + sizeof(mdr_packet_eqebb_param_eq_t));
                    COPY_TO_PACKET(4);
                    READ_VARIABLE_LEN(
                            packet->data.eqebb_ret_param.eq.levels,
                            4,
                            packet->data.eqebb_ret_param.eq.num_levels,
                            uint8_t);
                    return packet;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_eqebb_param_ebb_t));
            }

        case MDR_PACKET_EQEBB_SET_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE);

            switch (payload[1])
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    ALLOC_PACKET(2 + sizeof(mdr_packet_eqebb_param_eq_t));
                    COPY_TO_PACKET(4);
                    READ_VARIABLE_LEN(
                            packet->data.eqebb_set_param.eq.levels,
                            4,
                            packet->data.eqebb_set_param.eq.num_levels,
                            uint8_t);
                    return packet;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_eqebb_param_ebb_t));
            }

        case MDR_PACKET_EQEBB_NTFY_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ,
                    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE);

            switch (payload[1])
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    ALLOC_PACKET(2 + sizeof(mdr_packet_eqebb_param_eq_t));
                    COPY_TO_PACKET(4);
                    READ_VARIABLE_LEN(
                            packet->data.eqebb_ntfy_param.eq.levels,
                            4,
                            packet->data.eqebb_ntfy_param.eq.num_levels,
                            uint8_t);
                    return packet;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_eqebb_param_ebb_t));
            }

        case MDR_PACKET_NCASM_GET_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING,
                    MDR_PACKET_NCASM_INQUIRED_TYPE_ASM);

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_common_ret_battery_level_t));

        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING,
                    MDR_PACKET_NCASM_INQUIRED_TYPE_ASM);

            switch (payload[1])
            {
                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_ncasm_param_noise_cancelling_t));

                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_ncasm_param_noise_cancelling_asm_t));

                case MDR_PACKET_NCASM_INQUIRED_TYPE_ASM:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_ncasm_param_asm_t));
            }

        case MDR_PACKET_PLAY_GET_PARAM:
            ASSERT_LEN_AT_LEAST(3);
            ASSERT_ITH_EQUALS(1,
                    MDR_PACKET_PLAY_INQUIRED_TYPE_PLAYBACK_CONTROLLER);
            switch (payload[2])
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    break;

                default:
                    INVALID_FRAME;
            }

            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_play_get_param_t));

        case MDR_PACKET_PLAY_RET_PARAM:
            ASSERT_LEN_AT_LEAST(4);
            ASSERT_ITH_EQUALS(1,
                    MDR_PACKET_PLAY_INQUIRED_TYPE_PLAYBACK_CONTROLLER);
            switch (payload[2])
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                {
                    ASSERT_ITH_IN_RANGE(3,
                            MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_UNSETTLED,
                            MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_SETTLED);

                    uint8_t length = payload[4];
                    if (length > 128) length = 128;
                    ASSERT_LEN_AT_LEAST(5 + length);

                    ALLOC_PACKET(1 + sizeof(mdr_packet_play_ret_param_t));
                    COPY_TO_PACKET(5);

                    packet->data.play_ret_param.string.data = malloc(length);

                    if (packet->data.play_ret_param.string.data == NULL)
                    {
                        free(packet);
                        INVALID_FRAME;
                    }

                    memcpy(packet->data.play_ret_param.string.data,
                           &payload[5],
                           length);

                    return packet;
                }

                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    ALLOC_PACKET(4);
                    COPY_TO_PACKET(4);
                    return packet;

                default:
                    INVALID_FRAME;
            }
            break;

        case MDR_PACKET_PLAY_SET_PARAM:
            ASSERT_LEN_AT_LEAST(4);
            ASSERT_ITH_EQUALS(1,
                    MDR_PACKET_PLAY_INQUIRED_TYPE_PLAYBACK_CONTROLLER);
            switch (payload[2])
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                {
                    ASSERT_ITH_IN_RANGE(3,
                            MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_UNSETTLED,
                            MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_SETTLED);

                    uint8_t length = payload[4];
                    if (length > 128) length = 128;
                    ASSERT_LEN_AT_LEAST(5 + length);

                    ALLOC_PACKET(1 + sizeof(mdr_packet_play_set_param_t));
                    COPY_TO_PACKET(5);

                    packet->data.play_set_param.string.data = malloc(length);

                    if (packet->data.play_set_param.string.data == NULL)
                    {
                        free(packet);
                        INVALID_FRAME;
                    }

                    memcpy(packet->data.play_set_param.string.data,
                           &payload[5],
                           length);

                    return packet;
                }

                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    ALLOC_PACKET(4);
                    COPY_TO_PACKET(4);
                    return packet;

                default:
                    INVALID_FRAME;
            }
            break;

        case MDR_PACKET_PLAY_NTFY_PARAM:
            ASSERT_LEN_AT_LEAST(4);
            ASSERT_ITH_EQUALS(1,
                    MDR_PACKET_PLAY_INQUIRED_TYPE_PLAYBACK_CONTROLLER);
            switch (payload[2])
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                {
                    ASSERT_ITH_IN_RANGE(3,
                            MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_UNSETTLED,
                            MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_SETTLED);

                    uint8_t length = payload[4];
                    if (length > 128) length = 128;
                    ASSERT_LEN_AT_LEAST(5 + length);

                    ALLOC_PACKET(1 + sizeof(mdr_packet_play_ntfy_param_t));
                    COPY_TO_PACKET(5);

                    packet->data.play_ntfy_param.string.data = malloc(length);

                    if (packet->data.play_ntfy_param.string.data == NULL)
                    {
                        free(packet);
                        INVALID_FRAME;
                    }

                    memcpy(packet->data.play_ntfy_param.string.data,
                           &payload[5],
                           length);

                    return packet;
                }

                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    ALLOC_PACKET(4);
                    COPY_TO_PACKET(4);
                    return packet;

                default:
                    INVALID_FRAME;
            }
            break;

        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS);
            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_system_get_capability_t));

        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS);

            switch (payload[1])
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    ASSERT_LEN_AT_LEAST(2
                            + sizeof(mdr_packet_system_capability_vibrator_t));
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_capability_vibrator_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    ASSERT_LEN_AT_LEAST(2
                            + sizeof(mdr_packet_system_capability_power_saving_mode_t));
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_capability_power_saving_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    ASSERT_LEN_AT_LEAST(2
                            + sizeof(mdr_packet_system_control_by_wearing_setting_type_t));
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_control_by_wearing_setting_type_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    ASSERT_LEN_AT_LEAST(3);
                    ASSERT_LEN_AT_LEAST(3 + payload[2]);

                    for (int i = 0; i < payload[2]; i++)
                    {
                        if (!(
                            payload[3+i] < MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_180_MIN
                            || payload[3+i] == MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_WHEN_REMOVED_FROM_EARS
                            || payload[3+i] == MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_DISABLE))
                        {
                            INVALID_FRAME;
                        }
                    }

                    ALLOC_PACKET(2 + sizeof(mdr_packet_system_capability_auto_power_off_t));
                    COPY_TO_PACKET(3);
                    READ_VARIABLE_LEN(
                            packet->data.system_ret_capability.auto_power_off.element_ids,
                            3,
                            packet->data.system_ret_capability.auto_power_off.element_id_count,
                            mdr_packet_system_auto_power_off_element_id_t);

                    return packet;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_capability_smart_talking_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                {
                    ALLOC_PACKET(2
                            + sizeof(mdr_packet_system_capability_assignable_settings_t));
                    COPY_TO_PACKET(3);

                    mdr_packet_system_capability_assignable_settings_t*
                        assignable_settings
                            = &packet->data.system_ret_capability
                                .assignable_settings;

                    assignable_settings->capability_keys
                        = malloc(assignable_settings->num_capability_keys
                                * sizeof(mdr_packet_system_assignable_settings_capability_key_t*));

                    if (assignable_settings->capability_keys != NULL)
                    {
                        free(packet);
                        return NULL;
                    }

                    int pos = 3;
                    for (int i = 0;
                         i < assignable_settings->num_capability_keys;
                         i++)
                    {
                        const int key_fixed_size
                            = sizeof(mdr_packet_system_assignable_settings_key_t)
                            + sizeof(mdr_packet_system_assignable_settings_key_type_t)
                            + sizeof(mdr_packet_system_assignable_settings_preset_t)
                            + sizeof(uint8_t);

                        ASSERT_LEN_AT_LEAST(pos + key_fixed_size);

                        mdr_packet_system_assignable_settings_capability_key_t*
                            key = &assignable_settings->capability_keys[i];

                        memcpy(key,
                               &payload[pos],
                               key_fixed_size);

                        pos += key_fixed_size;

                        key->capability_presets
                            = malloc(key->num_capability_presets
                                * sizeof(mdr_packet_system_assignable_settings_capability_preset_t));

                        if (key->capability_presets == NULL)
                        {
                            for (int x = 0;
                                 x < i;
                                 x++)
                            {
                                for (int y = 0;
                                     y < assignable_settings
                                            ->capability_keys[x]
                                            .num_capability_presets;
                                     y++)
                                {
                                    free(assignable_settings
                                            ->capability_keys[x]
                                            .capability_presets[y]
                                            .capability_actions);
                                }
                                free(assignable_settings
                                        ->capability_keys[x]
                                        .capability_presets);
                            }
                            free(assignable_settings->capability_keys);
                            free(packet);
                            return NULL;
                        }

                        for (int j = 0;
                             j < key->num_capability_presets;
                             j++)
                        {
                            const int preset_fixed_size
                                = sizeof(mdr_packet_system_assignable_settings_preset_t)
                                + sizeof(uint8_t);

                            // TODO replace asserts
                            ASSERT_LEN_AT_LEAST(pos + preset_fixed_size);

                            mdr_packet_system_assignable_settings_capability_preset_t*
                                preset = &key->capability_presets[j];

                            memcpy(preset,
                                   &payload[pos],
                                   preset_fixed_size);

                            pos += preset_fixed_size;

                            ASSERT_LEN_AT_LEAST(pos
                                    + preset->num_capability_actions
                                    * sizeof(mdr_packet_system_assignable_settings_capability_caption_t));

                            preset->capability_actions = malloc(
                                    preset->num_capability_actions
                                    * sizeof(mdr_packet_system_assignable_settings_capability_caption_t));

                            if (preset->capability_actions == NULL)
                            {
                                for (int x = 0;
                                     x < j;
                                     x++)
                                {
                                    free(assignable_settings
                                            ->capability_keys[i]
                                            .capability_presets[j]
                                            .capability_actions);
                                }

                                free(assignable_settings
                                        ->capability_keys[i]
                                        .capability_presets);

                                for (int x = 0;
                                     x < i;
                                     x++)
                                {
                                    for (int y = 0;
                                         y < assignable_settings
                                                ->capability_keys[x]
                                                .num_capability_presets;
                                         y++)
                                    {
                                        free(assignable_settings
                                                ->capability_keys[x]
                                                .capability_presets[y]
                                                .capability_actions);
                                    }
                                    free(assignable_settings
                                            ->capability_keys[x]
                                            .capability_presets);
                                }
                                free(assignable_settings->capability_keys);
                                free(packet);
                                return NULL;
                            }

                            memcpy(&preset->capability_actions,
                                   &payload[pos],
                                   preset->num_capability_actions
                                       * sizeof(mdr_packet_system_assignable_settings_capability_caption_t));

                            pos += preset->num_capability_actions
                                * sizeof(mdr_packet_system_assignable_settings_capability_caption_t);
                        }
                    }

                    return packet;
                }
            }

        case MDR_PACKET_SYSTEM_GET_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS);
            RETURN_FIXED_LENGTH_PACKET(1
                    + sizeof(mdr_packet_system_get_param_t));

        case MDR_PACKET_SYSTEM_RET_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS);

            switch (payload[1])
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_vibrator_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_power_saving_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_control_by_wearing_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_auto_power_off_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_smart_talking_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    ALLOC_PACKET(2 + sizeof(mdr_packet_system_param_assignable_settings_t));
                    COPY_TO_PACKET(3);
                    READ_VARIABLE_LEN(
                        packet->data.system_ret_param.assignable_settings.presets,
                        3,
                        packet->data.system_ret_param.assignable_settings.num_presets,
                        mdr_packet_system_assignable_settings_preset_t);
                    return packet;
            }

        case MDR_PACKET_SYSTEM_SET_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS);

            switch (payload[1])
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_vibrator_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_power_saving_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_control_by_wearing_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_auto_power_off_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_smart_talking_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    ALLOC_PACKET(2 + sizeof(mdr_packet_system_param_assignable_settings_t));
                    COPY_TO_PACKET(3);
                    READ_VARIABLE_LEN(
                        packet->data.system_set_param.assignable_settings.presets,
                        3,
                        packet->data.system_set_param.assignable_settings.num_presets,
                        mdr_packet_system_assignable_settings_preset_t);
                    return packet;
            }

        case MDR_PACKET_SYSTEM_NTFY_PARAM:
            ASSERT_LEN_AT_LEAST(2);
            ASSERT_ITH_IN_RANGE(1,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR,
                    MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS);

            switch (payload[1])
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_vibrator_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_power_saving_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_control_by_wearing_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_auto_power_off_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    RETURN_FIXED_LENGTH_PACKET(2
                            + sizeof(mdr_packet_system_param_smart_talking_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    ALLOC_PACKET(2 + sizeof(mdr_packet_system_param_assignable_settings_t));
                    COPY_TO_PACKET(3);
                    READ_VARIABLE_LEN(
                        packet->data.system_ntfy_param.assignable_settings.presets,
                        3,
                        packet->data.system_ntfy_param.assignable_settings.num_presets,
                        mdr_packet_system_assignable_settings_preset_t);
                    return packet;
            }

        default:
            INVALID_FRAME;
    }
}

mdr_frame_t* mdr_packet_to_frame(mdr_packet_t* packet)
{
    mdr_frame_t* frame;

#define ALLOC_FRAME(len) \
    frame = malloc(MDR_FRAME_EMPTY_LEN + len); \
    if (frame == NULL) return NULL; \
    frame->data_type = MDR_FRAME_DATA_TYPE_DATA_MDR; \
    frame->sequence_id = 0; \
    frame->payload_length = len;

#define COPY_FRAME(len) \
    memcpy(mdr_frame_payload(frame), \
           packet, \
           len);

#define RETURN_FIXED_LENGTH_FRAME(len) \
    ALLOC_FRAME(len); \
    COPY_FRAME(len); \
    return frame;

    switch (packet->type)
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_connect_get_protocol_info_t));

        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_connect_ret_protocol_info_t));

        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_connect_get_device_info_t));

        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
            switch (packet->data.connect_ret_device_info.inquired_type)
            {
                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME:
                    ALLOC_FRAME(3 + packet->data.connect_ret_device_info
                            .model_name.len);
                    COPY_FRAME(3);
                    memcpy(&mdr_frame_payload(frame)[3],
                           packet->data.connect_ret_device_info
                               .model_name.string,
                           packet->data.connect_ret_device_info.model_name.len);
                    return frame;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION:
                    ALLOC_FRAME(3 + packet->data.connect_ret_device_info
                            .fw_version.len);
                    COPY_FRAME(3);
                    memcpy(&mdr_frame_payload(frame)[3],
                           packet->data.connect_ret_device_info
                               .fw_version.string,
                           packet->data.connect_ret_device_info.fw_version.len);
                    return frame;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR:
                    RETURN_FIXED_LENGTH_FRAME(2 +
                            sizeof(mdr_packet_device_info_series_and_color_t));

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE:
                    ALLOC_FRAME(3 + packet->data.connect_ret_device_info
                            .instruction_guide.count);
                    COPY_FRAME(3);
                    memcpy(&mdr_frame_payload(frame)[3],
                           packet->data.connect_ret_device_info
                               .instruction_guide.guidance_categories,
                           packet->data.connect_ret_device_info
                               .instruction_guide.count);
                    return frame;
            }

        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_connect_get_support_function_t));

        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
            ALLOC_FRAME(3 + packet->data.connect_ret_support_function
                    .num_function_types);
            COPY_FRAME(3);
            memcpy(&mdr_frame_payload(frame)[3],
                   packet->data.connect_ret_support_function
                       .function_types,
                   packet->data.connect_ret_support_function
                       .num_function_types);
            return frame;

        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_common_get_battery_level_t));

        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
            switch (packet->data.common_ret_battery_level.inquired_type)
            {
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY:
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_battery_status_t));

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_battery_status_left_right_t));
            }

        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
            switch (packet->data.common_ntfy_battery_level.inquired_type)
            {
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY:
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_battery_status_t));

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_battery_status_left_right_t));
            }

        case MDR_PACKET_COMMON_SET_POWER_OFF:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_common_set_power_off_t));

        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_common_get_connection_status_t));

        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
            switch (packet->data.common_ret_connection_status.inquired_type)
            {
                case MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_connection_status_left_right_status_t));
            }

        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
            switch (packet->data.common_ret_connection_status.inquired_type)
            {
                case MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_connection_status_left_right_status_t));
            }

        case MDR_PACKET_EQEBB_GET_CAPABILITY:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_eqebb_get_capability_t));

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
            switch (packet->data.eqebb_ret_capability.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                {
                    size_t payload_len = 5
                        + packet->data.eqebb_ret_capability.eq.num_presets * 2;

                    for (int i = 0;
                         i < packet->data.eqebb_ret_capability.eq.num_presets;
                         i++)
                    {
                        payload_len += packet->data.eqebb_ret_capability.eq
                            .presets[i].name_len;
                    }

                    ALLOC_FRAME(payload_len);
                    COPY_FRAME(5);

                    size_t pos = 5;

                    for (int i = 0;
                         i < packet->data.eqebb_ret_capability.eq.num_presets;
                         i++)
                    {
                        memcpy(&mdr_frame_payload(frame)[pos],
                               &packet->data.eqebb_ret_capability.eq.presets[i],
                               2);
                        pos += 2;

                        memcpy(&mdr_frame_payload(frame)[pos],
                               &packet->data.eqebb_ret_capability.eq
                                   .presets[i].name,
                               packet->data.eqebb_ret_capability.eq
                                   .presets[i].name_len);
                        pos += packet->data.eqebb_ret_capability.eq
                            .presets[i].name_len;
                    }

                    return frame;
                }

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_eqebb_capability_ebb_t));
            }
        break;

        case MDR_PACKET_EQEBB_GET_PARAM:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_eqebb_get_param_t));

        case MDR_PACKET_EQEBB_RET_PARAM:
            switch (packet->data.eqebb_ret_param.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    ALLOC_FRAME(4 + packet->data.eqebb_ret_param.eq.num_levels);
                    COPY_FRAME(4);
                    memcpy(&mdr_frame_payload(frame)[4],
                           packet->data.eqebb_ret_param.eq.levels,
                           packet->data.eqebb_ret_param.eq.num_levels);
                    return frame;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_eqebb_param_ebb_t));
            }

        case MDR_PACKET_EQEBB_SET_PARAM:
            switch (packet->data.eqebb_set_param.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    ALLOC_FRAME(4 + packet->data.eqebb_set_param.eq.num_levels);
                    COPY_FRAME(4);
                    memcpy(&mdr_frame_payload(frame)[4],
                           packet->data.eqebb_set_param.eq.levels,
                           packet->data.eqebb_set_param.eq.num_levels);
                    return frame;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_eqebb_param_ebb_t));
            }

        case MDR_PACKET_EQEBB_NTFY_PARAM:
            switch (packet->data.eqebb_set_param.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    ALLOC_FRAME(4 + packet->data.eqebb_ntfy_param.eq.num_levels);
                    COPY_FRAME(4);
                    memcpy(&mdr_frame_payload(frame)[4],
                           packet->data.eqebb_ntfy_param.eq.levels,
                           packet->data.eqebb_ntfy_param.eq.num_levels);
                    return frame;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_eqebb_param_ebb_t));
            }

        case MDR_PACKET_NCASM_GET_PARAM:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_ncasm_get_param_t));

        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
            switch (packet->data.ncasm_ret_param.inquired_type)
            {
                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_ncasm_param_noise_cancelling_t));

                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_ncasm_param_noise_cancelling_asm_t));

                case MDR_PACKET_NCASM_INQUIRED_TYPE_ASM:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_ncasm_param_asm_t));
            }

        case MDR_PACKET_PLAY_GET_PARAM:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_play_get_param_t));

        case MDR_PACKET_PLAY_RET_PARAM:
            switch (packet->data.play_ret_param.detailed_data_type)
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                    ALLOC_FRAME(5 + packet->data.play_ret_param.string.len);
                    COPY_FRAME(5);
                    memcpy(&mdr_frame_payload(frame)[5],
                           packet->data.play_ret_param.string.data,
                           packet->data.play_ret_param.string.len);
                    return frame;

                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    RETURN_FIXED_LENGTH_FRAME(4);
            }

        case MDR_PACKET_PLAY_SET_PARAM:
            switch (packet->data.play_set_param.detailed_data_type)
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                    ALLOC_FRAME(5 + packet->data.play_set_param.string.len);
                    COPY_FRAME(5);
                    memcpy(&mdr_frame_payload(frame)[5],
                           packet->data.play_set_param.string.data,
                           packet->data.play_set_param.string.len);
                    return frame;

                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    RETURN_FIXED_LENGTH_FRAME(4);
            }

        case MDR_PACKET_PLAY_NTFY_PARAM:
            switch (packet->data.play_ntfy_param.detailed_data_type)
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                    ALLOC_FRAME(5 + packet->data.play_ntfy_param.string.len);
                    COPY_FRAME(5);
                    memcpy(&mdr_frame_payload(frame)[5],
                           packet->data.play_ntfy_param.string.data,
                           packet->data.play_ntfy_param.string.len);
                    return frame;

                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    RETURN_FIXED_LENGTH_FRAME(4);
            }

        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_system_get_capability_t));

        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
            switch (packet->data.system_ret_capability.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_capability_vibrator_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_capability_power_saving_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_capability_control_by_wearing_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    ALLOC_FRAME(3 + packet->data.system_ret_capability.auto_power_off.element_id_count);
                    COPY_FRAME(3);
                    memcpy(&mdr_frame_payload(frame)[3],
                           packet->data.system_ret_capability.auto_power_off.element_ids,
                           packet->data.system_ret_capability.auto_power_off.element_id_count);
                    return frame;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_capability_smart_talking_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                {
                    mdr_packet_system_capability_assignable_settings_t*
                        assignable_settings = &packet->data.system_ret_capability.assignable_settings;

                    size_t frame_size = 3;

                    frame_size += assignable_settings->num_capability_keys
                        * (sizeof(mdr_packet_system_assignable_settings_capability_key_t)
                               - sizeof(mdr_packet_system_assignable_settings_capability_preset_t*));

                    for (int i = 0;
                         i < assignable_settings->num_capability_keys;
                         i++)
                    {
                        frame_size += assignable_settings->capability_keys[i].num_capability_presets
                            * (sizeof(mdr_packet_system_assignable_settings_capability_preset_t)
                                   - sizeof(mdr_packet_system_assignable_settings_capability_caption_t*));

                        for (int j = 0;
                             j < assignable_settings->capability_keys[i].num_capability_presets;
                             j++)
                        {
                            frame_size += assignable_settings->capability_keys[i].capability_presets[j].num_capability_actions
                                * sizeof(mdr_packet_system_assignable_settings_capability_caption_t);
                        }
                    }

                    ALLOC_FRAME(frame_size);

                    COPY_FRAME(3);
                    int offset = 3;
                    for (int i = 0;
                         i < assignable_settings->num_capability_keys;
                         i++)
                    {
                        memcpy(&mdr_frame_payload(frame)[offset],
                               &assignable_settings->capability_keys[i],
                               sizeof(mdr_packet_system_assignable_settings_capability_key_t)
                                   - sizeof(mdr_packet_system_assignable_settings_capability_preset_t*));

                        offset += sizeof(mdr_packet_system_assignable_settings_capability_key_t)
                            - sizeof(mdr_packet_system_assignable_settings_capability_preset_t*);

                        for (int j = 0;
                             j < assignable_settings->capability_keys[i].num_capability_presets;
                             j++)
                        {
                            memcpy(&mdr_frame_payload(frame)[offset],
                                   &assignable_settings->capability_keys[i].capability_presets[j],
                                   sizeof(mdr_packet_system_assignable_settings_capability_preset_t)
                                       - sizeof(mdr_packet_system_assignable_settings_capability_caption_t*));

                            offset += sizeof(mdr_packet_system_assignable_settings_capability_preset_t)
                                - sizeof(mdr_packet_system_assignable_settings_capability_caption_t*);

                            for (int k = 0;
                                 k < assignable_settings->capability_keys[i].capability_presets[j].num_capability_actions;
                                 k++)
                            {
                                memcpy(&mdr_frame_payload(frame)[offset],
                                       &assignable_settings->capability_keys[i].capability_presets[j].capability_actions[k],
                                       sizeof(mdr_packet_system_assignable_settings_capability_caption_t));

                                offset += sizeof(mdr_packet_system_assignable_settings_capability_caption_t);
                            }
                        }
                    }

                    return frame;
                }
            }

        case MDR_PACKET_SYSTEM_GET_PARAM:
            RETURN_FIXED_LENGTH_FRAME(1
                    + sizeof(mdr_packet_system_get_param_t));

        case MDR_PACKET_SYSTEM_RET_PARAM:
            switch (packet->data.system_ret_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_vibrator_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_power_saving_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_control_by_wearing_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_auto_power_off_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_smart_talking_mode_t));


                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    ALLOC_FRAME(3 + packet->data.system_ret_param
                            .assignable_settings.num_presets);
                    COPY_FRAME(3);
                    memcpy(&mdr_frame_payload(frame)[3],
                           &packet->data.system_ret_param
                                   .assignable_settings.num_presets,
                           packet->data.system_ret_param
                                   .assignable_settings.num_presets);
                    return frame;
            }

        case MDR_PACKET_SYSTEM_SET_PARAM:
            switch (packet->data.system_set_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_vibrator_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_power_saving_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_control_by_wearing_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_auto_power_off_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_smart_talking_mode_t));


                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    ALLOC_FRAME(3 + packet->data.system_set_param
                            .assignable_settings.num_presets);
                    COPY_FRAME(3);
                    memcpy(&mdr_frame_payload(frame)[3],
                           &packet->data.system_set_param
                                   .assignable_settings.num_presets,
                           packet->data.system_set_param
                                   .assignable_settings.num_presets);
                    return frame;
            }

        case MDR_PACKET_SYSTEM_NTFY_PARAM:
            switch (packet->data.system_ntfy_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_vibrator_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_power_saving_mode_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_control_by_wearing_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_auto_power_off_t));

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    RETURN_FIXED_LENGTH_FRAME(2
                            + sizeof(mdr_packet_system_param_smart_talking_mode_t));


                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    ALLOC_FRAME(3 + packet->data.system_ntfy_param
                            .assignable_settings.num_presets);
                    COPY_FRAME(3);
                    memcpy(&mdr_frame_payload(frame)[3],
                           &packet->data.system_ntfy_param
                                   .assignable_settings.num_presets,
                           packet->data.system_ntfy_param
                                   .assignable_settings.num_presets);
                    return frame;
            }
    }

    return NULL;
}
