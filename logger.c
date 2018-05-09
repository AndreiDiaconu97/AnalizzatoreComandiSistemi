#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
//#include "myLibrary.h"

int main(int argc, char *argv[]) {
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
    return 0;
}
