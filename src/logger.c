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
    char container[1024];
    char *separator = "\n-----------------------\n";
    char *aCapo = "\n";
    char *c_time_string;
    char sizz[BUFF_S] = "SIZE:\n";
    char *inputs[4];

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

        int x = 0;
        int inNum = 1;
        /* read FIFO until open */
        while (size = read(myFifo, buffer, sizeof buffer)) {
            // command
            // write(myLog, buffer, size - 1);
            inputs[0] = container;
            inNum = 1;

            for (int i = 0; i < size; i++) {
                container[x] = buffer[i];
                x++;
            }

            totSize += size;
            //write(myLog, buffer, size);
        }
        write(myLog, container, x);
        write(myLog, aCapo, strlen(aCapo));

        for (int i = 0; i < x; i++) {
            if (container[i] == '\0') {
                inputs[inNum++] = &container[i + 1];
                sprintf(sizz, "Size:%d", i);
                write(myLog, sizz, strlen(sizz));
                write(myLog, aCapo, strlen(aCapo));
            }
        }

        for (int i = 0; i < sizeof inputs / sizeof inputs[0]; i++) {
            write(myLog, inputs[i], strlen(inputs[i]));
            write(myLog, aCapo, strlen(aCapo));
        }

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
