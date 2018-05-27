#include "myLibrary.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void segmentcpy(char *dst, char *src, int from, int to) {
    int x;
    for (x = 0; x <= to - from; x++) {
        dst[x] = src[x + from];
    }
    dst[to - from + 1] = '\0';
}

int appendPack(Pk *data, settings *s, char *superstring) {
    /* check if return code flag is set */
    if (!s->code) {
        strcpy(data->returnC, "false");
    }
    char *pos = superstring + 0;

    /* elements lengths */
    int cmdIDSize = strlen(data->cmdID) + 1;
    int fatherIDSize = strlen(data->fatherID) + 1;
    int shellIDSize = strlen(data->shellID) + 1;
    int beginDateSize = strlen(data->beginDate) + 1;
    int completionDateSize = strlen(data->completionDate) + 1;
    int durationSize = strlen(data->duration) + 1;
    int outTypeSize = strlen(data->outType) + 1;
    int origCmdSize = strlen(data->origCmd) + 1;
    int returnSize = strlen(data->returnC) + 1;
    int cmdSize = strlen(data->cmd) + 1;

    /* limit output printed on log */
    int outSize;
    if (s->maxOut < strlen(data->out) + 1) {
        outSize = s->maxOut;
    } else {
        outSize = strlen(data->out);
    }
    outSize++; /* for ending '\0' */

    int dataSize = 0;
    dataSize += cmdIDSize;
    dataSize += fatherIDSize;
    dataSize += shellIDSize;
    dataSize += beginDateSize;
    dataSize += completionDateSize;
    dataSize += durationSize;
    dataSize += outTypeSize;
    dataSize += origCmdSize;
    dataSize += cmdSize;
    dataSize += outSize;
    dataSize += returnSize;

    /* concatenate all data in one single string (order matters) */

    strcpy(pos, data->cmdID);
    pos += cmdIDSize;
    strcpy(pos, data->fatherID);
    pos += fatherIDSize;
    strcpy(pos, data->shellID);
    pos += shellIDSize;
    strcpy(pos, data->beginDate);
    pos += beginDateSize;
    strcpy(pos, data->completionDate);
    pos += completionDateSize;
    strcpy(pos, data->duration);
    pos += durationSize;
    strcpy(pos, data->outType);
    pos += outTypeSize;
    strcpy(pos, data->origCmd);
    pos += origCmdSize;
    strcpy(pos, data->cmd);
    pos += cmdSize;

    /* managing output limit */
    strncpy(pos, data->out, outSize);
    pos += outSize;
    strcpy(pos - 1, "\0");

    strcpy(pos, data->returnC);
    pos += returnSize;
    return dataSize;
}

void executeCommand(int toShell, int fromShell, Pk *data, bool piping, bool *proceed) {
    clock_t t;
    int count;
    char tmp[32];
    sprintf(tmp, "; echo $?; kill -10 %d\n", getpid());

    /* write and wait for shell response */
    data->beginDate = getcTime();
    t = clock();
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
    t = clock() - t;
    double elapsed = ((double)t) / CLOCKS_PER_SEC;
    data->completionDate = getcTime();
    snprintf(data->duration, 19, "%f", elapsed);

    /* read from pipe */
    count = read(fromShell, data->out, PK_BIG);
    if (count >= PK_BIG) {
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
        strcpy(data->out, "");
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

    int res = kill(loggerID, 0);
    while (res == 0 || (res < 0 && errno == EPERM)) {
        printf("...\n");
        sleep(1);
        res = kill(loggerID, 0);
    }
    printf("logger killed\n");
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