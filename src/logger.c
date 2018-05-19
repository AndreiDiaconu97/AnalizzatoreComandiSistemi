#include "myLibrary.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <syslog.h>
#include <unistd.h>

char *loggerIDfile;
char *fifoPipe;

void printTxt(char **inputs);
void usr1_handler(int sig);

void logger(char *argv[]) {
    loggerIDfile = argv[1];
    fifoPipe = argv[2];

    /* logger is closed with a custom signal handler */
    signal(SIGUSR1, usr1_handler);

    /* allocations needed */
    ssize_t count = 0;
    char buffer[2 * CMD_S + PK_T + PK_O + PK_R];
    int inNum = 5;
    char *inputs[inNum];
    char size[65];

    /* open FIFO from reading side */
    int myFifo = open(fifoPipe, O_RDWR);

    /* open/create log file and move pipe to stdout */
    int myLog = open(argv[0], O_WRONLY | O_APPEND | O_CREAT, 0777);
    dup2(myLog, STDOUT_FILENO);
    close(myLog);

    ///* actual program *///
    while (1) {
        //kill(getpid(), SIGSTOP);

        /* read next data-packet size */
        size[0] = '\0';
        do {
            read(myFifo, buffer, 1);
            strncat(size, buffer, 1);
        } while (*buffer != '\0');

        /* read all data even if frammented */
        int left = atoi(size);
        int lastIndex = -1;
        while (left != 0) {
            count = read(myFifo, &buffer[lastIndex + 1], left);
            lastIndex += count;
            left -= count;
        }

        /* analysing data */
        int inTmp = 0;
        int i;
        inputs[inTmp++] = buffer;
        for (i = 0; i < count; i++) {
            if (buffer[i] == '\0') {
                inputs[inTmp++] = &buffer[i + 1];
            }
        }

        printf("SIZE:\t%s\n", size);
        printTxt(inputs);
    }
}

void usr1_handler(int sig) {
    remove(loggerIDfile);
    remove(fifoPipe);
    remove(LOGGER_QUEUE);
    exit(EXIT_SUCCESS);
}

void printTxt(char **inputs) {
    printf("ID:\t\t\t%s\n", "1.1.1");
    printf("TYPE:\t\t%s\n", inputs[0]);
    printf("COMMAND:\t%s\n", inputs[1]);
    printf("SUBCOMMAND:\t%s\n", inputs[2]);
    char *c_time_string = getcTime();
    printf("DATE:\t\t%s\n", c_time_string);
    printf("OUTPUT:\n\n%s\n\n", inputs[3]);
    printf("RETURN CODE: %s\n", inputs[4]);
    printf("---------------------------------------------------\n");
}
