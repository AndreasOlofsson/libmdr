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

#ifndef __MDR_PACKET_UTIL_H__
#define __MDR_PACKET_UTIL_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <mdr/frame.h>
#include <mdr/packet.h>

#define FIELD(field) \
    packet->data.PACKET_FIELD.field

#define min(a, b) ((a) < (b) ? (a) : (b))

#define FREE_FIELD(field) \
    free(packet->data.PACKET_FIELD.field);

#define FREE_FOR_EACH_IN_VALUE(count, field, field_type, field_name) \
    if ((field) != NULL) \
    { \
        for (field_type* field_name = (field); \
                field_name != &(field)[count]; \
                field_name++) \
        { \

#define FREE_FOR_EACH_IN_PACKET(count_field, field, field_type, field_name) \
    FREE_FOR_EACH_IN_VALUE(packet->data.PACKET_FIELD.count_field, \
                           packet->data.PACKET_FIELD.field, \
                           field_type, \
                           field_name)

#define FREE_FOR_EACH_END \
        } \
    }

#define PARSE_INIT(frame) \
    uint8_t* payload = mdr_frame_payload(frame); \
    uint32_t payload_length = frame->payload_length; \
    if (payload_length == 0) \
    { \
        errno = MDR_E_INVALID_FRAME; \
        return NULL; \
    } \
    uint32_t offset = 1; \
    mdr_packet_t* packet = malloc(sizeof(mdr_packet_t)); \
    if (packet == NULL) \
    { \
        return NULL; \
    } \
    memset(packet, 0, sizeof(mdr_packet_t)); \
    packet->type = payload[0];

#ifdef __DEBUG
#define INVALID_FRAME \
    { \
        if (packet != NULL) \
        { \
            mdr_packet_free(packet); \
        } \
        printf("Parse error at " __FILE__ ":%d\n", __LINE__); \
        errno = MDR_E_INVALID_FRAME; \
        return NULL; \
    }
#else
#define INVALID_FRAME \
    { \
        if (packet != NULL) \
        { \
            mdr_packet_free(packet); \
        } \
        errno = MDR_E_INVALID_FRAME; \
        return NULL; \
    }
#endif

#define PARSE_BYTE_INTO_VALUE(value) \
    if (offset >= payload_length) \
    { \
        mdr_packet_free(packet); \
        INVALID_FRAME; \
    } \
    value = payload[offset]; \
    offset++;

#define PARSE_BYTE_INTO_PACKET(field) \
    PARSE_BYTE_INTO_VALUE(FIELD(field))

#define PARSE_BYTES_INTO_VALUE(value, length) \
    if (offset + (length) > payload_length) \
    { \
        mdr_packet_free(packet); \
        INVALID_FRAME; \
    } \
    (value) = malloc(length); \
    if ((value) == NULL) \
    { \
        mdr_packet_free(packet); \
        return NULL; \
    } \
    memcpy((value), &payload[offset], length); \
    offset += length;

#define PARSE_BYTES_INTO_PACKET(field, length) \
    PARSE_BYTES_INTO_VALUE(FIELD(field), length)

#define PARSE_ENUM_INTO_PACKET(field) \
    PARSE_BYTE_INTO_PACKET(field) \
    switch (packet->data.PACKET_FIELD.field) \
    { \

#define PARSE_ENUM_INTO_VALUE(value) \
    PARSE_BYTE_INTO_VALUE(value) \
    switch (value) \
    { \

#define PARSE_ENUM_FIELD(name) \
        case name: break; \

#define PARSE_ENUM_END \
        default: \
            mdr_packet_free(packet); \
            INVALID_FRAME \
    }

#define PARSE_ALLOC_VALUE(value, size) \
    (value) = malloc(size); \
    if ((value) == NULL) \
    { \
        mdr_packet_free(packet); \
        return NULL; \
    } \
    memset((value), 0, size);


#define PARSE_ALLOC_FIELD(field, size) \
        PARSE_ALLOC_VALUE(FIELD(field), size)

#define PARSE_FOR_EACH_INTO_VALUE(count, \
                                  value, \
                                  value_type, \
                                  value_name) \
    PARSE_ALLOC_VALUE(value, sizeof(value_type) * (count)); \
    for (value_type* value_name = (value); \
            value_name != &(value)[count]; \
            value_name++) \
    { \

#define PARSE_FOR_EACH_IN_PACKET(count_field, \
                                 field, \
                                 field_type, \
                                 field_name) \
    PARSE_FOR_EACH_INTO_VALUE( \
            packet->data.PACKET_FIELD.count_field, \
            packet->data.PACKET_FIELD.field, \
            field_type, \
            field_name)

#define PARSE_FOR_EACH_END \
    }

#define WRITE_INIT(packet) \
    mdr_frame_t* frame = NULL; \
    uint8_t* payload = NULL; \
    uint32_t offset = 0;

#define WRITE_ALLOC_FRAME(size) \
    frame = malloc(MDR_FRAME_EMPTY_LEN + size); \
    if (frame == NULL) \
    { \
        return NULL; \
    } \
    frame->data_type = MDR_FRAME_DATA_TYPE_DATA_MDR; \
    frame->sequence_id = 0; \
    frame->payload_length = size; \
    payload = mdr_frame_payload(frame);

#define WRITE_START(size) \
    frame = malloc(MDR_FRAME_EMPTY_LEN + 1 + (size)); \
    if (frame == NULL) \
    { \
        return NULL; \
    } \
    frame->data_type = MDR_FRAME_DATA_TYPE_DATA_MDR; \
    frame->sequence_id = 0; \
    frame->payload_length = 1 + (size); \
    payload = mdr_frame_payload(frame); \
    payload[0] = packet->type; \
    offset = 1;

#define WRITE_BYTE(byte) \
    payload[offset] = byte; \
    offset++;

#define WRITE_FIELD(field) \
    WRITE_BYTE(packet->data.PACKET_FIELD.field)

#define WRITE_BYTES(bytes, length) \
    memcpy(&payload[offset], (bytes), (length)); \
    offset += (length);

#define WRITE_FIELD_BYTES(field, length) \
    WRITE_BYTES(FIELD(field), length)

#define WRITE_FOR_EACH_IN_VALUE(count, field, field_type, field_name) \
    for (field_type* field_name = (field); \
            field_name != &(field)[count]; \
            field_name++) \
    { \

#define WRITE_FOR_EACH_IN_PACKET(count_field, field, field_type, field_name) \
    WRITE_FOR_EACH_IN_VALUE(packet->data.PACKET_FIELD.count_field, \
                           packet->data.PACKET_FIELD.field, \
                           field_type, \
                           field_name)

#define WRITE_FOR_EACH_END \
    }

#endif /* __MDR_PACKET_UTIL_H__ */
