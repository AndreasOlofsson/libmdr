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

#include "mdr/frame.h"

#include <stdlib.h>
#include <string.h>

mdr_frame_t* mdr_frame_dup(mdr_frame_t* frame)
{
    mdr_frame_t* new_frame =
        malloc(MDR_FRAME_EMPTY_LEN + frame->payload_length);
    if (new_frame == NULL) return NULL;

    memcpy(new_frame, frame, MDR_FRAME_EMPTY_LEN + frame->payload_length);

    return new_frame;
}

uint8_t* mdr_frame_payload(mdr_frame_t* frame)
{
    return &frame->payload;
}

uint8_t* mdr_frame_checksum(mdr_frame_t* frame)
{
    return &(&frame->payload)[frame->payload_length];
}

uint8_t mdr_frame_compute_checksum(mdr_frame_t* frame)
{
    uint8_t* frame_bytes = (uint8_t*) frame;
    size_t frame_size = MDR_FRAME_EMPTY_LEN + frame->payload_length;

    uint8_t checksum = 0;
    for (size_t i = 0; i < frame_size - 1; i++)
    {
        checksum += frame_bytes[i];
    }

    return checksum;
}
