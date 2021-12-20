#include "receive_tui.h"

#include "tui.h"
#include "system.h"

#include <stdbool.h>
#include <string.h>

static pthread_t receiveThread;
static pthread_mutex_t* ncursesMutexPointer;

static void* ReceiveTuiThread(void* args) {
    WINDOW* receiveWindow;
    char buffer[1024] = {0};
    char* line = NULL;
    unsigned int currentLine = 1;

    pthread_mutex_lock(ncursesMutexPointer);
    receiveWindow = CreateNewNcursesWindow(0, 0, LINES / 2, 0);
    pthread_mutex_unlock(ncursesMutexPointer);

    while (true) {
        pthread_mutex_lock(ncursesMutexPointer);
        DrawWindowTitle(receiveWindow, "Receive Window");

        ReadFromServer(buffer, 1024);
        if (buffer[0]) {
            line = strtok(buffer, "\n");
            do {
                mvwprintw(receiveWindow, currentLine, 2, line);
                currentLine++;
            } while ((line = strtok(NULL, "\n")));
            currentLine++;
            memset(buffer, 0, 1024);
        }
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