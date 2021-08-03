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

#ifndef __MDR_PACKET_SYSTEM_H__
#define __MDR_PACKET_SYSTEM_H__

#include <stdint.h>
#define PACKED __attribute__((__packed__))

// Fields

typedef enum PACKED mdr_packet_system_inquired_type
{
    MDR_PACKET_SYSTEM_INQUIRED_TYPE_NO_USE              = 0x00,
    MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR            = 0x01,
    MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE   = 0x02,
    MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING  = 0x03,
    MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF      = 0x04,
    MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE  = 0x05,
    MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS = 0x06,
}
mdr_packet_system_inquired_type_t;

typedef enum PACKED mdr_packet_system_vibrator_setting_type
{
    MDR_PACKET_SYSTEM_VIBRATOR_SETTING_TYPE_ON_OFF = 0x00,
}
mdr_packet_system_vibrator_setting_type_t;

typedef enum PACKED mdr_packet_system_vibrator_setting_value
{
    MDR_PACKET_SYSTEM_VIBRATOR_SETTING_VALUE_OFF = 0x00,
    MDR_PACKET_SYSTEM_VIBRATOR_SETTING_VALUE_ON  = 0x01,
}
mdr_packet_system_vibrator_setting_value_t;

typedef struct PACKED mdr_packet_system_capability_vibrator
{
    mdr_packet_system_vibrator_setting_type_t setting_type;
}
mdr_packet_system_capability_vibrator_t;

typedef struct PACKED mdr_packet_system_param_vibrator
{
    mdr_packet_system_vibrator_setting_type_t setting_type;

    mdr_packet_system_vibrator_setting_value_t setting_value;
}
mdr_packet_system_param_vibrator_t;

typedef enum PACKED mdr_packet_system_power_saving_mode_setting_type
{
    MDR_PACKET_SYSTEM_POWER_SAVING_MODE_SETTING_TYPE_ON_OFF = 0x00,
}
mdr_packet_system_power_saving_mode_setting_type_t;

typedef enum PACKED mdr_packet_system_power_saving_mode_setting_value
{
    MDR_PACKET_SYSTEM_POWER_SAVING_MODE_SETTING_VALUE_OFF = 0x00,
    MDR_PACKET_SYSTEM_POWER_SAVING_MODE_SETTING_VALUE_ON  = 0x01,
}
mdr_packet_system_power_saving_mode_setting_value_t;

typedef struct PACKED mdr_packet_system_capability_power_saving_mode
{
    mdr_packet_system_power_saving_mode_setting_type_t setting_type;
}
mdr_packet_system_capability_power_saving_mode_t;

typedef struct PACKED mdr_packet_system_param_power_saving_mode
{
    mdr_packet_system_power_saving_mode_setting_type_t setting_type;

    mdr_packet_system_power_saving_mode_setting_value_t setting_value;
}
mdr_packet_system_param_power_saving_mode_t;

typedef enum PACKED mdr_packet_system_control_by_wearing_setting_type
{
    MDR_PACKET_SYSTEM_CONTROL_BY_WEARING_SETTING_TYPE_ON_OFF = 0x00,
}
mdr_packet_system_control_by_wearing_setting_type_t;

typedef enum PACKED mdr_packet_system_control_by_wearing_setting_value
{
    MDR_PACKET_SYSTEM_CONTROL_BY_WEARING_SETTING_VALUE_OFF = 0x00,
    MDR_PACKET_SYSTEM_CONTROL_BY_WEARING_SETTING_VALUE_ON  = 0x01,
}
mdr_packet_system_control_by_wearing_setting_value_t;

typedef struct PACKED mdr_packet_system_capability_control_by_wearing
{
    mdr_packet_system_control_by_wearing_setting_type_t setting_type;
}
mdr_packet_system_capability_control_by_wearing_t;

typedef struct PACKED mdr_packet_system_param_control_by_wearing
{
    mdr_packet_system_control_by_wearing_setting_type_t setting_type;

    mdr_packet_system_control_by_wearing_setting_value_t setting_value;
}
mdr_packet_system_param_control_by_wearing_t;

typedef enum PACKED mdr_packet_system_auto_power_off_parameter_type
{
    MDR_PACKET_SYSTEM_AUTO_POWER_OFF_PARAMETER_TYPE_ACTIVE_AND_SELECT_TIME_ID
        = 0x01,
}
mdr_packet_system_auto_power_off_parameter_type_t;

typedef enum PACKED mdr_packet_system_auto_power_off_element_id
{
    MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_5_MIN               = 0x00,
    MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_30_MIN              = 0x01,
    MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_60_MIN              = 0x02,
    MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_180_MIN             = 0x03,
    MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_WHEN_REMOVED_FROM_EARS = 0x00,
    MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_DISABLE                = 0x00,
}
mdr_packet_system_auto_power_off_element_id_t;

typedef struct PACKED mdr_packet_system_capability_auto_power_off
{
    uint8_t element_id_count;

    mdr_packet_system_auto_power_off_element_id_t* element_ids;
}
mdr_packet_system_capability_auto_power_off_t;

typedef struct PACKED mdr_packet_system_param_auto_power_off
{
    mdr_packet_system_auto_power_off_parameter_type_t parameter_type;

    mdr_packet_system_auto_power_off_element_id_t element_id;

    mdr_packet_system_auto_power_off_element_id_t select_time_element_id;
}
mdr_packet_system_param_auto_power_off_t;

typedef enum PACKED mdr_packet_system_smart_talking_mode_setting_type
{
    MDR_PACKET_SYSTEM_SMART_TALKING_MODE_SETTING_TYPE_ON_OFF = 0x00,
}
mdr_packet_system_smart_talking_mode_setting_type_t;

typedef enum PACKED mdr_packet_system_smart_talking_mode_setting_value
{
    MDR_PACKET_SYSTEM_SMART_TALKING_MODE_SETTING_VALUE_OFF = 0x00,
    MDR_PACKET_SYSTEM_SMART_TALKING_MODE_SETTING_VALUE_ON  = 0x01,
}
mdr_packet_system_smart_talking_mode_setting_value_t;

typedef enum PACKED mdr_packet_system_smart_talking_mode_preview_type
{
    MDR_PACKET_SYSTEM_SMART_TALKING_MODE_PREVIEW_TYPE_NOT_SUPPORT = 0x00,
    MDR_PACKET_SYSTEM_SMART_TALKING_MODE_PREVIEW_TYPE_SUPPORT     = 0x01,
}
mdr_packet_system_smart_talking_mode_preview_type_t;

typedef enum PACKED mdr_packet_system_smart_talking_mode_detail_setting_type
{
    MDR_PACKET_SYSTEM_SMART_TALKING_MODE_DETAIL_SETTING_TYPE_TYPE_1 = 0x00,
}
mdr_packet_system_smart_talking_mode_detail_setting_type_t;

typedef enum PACKED mdr_packet_system_smart_talking_mode_detection_sensitivity_type
{
    MDR_PACKET_SYSTEM_SMART_TALKING_MODE_DETECTION_SENSITIVITY_TYPE_AUTO_HIGH_LOW = 0x00,
}
mdr_packet_system_smart_talking_mode_detection_sensitivity_type_t;

typedef enum PACKED mdr_packet_system_smart_talking_mode_voice_focus_type
{
    MDR_PACKET_SYSTEM_SMART_TALKING_MODE_VOICE_FOCUS_TYPE_ON_OFF = 0x00,
}
mdr_packet_system_smart_talking_mode_voice_focus_type_t;

typedef enum PACKED mdr_packet_system_smart_talking_mode_mode_out_time_type
{
    MDR_PACKET_SYSTEM_SMART_TALKING_MODE_MODE_OUT_TIME_TYPE_TYPE_1 = 0x00,
}
mdr_packet_system_smart_talking_mode_mode_out_time_type_t;

typedef struct PACKED mdr_packet_system_capability_smart_talking_mode
{
    mdr_packet_system_smart_talking_mode_setting_type_t               setting_type;
    mdr_packet_system_smart_talking_mode_preview_type_t               preview_type;
    mdr_packet_system_smart_talking_mode_detail_setting_type_t        detail_setting;
    mdr_packet_system_smart_talking_mode_detection_sensitivity_type_t detection_sensitivity_type;
    mdr_packet_system_smart_talking_mode_voice_focus_type_t           voice_focus_type;
    mdr_packet_system_smart_talking_mode_mode_out_time_type_t         mode_out_time_type_t;
    uint8_t                                                           timeouts[4];
}
mdr_packet_system_capability_smart_talking_mode_t;

typedef struct PACKED mdr_packet_system_param_smart_talking_mode
{
    mdr_packet_system_smart_talking_mode_setting_type_t setting_type;

    mdr_packet_system_smart_talking_mode_setting_value_t setting_value;
}
mdr_packet_system_param_smart_talking_mode_t;

typedef enum PACKED mdr_packet_system_assignable_settings_preset
{
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_AMBIENT_SOUND_CONTROL = 0x00,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_VOLUME_CONTROL        = 0x10,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_PLAYBACK_CONTROL      = 0x20,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_VOICE_RECOGNITION     = 0x30,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_GOOGLE_ASSISTANT      = 0x31,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_AMAZON_ALEXA          = 0x32,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_TENCENT_XIAOWEI       = 0x33,
}
mdr_packet_system_assignable_settings_preset_t;

typedef enum PACKED mdr_packet_system_assignable_settings_key
{
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEY_LEFT_SIDE_KEY  = 0x00,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEY_RIGHT_SIDE_KEY = 0x01,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEY_CUSTOM_KEY     = 0x02,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEY_C_KEY          = 0x03,
}
mdr_packet_system_assignable_settings_key_t;

typedef enum PACKED mdr_packet_system_assignable_settings_key_type
{
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEYS_TYPE_TOUCH_SENSOR = 0x00,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEYS_TYPE_BUTTON       = 0x01,
}
mdr_packet_system_assignable_settings_key_type_t;

typedef enum PACKED mdr_packet_system_assignable_settings_action
{
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_SINGLE_TAP = 0x00,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_DOUBLE_TAP = 0x01,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_TRIPLE_TAP = 0x02,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_SINGLE_TAP_AND_HOLD = 0x10,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_DOUBLE_TAP_AND_HOLD = 0x11,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_LONG_PRESS_THEN_ACTIVATE = 0x21,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_LONG_PRESS_DURING_ACTIVATION = 0x22,
}
mdr_packet_system_assignable_settings_action_t;

typedef enum PACKED mdr_packet_system_assignable_settings_function
{
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_NO_FUNCTION = 0x00,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_NC_ASM_OFF = 0x01,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_NC_OPTIMIZER = 0x02,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_QUICK_ATTENTION = 0x10,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_VOLUME_UP = 0x11,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_VOLUME_DOWN = 0x12,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_PLAY_PAUSE = 0x20,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_NEXT_TRACK = 0x21,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_PREVIOUS_TRACK = 0x22,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_VOICE_RECOGNITION = 0x30,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_GET_YOUR_NOTIFICATION = 0x31,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_TALK_TO_GA = 0x32,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_STOP_GA = 0x33,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_VOICE_INPUT_CANCEL_AA = 0x34,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_TALK_TO_TENCENT_XIAOWEI = 0x35,
    MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_CANCEL_VOICE_RECOGNITION = 0x36,
}
mdr_packet_system_assignable_settings_function_t;

typedef struct PACKED mdr_packet_system_assignable_settings_capability_caption
{
    mdr_packet_system_assignable_settings_action_t   action;
    mdr_packet_system_assignable_settings_function_t function;
}
mdr_packet_system_assignable_settings_capability_caption_t;

typedef struct PACKED mdr_packet_system_assignable_settings_capability_preset {
    mdr_packet_system_assignable_settings_preset_t preset;

    uint8_t num_capability_actions;
    mdr_packet_system_assignable_settings_capability_caption_t capability_actions;
}
mdr_packet_system_assignable_settings_capability_preset_t;

typedef struct PACKED mdr_packet_system_assignable_settings_capability_key
{
    mdr_packet_system_assignable_settings_key_t      key;
    mdr_packet_system_assignable_settings_key_type_t key_type;
    mdr_packet_system_assignable_settings_preset_t   default_preset;

    // The app has a bug where if `num_capability_presets` if 0, the entire
    // `mdr_packet_system_assignable_settings_key_info` is considered to have
    // length 1 (rather then 4 which is correct) when read as part of
    // `mdr_packet_system_capability_assignable_settings`.
    uint8_t num_capability_presets;
    mdr_packet_system_assignable_settings_capability_preset_t capability_presets;
}
mdr_packet_system_assignable_settings_capability_key_t;

typedef struct PACKED mdr_packet_system_capability_assignable_settings
{
    uint8_t num_capability_keys;
    mdr_packet_system_assignable_settings_capability_key_t* capability_keys;
}
mdr_packet_system_capability_assignable_settings_t;

typedef struct PACKED mdr_packet_system_param_assignable_settings
{
    uint8_t num_presets;

    mdr_packet_system_assignable_settings_preset_t* presets;
}
mdr_packet_system_param_assignable_settings_t;

// Packet payloads

typedef struct PACKED mdr_packet_system_get_capability
{
    mdr_packet_system_inquired_type_t inquired_type;
}
mdr_packet_system_get_capability_t;

typedef struct PACKED mdr_packet_system_ret_capability
{
    mdr_packet_system_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_system_capability_vibrator_t            vibrator;
        mdr_packet_system_capability_power_saving_mode_t   power_saving_mode;
        mdr_packet_system_capability_control_by_wearing_t  control_by_wearing;
        mdr_packet_system_capability_auto_power_off_t      auto_power_off;
        mdr_packet_system_capability_smart_talking_mode_t  smart_talking_mode;
        mdr_packet_system_capability_assignable_settings_t assignable_settings;
    };
}
mdr_packet_system_ret_capability_t;

typedef struct PACKED mdr_packet_system_get_param
{
    mdr_packet_system_inquired_type_t inquired_type;
}
mdr_packet_system_get_param_t;

typedef struct PACKED mdr_packet_system_ret_param
{
    mdr_packet_system_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_system_param_vibrator_t            vibrator;
        mdr_packet_system_param_power_saving_mode_t   power_saving_mode;
        mdr_packet_system_param_control_by_wearing_t  control_by_wearing;
        mdr_packet_system_param_auto_power_off_t      auto_power_off;
        mdr_packet_system_param_smart_talking_mode_t  smart_talking_mode;
        mdr_packet_system_param_assignable_settings_t assignable_settings;
    };
}
mdr_packet_system_ret_param_t;

typedef struct PACKED mdr_packet_system_set_param
{
    mdr_packet_system_inquired_type_t inquired_type;

    union PACKED
    {
        mdr_packet_system_param_vibrator_t            vibrator;
        mdr_packet_system_param_power_saving_mode_t   power_saving_mode;
        mdr_packet_system_param_control_by_wearing_t  control_by_wearing;
        mdr_packet_system_param_auto_power_off_t      auto_power_off;
        mdr_packet_system_param_smart_talking_mode_t  smart_talking_mode;
        mdr_packet_system_param_assignable_settings_t assignable_settings;
    };
}
mdr_packet_system_set_param_t;

#endif /* __MDR_PACKET_SYSTEM_H__ */

