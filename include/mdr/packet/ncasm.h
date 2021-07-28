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

#ifndef __MDR_PACKET_NCASM_H__
#define __MDR_PACKET_NCASM_H__

#include <stdint.h>
#define PACKED __attribute__((__packed__))

// Fields

typedef enum PACKED mdr_packet_ncasm_inquired_type
{
    MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING         = 0x01,
    MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM = 0x02,
    MDR_PACKET_NCASM_INQUIRED_TYPE_ASM                      = 0x03,
}
mdr_packet_ncasm_inquired_type_t;

typedef enum PACKED mdr_packet_ncasm_nc_setting_type
{
    MDR_PACKET_NCASM_NC_SETTING_TYPE_ON_OFF           = 0x00,
    MDR_PACKET_NCASM_NC_SETTING_TYPE_LEVEL_ADJUSTMENT = 0x01,
}
mdr_packet_ncasm_nc_setting_type_t;

typedef enum PACKED mdr_packet_ncasm_nc_setting_value
{
    MDR_PACKET_NCASM_NC_SETTING_VALUE_OFF = 0x00,
    MDR_PACKET_NCASM_NC_SETTING_VALUE_ON  = 0x01,
}
mdr_packet_ncasm_nc_setting_value_t;

typedef enum PACKED mdr_packet_ncasm_ncasm_effect
{
    MDR_PACKET_NCASM_NCASM_EFFECT_OFF                     = 0x00,
    MDR_PACKET_NCASM_NCASM_EFFECT_ON                      = 0x01,
    MDR_PACKET_NCASM_NCASM_EFFECT_ADJUSTEMENT_IN_PROGRESS = 0x10,
    MDR_PACKET_NCASM_NCASM_EFFECT_ADJUSTEMENT_COMPLETION  = 0x11,
}
mdr_packet_ncasm_ncasm_effect_t;

typedef enum PACKED mdr_packet_ncasm_ncasm_setting_type
{
    MDR_PACKET_NCASM_NCASM_SETTING_TYPE_ON_OFF           = 0x00,
    MDR_PACKET_NCASM_NCASM_SETTING_TYPE_LEVEL_ADJUSTMENT = 0x01,
    MDR_PACKET_NCASM_NCASM_SETTING_TYPE_DUAL_SINGLE_OFF  = 0x02,
}
mdr_packet_ncasm_ncasm_setting_type_t;

typedef enum PACKED mdr_packet_ncasm_asm_setting_type
{
    MDR_PACKET_NCASM_ASM_SETTING_TYPE_ON_OFF           = 0x00,
    MDR_PACKET_NCASM_ASM_SETTING_TYPE_LEVEL_ADJUSTMENT = 0x01,
}
mdr_packet_ncasm_asm_setting_type_t;

typedef enum PACKED mdr_packet_ncasm_asm_id
{
    MDR_PACKET_NCASM_ASM_ID_NORMAL = 0x00,
    MDR_PACKET_NCASM_ASM_ID_VOICE  = 0x01,
}
mdr_packet_ncasm_asm_id_t;

typedef struct PACKED mdr_packet_ncasm_param_noise_cancelling
{
    mdr_packet_ncasm_nc_setting_type_t nc_setting_type;

    mdr_packet_ncasm_nc_setting_value_t nc_setting_value;
}
mdr_packet_ncasm_param_noise_cancelling_t;

typedef struct PACKED mdr_packet_ncasm_param_noise_cancelling_asm
{
    mdr_packet_ncasm_ncasm_effect_t       ncasm_effect;
    mdr_packet_ncasm_ncasm_setting_type_t ncasm_setting_type;
    uint8_t                               ncasm_amount;
    mdr_packet_ncasm_asm_setting_type_t   asm_setting_type;
    mdr_packet_ncasm_asm_id_t             asm_id;
    uint8_t                               asm_amount;
}
mdr_packet_ncasm_param_noise_cancelling_asm_t;

typedef struct PACKED mdr_packet_ncasm_param_asm
{
    mdr_packet_ncasm_ncasm_effect_t       ncasm_effect;
    mdr_packet_ncasm_asm_setting_type_t   asm_setting_type;
    mdr_packet_ncasm_asm_id_t             asm_id;
    uint8_t                               asm_amount;
}
mdr_packet_ncasm_param_asm_t;

// Packet payloads

typedef struct PACKED mdr_packet_ncasm_get_param
{
    mdr_packet_ncasm_inquired_type_t inquired_type;
}
mdr_packet_ncasm_get_param_t;

typedef struct PACKED mdr_packet_ncasm_ret_param
{
    mdr_packet_ncasm_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_ncasm_param_noise_cancelling_t     noise_cancelling;
        mdr_packet_ncasm_param_noise_cancelling_asm_t noise_cancelling_asm;
        mdr_packet_ncasm_param_asm_t                  ambient_sound_mode;
    };
}
mdr_packet_ncasm_ret_param_t;

typedef struct PACKED mdr_packet_ncasm_set_param
{
    mdr_packet_ncasm_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_ncasm_param_noise_cancelling_t     noise_cancelling;
        mdr_packet_ncasm_param_noise_cancelling_asm_t noise_cancelling_asm;
        mdr_packet_ncasm_param_asm_t                  ambient_sound_mode;
    };
}
mdr_packet_ncasm_set_param_t;

typedef struct PACKED mdr_packet_ncasm_ntfy_param
{
    mdr_packet_ncasm_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_ncasm_param_noise_cancelling_t     noise_cancelling;
        mdr_packet_ncasm_param_noise_cancelling_asm_t noise_cancelling_asm;
        mdr_packet_ncasm_param_asm_t                  ambient_sound_mode;
    };
}
mdr_packet_ncasm_ntfy_param_t;

#endif /* __MDR_PACKET_NCASM_H__ */

