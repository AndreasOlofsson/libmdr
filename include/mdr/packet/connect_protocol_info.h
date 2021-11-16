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

#ifndef __MDR_PACKET_CONNECT_PROTOCOL_INFO_H__
#define __MDR_PACKET_CONNECT_PROTOCOL_INFO_H__

#include <stdint.h>

// Packet payloads

typedef struct
{
    uint8_t fixed_value; // fixed 0x00
}
mdr_packet_connect_get_protocol_info_t;

typedef struct
{
    uint8_t fixed_value; // fixed 0x00
    uint8_t version_high;
    uint8_t version_low;
}
mdr_packet_connect_ret_protocol_info_t;

#endif /* __MDR_PACKET_CONNECT_PROTOCOL_INFO_H__ */
