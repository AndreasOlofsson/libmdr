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

#ifndef __MDR_PACKET_COMMON_POWER_OFF_H__
#define __MDR_PACKET_COMMON_POWER_OFF_H__

#include <stdint.h>
#define PACKED __attribute__((__packed__))

// Fields

typedef enum PACKED mdr_packet_common_power_off_inquired_type
{
    MDR_PACKET_COMMON_POWER_OFF_INQUIRED_TYPE_FIXED_VALUE = 0x00,
}
mdr_packet_common_power_off_inquired_type_t;

typedef enum PACKED mdr_packet_common_power_off_setting_value
{
    MDR_PACKET_COMMON_POWER_OFF_SETTING_VALUE_USER_POWER_OFF = 0x01,
}
mdr_packet_common_power_off_setting_value_t;

// Packet payloads

typedef struct PACKED mdr_packet_common_set_power_off
{
    mdr_packet_common_power_off_inquired_type_t inquired_type;

    mdr_packet_common_power_off_setting_value_t setting_value;
}
mdr_packet_common_set_power_off_t;

#endif /* __MDR_PACKET_COMMON_POWER_OFF */
