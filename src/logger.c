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
    char buffer[BUFF_S];
    char *separator = "\n-----------------------\n";
    char *c_time_string;
    char sizz[BUFF_S] = "SIZE:\n";

    /* open FIFO from reading side */
    //char *myFifo = argv[2];
    int myFifo = open(argv[2], O_RDONLY);
    //FILE *fd = fopen(myFifo, "r");

    /* open logFile (a+ -> create+append) */
    /*
    FILE *myLog = fopen(argv[0], "a+");
    if (myLog == NULL) { //smth is wrong
        perror("Opening logFile");
        exit(EXIT_FAILURE);
    }
    */
    int myLog = open(argv[0], O_WRONLY | O_APPEND | O_CREAT, 0777);

    char *dStart = "Daemon started\n\n";
    write(myLog, dStart, strlen(dStart));

    ssize_t size;
    while (strcmp(buffer, "kill")) {
        kill(getpid(), SIGSTOP);
        size = read(myFifo, buffer, sizeof buffer) - 1;

        write(myLog, separator, strlen(separator)); //SEP

        /* timestamp */
        c_time_string = getcTime();
        write(myLog, c_time_string, strlen(c_time_string));

        write(myLog, separator, strlen(separator)); //SEP

        /* command output size */
        sprintf(sizz, "Size:%zi\n", size);
        write(myLog, sizz, strlen(sizz));

        /* command output */
        write(myLog, buffer, strlen(buffer));

        write(myLog, separator, strlen(separator)); //SEP

        //timer--;
        //sleep(1);
    }
    char *dEnd = "\nDaemon ended\n";
    write(myLog, dEnd, strlen(dEnd));

    /* TODO: choose if delete files after daemon dies */
    removeFifo(argv[1]);
    removeFifo(argv[2]);
    //int myFd = open(argv[1], O_WRONLY | O_TRUNC);

    close(myLog);
}
