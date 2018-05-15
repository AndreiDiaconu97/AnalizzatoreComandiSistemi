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
    removeFile(loggerIDfile);
    removeFile(fifoPipe);
    exit(EXIT_SUCCESS);
}

void logger(char *argv[]) {
    /// initialization ///
    loggerIDfile = argv[1];
    fifoPipe = argv[2];
    /* logger is closed with a custom signal handler */
    signal(SIGUSR1, usr1_handler);

    /* allocations needed */
    ssize_t count;
    char buffer[500];
    char *c_time_string;
    int inNum = 4;
    char *inputs[inNum];

    /* open FIFO from reading side */
    int myFifo = open(argv[2], O_RDONLY);

    /* open/create log file and move pipe to stdout */
    int myLog = open(argv[0], O_WRONLY | O_APPEND | O_CREAT, 0777);
    dup2(myLog, STDOUT_FILENO);
    close(myLog);

    /// actual program ///
    printf("---------------------------------------------------\n");
    while (1) {
        kill(getpid(), SIGSTOP);

        /* read data from fifo */
        count = read(myFifo, buffer, 500);

        /* buffer overflow check */
        if (read(myFifo, buffer, 500)) {
            printf("ERROR: fifo contained more than buffer capacity: %d chars\n", 500);
            kill(getpid(), SIGUSR1);
        }

        //printf("SIZE:\t\t%zibyte\n", count);

        /* get every input data from last buffer */
        int inTmp = 0;
        inputs[inTmp++] = buffer;
        for (int i = 0; i < count; i++) {
            if (buffer[i] == '\0') {
                inputs[inTmp++] = &buffer[i + 1];
            }
        }

        /* ID */
        printf("ID:\t\t\t%s\n", "1.1.1");

        /* original command */
        printf("COMMAND:\t%s\n", inputs[1]);

        /* subcommand */
        printf("SUBCOMMAND:\t%s\n", "blab");

        /* timestamp */
        c_time_string = getcTime();
        printf("%s\n", c_time_string);

        /* command output */
        printf("OUTPUT:\n\n%s\n\n", inputs[2]);

        /* return code */
        printf("RETURN CODE: %s\n", inputs[3]);
        printf("---------------------------------------------------\n");

        /* pause mode after getting data */
        /**
        * Processes can check if logger is in 
        * pause mode in order to send data safely 
        **/
    }
}
