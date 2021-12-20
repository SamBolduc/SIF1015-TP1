#include "tui.h"

#include "receive_tui.h"
#include "send_tui.h"

#include <pthread.h>

static pthread_mutex_t ncursesMutex;

void InitNcurses() {
    initscr();
    pthread_mutex_init(&ncursesMutex, NULL);
}

void CloseNcurses() {
    endwin();
    pthread_mutex_destroy(&ncursesMutex);
}

WINDOW* CreateNewNcursesWindow(unsigned int Y, unsigned int X, unsigned int H, unsigned int W) {
    WINDOW* newNcursesWindow = NULL;

    newNcursesWindow = newwin(H, W, Y, X);
    box(newNcursesWindow, 0, 0);

    return newNcursesWindow;
}

void DrawWindowTitle(WINDOW* window, char* title) {
    mvwprintw(window, 0, 2, "| %s |", title);
}

/*
    StartTui() : Démare l'interface du terminal
*/
void StartTui() {
    InitNcurses();

    StartReciveTui(&ncursesMutex);
    StartSendTui(&ncursesMutex);
}

void WaitTui() {
    WaitReceiveTui();
    WaitSendTui();

    CloseNcurses();
}