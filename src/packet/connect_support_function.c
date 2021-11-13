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
