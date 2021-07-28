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

#ifndef __MDR_CONNECTION_H__
#define __MDR_CONNECTION_H__

#include "mdr/frame.h"

#include <bluetooth/bluetooth.h>

#include <poll.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define MDR_SERVICE_UUID "96CC203E-5068-46AD-B32D-E316F5E069BA"

/*
 * A connection to an MDR socket.
 *
 * This is a low-level interface which simply sends and receives frames.
 */
typedef struct mdr_connection mdr_connection_t;

/*
 * Open a new connection to an MDR socket.
 *
 * Returns NULL and sets errno on error.
 */
mdr_connection_t* mdr_connect(bdaddr_t addr, uint8_t channel);

/*
 * Create an MDR connection by wrapping an existing, connected socket.
 *
 * Returns NULL and sets errno on error.
 */
mdr_connection_t* mdr_connection_new(int sock);

int mdr_connection_get_socket(mdr_connection_t*);

bool mdr_connection_waiting_read(mdr_connection_t*);
bool mdr_connection_waiting_write(mdr_connection_t*);

/*
 * Close a connection to an MDR socket and free any associated resources.
 */
void mdr_connection_close(mdr_connection_t*);

/*
 * Free any resources allocated by the connection without closing the socket.
 */
void mdr_connection_free(mdr_connection_t*);

/*
 * Try to flush any buffered writes.
 *
 * Returns 0 on success, returns -1 and sets errno on error.
 */
int mdr_connection_flush_write(mdr_connection_t*);

/*
 * Read a single frame from the connection.
 *
 * The frame is allocated by this function and must be freed manually.
 */
mdr_frame_t* mdr_connection_read_frame(mdr_connection_t*);

/*
 * Write a frame to this connection.
 *
 * The frame must be allocated using malloc and ownership
 * is transferred to the function, which frees it.
 *
 * Returns
 *   -1 on error and sets errno
 *    0 on success and the entire frame was sent
 *    1 on success but the connection is non-blocking and
 *        only part of the frame was sent.
 *        mdr_connection_flush_write must be called until the entire
 *        frame can be sent.
 */
int mdr_connection_write_frame(mdr_connection_t*, mdr_frame_t*);

#endif /* __MDR_CONNETION_H__ */
