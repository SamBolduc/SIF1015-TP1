#include "tui.h"

#include "receive_tui.h"
#include "send_tui.h"

#include <pthread.h>
#include <string.h>

static pthread_mutex_t ncursesMutex;

void InitNcurses() {
    initscr();
    raw();
    keypad(stdscr, true);
    noecho();
    pthread_mutex_init(&ncursesMutex, NULL);
}

void CloseNcurses() {
    endwin();
    pthread_mutex_destroy(&ncursesMutex);
}

static void DrawWindowTitle(WINDOW* window, char* title) {
    mvwprintw(window, 0, 2, "| %s |", title);
}

void DrawNcursesWindow(NcursesWindow* window) {
    box(window->window, 0, 0);
    DrawWindowTitle(window->window, window->title);
    wrefresh(window->window);
}

NcursesWindow CreateNewNcursesWindow(char* title, unsigned int Y, unsigned int X, unsigned int H, unsigned int W) {
    NcursesWindow newNcursesWindow = {0};

    newNcursesWindow.window = newwin(H, W, Y, X);
    strcpy(newNcursesWindow.title, title);

    DrawNcursesWindow(&newNcursesWindow);

    return newNcursesWindow;
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
    WaitSendTui();
    CancelReceiveTui();

    CloseNcurses();
}