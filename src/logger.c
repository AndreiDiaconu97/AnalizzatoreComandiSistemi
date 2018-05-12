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
typedef enum input_format {CMD,OUT,DATA,OTHER} i_f;


void logger(int argc, char *argv[]) {
    char buffer[BUFF_S];
    char container[1024];
    char *separator = "\n-----------------------\n";
    char *aCapo = "\n";
    char *c_time_string;
    char *inputs[4]; 

    /* open FIFO from reading side */
    int myFifo = open(argv[2], O_RDONLY);

    /* open/create log file */
    int myLog = open(argv[0], O_WRONLY | O_APPEND | O_CREAT, 0777);

    char *dStart = "Daemon started\n\n";
    write(myLog, dStart, strlen(dStart));

    ssize_t size;
    while (strcmp(buffer, "kill")) {
        write(myLog, separator, strlen(separator)); //SEP
        /* timestamp */
        c_time_string = getcTime();
        write(myLog, c_time_string, strlen(c_time_string));

        int x = 0;
        int inNum = 1;
        /* read FIFO until open */
        while (size = read(myFifo, buffer, sizeof buffer)) {
            // command
            inputs[0] = container;
            inNum = 1;
            for (int i = 0; i < size; i++) {
                container[x] = buffer[i];
                x++;
            }
        }
        write(myLog, container, x);
        write(myLog, aCapo, strlen(aCapo));

        for (int i = 0; i < x; i++) {
            if (container[i] == '\0') {
                inputs[inNum++] = &container[i + 1];
            }
        }

        for (int i = 0; i < sizeof inputs / sizeof inputs[0]; i++) {
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
