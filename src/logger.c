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

/* to use with char *inputs[4] */
typedef enum input_format { CMD,
                            DATA,
                            OUT,
                            OTHER } i_f;

void logger(int argc, char *argv[]) {
    char buffer[BUFF_S];
    char *separator = "\n-----------------------\n";
    char *aCapo = "\n";
    char *c_time_string;
    char ssize[80];

    int inNum = 4;
    char *inputs[inNum];

    /* open FIFO from reading side */
    int myFifo = open(argv[2], O_RDONLY);

    /* open/create log file */
    int myLog = open(argv[0], O_WRONLY | O_APPEND | O_CREAT, 0777);

    char *dStart = "Daemon started\n\n";
    write(myLog, dStart, strlen(dStart));

    ssize_t size;
    while (strcmp(buffer, "kill")) {
        /* timestamp */
        write(myLog, separator, strlen(separator)); //SEP
        c_time_string = getcTime();
        write(myLog, c_time_string, strlen(c_time_string));
        write(myLog, separator, strlen(separator)); //SEP

        /* read FIFO until open */
        /*
        while (size = read(myFifo, buffer, sizeof buffer)) {
            // command
        }
        */
        size = read(myFifo, buffer, BUFF_S);
        if ((read(myFifo, buffer, BUFF_S)) != 0) {
            write(myLog, separator, strlen(separator)); //SEP
            sprintf(ssize, "ERROR: fifo contained more than BUFF_S=%d chars", BUFF_S);
            write(myLog, ssize, strlen(ssize));
            write(myLog, separator, strlen(separator)); //SEP
            break;
        }

        write(myLog, separator, strlen(separator)); //SEP
        sprintf(ssize, "SIZE: %zi", size);
        write(myLog, ssize, strlen(ssize));
        write(myLog, separator, strlen(separator)); //SEP

        /* get every input data from last buffer */
        int inTmp = 0;
        inputs[inTmp++] = buffer;
        for (int i = 0; i < size; i++) {
            if (buffer[i] == '\0') {
                inputs[inTmp++] = &buffer[i + 1];
            }
        }

        /* print every input data */
        for (int i = 0; i < inNum; i++) {
            write(myLog, inputs[i], strlen(inputs[i]));
            write(myLog, aCapo, strlen(aCapo));
        }

        /* pause mode after getting data */
        /* processes can check if logger is in */
        /* pause mode in order to send data safely */
        kill(getpid(), SIGSTOP);
    }

    char *dEnd = "\nDaemon ended\n";
    write(myLog, dEnd, strlen(dEnd));

    /* TODO: choose if delete files after daemon dies */
    removeFifo(argv[1]);
    removeFifo(argv[2]);
    close(myLog);
    close(myFifo);
}
