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

#include "linkedList.h"

#include <stdlib.h>
#include <string.h>

/* Add a new node at the end of the list pointer and returns its address */
LinkedList* AppendToLinkedList(LinkedList** List, void* newData, size_t dataSize) {
    if (!List)
        return NULL;

    while (*List)
        List = &(*List)->next;
    (*List) = (LinkedList*)malloc(sizeof(LinkedList));
    (*List)->data = malloc(dataSize);
    (*List)->next = NULL;

    memcpy((*List)->data, newData, dataSize);

    return *List;
}

LinkedList* AppendRefToLinkedList(LinkedList** List, void* newDataRef) {
    if (!List)
        return NULL;

    while (*List)
        List = &(*List)->next;
    (*List) = (LinkedList*)malloc(sizeof(LinkedList));
    (*List)->data = newDataRef;
    (*List)->next = NULL;

    return *List;
}

void FreeLinkedList(LinkedList** List){
    if (List){
        if (*List){
            if ((*List)->next){
                FreeLinkedList(&(*List)->next);
            }
            free(*List);
            *List = NULL;
        }
    }
}

LinkedList** SearchNodeInList(LinkedList** list, LinkedList* node) {
    if (!list)
        return NULL;
    while (*list) {
        if ((*list) == node)
            return list;
        list = &(*list)->next;
    }
    return NULL;
}

LinkedList** SearchDataInList(LinkedList** list, void* data) {
    if (!list)
        return NULL;
    while (*list) {
        if ((*list)->data == data)
            return list;
        list = &(*list)->next;
    }
    return NULL;
}

void DeleteLinkedListNode(LinkedList** node){
    LinkedList* nextNode = NULL;
    if (!node)
        return;
    nextNode = (*node)->next;
    free((*node));
    *node = nextNode;
}