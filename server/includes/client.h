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

#ifndef _CLIENT_H
#define _CLIENT_H

    #include "fifoManager.h"
    #include "linkedList.h"
    #include "virtualMachine.h"

    typedef LinkedList clientList;

    typedef struct {
        __pid_t clientPID;
        fifoFileDescriptor clientFifo;
        unsigned int nbVM;
        VMList* vms;
    } ClientContext;

    void addItem(ClientContext* client);
    void removeItem(ClientContext* client, const unsigned int noVM);
    void listItems(ClientContext* client, const int start, const int end);
    int  dispatchJob(ClientContext* client, int noVM, char* sourcefname);

#endif