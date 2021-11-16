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

static void mdr_packet_ncasm_free(mdr_packet_t* packet)
{
    switch (packet->type)
    {
        case MDR_PACKET_NCASM_GET_PARAM:
        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
            break;

        default:
            break;
    }
}

static mdr_packet_t* mdr_packet_ncasm_from_frame(mdr_frame_t* frame)
{
    PARSE_INIT(frame)

    switch (mdr_frame_payload(frame)[0])
    {
        case MDR_PACKET_NCASM_GET_PARAM:
#define PACKET_FIELD ncasm_get_param

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING)
                PARSE_ENUM_FIELD(MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM)
                PARSE_ENUM_FIELD(MDR_PACKET_NCASM_INQUIRED_TYPE_ASM)
            PARSE_ENUM_END

            break;

#undef PACKET_FIELD

        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
#define PACKET_FIELD ncasm_ret_param

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING)
                PARSE_ENUM_FIELD(MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM)
                PARSE_ENUM_FIELD(MDR_PACKET_NCASM_INQUIRED_TYPE_ASM)
            PARSE_ENUM_END

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING:
                    PARSE_ENUM_INTO_PACKET(noise_cancelling.nc_setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NC_SETTING_TYPE_ON_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NC_SETTING_TYPE_LEVEL_ADJUSTMENT)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(noise_cancelling.nc_setting_value)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NC_SETTING_VALUE_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NC_SETTING_VALUE_ON)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM:
                    PARSE_ENUM_INTO_PACKET(noise_cancelling_asm.ncasm_effect)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_EFFECT_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_EFFECT_ON)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_EFFECT_ADJUSTEMENT_IN_PROGRESS)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_EFFECT_ADJUSTEMENT_COMPLETION)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(noise_cancelling_asm.ncasm_setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_SETTING_TYPE_ON_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_SETTING_TYPE_LEVEL_ADJUSTMENT)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_SETTING_TYPE_DUAL_SINGLE_OFF)
                    PARSE_ENUM_END
                    PARSE_BYTE_INTO_PACKET(noise_cancelling_asm.ncasm_amount)
                    PARSE_ENUM_INTO_PACKET(noise_cancelling_asm.asm_setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_ASM_SETTING_TYPE_ON_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_ASM_SETTING_TYPE_LEVEL_ADJUSTMENT)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(noise_cancelling_asm.asm_id)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_ASM_ID_NORMAL)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_ASM_ID_VOICE)
                    PARSE_ENUM_END
                    PARSE_BYTE_INTO_PACKET(noise_cancelling_asm.asm_amount)

                    break;

                case MDR_PACKET_NCASM_INQUIRED_TYPE_ASM:
                    PARSE_ENUM_INTO_PACKET(ambient_sound_mode.ncasm_effect)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_EFFECT_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_EFFECT_ON)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_EFFECT_ADJUSTEMENT_IN_PROGRESS)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_NCASM_EFFECT_ADJUSTEMENT_COMPLETION)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(ambient_sound_mode.asm_setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_ASM_SETTING_TYPE_ON_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_ASM_SETTING_TYPE_LEVEL_ADJUSTMENT)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(ambient_sound_mode.asm_id)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_ASM_ID_NORMAL)
                        PARSE_ENUM_FIELD(MDR_PACKET_NCASM_ASM_ID_VOICE)
                    PARSE_ENUM_END
                    PARSE_BYTE_INTO_PACKET(ambient_sound_mode.asm_amount)

                    break;
            }

            break;

#undef PACKET_FIELD

        default:
            INVALID_FRAME
    }

    return packet;
}

static mdr_frame_t* mdr_packet_ncasm_to_frame(mdr_packet_t* packet)
{
    WRITE_INIT(packet)

    switch (packet->type)
    {
        case MDR_PACKET_NCASM_GET_PARAM:
#define PACKET_FIELD ncasm_get_param

            WRITE_START(1)
            WRITE_FIELD(inquired_type)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_NCASM_RET_PARAM:
        case MDR_PACKET_NCASM_SET_PARAM:
        case MDR_PACKET_NCASM_NTFY_PARAM:
#define PACKET_FIELD ncasm_ret_param

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING:
                    WRITE_START(3)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(noise_cancelling.nc_setting_type)
                    WRITE_FIELD(noise_cancelling.nc_setting_value)

                    break;

                case MDR_PACKET_NCASM_INQUIRED_TYPE_NOISE_CANCELLING_AND_ASM:
                    WRITE_START(7)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(noise_cancelling_asm.ncasm_effect)
                    WRITE_FIELD(noise_cancelling_asm.ncasm_setting_type)
                    WRITE_FIELD(noise_cancelling_asm.ncasm_amount)
                    WRITE_FIELD(noise_cancelling_asm.asm_setting_type)
                    WRITE_FIELD(noise_cancelling_asm.asm_id)
                    WRITE_FIELD(noise_cancelling_asm.asm_amount)

                    break;

                case MDR_PACKET_NCASM_INQUIRED_TYPE_ASM:
                    WRITE_START(5)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(ambient_sound_mode.ncasm_effect)
                    WRITE_FIELD(ambient_sound_mode.asm_setting_type)
                    WRITE_FIELD(ambient_sound_mode.asm_id)
                    WRITE_FIELD(ambient_sound_mode.asm_id)

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

