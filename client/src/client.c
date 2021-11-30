// Here's the client, client.c. The first part of this program opens the server FIFO,
// if it already exists, as a file. It then gets its own process ID, which forms some
// of the data that will be sent to the server. The client FIFO is created, ready for
// the next section.
#define _GNU_SOURCE

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
#include <signal.h>

int PARENT_X = 110;
int PARENT_Y;
WINDOW *w_tx;
WINDOW *w_rx;

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

void ResizeWindows() {
    int new_y, new_x;
    getmaxyx(stdscr, new_y, new_x);
    if (new_y != PARENT_Y || new_x != PARENT_X) {
        PARENT_Y = new_y;
        
        wresize(w_tx, new_y, getmaxx(w_tx));
        
        wclear(stdscr);
    }
}

void DrawCompleteWindows() {
    ResizeWindows();
    DrawBorders(w_tx);
    DrawBorders(w_rx);
    DrawWindowTitle(w_tx, "Transmit [TX]");
    DrawWindowTitle(w_rx, "Receive [RX]");
    wrefresh(w_tx);
    wrefresh(w_rx);
}

void ConnectToServer(pid_t clientPID, int server_fifo_fd, int client_fifo_fd) {
    const unsigned int maxCommandLength = 100;
    char commandBuffer[maxCommandLength];
    char responseBuffer[maxCommandLength];

    mvwprintw(w_tx, 5, 1, "Connecting to server\n");
    DrawCompleteWindows();

    int tx_linesCounter = 7;
    int rx_linesCounter = 1;
    int commandCounter = 1;
    while(true) { // Main loop
        memset(commandBuffer, 0, sizeof(commandBuffer));
        memset(responseBuffer, 0, sizeof(responseBuffer));
        
        // Draw to our windows
        DrawCompleteWindows();
        mvwprintw(w_tx, tx_linesCounter++, 1, "Command (Press Enter to send): %s", commandBuffer);

        // Repeat until the user entered something
        while(commandBuffer[0] == 0)
            wgetstr(w_tx, commandBuffer);
            
        // Print to FIFO
        dprintf(server_fifo_fd, "%u %s", clientPID, commandBuffer);
        if (commandBuffer[0] == 'q') {
            mvwprintw(stdscr, 0, 0, "EXITING NOW");
            wrefresh(stdscr);
            return;
        }

        // Read from FIFO
        read(client_fifo_fd, responseBuffer, sizeof(responseBuffer));

        // Print the read message
        if (rx_linesCounter + 2 >= PARENT_Y) {
            rx_linesCounter = 1;
        }
        mvwprintw(w_rx, rx_linesCounter++, 1, "Server response... #%d", commandCounter++);
        mvwprintw(w_rx, rx_linesCounter++, 1, "%s", responseBuffer);
        rx_linesCounter++;
        
        int x, y;
        getyx(w_tx, x, y);
        tx_linesCounter = x++;
        if (tx_linesCounter + 2 >= PARENT_Y) {
            tx_linesCounter = 7;
        }
    }
}

void CleanTerminal(void) {
    endwin();
}

int main() {
    //set up initial windows
    initscr();
    curs_set(TRUE);
    
    atexit(CleanTerminal);

    PARENT_Y = getmaxy(stdscr);

    w_tx = newwin(PARENT_Y, (PARENT_X / 2) - 3, 0, 0);
    w_rx = newwin(PARENT_Y, (PARENT_X / 2) - 3, 0, (PARENT_X / 2) + 2);

    DrawCompleteWindows();

    //set up initial connection
    pid_t clientPID;
    int server_fifo_fd, client_fifo_fd;
    char* client_fifo = NULL;
    
    // Open server fifo
    mvwprintw(w_tx, 1, 1, "Opening Server FIFO ...\n");
    DrawCompleteWindows();
    
    
    server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Sorry, no server\n");
        refresh();
        exit(EXIT_FAILURE);
    }
    mvwprintw(w_tx, 1, 27, "Success !\n");
    DrawCompleteWindows();
    
    // Create client fifo
    mvwprintw(w_tx, 2, 1, "Creating Client FIFO ...\n");
    clientPID = getpid();
    asprintf(&client_fifo, CLIENT_FIFO_NAME, clientPID);
    if (mkfifo(client_fifo, 0777) == -1) {
        fprintf(stderr, "Sorry, can't make %s\n", client_fifo);
        refresh();
        goto Error;
    }
    mvwprintw(w_tx, 2, 27, "Success !\n");
    DrawCompleteWindows();
    
    // Sending pid to server
    mvwprintw(w_tx, 3, 1, "Sending PID ...\n");
    DrawCompleteWindows();
    dprintf(server_fifo_fd, "%u\n", clientPID);
    mvwprintw(w_tx, 3, 27, "Success !\n");
    DrawCompleteWindows();

    // Open client fifo
    mvwprintw(w_tx, 4, 1, "Oppening Client FIFO ...\n");
    DrawCompleteWindows();
    if ((client_fifo_fd = open(client_fifo, O_RDONLY)) == -1) {
        goto Error;
    }
    mvwprintw(w_tx, 4, 27, "Success !\n");
    DrawCompleteWindows();

    ConnectToServer(clientPID, server_fifo_fd, client_fifo_fd);

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

