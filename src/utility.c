#include "myLibrary.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <sys/wait.h>
//#include <syslog.h>
#include <time.h>
#include <unistd.h>

void segmentcpy(char *dst, char *src, int from, int to) {
    int x;
    for (x = 0; x <= to - from; x++) {
        dst[x] = src[x + from];
    }
    dst[to - from + 1] = '\0';
}

void sendData(Pk *data) {
    /* elements lengths */
    int outTypeSize = strlen(data->outType) + 1;
    int origCmdSize = strlen(data->origCmd) + 1;
    int cmdSize = strlen(data->cmd) + 1;
    int outSize = strlen(data->out) + 1;
    int returnSize = strlen(data->returnC) + 1;
    int dataSize = outTypeSize + origCmdSize + cmdSize + outSize + returnSize;

    char dataLen[65];
    sprintf(dataLen, "%d", dataSize);

    /* concatenating all data on single string */
    int i = 0;
    char superstring[dataSize + strlen(dataLen) + 1];
    strcpy(&superstring[i], dataLen);
    i += strlen(dataLen) + 1;
    strcpy(&superstring[i], data->outType);
    i += outTypeSize;
    strcpy(&superstring[i], data->origCmd);
    i += origCmdSize;
    strcpy(&superstring[i], data->cmd);
    i += cmdSize;
    strcpy(&superstring[i], data->out);
    i += outSize;
    strcpy(&superstring[i], data->returnC);
    i += returnSize;

    /* send superstring */
    int loggerFd = open(LOGGER_FIFO, O_WRONLY);
    write(loggerFd, superstring, dataSize + strlen(dataLen) + 1);

    /* multi-send variant (more buggy) */
    // write(loggerFd, dataLen, strlen(dataLen) + 1);
    // write(loggerFd, data->outType, outTypeSize);
    // write(loggerFd, data->origCmd, origCmdSize);
    // write(loggerFd, data->cmd, cmdSize);
    // write(loggerFd, data->out, outSize);
    // write(loggerFd, data->returnC, returnSize);
    //kill(loggerID, SIGCONT);
    close(loggerFd);
}

void executeCommand(int toShell, int fromShell, Pk *data, bool piping, bool *proceed) {
    int count;
    char tmp[32];
    sprintf(tmp, "; echo $?; kill -10 %d\n", getpid());
    /* write and wait for shell response */
    if (piping) {
        if (data->noOut) {
            write(toShell, "false|", strlen("false|"));
        } else {
            write(toShell, "echo \"", strlen("echo \""));
            write(toShell, data->out, strlen(data->out));
            write(toShell, "\"|", strlen("\"|"));
        }
    }
    write(toShell, data->cmd, strlen(data->cmd));
    write(toShell, tmp, strlen(tmp));

    while(1) {
        if (*proceed == true) {
            *proceed = false;
            break;
        }
    }


    /* reading from fifo */
    count = read(fromShell, data->out, PK_O);
    if (count >= PK_O) {
        printf("Reading from Shell: buffer is too small\nClosing...\n");
        exit(EXIT_FAILURE);
    }
    if (count > 1) {
        /* delete last '\n' */
        data->out[count - 1] = '\0';
    } else {
        printf("WARNING: got no output from command!\n");
    }

    /* return code */
    char *tmpReturn = strrchr(data->out, '\n');
    if (tmpReturn != NULL) {
        *tmpReturn = '\0';
        strcpy(data->returnC, tmpReturn + 1);
        data->noOut = false;
    } else {
        strcpy(data->returnC, data->out);
        strcpy(data->out, "(null)");
        data->noOut = true;
    }

    /* output type */
    if (atoi(data->returnC) == 0) {
        strcpy(data->outType, "StdOut");
    } else {
        strcpy(data->outType, "StdErr");
    }
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