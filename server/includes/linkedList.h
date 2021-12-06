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

#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

    #include <unistd.h> /* size_t */

    typedef struct LinkedList LinkedList;
    struct LinkedList{
        void* data;
        LinkedList* next;
    };

    LinkedList*  AppendToLinkedList(LinkedList** List, void* newData, size_t dataSize);
    LinkedList*  AppendRefToLinkedList(LinkedList** List, void* newDataRef);
    void         FreeLinkedList(LinkedList** List);
    LinkedList** SearchNodeInList(LinkedList** list, LinkedList* node);
    LinkedList** SearchDataInList(LinkedList** list, void* data);
    void         DeleteLinkedListNode(LinkedList** node);

#endif