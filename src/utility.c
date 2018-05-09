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

/**
 * first function main program calls, true return code is required 
 */
bool readArguments(char argc, char **argv, char *outF, char *errF, int *maxLen, bool *returnCode) {
    bool result = true;
    char tmpArg[BUFF_S] = "";
    char tmpVal[BUFF_S] = "";
    /* reading program arguments */
    for (int i = 0; i < argc; i++) {
        /* finding command arguments (commands begin with "--") */
        if (!strncmp(argv[i], "--", 2)) {
            /** arguments are in form '--command=value',thus
             * finding the '=' gives the splitting point */
            for (int t = 0; t < strlen(argv[i]); t++) {
                /* getting command */
                if (argv[i][t] != '=') {
                    tmpArg[strlen(tmpArg)] = argv[i][t];
                    tmpArg[strlen(tmpArg)] = '\0';
                } else {
                    /* found the '=' char, getting value */
                    for (int x = t + 1; x < strlen(argv[i]); x++) {
                        tmpVal[strlen(tmpVal)] = argv[i][x];
                        tmpVal[strlen(tmpVal)] = '\0';
                    }
                    break; // exiting form command loop too
                }
            }
            printf("Arg:%s - Val:%s\n", tmpArg, tmpVal);

            /* evaluating given command */
            if ((!strcmp(tmpArg, "--outfile")) || (!strcmp(tmpArg, "--o"))) {
                strcpy(outF, tmpVal);
            } else if ((!strcmp(tmpArg, "--errfile")) || (!strcmp(tmpArg, "--e"))) {
                strcpy(errF, tmpVal);
            } else if ((!strcmp(tmpArg, "--maxLen")) || (!strcmp(tmpArg, "--m"))) {
                if (*maxLen = atoi(tmpVal)) {
                    printf("maxLen set to %d\n", *maxLen);
                } else {
                    printf("Couldn't parse given value:'%s'\n", tmpVal);
                    result = false;
                }
            } else if ((!strcmp(tmpArg, "--code")) || (!strcmp(tmpArg, "--c"))) {
                if (!strcmp(tmpVal, "true")) {
                    *returnCode = true;
                } else if (!strcmp(tmpVal, "false")) {
                    *returnCode = false;
                } else {
                    printf("Argument:%s - Invalid input:%s\n", tmpArg, tmpVal);
                    result = false;
                }
            } else {
                printf("Invalid argument found:'%s=%s'\n", tmpArg, tmpVal);
                result = false;
            }
        }
    }
    return result;
}

void logger(int argc, char *argv[]) {
    char *myFifo = "/tmp/myfifo";
    mkfifo(myFifo, 0666);            //0666 -> every type of user can read and write
    int fd = open(myFifo, O_RDONLY); //opening FIFO read only
    //fdup2(fd,STDIN_FILENO);
    int timer = 5;
    int maxLen = 80;
    char buffer[maxLen];
    char callerID[maxLen];

    FILE *myLog = fopen(argv[1], "a+"); //opening logFile (a+ -> create+append)
    if (myLog == NULL) {                //smth is wrong
        perror("Opening logFile");
        exit(EXIT_FAILURE);
    }

    fprintf(myLog, "Daemon started\n\n");
    fflush(myLog);
    int size;
    while (strcmp(buffer, "kill")) {
        kill(getpid(), SIGSTOP);
        //read(fd, callerID, maxLen);
        size = read(fd, buffer, maxLen) - 1;
        fprintf(myLog, "%s  |  size:%d\n", buffer, size);
        fflush(myLog);

        //timer--;
        sleep(1);
    }

    int myFd = open(argv[2], O_WRONLY | O_TRUNC);

    fprintf(myLog, "\nDaemon terminated\n");
    fclose(myLog);
}
