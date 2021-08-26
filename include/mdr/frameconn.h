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

#ifndef __MDR_FRAMECONN_H__
#define __MDR_FRAMECONN_H__

#include "mdr/frame.h"

#include <bluetooth/bluetooth.h>

#include <stdbool.h>
#include <stdint.h>

#define MDR_SERVICE_UUID "96CC203E-5068-46AD-B32D-E316F5E069BA"

/*
 * A frame-connection to an MDR socket.
 *
 * This is a low-level interface which simply sends and receives frames.
 *
 * See `mdr_packetconn_t` or `mdr_device_t` for higher-level interfaces.
 */
typedef struct mdr_frameconn mdr_frameconn_t;

/*
 * Open a new frame-connection to an MDR socket.
 *
 * Returns NULL and sets errno on error.
 */
mdr_frameconn_t* mdr_frameconn_connect(bdaddr_t addr, uint8_t channel);

/*
 * Create a frame-connection by wrapping an existing, connected socket.
 *
 * Returns NULL and sets errno on error.
 */
mdr_frameconn_t* mdr_frameconn_new(int sock);

/*
 * Get the socket associated with this frame-connection.
 */
int mdr_frameconn_get_socket(mdr_frameconn_t*);

/*
 * Checks if the frameconn wants to write data.
 *
 * This is intended to be used to poll the
 * underlying socket if it is set to non-blocking.
 */
bool mdr_frameconn_waiting_write(mdr_frameconn_t*);

/*
 * Close a frame-connection and free any associated resources.
 */
void mdr_frameconn_close(mdr_frameconn_t*);

/*
 * Free any resources allocated by the frame-connection
 * without closing the socket.
 */
void mdr_frameconn_free(mdr_frameconn_t*);

/*
 * Try to flush any buffered writes.
 *
 * Returns 0 on success, returns -1 and sets errno on error.
 */
int mdr_frameconn_flush_write(mdr_frameconn_t*);

/*
 * Read a single frame from the connection.
 *
 * The returned frame is allocated using malloc and is caller-freed.
 *
 * Returns NULL and sets errno on error.
 */
mdr_frame_t* mdr_frameconn_read_frame(mdr_frameconn_t*);

/*
 * Write a frame to this connection.
 *
 * The frame must be allocated using malloc and is callee-freed.
 *
 * Returns
 *   -1 on error and sets errno
 *    0 on success
 */
int mdr_frameconn_write_frame(mdr_frameconn_t*, mdr_frame_t*);

#endif /* __MDR_FRAMECONN_H__ */
