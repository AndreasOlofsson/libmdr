#include "mdr/packet/eqebb.h"

#include <stdlib.h>

const char* mdr_packet_eqebb_get_preset_name(
        mdr_packet_eqebb_eq_preset_id_t preset_id)
{
    switch (preset_id)
    {
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_OFF:
            return "Off";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_ROCK:
            return "Rock";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_POP:
            return "Pop";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_JAZZ:
            return "Jazz";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_DANCE:
            return "Dance";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_EDM:
            return "EDM";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_R_AND_B_HIP_HOP:
            return "R&B & Hip Hop";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_ACOUSTIC:
            return "Acoustic";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_BRIGHT:
            return "Bright";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_EXCITED:
            return "Excited";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_MELLOW:
            return "Mellow";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_RELAXED:
            return "Relaxed";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_VOCAL:
            return "Vocal";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_TREBLE:
            return "Treble";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_BASS:
            return "Bass";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_SPEECH:
            return "Speech";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_CUSTOM:
            return "Custom";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING1:
            return "User Setting 1";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING2:
            return "User Setting 2";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING3:
            return "User Setting 3";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING4:
            return "User Setting 4";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_USER_SETTING5:
            return "User Setting 5";
        case MDR_PACKET_EQEBB_EQ_PRESET_ID_UNSPECIFIED:
            return "Unspecified";
        default:
            return NULL;
    }
}
