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

void executeCommand(int toShell, int fromShell, char *command, char retCode[3], char *outBuff, int buffSize, int shellID) {
    int count = 1;

    /* writing phase */
    write(toShell, command, strlen(command));

    //kill(shellID, SIGCONT);
    /* reading phase */
    count = read(fromShell, outBuff, buffSize);
    printf("RD:%d\n", count);
    //waitpid(-1, NULL, WUNTRACED);
    if (count >= buffSize) {
        printf("Reading from Shell: buffer is too small\nClosing...\n");
        exit(EXIT_FAILURE);
    }
    outBuff[count] = '\0';
    printf("%s-------------------\n", outBuff);
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