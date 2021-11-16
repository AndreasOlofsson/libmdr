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

#ifndef __MDR_PACKET_COMMON_CONNECTION_STATUS_H__
#define __MDR_PACKET_COMMON_CONNECTION_STATUS_H__

#include <stdint.h>

// Fields

typedef enum
{
    MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT = 0x01,
}
mdr_packet_connection_status_inquired_type_t;

typedef enum
{
    MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_NOT_CONNECTED = 0x00,
    MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_CONNECTED = 0x01,
}
mdr_packet_connection_status_connection_status_t;

typedef struct
{
    mdr_packet_connection_status_connection_status_t left_status;
    mdr_packet_connection_status_connection_status_t right_status;
}
mdr_packet_connection_status_left_right_status_t;

// Packet payloads

typedef struct
{
    mdr_packet_connection_status_inquired_type_t inquired_type;
}
mdr_packet_common_get_connection_status_t;

typedef struct
{
    mdr_packet_connection_status_inquired_type_t inquired_type;

    union
    {
        mdr_packet_connection_status_left_right_status_t left_right;
    };
}
mdr_packet_common_ret_connection_status_t;

typedef mdr_packet_common_ret_connection_status_t
        mdr_packet_common_ntfy_connection_status_t;

#endif /* __MDR_PACKET_COMMON_CONNECTION_STATUS_H__ */
