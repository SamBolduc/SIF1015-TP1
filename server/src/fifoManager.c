/*
    Copyright (C) 2021 Killian RAIMBAUD [Asayu] (killian.rai@gmail.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "fifoManager.h"
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum {
    pathNotFound = 1,
    noFifoProvided,
    unableToCreateFifo,
    unableToOpenFifo,
    unableToCloseFifo
} fifoErrorCodes;

int openFifo(char* path, fifoFileDescriptor* fifo, fifoOpenMode flags) {
    if (!path)
        return -pathNotFound;
    if (!fifo)
        return -noFifoProvided;

    // Create fifo if necessary
    if (access(path, F_OK) == -1) {
        if (mkfifo(path, 0777) == -1)
            return -unableToCreateFifo;
    }

    if ((*fifo = open(path, flags)) == -1)
        return -unableToOpenFifo;
    return 0;    
}

int closeFifo(fifoFileDescriptor* fifo) {
    if (!fifo)
        return -noFifoProvided;
    if (close(*fifo) == -1)
        return -unableToCloseFifo;
    return 0;
}