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

#ifndef __MDR_PACKET_COMMON_BATTERY_LEVEL_H__
#define __MDR_PACKET_COMMON_BATTERY_LEVEL_H__

#include <stdint.h>
#define PACKED __attribute__((__packed__))

// Fields

typedef enum PACKED mdr_packet_battery_inquired_type
{
    MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY            = 0x00,
    MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY = 0x01,
    MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY     = 0x02,
}
mdr_packet_battery_inquired_type_t;

typedef struct PACKED mdr_packet_battery_status
{
    uint8_t level;
    uint8_t charging;
}
mdr_packet_battery_status_t;

typedef struct PACKED mdr_packet_battery_status_left_right
{
    mdr_packet_battery_status_t left;
    mdr_packet_battery_status_t right;
}
mdr_packet_battery_status_left_right_t;

typedef union PACKED mdr_packet_battery_level
{
    mdr_packet_battery_status_t battery;

    mdr_packet_battery_status_left_right_t left_right_battery;

    mdr_packet_battery_status_t cradle_battery;
}
mdr_packet_battery_level_t;

// Packet payloads

typedef struct PACKED mdr_packet_common_get_battery_level
{
    mdr_packet_battery_inquired_type_t inquired_type;
}
mdr_packet_common_get_battery_level_t;

typedef struct PACKED mdr_packet_common_ret_battery_level
{
    mdr_packet_battery_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_battery_status_t battery;

        mdr_packet_battery_status_left_right_t left_right_battery;

        mdr_packet_battery_status_t cradle_battery;
    };
}
mdr_packet_common_ret_battery_level_t;

typedef struct PACKED mdr_packet_common_ntfy_battery_level
{
    mdr_packet_battery_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_battery_status_t battery;

        mdr_packet_battery_status_left_right_t left_right_battery;

        mdr_packet_battery_status_t cradle_battery;
    };
}
mdr_packet_common_ntfy_battery_level_t;

#endif /* __MDR_PACKET_COMMON_BATTERY_LEVEL_H__ */
