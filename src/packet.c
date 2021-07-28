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

        case MDR_PACKET_EQEBB_GET_CAPABILITY:
            break;

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
            switch (packet->data.eqebb_ret_capability.inquired_type)
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    for (int i = 0;
                         i < packet->data.eqebb_ret_capability.eq.num_presets;
                         i++)
                    {
                        free(packet->data.eqebb_ret_capability.eq.presets[i]
                                .name);
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

        case MDR_PACKET_NCASM_GET_PARAM:
        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
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

    mdr_packet_t* packet;

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
    }

    return NULL;
}
