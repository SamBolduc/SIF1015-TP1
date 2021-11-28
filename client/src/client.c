// Here's the client, client.c. The first part of this program opens the server FIFO,
// if it already exists, as a file. It then gets its own process ID, which forms some
// of the data that will be sent to the server. The client FIFO is created, ready for
// the next section.

#define _GNU_SOURCE
int PARENT_X;
int PARENT_Y;

#include "client.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <ncurses.h>

void DrawWindowTitle(WINDOW *win, char* title) {
    int middle_position = (((PARENT_X / 2) - 3) / 2) - 6;
    mvwprintw(win, 0, middle_position, title);
}

void DrawBorders(WINDOW *screen) {
    int x, y, i;
    
    getmaxyx(screen, y, x);
    
    // 4 corners
    mvwprintw(screen, 0, 0, "+");
    mvwprintw(screen, y - 1, 0, "+");
    mvwprintw(screen, 0, x - 1, "+");
    mvwprintw(screen, y - 1, x - 1, "+");
    
    // Sides
    for (i = 1; i < (y - 1); i++) {
        mvwprintw(screen, i, 0, "|");
        mvwprintw(screen, i, x - 1, "|");
    }
    
    // Top and bottom
    for (i = 1; i < (x - 1); i++) {
        mvwprintw(screen, 0, i, "-");
        mvwprintw(screen, y - 1, i, "-");
    }

}

void DrawCompleteWindows(WINDOW *w1, WINDOW *w2){
    DrawBorders(w1);
    DrawBorders(w2);
    DrawWindowTitle(w1, "Transmit [TX]");
    DrawWindowTitle(w2, "Receive [RX]");
    wrefresh(w1);
    wrefresh(w2);
}

void ConnectToServer(pid_t clientPID, int server_fifo_fd, int client_fifo_fd, WINDOW *w_tx, WINDOW *w_rx) {
    const unsigned int maxCommandLength = 100;
    char commandBuffer[maxCommandLength];

    mvwprintw(w_tx, 5, 1, "Connecting to server\n");
    DrawCompleteWindows(w_tx, w_rx);

    int new_y, new_x;
    int linesCounter = 7;
    while(true) { // Main loop
        commandBuffer[0] = '\0';
        getmaxyx(stdscr, new_y, new_x);
        
        if (new_y != PARENT_Y || new_x != PARENT_X) {
            PARENT_X = new_x;
            PARENT_Y = new_y;
            
            wresize(w_tx, new_y, new_x);
            wresize(w_rx, new_y, new_x/2);
            
            wclear(stdscr);
            wclear(w_tx);
            wclear(w_rx);
            
            DrawBorders(w_tx);
            DrawBorders(w_rx);
        }
        
        // Draw to our windows
        int middle_position = ((PARENT_X / 2) - 3) / 2 - 6;
        mvwprintw(w_tx, 0, middle_position, "Transmit [TX]");
        mvwprintw(w_rx, 0, middle_position, "Receive [RX]");
        mvwprintw(w_tx, linesCounter, 1, "Requested command (Press Enter to send): %s", commandBuffer);
        mvwprintw(w_rx, 1, 1, "Server response...");
        
        // refresh each window
        wrefresh(w_tx);
        wrefresh(w_rx);
        
        wgetstr(w_tx, commandBuffer);
        if (commandBuffer[0] == 'q')
            break;
        dprintf(server_fifo_fd, "%u %s", clientPID, commandBuffer);
    }


}
int main() {
    //set up initial windows
    initscr();
    curs_set(FALSE);

    getmaxyx(stdscr, PARENT_Y, PARENT_X);

    WINDOW *w_tx = newwin(PARENT_Y, (PARENT_X / 2) - 3, 0, 0);
    WINDOW *w_rx = newwin(PARENT_Y, (PARENT_X / 2) - 3, 0, (PARENT_X / 2) + 2);

    DrawCompleteWindows(w_tx, w_rx);

    //set up initial connection
    pid_t clientPID;
    int server_fifo_fd, client_fifo_fd;
    char* client_fifo = NULL;
    
    // Open server fifo
    mvwprintw(w_tx, 1, 1, "Opening Server FIFO ...\n");
    DrawCompleteWindows(w_tx, w_rx);

    server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Sorry, no server\n");
        refresh();
        exit(EXIT_FAILURE);
    }
    mvwprintw(w_tx, 1, 40, "Success !\n");
    DrawCompleteWindows(w_tx, w_rx);
    
    // Create client fifo
    mvwprintw(w_tx, 2, 1, "Creating Client FIFO ...\n");
    clientPID = getpid();
    asprintf(&client_fifo, CLIENT_FIFO_NAME, clientPID);
    if (mkfifo(client_fifo, 0777) == -1) {
        fprintf(stderr, "Sorry, can't make %s\n", client_fifo);
        refresh();
        goto Error;
    }
    mvwprintw(w_tx, 2, 40, "Success !\n");
    DrawCompleteWindows(w_tx, w_rx);
    
    // Sending pid to server
    mvwprintw(w_tx, 3, 1, "Sending PID ...\n");
    DrawCompleteWindows(w_tx, w_rx);
    dprintf(server_fifo_fd, "%u\n", clientPID);
    mvwprintw(w_tx, 3, 40, "Success !\n");
    DrawCompleteWindows(w_tx, w_rx);

    // Open client fifo
    mvwprintw(w_tx, 4, 1, "Oppening Client FIFO ...\n");
    DrawCompleteWindows(w_tx, w_rx);
    if ((client_fifo_fd = open(client_fifo, O_RDONLY)) == -1) {
        goto Error;
    }
    mvwprintw(w_tx, 4, 40, "Success !\n");
    DrawCompleteWindows(w_tx, w_rx);

    ConnectToServer(clientPID, server_fifo_fd, client_fifo_fd, w_tx, w_rx);

    close(server_fifo_fd);
    unlink(client_fifo);

    if (client_fifo)
        free(client_fifo);

    exit(EXIT_SUCCESS);

Error:

    if (client_fifo)
        free(client_fifo);

    exit(EXIT_FAILURE);
}

