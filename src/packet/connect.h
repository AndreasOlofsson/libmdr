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

#include <errno.h>

#include "./util.h"

#include <mdr/errors.h>

static void mdr_packet_connect_free(mdr_packet_t* packet)
{
    switch (packet->type)
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
            break;

        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
            break;

        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
#define PACKET_FIELD connect_ret_device_info

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME:
                    FREE_FIELD(model_name.string)

                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION:
                    FREE_FIELD(fw_version.string)

                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR:
                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE:
                    FREE_FIELD(instruction_guide.guidance_categories)

                    break;
            }

            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
            break;

        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
#define PACKET_FIELD connect_ret_support_function

            FREE_FIELD(function_types)

            break;

#undef PACKET_FIELD

        default:
            break;
    }
}

static mdr_packet_t* mdr_packet_connect_from_frame(mdr_frame_t* frame)
{
    PARSE_INIT(frame)

    switch (mdr_frame_payload(frame)[0])
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
#define PACKET_FIELD connect_get_protocol_info

            PARSE_ENUM_INTO_PACKET(fixed_value)
                PARSE_ENUM_FIELD(0)
            PARSE_ENUM_END

            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
#define PACKET_FIELD connect_ret_protocol_info

            PARSE_ENUM_INTO_PACKET(fixed_value)
                PARSE_ENUM_FIELD(0)
            PARSE_ENUM_END
            PARSE_BYTE_INTO_PACKET(version_high)
            PARSE_BYTE_INTO_PACKET(version_low)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
#define PACKET_FIELD connect_get_device_info

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION)
                PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR)
                PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE)
            PARSE_ENUM_END
            
            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
#define PACKET_FIELD connect_ret_device_info

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION)
                PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR)
                PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE)
            PARSE_ENUM_END

            switch (packet->data.connect_ret_device_info.inquired_type)
            {
                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME:
                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION:
                    PARSE_BYTE_INTO_PACKET(model_name.len)
                    PARSE_ALLOC_FIELD(model_name.string,
                                      min(128, FIELD(model_name.len)))

                    PARSE_BYTES_INTO_PACKET(model_name.string,
                                            min(128, FIELD(model_name.len)))

                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR:
                    PARSE_ENUM_INTO_PACKET(series_and_color.series)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_SERIES_NO_SERIES)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_SERIES_EXTRA_BASS)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_SERIES_HEAR)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_SERIES_PREMIUM)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_SERIES_SPORTS)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_SERIES_CASUAL)
                    PARSE_ENUM_END

                    PARSE_ENUM_INTO_PACKET(series_and_color.color)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_DEFAULT)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_BLACK)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_WHITE)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_SILVER)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_RED)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_BLUE)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_PINK)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_YELLOW)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_GREEN)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_GRAY)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_GOLD)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_CREAM)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_ORANGE)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_BROWN)
                        PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_MODEL_COLOR_VIOLET)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE:
                    PARSE_BYTE_INTO_PACKET(instruction_guide.num_guidance_categories)

                    PARSE_FOR_EACH_IN_PACKET(
                            instruction_guide.num_guidance_categories,
                            instruction_guide.guidance_categories,
                            mdr_packet_device_info_guidance_category_t,
                            guidance_category)

                        PARSE_ENUM_INTO_VALUE(*guidance_category)
                            PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_CHANGE_EARPIECE)
                            PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_WEAR_EARPHONE)
                            PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_PLAY_BUTTON_OPERATION)
                            PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_TOUCH_PAD_OPERATION)
                            PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_MAIN_BODY_OPERATION)
                            PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_QUICK_ATTENTION)
                            PARSE_ENUM_FIELD(MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_ASSIGNABLE_BUTTON_SETTINGS)
                        PARSE_ENUM_END

                    PARSE_FOR_EACH_END

                    break;
            }
            
            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
#define PACKET_FIELD connect_get_support_function

            PARSE_ENUM_INTO_PACKET(fixed_value)
                PARSE_ENUM_FIELD(0)
            PARSE_ENUM_END

            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
#define PACKET_FIELD connect_ret_support_function

            PARSE_ENUM_INTO_PACKET(fixed_value)
                PARSE_ENUM_FIELD(0)
            PARSE_ENUM_END
            
            PARSE_BYTE_INTO_PACKET(num_function_types)

            PARSE_FOR_EACH_IN_PACKET(num_function_types,
                                     function_types,
                                     mdr_packet_support_function_type_t,
                                     function_type)

                PARSE_ENUM_INTO_VALUE(*function_type)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_NO_USE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_BATTERY_LEVEL)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_UPSCALING_INDICATOR)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_CODEC_INDICATOR)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_BLE_SETUP)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_LEFT_RIGHT_BATTERY_LEVEL)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_LEFT_RIGHT_CONNECTION_STATUS)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_CRADLE_BATTERY_LEVEL)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_POWER_OFF)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_CONCIERGE_DATA)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_TANDEM_KEEP_ALIVE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_FW_UPDATE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_PAIRING_DEVICE_MANAGEMENT_CLASSIC_BT)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_VOICE_GUIDANCE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_VPT)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_SOUND_POSITION)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_PRESET_EQ)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_EBB)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_PRESET_EQ_NONCUSTOMIZABLE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_NOISE_CANCELLING)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_NOISE_CANCELLING_AND_AMBIENT_SOUND_MODE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_AMBIENT_SOUND_MODE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_AUTO_NC_ASM)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_NC_OPTIMIZER)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_VIBRATOR_ALERT_NOTIFICATION)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_PLAYBACK_CONTROLLER)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_TRAINING_MODE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_ACTION_LOG_NOTIFIER)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_GENERAL_SETTING1)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_GENERAL_SETTING2)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_GENERAL_SETTING3)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_CONNECTION_MODE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_UPSCALING)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_VIBRATOR)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_POWER_SAVING_MODE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_CONTROL_BY_WEARING)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_AUTO_POWER_OFF)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_SMART_TALKING_MODE)
                    PARSE_ENUM_FIELD(MDR_PACKET_SUPPORT_FUNCTION_TYPE_ASSIGNABLE_SETTINGS)
                PARSE_ENUM_END

            PARSE_FOR_EACH_END

            break;

#undef PACKET_FIELD

        default:
            INVALID_FRAME
    }

    return packet;
}

static mdr_frame_t* mdr_packet_connect_to_frame(mdr_packet_t* packet)
{
    WRITE_INIT(packet)

    switch (packet->type)
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
#define PACKET_FIELD connect_get_protocol_info

            WRITE_START(1)
            WRITE_FIELD(fixed_value)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
#define PACKET_FIELD connect_ret_protocol_info

            WRITE_START(3)
            WRITE_FIELD(fixed_value)
            WRITE_FIELD(version_high)
            WRITE_FIELD(version_low)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
#define PACKET_FIELD connect_get_device_info

            WRITE_START(1)
            WRITE_FIELD(inquired_type)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
#define PACKET_FIELD connect_ret_device_info

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME:
                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION:
                    WRITE_START(2 + FIELD(model_name.len))
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(model_name.len)
                    WRITE_FIELD_BYTES(model_name.string, FIELD(model_name.len))

                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR:
                    WRITE_START(3)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(series_and_color.series)
                    WRITE_FIELD(series_and_color.color)

                    break;

                case MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE:
                    WRITE_START(2 + FIELD(instruction_guide.num_guidance_categories))
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(instruction_guide.num_guidance_categories)

                    WRITE_FOR_EACH_IN_PACKET(
                            instruction_guide.num_guidance_categories,
                            instruction_guide.guidance_categories,
                            mdr_packet_device_info_guidance_category_t,
                            guidance_category)

                        WRITE_BYTE(*guidance_category)

                    WRITE_FOR_EACH_END

                    break;
            }

            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
#define PACKET_FIELD connect_get_support_function

            WRITE_START(1)
            WRITE_FIELD(fixed_value)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
#define PACKET_FIELD connect_ret_support_function

            WRITE_START(2 + FIELD(num_function_types))
            WRITE_FIELD(fixed_value)
            WRITE_FIELD(num_function_types)
            WRITE_FOR_EACH_IN_PACKET(num_function_types,
                                     function_types,
                                     mdr_packet_support_function_type_t,
                                     function_type)

                WRITE_BYTE(*function_type)

            WRITE_FOR_EACH_END

            break;

#undef PACKET_FIELD

        default:
            errno = MDR_E_INVALID_PACKET;
            return NULL;
    }

    return frame;
}

