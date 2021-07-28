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

#ifndef __MDR_PACKET_SUPPORT_FUNCTION_H__
#define __MDR_PACKET_SUPPORT_FUNCTION_H__

#include <stdint.h>
#include <stdbool.h>

#define PACKED __attribute__((__packed__))

// Fields

typedef enum PACKED mdr_packet_support_function_type
{
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_NO_USE                                  = 0x00,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_BATTERY_LEVEL                           = 0x11,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_UPSCALING_INDICATOR                     = 0x12,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_CODEC_INDICATOR                         = 0x13,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_BLE_SETUP                               = 0x14,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_LEFT_RIGHT_BATTERY_LEVEL                = 0x15,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_LEFT_RIGHT_CONNECTION_STATUS            = 0x17,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_CRADLE_BATTERY_LEVEL                    = 0x18,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_POWER_OFF                               = 0x21,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_CONCIERGE_DATA                          = 0x22,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_TANDEM_KEEP_ALIVE                       = 0x23,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_FW_UPDATE                               = 0x30,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_PAIRING_DEVICE_MANAGEMENT_CLASSIC_BT    = 0x38,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_VOICE_GUIDANCE                          = 0x39,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_VPT                                     = 0x41,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_SOUND_POSITION                          = 0x42,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_PRESET_EQ                               = 0x51,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_EBB                                     = 0x52,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_PRESET_EQ_NONCUSTOMIZABLE               = 0x53,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_NOISE_CANCELLING                        = 0x61,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_NOISE_CANCELLING_AND_AMBIENT_SOUND_MODE = 0x62,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_AMBIENT_SOUND_MODE                      = 0x63,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_AUTO_NC_ASM                             = 0x71,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_NC_OPTIMIZER                            = 0x81,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_VIBRATOR_ALERT_NOTIFICATION             = 0x92,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_PLAYBACK_CONTROLLER                     = 0xa1,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_TRAINING_MODE                           = 0xb1,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_ACTION_LOG_NOTIFIER                     = 0xc1,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_GENERAL_SETTING1                        = 0xd1,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_GENERAL_SETTING2                        = 0xd2,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_GENERAL_SETTING3                        = 0xd3,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_CONNECTION_MODE                         = 0xe1,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_UPSCALING                               = 0xe2,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_VIBRATOR                                = 0xf1,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_POWER_SAVING_MODE                       = 0xf2,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_CONTROL_BY_WEARING                      = 0xf3,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_AUTO_POWER_OFF                          = 0xf4,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_SMART_TALKING_MODE                      = 0xf5,
    MDR_PACKET_SUPPORT_FUNCTION_TYPE_ASSIGNABLE_SETTINGS                     = 0xf6,
}
mdr_packet_support_function_type_t;

// Packet payloads

typedef struct PACKED mdr_packet_connect_get_support_function
{
    uint8_t fixed_value; // fixed 0x00
}
mdr_packet_connect_get_support_function_t;

typedef struct PACKED mdr_packet_connect_ret_support_function
{
    uint8_t fixed_value; // fixed 0x00

    uint8_t num_function_types;
    mdr_packet_support_function_type_t* function_types;
}
mdr_packet_connect_ret_support_function_t;

// Functions

bool mdr_packet_support_function_contains(
        mdr_packet_connect_ret_support_function_t*,
        mdr_packet_support_function_type_t);

#endif /* __MDR_PACKET_SUPPORT_FUNCTION_H__ */
