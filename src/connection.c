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

#include "mdr/connection.h"

#include "mdr/errors.h"

#include <bluetooth/rfcomm.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

#ifndef FRAME_BUF_SIZE
#define FRAME_BUF_SIZE 8192
#endif

struct mdr_connection
{
    int sock;
    uint8_t read_buf[FRAME_BUF_SIZE];
    size_t read_buf_len;
    bool read_started;

    uint8_t write_buf[FRAME_BUF_SIZE];
    size_t write_buf_len;

    bool non_blocking;
};

mdr_connection_t* mdr_connect(bdaddr_t addr, uint8_t channel)
{
    mdr_connection_t* connection = malloc(sizeof(struct mdr_connection));
    if (connection == NULL) return NULL;

    int sock;
    struct sockaddr_rc sock_addr;

    sock_addr.rc_family = AF_BLUETOOTH;
    sock_addr.rc_channel = channel;
    sock_addr.rc_bdaddr = addr;

    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock < 0) return NULL;

    if (connect(sock,
                (const struct sockaddr*) &sock_addr,
                sizeof(struct sockaddr_rc)) < 0)
    {
        return NULL;
    }

    connection->sock = sock;
    connection->read_buf_len = 0;
    connection->read_started = false;
    connection->write_buf_len = 0;
    connection->non_blocking = false;

    return connection;
}

mdr_connection_t* mdr_connection_new(int sock)
{
    mdr_connection_t *connection;

    connection = malloc(sizeof(struct mdr_connection));
    if (connection == NULL) return NULL;

    connection->sock = sock;

    connection->read_buf_len = 0;
    connection->read_started = false;
    connection->write_buf_len = 0;
    connection->non_blocking = fcntl(sock, F_GETFD) & O_NONBLOCK;

    return connection;
}

int mdr_connection_get_socket(mdr_connection_t* connection)
{
    return connection->sock;
}

bool mdr_connection_waiting_read(mdr_connection_t* connection)
{
    return connection->read_buf_len < FRAME_BUF_SIZE;
}

bool mdr_connection_waiting_write(mdr_connection_t* connection)
{
    return connection->write_buf_len > 0;
}

void mdr_connection_close(mdr_connection_t* connection)
{
    close(connection->sock);
    free(connection);
}

void mdr_connection_free(mdr_connection_t* connection)
{
    free(connection);
}

int mdr_connection_flush_write(mdr_connection_t* connection)
{
    size_t i = 0;
    while (i < connection->write_buf_len)
    {
        int bytes_written;
write_bytes:
        bytes_written = write(connection->sock,
                              &connection->write_buf[i],
                              connection->write_buf_len - i);
        if (bytes_written < 0)
        {
            if (errno == EINTR)
            {
                goto write_bytes;
            }
            if (i > 0)
            {
                memmove(connection->write_buf,
                        &connection->write_buf[i],
                        connection->read_buf_len - i);
                connection->write_buf_len -= i;
            }
            return -1;
        }
    }

    connection->write_buf_len = 0;
    return 0;
}

#define FRAME_START_BYTE  ((uint8_t) 0x3e)
#define FRAME_ESCAPE_BYTE ((uint8_t) 0x3d)
#define FRAME_END_BYTE    ((uint8_t) 0x3c)
#define FRAME_ESCAPE_MASK ((uint8_t) 0x10)

/*
 * Attempt to to unescape and read a frame.
 *
 * The parameter`escaped` is the bytes between the start and end bytes.
 *
 * Returns a pointer to an allocated frame or returns NULL and sets
 * errno to MDR_E_INVALID_FRAME if the frame is invalid.
 */
static mdr_frame_t* mdr_connection_unescape_frame(uint8_t* escaped,
                                                  size_t escaped_len)
{
    uint8_t* frame_bytes = malloc(escaped_len);

    size_t read, write = 0;
    for (read = 0; read < escaped_len; read++, write++)
    {
        uint8_t b = escaped[read];

        if (b == FRAME_ESCAPE_BYTE && read + 1 <escaped_len)
        {
            read++;
            frame_bytes[write] = escaped[read] | FRAME_ESCAPE_MASK;
        }
        else
        {
            frame_bytes[write] = b;
        }
    }

    if (write < MDR_FRAME_EMPTY_LEN)
    {
        errno = MDR_E_INVALID_FRAME;
        return NULL;
    }

    mdr_frame_t* frame = (mdr_frame_t*) frame_bytes;
    frame->payload_length = ntohl(frame->payload_length);
    size_t frame_len = MDR_FRAME_EMPTY_LEN + frame->payload_length;

    if (write < frame_len)
    {
        errno = MDR_E_INVALID_FRAME;
        return NULL;
    }

    // Shrink allocation if possible
    frame_bytes = realloc(frame_bytes,
                          MDR_FRAME_EMPTY_LEN + frame->payload_length);

    return (mdr_frame_t*) frame_bytes;
}

/*
 * Reads a frame from the read buffer.
 *
 * Returns a frame if a valid frame is found in the read buffer,
 * otherwise returns NULL.
 * If the buffer contained an invalid frame errno is set
 * to MDR_E_INVALID_FRAME, otherwise it is set to 0.
 */
static mdr_frame_t* mdr_connection_next_frame(mdr_connection_t* connection)
{
    for (size_t i = 0; i < connection->read_buf_len; i++)
    {
        if (connection->read_buf[i] == FRAME_START_BYTE)
        {
            memmove(connection->read_buf,
                    &connection->read_buf[i+1],
                    connection->read_buf_len - i+1);
            connection->read_buf_len -= i+1;
            connection->read_started = true;
            i = -1;
        }
        else if (connection->read_buf[i] == FRAME_END_BYTE
              && connection->read_started)
        {
            mdr_frame_t* frame =
                mdr_connection_unescape_frame(connection->read_buf, i);

            memmove(connection->read_buf,
                    &connection->read_buf[i+1],
                    connection->read_buf_len - i+1);
            connection->read_buf_len -= i+1;
            connection->read_started = false;

            if (frame != NULL) return frame;
        }
    }

    if (connection->read_buf_len >= FRAME_BUF_SIZE)
    {
        // Buffer is full and no start or end can be read from it.
        // Clear the buffer and start over.
        connection->read_buf_len = 0;
        connection->read_started = false;
    }

    errno = 0;
    return NULL;
}

mdr_frame_t* mdr_connection_read_frame(mdr_connection_t* connection)
{
    while (1)
    {
        mdr_frame_t* frame = mdr_connection_next_frame(connection);
        if (frame != NULL) return frame;

        if (errno != 0)
        {
            return NULL;
        }

        int bytes_read;
read_bytes:
        bytes_read = read(connection->sock,
                          &connection->read_buf[connection->read_buf_len],
                          FRAME_BUF_SIZE - connection->read_buf_len);
        if (bytes_read < 0)
        {
            if (errno == EINTR)
            {
                goto read_bytes;
            }
            return NULL;
        }

#ifdef __DEBUG
        fprintf(stderr, "read %d bytes\n", bytes_read);
        for (int i = 0; i < bytes_read; i++)
        {
            fprintf(stderr, "%02x ",
                    connection->read_buf[connection->read_buf_len + i]);
        }
        fprintf(stderr, "\n");
#endif

        connection->read_buf_len += bytes_read;
    }
}

static uint8_t* mdr_connection_escape_frame(mdr_frame_t* frame,
                                            size_t* escaped_len)
{
    size_t frame_len = 7 + frame->payload_length;

    frame->payload_length = htonl(frame->payload_length);

    uint8_t* frame_bytes = (uint8_t*) frame;

    size_t allocated = 2 + (frame_len + 2) * 9 / 8;
    uint8_t* escaped = malloc(allocated);
    if (escaped == NULL) return NULL;

#define ENSURE_ESCAPED_BUFFER(n) \
    if (write + (n - 1) >= allocated) \
    { \
        allocated = (allocated + 2) * 9 / 8; \
        escaped = realloc(escaped, allocated); \
        if (escaped == NULL) return NULL; \
    }

    size_t read, write = 0;

    escaped[write] = FRAME_START_BYTE;
    write++;

    for (read = 0; read < frame_len; read++, write++)
    {
        uint8_t b = frame_bytes[read];

        switch (b)
        {
            case FRAME_START_BYTE:
            case FRAME_ESCAPE_BYTE:
            case FRAME_END_BYTE:
                ENSURE_ESCAPED_BUFFER(2);

                escaped[write] = FRAME_ESCAPE_BYTE;
                write++;
                escaped[write] = b & ~FRAME_ESCAPE_MASK;

                break;

            default:
                ENSURE_ESCAPED_BUFFER(1);

                escaped[write] = b;
                break;
        }
    }

    ENSURE_ESCAPED_BUFFER(1);

    escaped[write] = FRAME_END_BYTE;
    write++;

    *escaped_len = write;
    escaped = realloc(escaped, *escaped_len);

    return escaped;
}

int mdr_connection_write_frame(mdr_connection_t* connection,
                               mdr_frame_t* frame)
{
    // Try to flush the buffer to free up some room
    // for the new frame if needed.
    {
        int result = mdr_connection_flush_write(connection);
        if (result < 0 && !(errno == EAGAIN || errno == EINVAL))
        {
            return -1;
        }
    }

    size_t escaped_len;
    uint8_t* escaped = mdr_connection_escape_frame(frame, &escaped_len);
    free(frame);
    if (escaped == NULL) return -1;

    if (connection->non_blocking)
    {
        if (FRAME_BUF_SIZE - connection->write_buf_len < escaped_len)
        {
            // It may be possible to write some bytes and buffer the rest
            // but it's not possible to know if enough bytes can be sent
            // right away. So instead EWOULDBLOCK is returned.
            free(escaped);
            errno = EWOULDBLOCK;
            return -1;
        }
    }

    size_t written = 0;
    while (written < escaped_len)
    {
        int bytes_written;
write_bytes:
        bytes_written = write(connection->sock,
                              &escaped[written],
                              escaped_len - written);
        if (bytes_written < 0)
        {
            if (errno == EINTR)
            {
                goto write_bytes;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Write blocked, buffer the remaining data and exit.
                memcpy(&connection->write_buf[connection->write_buf_len],
                       &escaped[written],
                       escaped_len - written);
                connection->write_buf_len += escaped_len - written;
            }
            free(escaped);
            return -1;
        }

#ifdef __DEBUG
        fprintf(stderr, "wrote %d bytes\n", bytes_written);
        for (int i = 0; i < bytes_written; i++)
        {
            fprintf(stderr, "%02x ", escaped[written + i]);
        }
        fprintf(stderr, "\n");
#endif

        written += bytes_written;
    }

    free(escaped);

    return 0;
}
