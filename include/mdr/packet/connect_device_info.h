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

#ifndef __MDR_PACKET_CONNECT_DEVICE_INFO_H__
#define __MDR_PACKET_CONNECT_DEVICE_INFO_H__

#include <stdint.h>
#define PACKED __attribute__((__packed__))

// Fields

typedef enum PACKED mdr_packet_device_info_inquired_type
{
    MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_MODEL_NAME        = 0x01,
    MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_FW_VERSION        = 0x02,
    MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_SERIES_AND_COLOR  = 0x03,
    MDR_PACKET_DEVICE_INFO_INQUIRED_TYPE_INSTRUCTION_GUIDE = 0x04,
}
mdr_packet_device_info_inquired_type_t;

typedef enum PACKED mdr_packet_device_info_model_series
{
    MDR_PACKET_DEVICE_INFO_MODEL_SERIES_NO_SERIES  = 0x00,
    MDR_PACKET_DEVICE_INFO_MODEL_SERIES_EXTRA_BASS = 0x10,
    MDR_PACKET_DEVICE_INFO_MODEL_SERIES_HEAR       = 0x20,
    MDR_PACKET_DEVICE_INFO_MODEL_SERIES_PREMIUM    = 0x30,
    MDR_PACKET_DEVICE_INFO_MODEL_SERIES_SPORTS     = 0x40,
    MDR_PACKET_DEVICE_INFO_MODEL_SERIES_CASUAL     = 0x50,
}
mdr_packet_device_info_model_series_t;

typedef enum PACKED mdr_packet_device_info_model_color
{
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_DEFAULT = 0x00,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_BLACK   = 0x01,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_WHITE   = 0x02,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_SILVER  = 0x03,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_RED     = 0x04,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_BLUE    = 0x05,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_PINK    = 0x06,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_YELLOW  = 0x07,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_GREEN   = 0x08,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_GRAY    = 0x09,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_GOLD    = 0x0a,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_CREAM   = 0x0b,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_ORANGE  = 0x0c,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_BROWN   = 0x0d,
    MDR_PACKET_DEVICE_INFO_MODEL_COLOR_VIOLET  = 0x0e,
}
mdr_packet_device_info_model_color_t;

typedef enum PACKED mdr_packet_device_info_guidance_category
{
    MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_CHANGE_EARPIECE            = 0X00,
    MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_WEAR_EARPHONE              = 0X10,
    MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_PLAY_BUTTON_OPERATION      = 0X20,
    MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_TOUCH_PAD_OPERATION        = 0X30,
    MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_MAIN_BODY_OPERATION        = 0X40,
    MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_QUICK_ATTENTION            = 0X50,
    MDR_PACKET_DEVICE_INFO_GUIDANCE_CATEGORY_ASSIGNABLE_BUTTON_SETTINGS = 0X60,
}
mdr_packet_device_info_guidance_category_t;

typedef struct PACKED mdr_packet_device_info_string
{
    uint8_t len;
    uint8_t* string;
}
mdr_packet_device_info_string_t;

typedef struct PACKED mdr_packet_device_info_series_and_color
{
    mdr_packet_device_info_model_series_t series;
    mdr_packet_device_info_model_color_t color;
}
mdr_packet_device_info_series_and_color_t;

typedef struct PACKED mdr_packet_device_info_instruction_guide
{
    uint8_t count;
    mdr_packet_device_info_guidance_category_t* guidance_categories;
}
mdr_packet_device_info_instruction_guide_t;

typedef union PACKED mdr_packet_device_info
{
    mdr_packet_device_info_string_t model_name;

    mdr_packet_device_info_string_t fw_version;

    mdr_packet_device_info_series_and_color_t series_and_color;

    mdr_packet_device_info_instruction_guide_t instruction_guide;
}
mdr_packet_device_info_t;

// Packet payloads

typedef struct PACKED mdr_packet_connect_get_device_info
{
    mdr_packet_device_info_inquired_type_t inquired_type;
}
mdr_packet_connect_get_device_info_t;

typedef struct PACKED mdr_packet_connect_ret_device_info
{
    mdr_packet_device_info_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_device_info_string_t model_name;

        mdr_packet_device_info_string_t fw_version;

        mdr_packet_device_info_series_and_color_t series_and_color;

        mdr_packet_device_info_instruction_guide_t instruction_guide;
    };
}
mdr_packet_connect_ret_device_info_t;

#endif /* __MDR_PACKET_CONNECT_DEVICE_INFO_H__ */
