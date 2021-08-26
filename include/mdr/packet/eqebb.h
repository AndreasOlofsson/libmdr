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

#ifndef __MDR_PACKET_EQEBB_H__
#define __MDR_PACKET_EQEBB_H__

#include <stdint.h>
#define PACKED __attribute__((__packed__))

// Fields

typedef enum PACKED mdr_packet_eqebb_inquired_type
{
    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ                 = 0x01,
    // Electro Bass Boster
    MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB                       = 0x02,
    MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE = 0x03,
}
mdr_packet_eqebb_inquired_type_t;

typedef enum PACKED mdr_packet_eqebb_display_language
{
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_UNDEFINED_LANGUAGE   = 0x00,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_ENGLISH              = 0x01,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_FRENCH               = 0x02,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_GERMAN               = 0x03,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_SPANISH              = 0x04,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_ITALIAN              = 0x05,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_PORTUGUESE           = 0x06,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_DUTCH                = 0x07,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_SWEDISH              = 0x08,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_FINNISH              = 0x09,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_RUSSIAN              = 0x0a,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_JAPANESE             = 0x0b,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_SIMPLIFIED_CHINESE   = 0x0c,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_BRAZILIAN_PORTUGUESE = 0x0d,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_TRADITIONAL_CHINESE  = 0x0e,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_KOREAN               = 0x0f,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_TURKISH              = 0x10,
    MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_CHINESE              = 0xf0,
}
mdr_packet_eqebb_display_language_t;

typedef enum PACKED mdr_packet_eqebb_eq_preset_id
{
    MDR_PACKET_EQEBB_EQ_PRESET_ID_OFF                      = 0x00,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_ROCK                     = 0x01,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_POP                      = 0x02,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_JAZZ                     = 0x03,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_DANCE                    = 0x04,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_EDM                      = 0x05,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_R_AND_B_HIP_HOP          = 0x06,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_ACOUSTIC                 = 0x07,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO8  = 0x08,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO9  = 0x09,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO10 = 0x0a,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO11 = 0x0b,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO12 = 0x0c,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO13 = 0x0d,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO14 = 0x0e,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO15 = 0x0f,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_BRIGHT                   = 0x10,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_EXCITED                  = 0x11,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_MELLOW                   = 0x12,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RELAXED                  = 0x13,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_VOCAL                    = 0x14,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_TREBLE                   = 0x15,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_BASS                     = 0x16,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_SPEECH                   = 0x17,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO24 = 0x18,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO25 = 0x19,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO26 = 0x1a,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO27 = 0x1b,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO28 = 0x1c,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO29 = 0x1d,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO30 = 0x1e,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO31 = 0x1f,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_CUSTOM                   = 0xa0,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING1            = 0xa1,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING2            = 0xa2,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING3            = 0xa3,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING4            = 0xa4,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING5            = 0xa5,
    MDR_PACKET_EQEBB_EQ_PRESET_ID_UNSPECIFIED              = 0xff,
}
mdr_packet_eqebb_eq_preset_id_t;

typedef struct PACKED mdr_packet_eqebb_capability_eq_preset_name
{
    mdr_packet_eqebb_eq_preset_id_t preset_id;

    uint8_t name_len;
    uint8_t* name;
}
mdr_packet_eqebb_capability_eq_preset_name_t;

typedef struct PACKED mdr_packet_eqebb_capability_eq
{
    uint8_t band_count;
    uint8_t level_steps;

    uint8_t num_presets;
    mdr_packet_eqebb_capability_eq_preset_name_t* presets;
}
mdr_packet_eqebb_capability_eq_t;

typedef struct PACKED mdr_packet_eqebb_capability_ebb
{
    uint8_t min_level;

    uint8_t max_level;
}
mdr_packet_eqebb_capability_ebb_t;

typedef struct PACKED mdr_packet_eqebb_param_eq
{
    mdr_packet_eqebb_eq_preset_id_t preset_id;

    uint8_t num_levels;
    uint8_t* levels;
}
mdr_packet_eqebb_param_eq_t;

typedef struct PACKED mdr_packet_eqebb_param_ebb
{
    uint8_t level;
}
mdr_packet_eqebb_param_ebb_t;

// Packet payloads

typedef struct PACKED mdr_packet_eqebb_get_capability
{
    mdr_packet_eqebb_inquired_type_t inquired_type;

    mdr_packet_eqebb_display_language_t display_language;
}
mdr_packet_eqebb_get_capability_t;

typedef struct PACKED mdr_packet_eqebb_ret_capability
{
    mdr_packet_eqebb_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_eqebb_capability_eq_t eq;

        mdr_packet_eqebb_capability_ebb_t ebb;
    };
}
mdr_packet_eqebb_ret_capability_t;

typedef struct PACKED mdr_packet_eqebb_get_param
{
    mdr_packet_eqebb_inquired_type_t inquired_type;
}
mdr_packet_eqebb_get_param_t;

typedef struct PACKED mdr_packet_eqebb_ret_param
{
    mdr_packet_eqebb_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_eqebb_param_eq_t eq;

        mdr_packet_eqebb_param_ebb_t ebb;
    };
}
mdr_packet_eqebb_ret_param_t;

typedef struct PACKED mdr_packet_eqebb_set_param
{
    mdr_packet_eqebb_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_eqebb_param_eq_t eq;

        mdr_packet_eqebb_param_ebb_t ebb;
    };
}
mdr_packet_eqebb_set_param_t;

typedef struct PACKED mdr_packet_eqebb_ntfy_param
{
    mdr_packet_eqebb_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_eqebb_param_eq_t eq;

        mdr_packet_eqebb_param_ebb_t ebb;
    };
}
mdr_packet_eqebb_ntfy_param_t;

const char* mdr_packet_eqebb_get_preset_name(mdr_packet_eqebb_eq_preset_id_t);

#endif /* __MDR_PACKET_EQEBB_H__ */
