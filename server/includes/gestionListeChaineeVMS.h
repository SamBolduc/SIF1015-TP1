#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#include "types.h"

#define VM_SEGMENT_SIZE 65536

typedef struct linkedList linkedList;
typedef struct infoVM infoVM;

struct linkedList {
	void* data;
	linkedList* next;
};

struct infoVM {
	unsigned int   noVM;
	unsigned char  busy;
	unsigned short *ptrDebutVM;
	pthread_t vmProcess;
	linkedList* binaryList;
	bool kill;
};
	
void cls(void);

linkedList *findItem(const int no);
linkedList **findPrev(const int no);

linkedList* appendToLinkedList(linkedList** List, void* newData, size_t dataSize);
void 		deleteLinkedListNode(linkedList** node);
void 		addItem();
void 		removeItem(const int noVM);
void 		listItems(const int start, const int end);
