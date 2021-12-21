#ifndef _RECEIVE_TUI_H
#define _RECEIVE_TUI_H

    #include <pthread.h>

    void StartReciveTui(pthread_mutex_t* ncursesMutex);
    void CancelReceiveTui();

#endif