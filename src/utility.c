#include "myLibrary.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

void loggerIsRunning(int *fdID, int *loggerID, char *loggerIDfile) {
    int pId_MaxLen = 10;
    char buffer[pId_MaxLen];

    /* if loggerID file not found, create one and write new logger ID */
    *fdID = open(loggerIDfile, O_RDWR);
    if (*fdID == -1) {
        perror("Opening logger ID file");
        if ((*fdID = open(loggerIDfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
            perror("Creating file");
            exit(EXIT_FAILURE);
        }
    } else {
        if (read(*fdID, buffer, pId_MaxLen)) { //if file is not empty
            printf("Found existing logger ID\n");
            read(*fdID, buffer, pId_MaxLen);
            *loggerID = atoi(buffer);
        }
    }
}

char *cmdOutSplitReturnCode(char *outBuff, char *retCode) {
    rmNewline(outBuff);               /* replace ending '\n' with '\0' */
    retCode = strrchr(outBuff, '\n'); /* return code is placed after actual last '\n' */
    *retCode = '\0';                  /* separate command output from return code */
    return ++retCode;
}

void removeFile(char *filePath) {
    char rmFile[PATH_S] = "rm ";
    strcat(rmFile, filePath);
    system(rmFile);
}

char *getcTime() {
    time_t current_time;
    char *c_time_string;
    current_time = time(NULL); // current time
    if (current_time == ((time_t)-1)) {
        fprintf(stderr, "Failure to obtain the current time.\n");
        exit(EXIT_FAILURE);
    }
    c_time_string = ctime(&current_time); // convert to readable format
    if (c_time_string == NULL) {
        fprintf(stderr, "Failure to convert the current time.\n");
        exit(EXIT_FAILURE);
    }
    rmNewline(c_time_string);
    return c_time_string;
}

void rmNewline(char *str) {
    str[strlen(str) - 1] = '\0';
}