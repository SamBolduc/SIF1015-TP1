#ifndef _IOMANAGER_H
#define _IOMANAGER_H

    #include <stddef.h>

    typedef struct IOClient IOClient;

    void      IOInit();
    IOClient* IOGetClient();
    void      IOCloseClient(IOClient* client);
    int       IORead(IOClient* client, char* buffer, size_t length);
    void      IOWrite(IOClient* client, char* format, ...);

#endif