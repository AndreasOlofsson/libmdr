#include "mdr/packet/connect_support_function.h"

bool mdr_packet_support_function_contains(
        mdr_packet_connect_ret_support_function_t* support_functions,
        mdr_packet_support_function_type_t target)
{
    for (int i = 0; i < support_functions->num_function_types; i++)
    {
        if (support_functions->function_types[i] == target)
        {
            return true;
        }
    }

    return false;
}
