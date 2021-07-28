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

#ifndef __MDR_FRAME_H__
#define __MDR_FRAME_H__

#include <stdlib.h>
#include <stdint.h>

/*
 * The length of a frame with an empty payload,
 * this is the minimum length a frame can be.
 *
 * This should be used to get the length of a frame as a frame
 * always has length `MDR_FRAME_EMPTY_LEN + frame->payload_length`.
 */
#define MDR_FRAME_EMPTY_LEN 7

typedef enum
{
    MDR_FRAME_DATA_TYPE_DATA              = 0x00,
    MDR_FRAME_DATA_TYPE_ACK               = 0x01,
    MDR_FRAME_DATA_TYPE_DATA_MC_NO_1      = 0x02,
    MDR_FRAME_DATA_TYPE_DATA_ICD          = 0x09,
    MDR_FRAME_DATA_TYPE_DATA_EV           = 0x0a,
    MDR_FRAME_DATA_TYPE_DATA_MDR          = 0x0c,
    MDR_FRAME_DATA_TYPE_DATA_COMMON       = 0x0d,
    MDR_FRAME_DATA_TYPE_DATA_MDR_NO_2     = 0x0e,
    MDR_FRAME_DATA_TYPE_SHOT              = 0x10,
    MDR_FRAME_DATA_TYPE_SHOT_MC_NO_1      = 0x12,
    MDR_FRAME_DATA_TYPE_SHOT_ICD          = 0x19,
    MDR_FRAME_DATA_TYPE_SHOT_EV           = 0x1a,
    MDR_FRAME_DATA_TYPE_SHOT_MDR          = 0x1c,
    MDR_FRAME_DATA_TYPE_SHOT_COMMON       = 0x1d,
    MDR_FRAME_DATA_TYPE_SHOT_MDR_NO_2     = 0x1e,
    MDR_FRAME_DATA_TYPE_LARGE_DATA_COMMON = 0x2d,
}
mdr_frame_data_type_t;

/*
 * An MDR frame.
 *
 * Note:
 * Frames are passed around as pointers where the allocated size
 * may be longer than the size of this struct. The payload element is the first
 * byte of the the payload (or the checksum of payload_length == 0),
 *
 * The allocated size must be equal to FRAME_EMPTY_LEN + payload_length.
 * Use `mdr_frame_payload` and `mdr_frame_checksum` to get pointers to
 * the payload and checksum byte, respectively.
 */
typedef struct __attribute__((__packed__))
{
    mdr_frame_data_type_t data_type : 8;
    uint8_t sequence_id;
    /*
     * The payload length is always passed to and from libmdr in host order
     * even though it's sent in network order.
     */
    uint32_t payload_length;
    uint8_t payload;
    // uint8_t checksum;
}
mdr_frame_t;

mdr_frame_t* mdr_frame_dup(mdr_frame_t*);

/*
 * Gets a pointer to the payload of the frame.
 */
uint8_t* mdr_frame_payload(mdr_frame_t*);

/*
 * Gets a pointer to the checksum byte of the frame.
 *
 * This is the checksum byte stored in the frame,
 * which may be different to the correct checksum of the frame.
 *
 * Note: Requires that payload_length is set correctly as the
 * checksum byte immediately follows the payload.
 */
uint8_t* mdr_frame_checksum(mdr_frame_t*);

/*
 * Calculate what the correct checksum of the frame should be.
 *
 * Use
 * `*mdr_frame_checksum(frame) = mdr_frame_compute_checksum(frame)`
 * to set the checksum of a frame and
 * `*mdr_frame_checksum(frame) == mdr_frame_compute_checksum(frame)`
 * to verify the checksum of a frame.
 */
uint8_t mdr_frame_compute_checksum(mdr_frame_t*);

#endif /* __MDR_FRAME_H__ */
