#include "send_tui.h"

#include "tui.h"
#include "system.h"

#include <unistd.h>

static pthread_t sendThread;
static pthread_mutex_t* ncursesMutexPointer;

static void* SendTuiThread(void* args) {
    WINDOW* sendWindow;

    pthread_mutex_lock(ncursesMutexPointer);
    sendWindow = CreateNewNcursesWindow(LINES / 2, 0, LINES / 2, 0);
    pthread_mutex_unlock(ncursesMutexPointer);

    SendToServer("A");
    sleep(1);
    SendToServer("L 1-1");

    while (true) {
        pthread_mutex_lock(ncursesMutexPointer);
        DrawWindowTitle(sendWindow, "Send Window");
        wrefresh(sendWindow);
        pthread_mutex_unlock(ncursesMutexPointer);
    }

    return NULL;
}

void StartSendTui(pthread_mutex_t* ncursesMutex) {
    ncursesMutexPointer = ncursesMutex;
    pthread_create(&sendThread, NULL, SendTuiThread, NULL);
}

void WaitSendTui() {
    pthread_join(sendThread, NULL);
}