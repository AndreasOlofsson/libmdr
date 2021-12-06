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

static void mdr_packet_common_free(mdr_packet_t* packet)
{
    switch (packet->type)
    {
        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_SET_POWER_OFF:
        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
            break;

        default:
            break;
    }
}

static mdr_packet_t* mdr_packet_common_from_frame(mdr_frame_t* frame)
{
    PARSE_INIT(frame)

    switch (mdr_frame_payload(frame)[0])
    {
        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
#define PACKET_FIELD common_get_battery_level

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY)
                PARSE_ENUM_FIELD(MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY)
                PARSE_ENUM_FIELD(MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY)
            PARSE_ENUM_END

            break;

#undef PACKET_FIELD

        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
#define PACKET_FIELD common_ret_battery_level

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY)
                PARSE_ENUM_FIELD(MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY)
                PARSE_ENUM_FIELD(MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY)
            PARSE_ENUM_END

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY:
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY:
                    PARSE_BYTE_INTO_PACKET(battery.level)
                    PARSE_BYTE_INTO_PACKET(battery.charging)

                    break;

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY:
                    PARSE_BYTE_INTO_PACKET(left_right_battery.left.level)
                    PARSE_BYTE_INTO_PACKET(left_right_battery.left.charging)
                    PARSE_BYTE_INTO_PACKET(left_right_battery.right.level)
                    PARSE_BYTE_INTO_PACKET(left_right_battery.right.charging)

                    break;
            }

            break;

#undef PACKET_FIELD

        case MDR_PACKET_COMMON_SET_POWER_OFF:
#define PACKET_FIELD common_set_power_off

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_COMMON_POWER_OFF_INQUIRED_TYPE_FIXED_VALUE)
            PARSE_ENUM_END

            PARSE_ENUM_INTO_PACKET(setting_value)
                PARSE_ENUM_FIELD(MDR_PACKET_COMMON_POWER_OFF_SETTING_VALUE_USER_POWER_OFF)
            PARSE_ENUM_END

            break;

#undef PACKET_FIELD

        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
#define PACKET_FIELD common_get_connection_status

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT)
            PARSE_ENUM_END

            break;

#undef PACKET_FIELD

        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
#define PACKET_FIELD common_ret_connection_status

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT)
            PARSE_ENUM_END

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT:
                    PARSE_ENUM_INTO_PACKET(left_right.left_status)
                        PARSE_ENUM_FIELD(MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_NOT_CONNECTED)
                        PARSE_ENUM_FIELD(MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_CONNECTED)
                    PARSE_ENUM_END

                    PARSE_ENUM_INTO_PACKET(left_right.right_status)
                        PARSE_ENUM_FIELD(MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_NOT_CONNECTED)
                        PARSE_ENUM_FIELD(MDR_PACKET_CONNECTION_STATUS_CONNECTION_STATUS_CONNECTED)
                    PARSE_ENUM_END

                    break;
            }

            break;

#undef PACKET_FIELD

        default:
            INVALID_FRAME
    }

    return packet;
}

static mdr_frame_t* mdr_packet_common_to_frame(mdr_packet_t* packet)
{
    WRITE_INIT(packet)

    switch (packet->type)
    {
        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
#define PACKET_FIELD common_get_battery_level

            WRITE_START(1)
            WRITE_FIELD(inquired_type)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
#define PACKET_FIELD common_ret_battery_level

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_BATTERY:
                case MDR_PACKET_BATTERY_INQUIRED_TYPE_CRADLE_BATTERY:
                    WRITE_START(3)
                    WRITE_FIELD(battery.level)
                    WRITE_FIELD(battery.charging)

                    break;

                case MDR_PACKET_BATTERY_INQUIRED_TYPE_LEFT_RIGHT_BATTERY:
                    WRITE_START(5)
                    WRITE_FIELD(left_right_battery.left.level)
                    WRITE_FIELD(left_right_battery.left.charging)
                    WRITE_FIELD(left_right_battery.right.level)
                    WRITE_FIELD(left_right_battery.right.charging)

                    break;
            }

            break;

#undef PACKET_FIELD

        case MDR_PACKET_COMMON_SET_POWER_OFF:
#define PACKET_FIELD common_set_power_off

            WRITE_START(2)
            WRITE_FIELD(inquired_type)
            WRITE_FIELD(setting_value)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
#define PACKET_FIELD common_get_connection_status

            WRITE_START(1)
            WRITE_FIELD(inquired_type)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
#define PACKET_FIELD common_ret_connection_status

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_CONNECTION_STATUS_INQUIRED_TYPE_LEFT_RIGHT:
                    WRITE_START(3)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(left_right.left_status)
                    WRITE_FIELD(left_right.right_status)

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

