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

#ifndef __MDR_PACKET_PLAY_H__
#define __MDR_PACKET_PLAY_H__

#include <stdint.h>

// Fields

typedef enum
{
    MDR_PACKET_PLAY_INQUIRED_TYPE_PLAYBACK_CONTROLLER = 0x01,
}
mdr_packet_play_inquired_type_t;

typedef enum
{
    MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME  = 0x00,
    MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME  = 0x01,
    MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME = 0x02,
    MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME  = 0x03,
    MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME = 0x10,
    MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME      = 0x20,
}
mdr_packet_play_playback_detailed_data_type_t;

typedef enum
{
    MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_UNSETTLED = 0x00,
    MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_NOTHING   = 0x01,
    MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_SETTLED   = 0x02,
}
mdr_packet_play_playback_name_status_t;

typedef struct
{
    mdr_packet_play_playback_name_status_t status;
    uint8_t                                len;
    uint8_t*                               data;

}
mdr_packet_play_playback_string_t;

// Packet payloads

typedef struct
{
    mdr_packet_play_inquired_type_t               inquired_type;
    mdr_packet_play_playback_detailed_data_type_t detailed_data_type;
}
mdr_packet_play_get_param_t;

typedef struct
{
    mdr_packet_play_inquired_type_t               inquired_type;
    mdr_packet_play_playback_detailed_data_type_t detailed_data_type;

    union
    {
        mdr_packet_play_playback_string_t string;
        uint8_t                           volume;
    };
}
mdr_packet_play_ret_param_t;

typedef mdr_packet_play_ret_param_t mdr_packet_play_set_param_t;
typedef mdr_packet_play_ret_param_t mdr_packet_play_ntfy_param_t;

#endif /* __MDR_PACKET_PLAY_H__ */

