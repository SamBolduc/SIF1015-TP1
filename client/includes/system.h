#ifndef _SYSTEM_H
#define _SYSTEM_H

    #include <stddef.h>

    void ConnectToServer();
    void CloseConnection();
    void ReadFromServer(char* buffer, size_t length);
    void SendToServer(char* buffer);

#endif