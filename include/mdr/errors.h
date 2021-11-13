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

#ifndef __MDR_ERRORS_H__
#define __MDR_ERRORS_H__

#define MDR_E_INVALID_FRAME     -1
#define MDR_E_INVALID_PACKET    -2
#define MDR_E_UNEXPECTED_PACKET -3
#define MDR_E_NO_ACK            -4
#define MDR_E_NO_REPLY          -5
#define MDR_E_CLOSED            -6
#define MDR_E_NOT_SUPPORTED     -7

#endif /* __MDR_ERRORS_H__ */
