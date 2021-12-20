#ifndef _SEND_TUI_H
#define _SEND_TUI_H

    #include <pthread.h>

    void StartSendTui(pthread_mutex_t* ncursesMutex);
    void WaitSendTui();

#endif