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

#include <stdio.h>

#define ASSERT_SIZE(type, n) \
    if (sizeof(type) != n) { \
        printf("Expected " #type " to have size " #n ", got %ld.\n", \
                sizeof(type)); \
        exit(1); \
    }

#define ASSERT_OFFSET(type, field, n) \
    { \
        type v; \
        if ((size_t) &v.field - (size_t) &v != n) \
        { \
            printf("Expected field " #field " of type " #type \
                    " to have offset " #n ", got %ld.\n", \
                    (size_t) &v.field - (size_t) &v); \
            exit(1); \
        } \
    }

int main(int argc, char* argv[])
{
    ASSERT_OFFSET(mdr_packet_t, data, 1);

    ASSERT_SIZE(mdr_packet_connect_get_protocol_info_t, 1);
    ASSERT_SIZE(mdr_packet_connect_ret_protocol_info_t, 3);

    ASSERT_SIZE(mdr_packet_connect_get_device_info_t, 1);
    ASSERT_OFFSET(mdr_packet_connect_ret_device_info_t, model_name, 1);
    ASSERT_OFFSET(mdr_packet_connect_ret_device_info_t, fw_version, 1);
    ASSERT_OFFSET(mdr_packet_connect_ret_device_info_t, series_and_color, 1);
    ASSERT_OFFSET(mdr_packet_connect_ret_device_info_t, instruction_guide, 1);
    ASSERT_OFFSET(mdr_packet_device_info_string_t, string, 1);
    ASSERT_SIZE(mdr_packet_device_info_series_and_color_t, 2);
    ASSERT_OFFSET(mdr_packet_device_info_instruction_guide_t, guidance_categories, 1);

    ASSERT_SIZE(mdr_packet_connect_get_support_function_t, 1);
    ASSERT_OFFSET(mdr_packet_connect_ret_support_function_t, function_types, 2);

    ASSERT_SIZE(mdr_packet_common_get_battery_level_t, 1);
    ASSERT_OFFSET(mdr_packet_common_ret_battery_level_t, battery, 1);
    ASSERT_OFFSET(mdr_packet_common_ret_battery_level_t, left_right_battery, 1);
    ASSERT_OFFSET(mdr_packet_common_ret_battery_level_t, cradle_battery, 1);
    ASSERT_OFFSET(mdr_packet_common_ntfy_battery_level_t, battery, 1);
    ASSERT_OFFSET(mdr_packet_common_ntfy_battery_level_t, left_right_battery, 1);
    ASSERT_OFFSET(mdr_packet_common_ntfy_battery_level_t, cradle_battery, 1);
    ASSERT_SIZE(mdr_packet_battery_status_t, 2);
    ASSERT_SIZE(mdr_packet_battery_status_left_right_t, 4);
}
