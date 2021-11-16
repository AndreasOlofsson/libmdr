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

static void mdr_packet_eqebb_free(mdr_packet_t* packet)
{
    switch (packet->type)
    {
        case MDR_PACKET_EQEBB_GET_CAPABILITY:
            break;

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
#define PACKET_FIELD eqebb_ret_capability

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    FREE_FOR_EACH_IN_PACKET(
                            eq.num_presets,
                            eq.presets,
                            mdr_packet_eqebb_capability_eq_preset_name_t,
                            preset)

                        free(preset->name);

                    FREE_FOR_EACH_END

                    FREE_FIELD(eq.presets)

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    break;
            }

            break;

#undef PACKET_FIELD

        case MDR_PACKET_EQEBB_GET_PARAM:
            break;

        case MDR_PACKET_EQEBB_RET_PARAM:
        case MDR_PACKET_EQEBB_SET_PARAM:
        case MDR_PACKET_EQEBB_NTFY_PARAM:
#define PACKET_FIELD eqebb_ret_param

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    FREE_FIELD(eq.levels)

                    break;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    break;
            }

            break;

#undef PACKET_FIELD

        default:
            break;
    }
}

static mdr_packet_t* mdr_packet_eqebb_from_frame(mdr_frame_t* frame)
{
    PARSE_INIT(frame)

    switch (mdr_frame_payload(frame)[0])
    {
        case MDR_PACKET_EQEBB_GET_CAPABILITY:
#define PACKET_FIELD eqebb_get_capability

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE)
            PARSE_ENUM_END

            PARSE_ENUM_INTO_PACKET(display_language)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_UNDEFINED_LANGUAGE)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_ENGLISH)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_FRENCH)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_GERMAN)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_SPANISH)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_ITALIAN)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_PORTUGUESE)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_DUTCH)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_SWEDISH)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_FINNISH)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_RUSSIAN)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_JAPANESE)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_SIMPLIFIED_CHINESE)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_BRAZILIAN_PORTUGUESE)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_TRADITIONAL_CHINESE)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_KOREAN)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_TURKISH)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_DISPLAY_LANGUAGE_CHINESE)
            PARSE_ENUM_END

            break;

#undef PACKET_FIELD

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
#define PACKET_FIELD eqebb_ret_capability

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE)
            PARSE_ENUM_END

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    PARSE_BYTE_INTO_PACKET(eq.band_count)
                    PARSE_BYTE_INTO_PACKET(eq.level_steps)
                    PARSE_BYTE_INTO_PACKET(eq.num_presets)

                    PARSE_FOR_EACH_IN_PACKET(
                            eq.num_presets,
                            eq.presets,
                            mdr_packet_eqebb_capability_eq_preset_name_t,
                            preset)

                        PARSE_ENUM_INTO_VALUE(preset->preset_id)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_OFF)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_ROCK)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_POP)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_JAZZ)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_DANCE)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_EDM)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_R_AND_B_HIP_HOP)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_ACOUSTIC)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO8)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO9)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO10)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO11)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO12)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO13)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO14)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO15)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_BRIGHT)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_EXCITED)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_MELLOW)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RELAXED)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_VOCAL)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_TREBLE)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_BASS)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_SPEECH)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO24)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO25)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO26)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO27)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO28)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO29)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO30)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO31)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_CUSTOM)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING1)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING2)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING3)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING4)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING5)
                            PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_UNSPECIFIED)
                        PARSE_ENUM_END

                        PARSE_BYTE_INTO_VALUE(preset->name_len)

                        PARSE_BYTES_INTO_VALUE(preset->name, preset->name_len)

                    PARSE_FOR_EACH_END

                    break;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    PARSE_BYTE_INTO_PACKET(ebb.min_level)
                    PARSE_BYTE_INTO_PACKET(ebb.max_level)

                    break;
            }

            break;

#undef PACKET_FIELD

        case MDR_PACKET_EQEBB_GET_PARAM:
#define PACKET_FIELD eqebb_get_param

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE)
            PARSE_ENUM_END

            break;

#undef PACKET_FIELD

        case MDR_PACKET_EQEBB_RET_PARAM:
        case MDR_PACKET_EQEBB_SET_PARAM:
        case MDR_PACKET_EQEBB_NTFY_PARAM:
#define PACKET_FIELD eqebb_ret_param

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB)
                PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE)
            PARSE_ENUM_END

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    PARSE_ENUM_INTO_PACKET(eq.preset_id)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_ROCK)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_POP)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_JAZZ)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_DANCE)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_EDM)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_R_AND_B_HIP_HOP)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_ACOUSTIC)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO8)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO9)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO10)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO11)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO12)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO13)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO14)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO15)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_BRIGHT)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_EXCITED)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_MELLOW)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RELAXED)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_VOCAL)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_TREBLE)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_BASS)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_SPEECH)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO24)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO25)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO26)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO27)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO28)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO29)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO30)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_RESERVED_FOR_FUTURE_NO31)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_CUSTOM)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING1)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING2)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING3)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING4)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING5)
                        PARSE_ENUM_FIELD(MDR_PACKET_EQEBB_EQ_PRESET_ID_UNSPECIFIED)
                    PARSE_ENUM_END

                    PARSE_BYTE_INTO_PACKET(eq.num_levels)

                    PARSE_BYTES_INTO_PACKET(eq.levels, FIELD(eq.num_levels))

                    break;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    PARSE_BYTE_INTO_PACKET(ebb.level)

                    break;
            }

            break;

#undef PACKET_FIELD

        default:
            INVALID_FRAME
    }

    return packet;
}

static mdr_frame_t* mdr_packet_eqebb_to_frame(mdr_packet_t* packet)
{
    WRITE_INIT(packet)
    int length;

    switch (packet->type)
    {
        case MDR_PACKET_EQEBB_GET_CAPABILITY:
#define PACKET_FIELD eqebb_get_capability

            WRITE_START(2)
            WRITE_FIELD(inquired_type)
            WRITE_FIELD(display_language)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_EQEBB_RET_CAPABILITY:
#define PACKET_FIELD eqebb_ret_capability

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    length = 4;

                    WRITE_FOR_EACH_IN_PACKET(
                            eq.num_presets,
                            eq.presets,
                            mdr_packet_eqebb_capability_eq_preset_name_t,
                            preset)

                        length += 2 + preset->name_len;

                    WRITE_FOR_EACH_END

                    WRITE_START(length)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(eq.band_count)
                    WRITE_FIELD(eq.level_steps)
                    WRITE_FIELD(eq.num_presets)

                    WRITE_FOR_EACH_IN_PACKET(
                            eq.num_presets,
                            eq.presets,
                            mdr_packet_eqebb_capability_eq_preset_name_t,
                            preset)

                        WRITE_BYTE(preset->preset_id)
                        WRITE_BYTE(preset->name_len)
                        WRITE_BYTES(preset->name, preset->name_len)

                    WRITE_FOR_EACH_END

                    break;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    WRITE_START(3)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(ebb.min_level)
                    WRITE_FIELD(ebb.max_level)

                    break;
            }

            break;

#undef PACKET_FIELD

        case MDR_PACKET_EQEBB_GET_PARAM:
#define PACKET_FIELD eqebb_get_param

            WRITE_START(1)
            WRITE_FIELD(inquired_type)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_EQEBB_RET_PARAM:
        case MDR_PACKET_EQEBB_SET_PARAM:
        case MDR_PACKET_EQEBB_NTFY_PARAM:
#define PACKET_FIELD eqebb_ret_param

            switch (FIELD(inquired_type))
            {
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ:
                case MDR_PACKET_EQEBB_INQUIRED_TYPE_PRESET_EQ_NONCUSTOMIZABLE:
                    WRITE_START(3 + FIELD(eq.num_levels))
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(eq.preset_id)
                    WRITE_FIELD(eq.num_levels)
                    WRITE_FIELD_BYTES(eq.levels, FIELD(eq.num_levels))

                    break;

                case MDR_PACKET_EQEBB_INQUIRED_TYPE_EBB:
                    WRITE_START(2)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(ebb.level)

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

