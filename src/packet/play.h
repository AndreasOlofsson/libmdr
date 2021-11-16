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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libmdr. If not, see <https://www.gnu.org/licenses/>.
 */

#include <errno.h>

#include "./util.h"

#include <mdr/errors.h>

static void mdr_packet_play_free(mdr_packet_t* packet)
{
    switch (packet->type)
    {
        case MDR_PACKET_PLAY_GET_PARAM:
            break;

        case MDR_PACKET_PLAY_RET_PARAM:
        case MDR_PACKET_PLAY_SET_PARAM:
        case MDR_PACKET_PLAY_NTFY_PARAM:
#define PACKET_FIELD play_ret_param

            switch (FIELD(detailed_data_type))
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                    FREE_FIELD(string.data)

                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    break;
            }

            break;

#undef PACKET_FIELD

        default:
            break;
    }
}

static mdr_packet_t* mdr_packet_play_from_frame(mdr_frame_t* frame)
{
    PARSE_INIT(frame)

    switch (mdr_frame_payload(frame)[0])
    {
        case MDR_PACKET_PLAY_GET_PARAM:
#define PACKET_FIELD play_get_param

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_INQUIRED_TYPE_PLAYBACK_CONTROLLER)
            PARSE_ENUM_END

            PARSE_ENUM_INTO_PACKET(detailed_data_type)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME)
            PARSE_ENUM_END

            break;

#undef PACKET_FIELD

        case MDR_PACKET_PLAY_RET_PARAM:
        case MDR_PACKET_PLAY_SET_PARAM:
        case MDR_PACKET_PLAY_NTFY_PARAM:
#define PACKET_FIELD play_ret_param

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_INQUIRED_TYPE_PLAYBACK_CONTROLLER)
            PARSE_ENUM_END

            PARSE_ENUM_INTO_PACKET(detailed_data_type)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME)
                PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME)
            PARSE_ENUM_END

            switch (FIELD(detailed_data_type))
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                    PARSE_ENUM_INTO_PACKET(string.status)
                        PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_UNSETTLED)
                        PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_NOTHING)
                        PARSE_ENUM_FIELD(MDR_PACKET_PLAY_PLAYBACK_NAME_STATUS_SETTLED)
                    PARSE_ENUM_END

                    PARSE_BYTE_INTO_PACKET(string.len)

                    if (FIELD(string.len) > 128)
                    {
                        FIELD(string.len) = 128;
                    }
                    
                    PARSE_BYTES_INTO_PACKET(string.data, FIELD(string.len))

                    break;

                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    PARSE_BYTE_INTO_PACKET(volume)

                    break;
            }

            break;

#undef PACKET_FIELD

        default:
            INVALID_FRAME
    }

    return packet;
}

static mdr_frame_t* mdr_packet_play_to_frame(mdr_packet_t* packet)
{
    WRITE_INIT(packet)

    switch (packet->type)
    {
        case MDR_PACKET_PLAY_GET_PARAM:
#define PACKET_FIELD play_get_param

            WRITE_START(2)
            WRITE_FIELD(inquired_type)
            WRITE_FIELD(detailed_data_type)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_PLAY_RET_PARAM:
        case MDR_PACKET_PLAY_SET_PARAM:
        case MDR_PACKET_PLAY_NTFY_PARAM:
#define PACKET_FIELD play_ret_param

            switch (FIELD(detailed_data_type))
            {
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_TRACK_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ALBUM_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_ARTIST_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_GENRE_NAME:
                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_PLAYER_NAME:
                    WRITE_START(4 + FIELD(string.len))
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(detailed_data_type)
                    WRITE_FIELD(string.status)
                    WRITE_FIELD(string.len)
                    WRITE_FIELD_BYTES(string.data, FIELD(string.len))

                    break;

                case MDR_PACKET_PLAY_PLAYBACK_DETAILED_DATA_TYPE_VOLUME:
                    WRITE_START(3)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(detailed_data_type)
                    WRITE_FIELD(volume)

                    break;
            }

            break;

#undef PACKET_FIELD

        default:
            errno = MDR_E_INVALID_PACKET;
            return NULL;
    }

    return frame;
}

