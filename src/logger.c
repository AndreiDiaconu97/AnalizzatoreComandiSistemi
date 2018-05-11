#include "myLibrary.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

void logger(int argc, char *argv[]) {
    //fdup2(fd,STDIN_FILENO);
    int timer = 5;
    int maxLen = 80;
    char buffer[maxLen];

    /* open FIFO from reading side */
    char *myFifo = argv[2];
    int fd = open(myFifo, O_RDONLY); 

    /* open logFile (a+ -> create+append) */
    FILE *myLog = fopen(argv[0], "a+");
    if (myLog == NULL) { //smth is wrong
        perror("Opening logFile");
        exit(EXIT_FAILURE);
    }

    fprintf(myLog, "Daemon started\n\n");
    fflush(myLog);
    ssize_t size;
    while (strcmp(buffer, "kill")) {
        kill(getpid(), SIGSTOP);
        //read(fd, buffer, maxLen);
        fprintf(myLog, "-------------\n");
        fflush;

        size = read(fd, buffer, maxLen);
        fprintf(myLog, "%s  |  size:%zi\n", buffer, size - 1);
        fflush(myLog);

        fprintf(myLog, "-------------\n");
        fflush(myLog);
        //timer--;
        //sleep(1);
    }

    /* TODO: choose if delete files after daemon dies */
    removeFifo(myFifo);
    removeFifo(argv[1]);
    //int myFd = open(argv[1], O_WRONLY | O_TRUNC);

    fprintf(myLog, "\nDaemon terminated\n");
    fclose(myLog);
}
