#include "myLibrary.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    /* customizable variables */
    int maxLen = BUFF_S;
    bool returnCode = false;
    char logfile[BUFF_S] = "log.txt";
    char outfile[BUFF_S] = "change this";
    char errfile[BUFF_S] = "change this";
    char absPath[PATH_S];

    /* reading and checking user arguments */
    if (!readArguments(argc, argv, outfile, errfile, &maxLen, &returnCode)) {
        printf("CLOSING PROGRAM\n");
        exit(EXIT_FAILURE);
    }

    /* program variables */
    int loggerID, fdID = 0;
    char loggerIDfile[PATH_S] = "loggerPid.txt";
    char *buffer = malloc(sizeof(char) * BUFF_S);

    /* searching for existing logger before any forking */
    int logIsRun = loggerIsRunning(loggerIDfile, buffer, &fdID);

    /* creating FIFO file if needed */
    char *myFifo = "/tmp/myfifo";
    if (mkfifo(myFifo, 0666)) {
        perror("FIFO creation");
    }

    /* forking for logger if needed */
    if ((loggerID = fork()) < 0) {
        perror("FORK result");
        exit(EXIT_FAILURE);
    }
    if (loggerID == 0) { //child
        if (!logIsRun) {
            printf("Child:%d\n", getpid());
            char *argss[2] = {logfile, loggerIDfile};
            logger(sizeof argss, argss);
        }
        exit(EXIT_SUCCESS);
    }

    /* father process */
    printf("Father:%d\n", getpid());

    if (!logIsRun) {
        printf("Saving actual logger ID\n");
        sprintf(buffer, "%d", loggerID);
        if (write(fdID, buffer, strlen(buffer)) == -1) { //writing actual running logger ID
            perror("Saving result");
        }
    } else {
        loggerID = logIsRun;
    }
    close(fdID);
    printf("Logger ID un use:%d\n", loggerID);

    int fdFIFO = open(myFifo, O_WRONLY); //opening one end of the pipe
    printf("If you read this means that child opened FIFO too.\n");

    size_t BUFF_SIZE = BUFF_S;
    while (strcmp(buffer, "quit")) {
        printf(">>");
        getline(&buffer, &BUFF_SIZE, stdin);
        buffer[strlen(buffer) - 1] = '\0'; //removing final '\n'
        write(fdFIFO, buffer, strlen(buffer) + 1);
        //kill(loggerID, SIGCONT);
    }
    close(fdFIFO);
    return EXIT_SUCCESS;
}