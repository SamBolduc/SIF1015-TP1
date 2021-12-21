#include "send_tui.h"

#include "tui.h"
#include "system.h"

#include <unistd.h>
#include <string.h>
#include <ctype.h>

static pthread_t sendThread;
static pthread_mutex_t* ncursesMutexPointer;

#define bound(a, min, max) {\
    if (a < min)            \
        a = min;            \
    if (a > max)            \
        a = max;            \
}                           \

static void* SendTuiThread(void* args) {
    NcursesWindow sendWindow;
    unsigned int currentLine = 2;
    char querryBuffer[256] = {0};
    int currentChar = 0, currentCharacterIndex = 0;
    bool running = true;


    pthread_mutex_lock(ncursesMutexPointer);
    sendWindow = CreateNewNcursesWindow("Send Window", LINES / 2, 0, LINES / 2, 0);
    nodelay(sendWindow.window, true);
    pthread_mutex_unlock(ncursesMutexPointer);

    while (running) {
        pthread_mutex_lock(ncursesMutexPointer);

        mvwprintw(sendWindow.window, currentLine, 2, "Please enter your querry: %s", querryBuffer);

        // Update querry buffer
        currentChar = wgetch(sendWindow.window);
        switch (currentChar) {
            case ERR:
            case '\0':
                break;
            
            case '\n':
                // Stop condition
                if ((tolower(querryBuffer[0]) == 'q') && (querryBuffer[1] == '\0'))
                    running = false;

                // Send querry
                SendToServer(querryBuffer);

                wmove(sendWindow.window, currentLine, 0);
                wclrtoeol(sendWindow.window);
                currentCharacterIndex = 0;

                mvwprintw(sendWindow.window, currentLine, 2, querryBuffer);
                currentLine++;

                memset(querryBuffer, 0, 256);

                DrawNcursesWindow(&sendWindow);
                break;

            case KEY_BACKSPACE:
            case 127:
            case '\b':
                wmove(sendWindow.window, currentLine, 0);
                wclrtoeol(sendWindow.window);
                currentCharacterIndex--;
                
                // TODO BOUND TO THE WINDOW
                bound(currentCharacterIndex, 0, 255);
                querryBuffer[currentCharacterIndex] = '\0';
                DrawNcursesWindow(&sendWindow);
                break;
            
            default:
                bound(currentCharacterIndex, 0, 255);
                querryBuffer[currentCharacterIndex] = currentChar;
                currentCharacterIndex++;
                break;
        }

        wrefresh(sendWindow.window);
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