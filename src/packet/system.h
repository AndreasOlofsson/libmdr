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

static void mdr_packet_system_free(mdr_packet_t* packet)
{
    switch (packet->type)
    {
        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
            break;

        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
#define PACKET_FIELD system_ret_capability

            switch (packet->data.system_ret_capability.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    FREE_FIELD(auto_power_off.element_ids);

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    FREE_FOR_EACH_IN_PACKET(
                            assignable_settings.num_capability_keys,
                            assignable_settings.capability_keys,
                            mdr_packet_system_assignable_settings_capability_key_t,
                            capability_keys)

                        FREE_FOR_EACH_IN_VALUE(
                                capability_keys->num_capability_presets,
                                capability_keys->capability_presets,
                                mdr_packet_system_assignable_settings_capability_preset_t,
                                capability_presets)

                            free(capability_presets->capability_actions);

                        FREE_FOR_EACH_END

                        free(capability_keys->capability_presets);

                    FREE_FOR_EACH_END

                    FREE_FIELD(assignable_settings.capability_keys)

                    break;
            }
            break;

#undef PACKET_FIELD

        case MDR_PACKET_SYSTEM_GET_PARAM:
            break;

        case MDR_PACKET_SYSTEM_RET_PARAM:
#define PACKET_FIELD system_ret_param

            switch (packet->data.system_ret_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    FREE_FIELD(assignable_settings.presets)

                    break;
            }

#undef PACKET_FIELD
            break;

        case MDR_PACKET_SYSTEM_SET_PARAM:
#define PACKET_FIELD system_set_param

            switch (packet->data.system_set_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    FREE_FIELD(assignable_settings.presets)

                    break;
            }

#undef PACKET_FIELD
            break;

        case MDR_PACKET_SYSTEM_NTFY_PARAM:
#define PACKET_FIELD system_ntfy_param

            switch (packet->data.system_ntfy_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    FREE_FIELD(assignable_settings.presets)

                    break;
            }

#undef PACKET_FIELD
            break;

        default:
            break;
    }
}

static mdr_packet_t* mdr_packet_system_from_frame(mdr_frame_t* frame)
{
    PARSE_INIT(frame)

    switch (mdr_frame_payload(frame)[0])
    {
        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
#define PACKET_FIELD system_get_capability

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS)
            PARSE_ENUM_END

#undef PACKET_FIELD
            break;

        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
#define PACKET_FIELD system_ret_capability

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS)
            PARSE_ENUM_END

            switch (packet->data.system_ret_capability.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    PARSE_ENUM_INTO_PACKET(vibrator.setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_VIBRATOR_SETTING_TYPE_ON_OFF)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    PARSE_ENUM_INTO_PACKET(power_saving_mode.setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_POWER_SAVING_MODE_SETTING_TYPE_ON_OFF)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    PARSE_ENUM_INTO_PACKET(control_by_wearing.setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_CONTROL_BY_WEARING_SETTING_TYPE_ON_OFF)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    PARSE_BYTE_INTO_PACKET(auto_power_off.element_id_count)

                    PARSE_FOR_EACH_IN_PACKET(auto_power_off.element_id_count,
                                             auto_power_off.element_ids,
                                             mdr_packet_system_auto_power_off_element_id_t,
                                             element_id)

                        PARSE_ENUM_INTO_VALUE(*element_id)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_5_MIN)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_30_MIN)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_60_MIN)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_180_MIN)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_WHEN_REMOVED_FROM_EARS)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_DISABLE)
                        PARSE_ENUM_END

                    PARSE_FOR_EACH_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    PARSE_ENUM_INTO_PACKET(smart_talking_mode.setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_SETTING_TYPE_ON_OFF)
                    PARSE_ENUM_END

                    PARSE_ENUM_INTO_PACKET(smart_talking_mode.preview_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_PREVIEW_TYPE_NOT_SUPPORT)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_PREVIEW_TYPE_SUPPORT)
                    PARSE_ENUM_END

                    PARSE_ENUM_INTO_PACKET(smart_talking_mode.detail_setting)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_DETAIL_SETTING_TYPE_TYPE_1)
                    PARSE_ENUM_END

                    PARSE_ENUM_INTO_PACKET(smart_talking_mode.detection_sensitivity_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_DETECTION_SENSITIVITY_TYPE_AUTO_HIGH_LOW)
                    PARSE_ENUM_END

                    PARSE_ENUM_INTO_PACKET(smart_talking_mode.voice_focus_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_VOICE_FOCUS_TYPE_ON_OFF)
                    PARSE_ENUM_END

                    PARSE_ENUM_INTO_PACKET(smart_talking_mode.mode_out_time_type_t)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_MODE_OUT_TIME_TYPE_TYPE_1)
                    PARSE_ENUM_END

                    PARSE_BYTE_INTO_PACKET(smart_talking_mode.timeouts[0])
                    PARSE_BYTE_INTO_PACKET(smart_talking_mode.timeouts[1])
                    PARSE_BYTE_INTO_PACKET(smart_talking_mode.timeouts[2])
                    PARSE_BYTE_INTO_PACKET(smart_talking_mode.timeouts[3])

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    PARSE_BYTE_INTO_PACKET(assignable_settings.num_capability_keys)

                    PARSE_FOR_EACH_IN_PACKET(
                            assignable_settings.num_capability_keys,
                            assignable_settings.capability_keys,
                            mdr_packet_system_assignable_settings_capability_key_t,
                            capability_key)

                        PARSE_ENUM_INTO_VALUE(capability_key->key)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEY_LEFT_SIDE_KEY)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEY_RIGHT_SIDE_KEY)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEY_CUSTOM_KEY)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEY_C_KEY)
                        PARSE_ENUM_END

                        PARSE_ENUM_INTO_VALUE(capability_key->key_type)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEYS_TYPE_TOUCH_SENSOR)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_KEYS_TYPE_BUTTON)
                        PARSE_ENUM_END

                        PARSE_ENUM_INTO_VALUE(capability_key->default_preset)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_AMBIENT_SOUND_CONTROL)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_VOLUME_CONTROL)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_PLAYBACK_CONTROL)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_VOICE_RECOGNITION)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_GOOGLE_ASSISTANT)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_AMAZON_ALEXA)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_TENCENT_XIAOWEI)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_NO_FUNCTION)
                        PARSE_ENUM_END

                        PARSE_BYTE_INTO_VALUE(capability_key->num_capability_presets)

                        PARSE_FOR_EACH_INTO_VALUE(
                                capability_key->num_capability_presets,
                                capability_key->capability_presets,
                                mdr_packet_system_assignable_settings_capability_preset_t,
                                capability_preset)

                            PARSE_ENUM_INTO_VALUE(capability_preset->preset)
                                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_AMBIENT_SOUND_CONTROL)
                                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_VOLUME_CONTROL)
                                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_PLAYBACK_CONTROL)
                                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_VOICE_RECOGNITION)
                                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_GOOGLE_ASSISTANT)
                                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_AMAZON_ALEXA)
                                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_TENCENT_XIAOWEI)
                                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_NO_FUNCTION)
                            PARSE_ENUM_END

                            PARSE_BYTE_INTO_VALUE(capability_preset->num_capability_actions)

                            PARSE_FOR_EACH_INTO_VALUE(
                                    capability_preset->num_capability_actions,
                                    capability_preset->capability_actions,
                                    mdr_packet_system_assignable_settings_capability_action_t,
                                    capability_action)

                                PARSE_ENUM_INTO_VALUE(capability_action->action)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_SINGLE_TAP)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_DOUBLE_TAP)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_TRIPLE_TAP)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_SINGLE_TAP_AND_HOLD)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_DOUBLE_TAP_AND_HOLD)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_LONG_PRESS_THEN_ACTIVATE)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_ACTION_LONG_PRESS_DURING_ACTIVATION)
                                PARSE_ENUM_END

                                PARSE_ENUM_INTO_VALUE(capability_action->function)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_NO_FUNCTION)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_NC_ASM_OFF)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_NC_OPTIMIZER)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_QUICK_ATTENTION)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_VOLUME_UP)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_VOLUME_DOWN)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_PLAY_PAUSE)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_NEXT_TRACK)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_PREVIOUS_TRACK)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_VOICE_RECOGNITION)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_GET_YOUR_NOTIFICATION)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_TALK_TO_GA)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_STOP_GA)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_VOICE_INPUT_CANCEL_AA)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_TALK_TO_TENCENT_XIAOWEI)
                                    PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_FUNCTION_CANCEL_VOICE_RECOGNITION)
                                PARSE_ENUM_END

                            PARSE_FOR_EACH_END

                        PARSE_FOR_EACH_END

                    PARSE_FOR_EACH_END

                    break;
            }

#undef PACKET_FIELD
            break;

        case MDR_PACKET_SYSTEM_GET_PARAM:
#define PACKET_FIELD system_get_param

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS)
            PARSE_ENUM_END

#undef PACKET_FIELD
            break;

        case MDR_PACKET_SYSTEM_RET_PARAM:
        case MDR_PACKET_SYSTEM_SET_PARAM:
        case MDR_PACKET_SYSTEM_NTFY_PARAM:
#define PACKET_FIELD system_ret_param

            PARSE_ENUM_INTO_PACKET(inquired_type)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE)
                PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS)
            PARSE_ENUM_END

            switch (packet->data.system_ret_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    PARSE_ENUM_INTO_PACKET(vibrator.setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_VIBRATOR_SETTING_TYPE_ON_OFF)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(vibrator.setting_value)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_VIBRATOR_SETTING_VALUE_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_VIBRATOR_SETTING_VALUE_ON)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    PARSE_ENUM_INTO_PACKET(power_saving_mode.setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_POWER_SAVING_MODE_SETTING_TYPE_ON_OFF)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(power_saving_mode.setting_value)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_POWER_SAVING_MODE_SETTING_VALUE_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_POWER_SAVING_MODE_SETTING_VALUE_ON)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    PARSE_ENUM_INTO_PACKET(control_by_wearing.setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_CONTROL_BY_WEARING_SETTING_TYPE_ON_OFF)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(control_by_wearing.setting_value)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_CONTROL_BY_WEARING_SETTING_VALUE_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_CONTROL_BY_WEARING_SETTING_VALUE_ON)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    PARSE_ENUM_INTO_PACKET(auto_power_off.parameter_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_PARAMETER_TYPE_ACTIVE_AND_SELECT_TIME_ID)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(auto_power_off.element_id)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_5_MIN)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_30_MIN)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_60_MIN)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_180_MIN)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_WHEN_REMOVED_FROM_EARS)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_DISABLE)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(auto_power_off.select_time_element_id)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_5_MIN)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_30_MIN)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_60_MIN)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_IN_180_MIN)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_WHEN_REMOVED_FROM_EARS)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_AUTO_POWER_OFF_ELEMENT_ID_POWER_OFF_DISABLE)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    PARSE_ENUM_INTO_PACKET(smart_talking_mode.setting_type)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_SETTING_TYPE_ON_OFF)
                    PARSE_ENUM_END
                    PARSE_ENUM_INTO_PACKET(smart_talking_mode.setting_value)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_SETTING_VALUE_OFF)
                        PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_SMART_TALKING_MODE_SETTING_VALUE_ON)
                    PARSE_ENUM_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    PARSE_BYTE_INTO_PACKET(assignable_settings.num_presets)

                    PARSE_FOR_EACH_IN_PACKET(
                            assignable_settings.num_presets,
                            assignable_settings.presets,
                            mdr_packet_system_assignable_settings_preset_t,
                            preset)
                        
                        PARSE_ENUM_INTO_VALUE(*preset)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_AMBIENT_SOUND_CONTROL)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_VOLUME_CONTROL)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_PLAYBACK_CONTROL)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_VOICE_RECOGNITION)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_GOOGLE_ASSISTANT)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_AMAZON_ALEXA)
                            PARSE_ENUM_FIELD(MDR_PACKET_SYSTEM_ASSIGNABLE_SETTINGS_PRESET_TENCENT_XIAOWEI)
                        PARSE_ENUM_END

                    PARSE_FOR_EACH_END

                    break;
            }

#undef PACKET_FIELD
            break;

        default:
            INVALID_FRAME
    }

    return packet;
}

static mdr_frame_t* mdr_packet_system_to_frame(mdr_packet_t* packet)
{
    WRITE_INIT(packet)
    int length;

    switch (packet->type)
    {
        case MDR_PACKET_SYSTEM_GET_CAPABILITY:
#define PACKET_FIELD system_get_capability

            WRITE_START(1)
            WRITE_FIELD(inquired_type)

            break;

#undef PACKET_FIELD

        case MDR_PACKET_SYSTEM_RET_CAPABILITY:
#define PACKET_FIELD system_ret_capability

            switch (packet->data.system_ret_capability.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    WRITE_START(2)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(vibrator.setting_type)

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    WRITE_START(2)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(power_saving_mode.setting_type)

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    WRITE_START(2)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(control_by_wearing.setting_type)

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    WRITE_START(2 + FIELD(auto_power_off.element_id_count))
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(auto_power_off.element_id_count)
                    WRITE_FOR_EACH_IN_PACKET(
                            auto_power_off.element_id_count,
                            auto_power_off.element_ids,
                            mdr_packet_system_auto_power_off_element_id_t,
                            element_id)

                        WRITE_BYTE(*element_id)

                    WRITE_FOR_EACH_END

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    WRITE_START(11)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(smart_talking_mode.setting_type)
                    WRITE_FIELD(smart_talking_mode.preview_type)
                    WRITE_FIELD(smart_talking_mode.detail_setting)
                    WRITE_FIELD(smart_talking_mode.detection_sensitivity_type)
                    WRITE_FIELD(smart_talking_mode.voice_focus_type)
                    WRITE_FIELD(smart_talking_mode.mode_out_time_type_t)
                    WRITE_FIELD(smart_talking_mode.timeouts[0])
                    WRITE_FIELD(smart_talking_mode.timeouts[1])
                    WRITE_FIELD(smart_talking_mode.timeouts[2])
                    WRITE_FIELD(smart_talking_mode.timeouts[3])

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    length = 2;

                    WRITE_FOR_EACH_IN_PACKET(
                            assignable_settings.num_capability_keys,
                            assignable_settings.capability_keys,
                            mdr_packet_system_assignable_settings_capability_key_t,
                            capability_key)

                        length += 4;

                        WRITE_FOR_EACH_IN_VALUE(
                            capability_key->num_capability_presets,
                            capability_key->capability_presets,
                            mdr_packet_system_assignable_settings_capability_preset_t,
                            capability_preset)

                            length += 2;

                            length += capability_preset->num_capability_actions
                                * 2;

                        WRITE_FOR_EACH_END

                    WRITE_FOR_EACH_END

                    WRITE_START(length)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(assignable_settings.num_capability_keys)

                    WRITE_FOR_EACH_IN_PACKET(
                            assignable_settings.num_capability_keys,
                            assignable_settings.capability_keys,
                            mdr_packet_system_assignable_settings_capability_key_t,
                            capability_key)

                        WRITE_BYTE(capability_key->key)
                        WRITE_BYTE(capability_key->key_type)
                        WRITE_BYTE(capability_key->default_preset)
                        WRITE_BYTE(capability_key->num_capability_presets)

                        WRITE_FOR_EACH_IN_VALUE(
                                capability_key->num_capability_presets,
                                capability_key->capability_presets,
                                mdr_packet_system_assignable_settings_capability_preset_t,
                                capability_preset)

                            WRITE_BYTE(capability_preset->preset)
                            WRITE_BYTE(capability_preset->num_capability_actions)

                            WRITE_FOR_EACH_IN_VALUE(
                                    capability_preset->num_capability_actions,
                                    capability_preset->capability_actions,
                                    mdr_packet_system_assignable_settings_capability_action_t,
                                    capability_action)

                                WRITE_BYTE(capability_action->action)
                                WRITE_BYTE(capability_action->function)

                            WRITE_FOR_EACH_END

                        WRITE_FOR_EACH_END

                    WRITE_FOR_EACH_END

                    break;
            }

            break;

#undef PACKET_FIELD

        case MDR_PACKET_SYSTEM_GET_PARAM:
#define PACKET_FIELD system_get_param

            WRITE_START(1)
            WRITE_FIELD(inquired_type)

#undef PACKET_FIELD

            break;

        case MDR_PACKET_SYSTEM_RET_PARAM:
        case MDR_PACKET_SYSTEM_SET_PARAM:
        case MDR_PACKET_SYSTEM_NTFY_PARAM:
#define PACKET_FIELD system_ret_param

            switch (packet->data.system_ret_param.inquired_type)
            {
                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_VIBRATOR:
                    WRITE_START(3)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(vibrator.setting_type)
                    WRITE_FIELD(vibrator.setting_value)

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_POWER_SAVING_MODE:
                    WRITE_START(3)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(power_saving_mode.setting_type)
                    WRITE_FIELD(power_saving_mode.setting_value)

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_CONTROL_BY_WEARING:
                    WRITE_START(3)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(control_by_wearing.setting_type)
                    WRITE_FIELD(control_by_wearing.setting_value)

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_AUTO_POWER_OFF:
                    WRITE_START(4)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(auto_power_off.parameter_type)
                    WRITE_FIELD(auto_power_off.element_id)
                    WRITE_FIELD(auto_power_off.select_time_element_id)

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_SMART_TALKING_MODE:
                    WRITE_START(3)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(smart_talking_mode.setting_type)
                    WRITE_FIELD(smart_talking_mode.setting_value)

                    break;

                case MDR_PACKET_SYSTEM_INQUIRED_TYPE_ASSIGNABLE_SETTINGS:
                    length = 2 + FIELD(assignable_settings.num_presets);

                    WRITE_START(length)
                    WRITE_FIELD(inquired_type)
                    WRITE_FIELD(assignable_settings.num_presets)

                    WRITE_FOR_EACH_IN_PACKET(
                            assignable_settings.num_presets,
                            assignable_settings.presets,
                            mdr_packet_system_assignable_settings_preset_t,
                            preset)

                        WRITE_BYTE(*preset)

                    WRITE_FOR_EACH_END

                    break;
            }

#undef PACKET_FIELD

            break;

        default:
            errno = MDR_E_INVALID_PACKET;
            return NULL;
    }

    return frame;
}

