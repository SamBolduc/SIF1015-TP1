#include "receive_tui.h"

#include "tui.h"
#include "system.h"

#include <stdbool.h>
#include <string.h>

static pthread_t receiveThread;
static pthread_mutex_t* ncursesMutexPointer;

static void* ReceiveTuiThread(void* args) {
    NcursesWindow receiveWindow;
    char buffer[1024] = {0};
    char* line = NULL;
    unsigned int currentLine = 1;

    pthread_mutex_lock(ncursesMutexPointer);
    receiveWindow = CreateNewNcursesWindow("Receive Window", 0, 0, LINES / 2, 0);
    scrollok(receiveWindow.window, true);
    pthread_mutex_unlock(ncursesMutexPointer);

    while (true) {
        pthread_mutex_lock(ncursesMutexPointer);

        ReadFromServer(buffer, 1024);
        if (buffer[0]) {
            line = strtok(buffer, "\n\0");
            do {
                currentLine++;
                // Scroll routine
                if (currentLine >= LINES / 2 - 1) {
                    wscrl(receiveWindow.window, 5);
                    currentLine -= 5;
                    wmove(receiveWindow.window, currentLine, 0);
                    wclrtoeol(receiveWindow.window);
                }
                mvwprintw(receiveWindow.window, currentLine, 2, line);
            } while ((line = strtok(NULL, "\n")));
            memset(buffer, 0, 1024);
            DrawNcursesWindow(&receiveWindow);
        }
        pthread_mutex_unlock(ncursesMutexPointer);
    }

    return NULL;
}

void StartReciveTui(pthread_mutex_t* ncursesMutex) {
    ncursesMutexPointer = ncursesMutex;
    pthread_create(&receiveThread, NULL, ReceiveTuiThread, NULL);
}

void CancelReceiveTui() {
    pthread_cancel(receiveThread);
}