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

char *loggerIDfile;
char *fifoPipe;

void usr1_handler(int sig) {
    remove(loggerIDfile);
    remove(fifoPipe);
    remove(LOGGER_QUEUE);
    exit(EXIT_SUCCESS);
}

void logger(char *argv[]) {
    loggerIDfile = argv[1];
    fifoPipe = argv[2];

    /* logger is closed with a custom signal handler */
    signal(SIGUSR1, usr1_handler);

    /* allocations needed */
    ssize_t count;
    char buffer[2 * CMD_S + PK_T + PK_O + PK_R];
    char *c_time_string;
    int inNum = 5;
    char *inputs[inNum];
    char activeSenderID[ID_S + 1] = "0";

    /* open FIFO from reading side */
    int myFifo = open(fifoPipe, O_RDONLY);

    /* open/create log file and move pipe to stdout */
    int myLog = open(argv[0], O_WRONLY | O_APPEND | O_CREAT, 0777);
    dup2(myLog, STDOUT_FILENO);
    close(myLog);

    /// actual program ///
    printf("---------------------------------------------------\n");
    while (1) {
        kill(getpid(), SIGSTOP);
        /* read data from fifo */
        count = read(myFifo, buffer, sizeof buffer);

        /* buffer overflow check */
        if (read(myFifo, buffer, sizeof buffer)) {
            printf("ERROR: fifo contained more than buffer capacity: %d chars\n", 500);
            kill(getpid(), SIGUSR1);
        }

        /* analysing string */
        int inTmp = 0;
        inputs[inTmp++] = buffer;
        int i;
        for (i = 0; i < count; i++) {
            if (buffer[i] == '\0') {
                inputs[inTmp++] = &buffer[i + 1];
            }
        }

        printf("ID:\t\t\t%s\n", "1.1.1");
        printf("TYPE:\t\t%s\n", inputs[0]);
        printf("COMMAND:\t%s\n", inputs[1]);
        printf("SUBCOMMAND:\t%s\n", inputs[2]);
        c_time_string = getcTime();
        printf("DATE:\t\t%s\n", c_time_string);
        printf("OUTPUT:\n\n%s\n\n", inputs[3]);
        printf("RETURN CODE: %s\n", inputs[4]);
        printf("---------------------------------------------------\n");
    }
}
