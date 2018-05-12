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
    char *aCapo = "\n";
    char *c_time_string;
    char sizz[BUFF_S] = "SIZE:\n";
    char *secondPart;

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
    ssize_t totSize = 0;
    while (strcmp(buffer, "kill")) {
        write(myLog, separator, strlen(separator)); //SEP

        /* timestamp */
        c_time_string = getcTime();
        write(myLog, c_time_string, strlen(c_time_string));
        write(myLog, separator, strlen(separator)); //SEP

        /* read FIFO until open */
        while (size = read(myFifo, buffer, sizeof buffer)) {
            if (size != BUFF_S) {
                size--;
                write(myLog, buffer, size);
            } else {
                for (int x = 0; x < BUFF_S; x++) {
                    if (buffer[x] == '\0') {
                        write(myLog, buffer, x);
                        secondPart = buffer + x + 1;
                        write(myLog, aCapo, strlen(aCapo));
                        write(myLog, secondPart, size - x - 1);
                    }
                }
            }

            totSize += size;
            //write(myLog, buffer, size);
        }
        write(myLog, aCapo, strlen(aCapo));

        /* command output size */
        sprintf(sizz, "Size:%zi", totSize);
        write(myLog, sizz, strlen(sizz));
        totSize = 0;

        write(myLog, separator, strlen(separator)); //SEP
        //timer--;
        //sleep(1);
        kill(getpid(), SIGSTOP);
    }
    char *dEnd = "\nDaemon ended\n";
    write(myLog, dEnd, strlen(dEnd));

    /* TODO: choose if delete files after daemon dies */
    removeFifo(argv[1]);
    removeFifo(argv[2]);
    //int myFd = open(argv[1], O_WRONLY | O_TRUNC);

    close(myLog);
}
