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

void sendData(Pk *data, settings *s) {
    /* elements lengths */
    int outTypeSize = strlen(data->outType) + 1;
    int origCmdSize = strlen(data->origCmd) + 1;
    int cmdSize = strlen(data->cmd) + 1;
    int returnSize = strlen(data->returnC) + 1;

    /* limit output printed on log */
    int outSize;
    if (s->maxOut < strlen(data->out) + 1) {
        outSize = s->maxOut;
    } else {
        outSize = strlen(data->out) + 1;
    }
    int dataSize = outTypeSize + origCmdSize + cmdSize + outSize + returnSize;

    char dataLen[65];
    sprintf(dataLen, "%d", dataSize);

    /* concatenate all data in one single string */
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
    int loggerFd = open(HOME TEMP_DIR LOGGER_FIFO_F, O_WRONLY);
    write(loggerFd, superstring, dataSize + strlen(dataLen) + 1);
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

    /* wait for shell response */
    while (1) {
        if (*proceed == true) {
            *proceed = false;
            break;
        }
    }

    /* read from pipe */
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

    /* get return code */
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

    /* get output type */
    if (atoi(data->returnC) == 0) {
        strcpy(data->outType, "StdOut");
    } else {
        strcpy(data->outType, "StdErr");
    }
}

/* send custom signal to logger which will kill it after emtying the pipe */
void killLogger(int loggerID) {
    kill(loggerID, SIGUSR1);
    waitpid(loggerID, NULL, 0);
    printf("Logger killed\n");
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
    /* remove newline char */
    c_time_string[strlen(c_time_string) - 1] = '\0';

    return c_time_string;
}