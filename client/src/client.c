// Here's the client, client.c. The first part of this program opens the server FIFO,
// if it already exists, as a file. It then gets its own process ID, which forms some
// of the data that will be sent to the server. The client FIFO is created, ready for
// the next section.

#define _GNU_SOURCE

#include "client.h"
#include <semaphore.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <ncurses.h>

WINDOW *w_tx;
WINDOW *w_rx;
int PARENT_X = 110;
int PARENT_Y;
sem_t console_sem;

void RX(int client_fifo_fd, char* responseBuffer) {
    char *received = malloc (sizeof (char) * 1000);
    read(client_fifo_fd, received, 1000);
    strcpy(responseBuffer, received);
}

bool TX(char* command, pid_t clientPID, int server_fifo_fd, int maxCommandLength) {
    dprintf(server_fifo_fd, "%d %s", clientPID, command);
    return toupper(command[0]) != 'Q';
}

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

void DrawCompleteWindows() {
    //ResizeWindows();
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
    pid_t pid;
    sem_init(&console_sem, 1, 1);

    printf("Connecting to server\n");
    dprintf(server_fifo_fd, "%d\n", clientPID);

    werase(stdscr);
    wrefresh(stdscr);
    DrawCompleteWindows();
    int tx_linesCounter = 1;
    int rx_linesCounter = 1;

    if (!(pid = fork())) {
        while (true) {
            /************* RX LOOP *************/
            sem_wait(&console_sem);
            char responseBuffer[maxCommandLength * 10];

            // Read from FIFO
            RX(client_fifo_fd, responseBuffer);

            // If the response's first char isn't the NULL character
            if (strncmp(responseBuffer, "", 1)) {
                // Print the read message
                if (rx_linesCounter + 2 >= PARENT_Y) {
                    // If the linesCounter is higher than the window size, then reset the linesCounter to 1
                    werase(w_rx);
                    wrefresh(w_rx);
                    rx_linesCounter = 1;
                }
                
                // Lines below are supposed to save the cursor position at TX window and re-set it after having printed it's lines on RX window
                int x, y;
                getyx(w_tx, x, y);
                mvwprintw(w_rx, rx_linesCounter++, 1, "%s", responseBuffer);
                rx_linesCounter++;

                Draw_w_rx();
                wmove(w_tx, x, y);
                wrefresh(w_tx);
            }
            sem_post(&console_sem);
        }
    } else {
        do { // Main loop
            /************* TX LOOP *************/
            sem_wait(&console_sem);
            memset(commandBuffer, 0, sizeof(commandBuffer));

            if (tx_linesCounter + 2 >= PARENT_Y) {
                // If the linesCounter is higher than the window size, then reset the linesCounter to 1
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
            
            sem_post(&console_sem);
        } while (TX(commandBuffer, clientPID, server_fifo_fd, maxCommandLength));
    }
    kill(pid, SIGTERM);
}

void CleanTerminal(void) {
    endwin();
}

int main() {
    //set up initial windows
    initscr();
    PARENT_Y = getmaxy(stdscr);
    w_tx = newwin(PARENT_Y, (PARENT_X / 2) - 3, 0, 0);
    w_rx = newwin(PARENT_Y, (PARENT_X / 2) - 3, 0, (PARENT_X / 2) + 2);
    curs_set(TRUE);
    atexit(CleanTerminal);
    DrawCompleteWindows();

    pid_t clientPID;
    int server_fifo_fd, client_fifo_fd;
    char* client_fifo = NULL;

    // Open server fifo
    printf("Opening Server FIFO ...\n");
    server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Sorry, no server\n");
        exit(EXIT_FAILURE);
    }
    printf("Success !\n");

    // Create client fifo
    printf("Creating Client FIFO ...\n");
    clientPID = getpid();
    asprintf(&client_fifo, CLIENT_FIFO_NAME, clientPID);
    if (mkfifo(client_fifo, 0777) == -1) {
        fprintf(stderr, "Sorry, can't make %s\n", client_fifo);
        goto Error;
    }
    printf("Success !\n");


    // Sending pid to server
    printf("Sending PID ...\n");
    dprintf(server_fifo_fd, "%u\n", clientPID);
    printf("Success !\n");

    // Open client fifo
    printf("Oppening Client FIFO ...\n");
    if ((client_fifo_fd = open(client_fifo, O_RDONLY)) == -1) {
        goto Error;
    }
    fcntl(client_fifo_fd, F_SETFL, fcntl(client_fifo_fd, F_GETFL, 0) | O_NONBLOCK);
    printf("Success !\n");

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