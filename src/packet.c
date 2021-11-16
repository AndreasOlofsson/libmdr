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

#include "mdr/packet.h"

#include "mdr/errors.h"

#include "packet/connect.h"
#include "packet/common.h"
#include "packet/eqebb.h"
#include "packet/ncasm.h"
#include "packet/play.h"
#include "packet/system.h"

#include <string.h>
#include <errno.h>

void mdr_packet_free(mdr_packet_t* packet)
{
    if (packet == NULL)
        return;

    switch (packet->type)
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
            mdr_packet_connect_free(packet);

            break;

        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_SET_POWER_OFF:
        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
            mdr_packet_common_free(packet);

            break;

        case MDR_PACKET_EQEBB_GET_CAPABILITY:
        case MDR_PACKET_EQEBB_RET_CAPABILITY:
        case MDR_PACKET_EQEBB_GET_PARAM:
        case MDR_PACKET_EQEBB_RET_PARAM:
        case MDR_PACKET_EQEBB_SET_PARAM:
        case MDR_PACKET_EQEBB_NTFY_PARAM:
            mdr_packet_eqebb_free(packet);

            break;

        case MDR_PACKET_NCASM_GET_PARAM:
        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
            mdr_packet_ncasm_free(packet);

            break;

        case MDR_PACKET_PLAY_GET_PARAM:
        case MDR_PACKET_PLAY_RET_PARAM:
        case MDR_PACKET_PLAY_SET_PARAM:
        case MDR_PACKET_PLAY_NTFY_PARAM:
            mdr_packet_play_free(packet);

            break;

        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
        case MDR_PACKET_SYSTEM_GET_PARAM:
        case MDR_PACKET_SYSTEM_RET_PARAM:
        case MDR_PACKET_SYSTEM_SET_PARAM:
        case MDR_PACKET_SYSTEM_NTFY_PARAM:
            mdr_packet_system_free(packet);

            break;
    }

    free(packet);
}

mdr_packet_t* mdr_packet_from_frame(mdr_frame_t* frame)
{
    if (frame->data_type != MDR_FRAME_DATA_TYPE_DATA_MDR)
    {
        errno = MDR_E_INVALID_PACKET;
        return NULL;
    }

    if (frame->payload_length < 1)
    {
        errno = MDR_E_INVALID_FRAME;
        return NULL;
    }

    switch (mdr_frame_payload(frame)[0])
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
            return mdr_packet_connect_from_frame(frame);

        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_SET_POWER_OFF:
        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
            return mdr_packet_common_from_frame(frame);

        case MDR_PACKET_EQEBB_GET_CAPABILITY:
        case MDR_PACKET_EQEBB_RET_CAPABILITY:
        case MDR_PACKET_EQEBB_GET_PARAM:
        case MDR_PACKET_EQEBB_RET_PARAM:
        case MDR_PACKET_EQEBB_SET_PARAM:
        case MDR_PACKET_EQEBB_NTFY_PARAM:
            return mdr_packet_eqebb_from_frame(frame);

        case MDR_PACKET_NCASM_GET_PARAM:
        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
            return mdr_packet_ncasm_from_frame(frame);

        case MDR_PACKET_PLAY_GET_PARAM:
        case MDR_PACKET_PLAY_RET_PARAM:
        case MDR_PACKET_PLAY_SET_PARAM:
        case MDR_PACKET_PLAY_NTFY_PARAM:
            return mdr_packet_play_from_frame(frame);

        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
        case MDR_PACKET_SYSTEM_GET_PARAM:
        case MDR_PACKET_SYSTEM_RET_PARAM:
        case MDR_PACKET_SYSTEM_SET_PARAM:
        case MDR_PACKET_SYSTEM_NTFY_PARAM:
            return mdr_packet_system_from_frame(frame);

        default:
            errno = MDR_E_INVALID_FRAME;
            return NULL;
    }
}

mdr_frame_t* mdr_packet_to_frame(mdr_packet_t* packet)
{
    switch (packet->type)
    {
        case MDR_PACKET_CONNECT_GET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_RET_PROTOCOL_INFO:
        case MDR_PACKET_CONNECT_GET_DEVICE_INFO:
        case MDR_PACKET_CONNECT_RET_DEVICE_INFO:
        case MDR_PACKET_CONNECT_GET_SUPPORT_FUNCTION:
        case MDR_PACKET_CONNECT_RET_SUPPORT_FUNCTION:
            return mdr_packet_connect_to_frame(packet);

        case MDR_PACKET_COMMON_GET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_RET_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_NTFY_BATTERY_LEVEL:
        case MDR_PACKET_COMMON_SET_POWER_OFF:
        case MDR_PACKET_COMMON_GET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_RET_CONNECTION_STATUS:
        case MDR_PACKET_COMMON_NTFY_CONNECTION_STATUS:
            return mdr_packet_common_to_frame(packet);

        case MDR_PACKET_EQEBB_GET_CAPABILITY:
        case MDR_PACKET_EQEBB_RET_CAPABILITY:
        case MDR_PACKET_EQEBB_GET_PARAM:
        case MDR_PACKET_EQEBB_RET_PARAM:
        case MDR_PACKET_EQEBB_SET_PARAM:
        case MDR_PACKET_EQEBB_NTFY_PARAM:
            return mdr_packet_eqebb_to_frame(packet);

        case MDR_PACKET_NCASM_GET_PARAM:
        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
            return mdr_packet_ncasm_to_frame(packet);

        case MDR_PACKET_PLAY_GET_PARAM:
        case MDR_PACKET_PLAY_RET_PARAM:
        case MDR_PACKET_PLAY_SET_PARAM:
        case MDR_PACKET_PLAY_NTFY_PARAM:
            return mdr_packet_play_to_frame(packet);

        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
        case MDR_PACKET_SYSTEM_GET_PARAM:
        case MDR_PACKET_SYSTEM_RET_PARAM:
        case MDR_PACKET_SYSTEM_SET_PARAM:
        case MDR_PACKET_SYSTEM_NTFY_PARAM:
            return mdr_packet_system_to_frame(packet);
    }

    return NULL;
}
