#ifndef _TUI_H
#define _TUI_H

    #include <ncurses.h>

    typedef struct {
        WINDOW* window;
        char title[100];
    } NcursesWindow;

    void          DrawNcursesWindow(NcursesWindow* window);
    NcursesWindow CreateNewNcursesWindow(char* title, unsigned int Y, unsigned int X, unsigned int H, unsigned int W);
    void          StartTui();
    void          WaitTui();

#endif