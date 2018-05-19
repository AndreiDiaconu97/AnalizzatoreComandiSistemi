#include "myLibrary.h"
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

void segmentcpy(char *dst, char *src, int from, int to) {
    int x;
    for (x = 0; x <= to - from; x++) {
        dst[x] = src[x + from];
    }
    dst[to - from + 1] = '\0';
}

void sendData(Pk *data, int loggerID) {
    int loggerFd = open(LOGGER_FIFO, O_WRONLY);
    /* allowed to send data to logger */
    write(loggerFd, data->outType, strlen(data->outType) + 1);
    write(loggerFd, data->origCmd, strlen(data->origCmd) + 1);
    write(loggerFd, data->cmd, strlen(data->cmd) + 1);
    write(loggerFd, data->out, strlen(data->out) + 1);
    write(loggerFd, data->returnC, strlen(data->returnC) + 1);
    kill(loggerID, SIGCONT);
    close(loggerFd);
}

void executeCommand(int toShell, int fromShell, Pk *data, bool piping) {
    ssize_t count;
    char tmp[32];
    sprintf(tmp, "; echo $?; kill -10 %d\n", getpid());

    /* write, then wait for shell to output everything on fifo */
    if (piping) {
        write(toShell, "echo \"", strlen("echo \""));
        write(toShell, data->out, strlen(data->out));
        write(toShell, "\"|", strlen("\"|"));
    }
    write(toShell, data->cmd, strlen(data->cmd));
    write(toShell, tmp, strlen(tmp));
    //kill(getpid(), SIGSTOP);
    pause();
    /* reading from fifo */
    count = read(fromShell, data->out, PK_O);
    if (count >= PK_O) {
        printf("Reading from Shell: buffer is too small\nClosing...\n");
        exit(EXIT_FAILURE);
    }
    data->out[count - 1] = '\0';

    /* retrieving return code from output + splitting */
    char *tmpReturn = strrchr(data->out, '\n');
    *tmpReturn = '\0';
    strcpy(data->returnC, tmpReturn + 1);

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