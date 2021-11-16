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

#ifndef __MDR_PACKET_H__
#define __MDR_PACKET_H__

#include "mdr/frame.h"

#include <stdlib.h>
#include <stdint.h>

typedef enum
{
    MDR_PACKET_CONNECT_GET_PROTOCOL_INFO        = 0x00,
    MDR_PACKET_CONNECT_RET_PROTOCOL_INFO        = 0x01,

    // MDR_PACKET_CONNECT_GET_CAPABILITY_INFO      = 0x02,
    // MDR_PACKET_CONNECT_RET_CAPABILITY_INFO      = 0x03,

    MDR_PACKET_CONNECT_GET_DEVICE_INFO          = 0x04,
    MDR_PACKET_CONNECT_RET_DEVICE_INFO          = 0x05,

    MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION     = 0x06,
    MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION     = 0x07,

    MDR_PACKET_COMMON_GET_BATTERY_LEVEL         = 0x10,
    MDR_PACKET_COMMON_RET_BATTERY_LEVEL         = 0x11,
    MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL        = 0x13,

    // MDR_PACKET_COMMON_GET_UPSCALING_EFFECT      = 0x14,
    // MDR_PACKET_COMMON_RET_UPSCALING_EFFECT      = 0x15,
    // MDR_PACKET_COMMON_NTFY_UPSCALING_EFFECT     = 0x17,

    // MDR_PACKET_COMMON_GET_AUDIO_CODEC           = 0x18,
    // MDR_PACKET_COMMON_RET_AUDIO_CODEC           = 0x19,
    // MDR_PACKET_COMMON_NTFY_AUDIO_CODEC          = 0x1b,

    // MDR_PACKET_COMMON_GET_BLUETOOTH_DEVICE_INFO = 0x1c,
    // MDR_PACKET_COMMON_RET_BLUETOOTH_DEVICE_INFO = 0x1d,

    MDR_PACKET_COMMON_SET_POWER_OFF             = 0x22,

    MDR_PACKET_COMMON_GET_CONNECTION_STATUS     = 0x24,
    MDR_PACKET_COMMON_RET_CONNECTION_STATUS     = 0x25,
    MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS    = 0x27,

    // MDR_PACKET_COMMON_GET_CONCIERGE_DATA        = 0x28,
    // MDR_PACKET_COMMON_RET_CONCIERGE_DATA        = 0x29,

    // MDR_PACKET_COMMON_SET_LINK_CONTROL          = 0x2e,
    // MDR_PACKET_COMMON_NTFY_LINK_CONTROL         = 0x2f,

    // MDR_PACKET_UPDT_SET_STATUS                  = 0x34,
    // MDR_PACKET_UPDT_NTFY_STATUS                 = 0x35,
    // MDR_PACKET_UPDT_GET_PARAM                   = 0x36,
    // MDR_PACKET_UPDT_RET_PARAM                   = 0x37,

    // MDR_PACKET_VPT_GET_CAPABILITY               = 0x40,
    // MDR_PACKET_VPT_RET_CAPABILITY               = 0x41,
    // MDR_PACKET_VPT_GET_STATUS                   = 0x42,
    // MDR_PACKET_VPT_RET_STATUS                   = 0x43,
    // MDR_PACKET_VPT_NTFY_STATUS                  = 0x45,
    // MDR_PACKET_VPT_GET_PARAM                    = 0x46,
    // MDR_PACKET_VPT_RET_PARAM                    = 0x47,
    // MDR_PACKET_VPT_SET_PARAM                    = 0x48,
    // MDR_PACKET_VPT_NTFY_PARAM                   = 0x49,

    MDR_PACKET_EQEBB_GET_CAPABILITY             = 0x50,
    MDR_PACKET_EQEBB_RET_CAPABILITY             = 0x51,
    // MDR_PACKET_EQEBB_GET_STATUS                 = 0x52,
    // MDR_PACKET_EQEBB_RET_STATUS                 = 0x53,
    // MDR_PACKET_EQEBB_NTFY_STATUS                = 0x55,
    MDR_PACKET_EQEBB_GET_PARAM                  = 0x56,
    MDR_PACKET_EQEBB_RET_PARAM                  = 0x57,
    MDR_PACKET_EQEBB_SET_PARAM                  = 0x58,
    MDR_PACKET_EQEBB_NTFY_PARAM                 = 0x59,
    // MDR_PACKET_EQEBB_GET_EXTENDED_INFO          = 0x5a,
    // MDR_PACKET_EQEBB_RET_EXTENDED_INFO          = 0x5b,

    // MDR_PACKET_NCASM_GET_CAPABILITY             = 0x60,
    // MDR_PACKET_NCASM_RET_CAPABILITY             = 0x61,
    // MDR_PACKET_NCASM_GET_STATUS                 = 0x62,
    // MDR_PACKET_NCASM_RET_STATUS                 = 0x63,
    // MDR_PACKET_NCASM_NTFY_STATUS                = 0x65,
    MDR_PACKET_NCASM_GET_PARAM                  = 0x66,
    MDR_PACKET_NCASM_RET_PARAM                  = 0x67,
    MDR_PACKET_NCASM_SET_PARAM                  = 0x68,
    MDR_PACKET_NCASM_NTFY_PARAM                 = 0x69,

    // MDR_PACKET_SENSE_GET_CAPABILITY             = 0x70,
    // MDR_PACKET_SENSE_RET_CAPABILITY             = 0x71,
    // MDR_PACKET_SENSE_SET_STATUS                 = 0x74,

    // MDR_PACKET_OPT_GET_CAPABILITY               = 0x80,
    // MDR_PACKET_OPT_RET_CAPABILITY               = 0x81,
    // MDR_PACKET_OPT_GET_STATUS                   = 0x82,
    // MDR_PACKET_OPT_RET_STATUS                   = 0x83,
    // MDR_PACKET_OPT_SET_STATUS                   = 0x84,
    // MDR_PACKET_OPT_NTFY_STATUS                  = 0x85,
    // MDR_PACKET_OPT_GET_PARAM                    = 0x86,
    // MDR_PACKET_OPT_RET_PARAM                    = 0x87,
    // MDR_PACKET_OPT_NTFY_PARAM                   = 0x89,

    // MDR_PACKET_ALERT_GET_CAPABILITY             = 0x90,
    // MDR_PACKET_ALERT_RET_CAPABILITY             = 0x91,
    // MDR_PACKET_ALERT_SET_STATUS                 = 0x94,
    // MDR_PACKET_ALERT_SET_PARAM                  = 0x98,
    // MDR_PACKET_ALERT_NTFY_PARAM                 = 0x99,

    // MDR_PACKET_PLAY_GET_CAPABILITY              = 0xa0,
    // MDR_PACKET_PLAY_RET_CAPABILITY              = 0xa1,
    // MDR_PACKET_PLAY_GET_STATUS                  = 0xa2,
    // MDR_PACKET_PLAY_RET_STATUS                  = 0xa3,
    // MDR_PACKET_PLAY_SET_STATUS                  = 0xa4,
    // MDR_PACKET_PLAY_NTFY_STATUS                 = 0xa5,
    MDR_PACKET_PLAY_GET_PARAM                   = 0xa6,
    MDR_PACKET_PLAY_RET_PARAM                   = 0xa7,
    MDR_PACKET_PLAY_SET_PARAM                   = 0xa8,
    MDR_PACKET_PLAY_NTFY_PARAM                  = 0xa9,

    // MDR_PACKET_SPORTS_GET_CAPABILITY            = 0xb0,
    // MDR_PACKET_SPORTS_RET_CAPABILITY            = 0xb1,
    // MDR_PACKET_SPORTS_GET_STATUS                = 0xb2,
    // MDR_PACKET_SPORTS_RET_STATUS                = 0xb3,
    // MDR_PACKET_SPORTS_NTFY_STATUS               = 0xb5,
    // MDR_PACKET_SPORTS_GET_PARAM                 = 0xb6,
    // MDR_PACKET_SPORTS_RET_PARAM                 = 0xb7,
    // MDR_PACKET_SPORTS_SET_PARAM                 = 0xb8,
    // MDR_PACKET_SPORTS_NTFY_PARAM                = 0xb9,
    // MDR_PACKET_SPORTS_GET_EXTENDED_PARAM        = 0xba,
    // MDR_PACKET_SPORTS_RET_EXTENDED_PARAM        = 0xbb,
    // MDR_PACKET_SPORTS_SET_EXTENDED_PARAM        = 0xbc,
    // MDR_PACKET_SPORTS_NTFY_EXTENDED_PARAM       = 0xbd,

    // MDR_PACKET_LOG_SET_STATUS                   = 0xc4,
    // MDR_PACKET_LOG_NTFY_PARAM                   = 0xc9,

    // MDR_PACKET_GENERAL_SETTING_GET_CAPABILITY   = 0xd0,
    // MDR_PACKET_GENERAL_SETTING_RET_CAPABILITY   = 0xd1,
    // MDR_PACKET_GENERAL_SETTING_GET_STATUS       = 0xd2,
    // MDR_PACKET_GENERAL_SETTING_RET_STATUS       = 0xd3,
    // MDR_PACKET_GENERAL_SETTING_NTFY_STATUS      = 0xd5,
    // MDR_PACKET_GENERAL_SETTING_GET_PARAM        = 0xd6,
    // MDR_PACKET_GENERAL_SETTING_RET_PARAM        = 0xd7,
    // MDR_PACKET_GENERAL_SETTING_SET_PARAM        = 0xd8,
    // MDR_PACKET_GENERAL_SETTING_NTNY_PARAM       = 0xd9,

    // MDR_PACKET_AUDIO_GET_CAPABILITY             = 0xe0,
    // MDR_PACKET_AUDIO_RET_CAPABILITY             = 0xe1,
    // MDR_PACKET_AUDIO_GET_STATUS                 = 0xe2,
    // MDR_PACKET_AUDIO_RET_STATUS                 = 0xe3,
    // MDR_PACKET_AUDIO_NTFY_STATUS                = 0xe5,
    // MDR_PACKET_AUDIO_GET_PARAM                  = 0xe6,
    // MDR_PACKET_AUDIO_RET_PARAM                  = 0xe7,
    // MDR_PACKET_AUDIO_SET_PARAM                  = 0xe8,
    // MDR_PACKET_AUDIO_NTFY_PARAM                 = 0xe9,

    MDR_PACKET_SYSTEM_GET_CAPABILITY            = 0xf0,
    MDR_PACKET_SYSTEM_RET_CAPABILITY            = 0xf1,
    // MDR_PACKET_SYSTEM_GET_STATUS                = 0xf2,
    // MDR_PACKET_SYSTEM_RET_STATUS                = 0xf3,
    // MDR_PACKET_SYSTEM_NTFY_STATUS               = 0xf5,
    MDR_PACKET_SYSTEM_GET_PARAM                 = 0xf6,
    MDR_PACKET_SYSTEM_RET_PARAM                 = 0xf7,
    MDR_PACKET_SYSTEM_SET_PARAM                 = 0xf8,
    MDR_PACKET_SYSTEM_NTFY_PARAM                = 0xf9,
    // MDR_PACKET_SYSTEM_GET_EXTENDED_PARAM        = 0xfa,
    // MDR_PACKET_SYSTEM_RET_EXTENDED_PARAM        = 0xfb,
    // MDR_PACKET_SYSTEM_SET_EXTENDED_PARAM        = 0xfc,
    // MDR_PACKET_SYSTEM_NTFY_EXTENDED_PARAM       = 0xfd,

    // MDR_PACKET_TEST_COMMAND                     = 0xff,
}
mdr_packet_type_t;

#include "mdr/packet/connect_protocol_info.h"
#include "mdr/packet/connect_device_info.h"
#include "mdr/packet/connect_support_function.h"
#include "mdr/packet/common_battery_level.h"
#include "mdr/packet/common_power_off.h"
#include "mdr/packet/common_connection_status.h"
#include "mdr/packet/eqebb.h"
#include "mdr/packet/ncasm.h"
#include "mdr/packet/play.h"
#include "mdr/packet/system.h"

typedef union PACKED
{
    // Connect
    mdr_packet_connect_get_protocol_info_t connect_get_protocol_info;
    mdr_packet_connect_ret_protocol_info_t connect_ret_protocol_info;

    mdr_packet_connect_get_device_info_t connect_get_device_info;
    mdr_packet_connect_ret_device_info_t connect_ret_device_info;

    mdr_packet_connect_get_support_function_t connect_get_support_function;
    mdr_packet_connect_ret_support_function_t connect_ret_support_function;

    // Common
    mdr_packet_common_get_battery_level_t  common_get_battery_level;
    mdr_packet_common_ret_battery_level_t  common_ret_battery_level;
    mdr_packet_common_ntfy_battery_level_t common_ntfy_battery_level;

    mdr_packet_common_set_power_off_t common_set_power_off;

    mdr_packet_common_get_connection_status_t  common_get_connection_status;
    mdr_packet_common_ret_connection_status_t  common_ret_connection_status;
    mdr_packet_common_ntfy_connection_status_t common_ntfy_connection_status;

    // EQEBB
    mdr_packet_eqebb_get_capability_t eqebb_get_capability;
    mdr_packet_eqebb_ret_capability_t eqebb_ret_capability;

    mdr_packet_eqebb_get_param_t eqebb_get_param;
    mdr_packet_eqebb_ret_param_t eqebb_ret_param;
    mdr_packet_eqebb_set_param_t eqebb_set_param;
    mdr_packet_eqebb_ntfy_param_t eqebb_ntfy_param;

    // Noise Cancelling - Ambient Sound Mode (NCASM)
    mdr_packet_ncasm_get_param_t ncasm_get_param;
    mdr_packet_ncasm_ret_param_t ncasm_ret_param;
    mdr_packet_ncasm_set_param_t ncasm_set_param;
    mdr_packet_ncasm_ntfy_param_t ncasm_ntfy_param;

    // Play
    mdr_packet_play_get_param_t play_get_param;
    mdr_packet_play_ret_param_t play_ret_param;
    mdr_packet_play_set_param_t play_set_param;
    mdr_packet_play_ntfy_param_t play_ntfy_param;

    // System
    mdr_packet_system_get_capability_t system_get_capability;
    mdr_packet_system_ret_capability_t system_ret_capability;

    mdr_packet_system_get_param_t system_get_param;
    mdr_packet_system_ret_param_t system_ret_param;
    mdr_packet_system_set_param_t system_set_param;
    mdr_packet_system_ntfy_param_t system_ntfy_param;
}
mdr_packet_data_t;

typedef struct
{
    mdr_packet_type_t type;
    mdr_packet_data_t data;
}
mdr_packet_t;

void mdr_packet_free(mdr_packet_t*);

/*
 * Read an MDR packet from the given frame.
 *
 * returns NULL if the frame does not contain a valid MDR packet.
 */
mdr_packet_t* mdr_packet_from_frame(mdr_frame_t*);

/*
 * Encode an MDR packet into a frame.
 *
 * May return NULL with errno set to ENOMEM.
 */
mdr_frame_t* mdr_packet_to_frame(mdr_packet_t*);

#endif /* __MDR_PACKET_H__ */
