#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define VM_SEGMENT_SIZE 65536

typedef struct linkedList linkedList;

typedef struct infoVM infoVM;
typedef struct tasks tasks;

struct linkedList {
	void* data;
	linkedList* next;
};

struct infoVM {
	unsigned char  busy;
	unsigned short *ptrDebutVM;
};
	
void cls(void);
void error(const int exitcode, const char * message);

linkedList *findItem(const int no);
linkedList *findPrev(const int no);

linkedList* appendToLinkedList(linkedList** List, void* newData, size_t dataSize);
void 		addItem();
void 		removeItem(const int noVM);
void 		listItems(const int start, const int end);