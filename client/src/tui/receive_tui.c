#include "receive_tui.h"

#include "tui.h"

#include <stdbool.h>

static pthread_t receiveThread;
static pthread_mutex_t* ncursesMutexPointer;

static void* ReceiveTuiThread(void* args) {
    WINDOW* receiveWindow;

    pthread_mutex_lock(ncursesMutexPointer);
    receiveWindow = CreateNewNcursesWindow(0, 0, LINES / 2, 0);
    pthread_mutex_unlock(ncursesMutexPointer);

    while (true) {
        pthread_mutex_lock(ncursesMutexPointer);
        DrawWindowTitle(receiveWindow, "Receive Window");
        wrefresh(receiveWindow);
        pthread_mutex_unlock(ncursesMutexPointer);
    }

    return NULL;
}

void StartReciveTui(pthread_mutex_t* ncursesMutex) {
    ncursesMutexPointer = ncursesMutex;
    pthread_create(&receiveThread, NULL, ReceiveTuiThread, NULL);
}

void WaitReceiveTui() {
    pthread_join(receiveThread, NULL);
}