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
    ASSERT_SIZE(mdr_packet_device_info_guidance_category_t, 1);

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

    ASSERT_SIZE(mdr_packet_common_set_power_off_t, 2);

    ASSERT_SIZE(mdr_packet_common_get_connection_status_t, 1);
    ASSERT_OFFSET(mdr_packet_common_ret_connection_status_t, left_right, 1);
    ASSERT_OFFSET(mdr_packet_common_ntfy_connection_status_t, left_right, 1);
    ASSERT_SIZE(mdr_packet_connection_status_left_right_status_t, 2);

    ASSERT_SIZE(mdr_packet_eqebb_get_capability_t, 2);
    ASSERT_OFFSET(mdr_packet_eqebb_ret_capability_t, eq, 1);
    ASSERT_OFFSET(mdr_packet_eqebb_ret_capability_t, ebb, 1);
    ASSERT_OFFSET(mdr_packet_eqebb_capability_eq_t, presets, 3);
    ASSERT_OFFSET(mdr_packet_eqebb_capability_eq_preset_name_t, name, 2);
    ASSERT_SIZE(mdr_packet_eqebb_param_ebb_t, 1);
    ASSERT_SIZE(mdr_packet_eqebb_get_param_t, 1);
    ASSERT_OFFSET(mdr_packet_eqebb_ret_param_t, eq, 1);
    ASSERT_OFFSET(mdr_packet_eqebb_ret_param_t, ebb, 1);
    ASSERT_OFFSET(mdr_packet_eqebb_set_param_t, eq, 1);
    ASSERT_OFFSET(mdr_packet_eqebb_set_param_t, ebb, 1);
    ASSERT_OFFSET(mdr_packet_eqebb_ntfy_param_t, eq, 1);
    ASSERT_OFFSET(mdr_packet_eqebb_ntfy_param_t, ebb, 1);

    ASSERT_SIZE(mdr_packet_ncasm_get_param_t, 1);
    ASSERT_OFFSET(mdr_packet_ncasm_ret_param_t, noise_cancelling, 1);
    ASSERT_OFFSET(mdr_packet_ncasm_ret_param_t, noise_cancelling_asm, 1);
    ASSERT_OFFSET(mdr_packet_ncasm_ret_param_t, ambient_sound_mode, 1);
    ASSERT_OFFSET(mdr_packet_ncasm_set_param_t, noise_cancelling, 1);
    ASSERT_OFFSET(mdr_packet_ncasm_set_param_t, noise_cancelling_asm, 1);
    ASSERT_OFFSET(mdr_packet_ncasm_set_param_t, ambient_sound_mode, 1);
    ASSERT_OFFSET(mdr_packet_ncasm_ntfy_param_t, noise_cancelling, 1);
    ASSERT_OFFSET(mdr_packet_ncasm_ntfy_param_t, noise_cancelling_asm, 1);
    ASSERT_OFFSET(mdr_packet_ncasm_ntfy_param_t, ambient_sound_mode, 1);
    ASSERT_SIZE(mdr_packet_ncasm_param_noise_cancelling_t, 2);
    ASSERT_SIZE(mdr_packet_ncasm_param_noise_cancelling_asm_t, 6);
    ASSERT_SIZE(mdr_packet_ncasm_param_asm_t, 4);

    ASSERT_SIZE(mdr_packet_play_get_param_t, 2);
    ASSERT_OFFSET(mdr_packet_play_ret_param_t, string, 2);
    ASSERT_OFFSET(mdr_packet_play_ret_param_t, volume, 2);
    ASSERT_OFFSET(mdr_packet_play_set_param_t, string, 2);
    ASSERT_OFFSET(mdr_packet_play_set_param_t, volume, 2);
    ASSERT_OFFSET(mdr_packet_play_ntfy_param_t, string, 2);
    ASSERT_OFFSET(mdr_packet_play_ntfy_param_t, volume, 2);
    ASSERT_OFFSET(mdr_packet_play_playback_string_t, data, 2);

    ASSERT_SIZE(mdr_packet_system_get_capability_t, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_capability_t, vibrator, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_capability_t, power_saving_mode, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_capability_t, control_by_wearing, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_capability_t, auto_power_off, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_capability_t, smart_talking_mode, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_capability_t, assignable_settings, 1);
    ASSERT_SIZE(mdr_packet_system_capability_vibrator_t, 1);
    ASSERT_SIZE(mdr_packet_system_capability_power_saving_mode_t, 1);
    ASSERT_SIZE(mdr_packet_system_capability_control_by_wearing_t, 1);
    ASSERT_OFFSET(mdr_packet_system_capability_auto_power_off_t, element_ids, 1);
    ASSERT_SIZE(mdr_packet_system_auto_power_off_element_id_t, 1);
    ASSERT_SIZE(mdr_packet_system_capability_smart_talking_mode_t, 10);
    ASSERT_OFFSET(mdr_packet_system_capability_assignable_settings_t, capability_keys, 1);
    ASSERT_OFFSET(mdr_packet_system_assignable_settings_capability_key_t, capability_presets, 4);
    ASSERT_OFFSET(mdr_packet_system_assignable_settings_capability_preset_t, capability_actions, 2);
    ASSERT_SIZE(mdr_packet_system_assignable_settings_capability_caption_t, 2);
    ASSERT_SIZE(mdr_packet_system_get_param_t, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_param_t, vibrator, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_param_t, power_saving_mode, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_param_t, control_by_wearing, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_param_t, auto_power_off, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_param_t, smart_talking_mode, 1);
    ASSERT_OFFSET(mdr_packet_system_ret_param_t, assignable_settings, 1);
    ASSERT_OFFSET(mdr_packet_system_set_param_t, vibrator, 1);
    ASSERT_OFFSET(mdr_packet_system_set_param_t, power_saving_mode, 1);
    ASSERT_OFFSET(mdr_packet_system_set_param_t, control_by_wearing, 1);
    ASSERT_OFFSET(mdr_packet_system_set_param_t, auto_power_off, 1);
    ASSERT_OFFSET(mdr_packet_system_set_param_t, smart_talking_mode, 1);
    ASSERT_OFFSET(mdr_packet_system_set_param_t, assignable_settings, 1);
    ASSERT_OFFSET(mdr_packet_system_ntfy_param_t, vibrator, 1);
    ASSERT_OFFSET(mdr_packet_system_ntfy_param_t, power_saving_mode, 1);
    ASSERT_OFFSET(mdr_packet_system_ntfy_param_t, control_by_wearing, 1);
    ASSERT_OFFSET(mdr_packet_system_ntfy_param_t, auto_power_off, 1);
    ASSERT_OFFSET(mdr_packet_system_ntfy_param_t, smart_talking_mode, 1);
    ASSERT_OFFSET(mdr_packet_system_ntfy_param_t, assignable_settings, 1);
    ASSERT_SIZE(mdr_packet_system_param_vibrator_t, 2);
    ASSERT_SIZE(mdr_packet_system_param_power_saving_mode_t, 2);
    ASSERT_SIZE(mdr_packet_system_param_control_by_wearing_t, 2);
    ASSERT_SIZE(mdr_packet_system_param_auto_power_off_t, 3);
    ASSERT_SIZE(mdr_packet_system_param_smart_talking_mode_t, 2);
    ASSERT_OFFSET(mdr_packet_system_param_assignable_settings_t, presets, 1);
    ASSERT_SIZE(mdr_packet_system_assignable_settings_preset_t, 1);
}

