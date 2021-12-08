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
#include <semaphore.h>

int PARENT_X = 110;
int PARENT_Y;
WINDOW *w_tx;
WINDOW *w_rx;
sem_t w_sem;
int tx_linesCounter = 7;
int rx_linesCounter = 1;

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

void Draw_w_tx() {
    DrawBorders(w_tx);
    DrawWindowTitle(w_tx, "Transmit [TX]");
    wrefresh(w_tx);
}

void Draw_w_rx() {
    DrawBorders(w_rx);
    DrawWindowTitle(w_rx, "Receive [RX]");
    wrefresh(w_rx);
}

void ConnectToServer(pid_t clientPID, int server_fifo_fd, int client_fifo_fd) {
    const unsigned int maxCommandLength = 100;
    char commandBuffer[maxCommandLength];
    char responseBuffer[maxCommandLength];
    sem_init(&w_sem, 1, 1);

    mvwprintw(w_tx, 5, 1, "Connecting to server\n");
    DrawCompleteWindows();
    int pid = fork();

    while(true) { // Main loop
        sem_wait(&w_sem);
        if (pid == 0) {
            /****************************** CHILD ******************************/
            memset(responseBuffer, 0, sizeof(responseBuffer));
            
            // Read from FIFO
            read(client_fifo_fd, responseBuffer, sizeof(responseBuffer));

            // If the response's first char isn't the NULL character
            if (responseBuffer[0] != '\0') {
                // Print the read message
                if (rx_linesCounter + 2 >= PARENT_Y) {
                    werase(w_rx);
                    wrefresh(w_rx);
                    rx_linesCounter = 1;
                }
                
                int x, y;
                getyx(w_tx, x, y);
                mvwprintw(w_rx, rx_linesCounter++, 1, "%s", responseBuffer);
                rx_linesCounter++;

                Draw_w_rx();
                wmove(w_tx, x, y);
                wrefresh(w_tx);

                //DrawCompleteWindows();
                // wmove(w_tx, tx_linesCounter, 32);
                // wrefresh(w_tx);
            }
            /*******************************************************************/
        }
        else {
            /***************************** PARENT ******************************/
            memset(commandBuffer, 0, sizeof(commandBuffer));
            int x, y;
            
            if (tx_linesCounter + 2 >= PARENT_Y) {
                werase(w_tx);
                wrefresh(w_tx);
                tx_linesCounter = 1;
            }

            Draw_w_tx();
            
            mvwprintw(w_tx, tx_linesCounter++, 1, "Command (Press Enter to send): %s", commandBuffer);

            // Repeat until the user entered something
            while(commandBuffer[0] == 0) {
                //mvwprintw(w_tx, tx_linesCounter++, 1, "yx: %d %d", x, y);
                wmove(w_tx, tx_linesCounter - 1, 32);
                wgetstr(w_tx, commandBuffer);
            }
            
            // Print to FIFO
            dprintf(server_fifo_fd, "%u %s", clientPID, commandBuffer);
            
            // If 'q' was pressed, then we exit the program
            if (commandBuffer[0] == 'q') {
                mvwprintw(stdscr, 0, 0, "EXITING NOW");
                wrefresh(stdscr);
                return;
            }
            /*******************************************************************/
        }
        sem_post(&w_sem);
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

