#ifndef _TUI_H
#define _TUI_H

    #include <ncurses.h>

    WINDOW* CreateNewNcursesWindow(unsigned int Y, unsigned int X, unsigned int H, unsigned int W);
    void    DrawWindowTitle(WINDOW* window, char* title);
    void    StartTui();
    void    WaitTui();

#endif