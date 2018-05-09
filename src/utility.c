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

int loggerIsRunning(char *loggerIDfile, char *buffer, int *pfdID) {
    int fdID = *pfdID;
    int loggerID = 0;

    fdID = open(loggerIDfile, O_RDWR);
    /* if loggerID file not found, create one and write new logger ID */
    if (fdID == -1) {
        perror("Opening logger ID file");
        if ((fdID = open(loggerIDfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
            perror("Creating file");
            exit(EXIT_FAILURE);
        } else {
            printf("File created\n");
        }
    }

    if (!read(fdID, buffer, BUFF_S)) { //if file is empty
        printf("File is empty\n");
    } else { //get existing logger ID
        printf("Found existing logger ID\n");
        read(fdID, buffer, BUFF_S);
        loggerID = atoi(buffer);
    }

    *pfdID = fdID;
    return loggerID;
}

void removeFifo(char *fifoPath) {
    char rmFifo[PATH_S] = "";
    strcat(rmFifo, "rm ");
    strcat(rmFifo, fifoPath);
    system(rmFifo);
}