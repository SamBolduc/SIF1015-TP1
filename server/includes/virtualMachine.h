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

#ifndef _VIRTUALMACHINE_H
#define _VIRTUALMACHINE_H

    #include <pthread.h>
    #include <stdbool.h>
    #include "linkedList.h"

    #define VM_SEGMENT_SIZE 65536

    typedef LinkedList VMList;
    typedef struct {
        pthread_mutex_t* consoleState;
        //fifoFileDescriptor* console;
        unsigned int   noVM;
        unsigned char  busy;
        unsigned short *ptrDebutVM;
        pthread_t vmProcess;
        LinkedList* binaryList;
        bool kill;
    } VirtualMachine;

    void* virtualMachine(void* args);

#endif