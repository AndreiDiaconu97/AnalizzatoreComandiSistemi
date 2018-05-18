#include "myLibrary.h"
#include <errno.h> //not used
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> //not used
#include <sys/wait.h>  //not used
#include <syslog.h>    //not used
#include <time.h>      //not used
#include <unistd.h>

void segmentcpy(char *dst, char *src, int from, int to) {
    int len = to - from;
    int x;
    for (x = 0; x <= len; x++) {
        dst[x] = src[x + from];
    }
    dst[len + 1] = '\0';
}

int main(int argc, char *argv[]) {
    /* user-settings container */
    settings sett;
    initSettings(&sett);

    /* read and check user arguments */
    if (!readArguments(argc, argv, &sett)) {
        printf("CLOSING PROGRAM\n");
        exit(EXIT_FAILURE);
    }
    showSettings(&sett);

    /* some program variables */
    int pidFD;
    bool needNew = false;
    int loggerID = 0;
    int shellID;
    char buffer[sett.maxBuff];

    /******************************************************************************/
    /******************************** LOGGER SETUP ********************************/
    /******************************************************************************/

    /* search for existing logger   any forking */
    pidFD = open(LOG_PID_F, O_RDONLY, 0777);
    if (pidFD == -1) {
        needNew = true;
    } else {
        char buffer[10];
        read(pidFD, buffer, sizeof buffer);
        loggerID = atoi(buffer);
        if (getpgid(loggerID) < 0) {
            needNew = true;
        } else {
            int fifoFD = open(LOGGER_FIFO, O_RDWR, 0777);
            if (fifoFD == -1) {
                needNew = true;
                kill(loggerID, SIGKILL);
            }
            close(fifoFD);
        }
    }
    close(pidFD);

    /* fork for logger if no existing one is found */
    if (needNew) {
        unlink(LOGGER_FIFO);
        mkfifo(LOGGER_FIFO, 0777);

        if ((loggerID = fork()) < 0) {
            perror("logger fork");
            exit(EXIT_FAILURE);
        }
        if (loggerID == 0) {
            /* logger must be leader of its own group in order to be a daemon */
            setsid();

            /* logger call with appropriate arguments */
            char *args[3] = {sett.logF, LOG_PID_F, LOGGER_FIFO};
            logger(args);
            exit(EXIT_SUCCESS);
        } else {
            /* if there is not logger found save childID on file */
            printf("Saving actual logger ID\n");
            sprintf(buffer, "%d", loggerID);
            pidFD = open(LOG_PID_F, O_WRONLY | O_TRUNC | O_CREAT, 0777);
            if (write(pidFD, buffer, strlen(buffer)) == -1) {
                perror("Saving result");
                exit(EXIT_FAILURE);
            }
            close(pidFD);
        }
    }

    /* 'kill' special command check */
    if (!strcmp(sett.cmd, "kill")) {
        int fdFIFO = open(LOGGER_FIFO, O_WRONLY);

        kill(loggerID, SIGUSR1);
        kill(loggerID, SIGCONT);
        waitpid(loggerID, NULL, 0);

        close(fdFIFO);
        printf("Daemon killed\n");
        exit(EXIT_SUCCESS);
    }

    /******************************************************************************/
    /********************************* SHELL SETUP ********************************/
    /******************************************************************************/
    unlink(TO_SHELL_FIFO); // USE PIPE //
    unlink(FROM_SHELL_FIFO);
    mkfifo(TO_SHELL_FIFO, 0777);
    mkfifo(FROM_SHELL_FIFO, 0777);
    int toShell;
    int fromShell;

    /* shell process */
    if ((shellID = fork()) < 0) {
        perror("shell fork");
        exit(EXIT_FAILURE);
    }
    if (shellID == 0) {
        //setsid();
        toShell = open(TO_SHELL_FIFO, O_RDONLY);
        fromShell = open(FROM_SHELL_FIFO, O_WRONLY);

        dup2(toShell, STDIN_FILENO);
        dup2(fromShell, STDOUT_FILENO);
        dup2(fromShell, STDERR_FILENO);

        close(toShell);
        close(fromShell);

        char *arguments[] = {"sh", 0};
        execvp(arguments[0], arguments);
        perror("shell process");
        exit(EXIT_FAILURE);
    }

    /******************************************************************************/
    /********************************* MAIN PROGRAM *******************************/
    /******************************************************************************/

    printf("Father: %d\n", getpid());
    printf("Logger ID: %d\n", loggerID);
    printf("ShellID: %d\n", shellID);

    toShell = open(TO_SHELL_FIFO, O_WRONLY);
    fromShell = open(FROM_SHELL_FIFO, O_RDONLY);
    Pk data;

    printf("COMMD: %s\n", sett.cmd);

    int i = 0;
    int f = 0;
    bool lastIsPipe = false;
    for (f = 0; f <= strlen(sett.cmd); f++) { // pwd; ls'\0'
        if ((sett.cmd[f] == ';') || (sett.cmd[f] == '\0')) {
            segmentcpy(data.cmd, sett.cmd, i, f - 1);
            printf("DATA: %s\n", data.cmd);
            executeCommand(toShell, fromShell, &data, lastIsPipe);
            lastIsPipe = false;
            i = f + 1;
            printf("COMMAND:\t%s\n", data.cmd);
            printf("OUTPUT:\t\t%s\n", data.out);
            printf("RETURN C.:\t%s\n\n", data.returnC);
        } else if (sett.cmd[f] == '|') {
            segmentcpy(data.cmd, sett.cmd, i, f - 1);
            executeCommand(toShell, fromShell, &data, lastIsPipe);
            lastIsPipe = true;
            i = f + 1;
            printf("COMMAND:\t%s\n", data.cmd);
            printf("OUTPUT:\t\t%s\n", data.out);
            printf("RETURN C.:\t%s\n\n", data.returnC);
        }
    }
    printf("FINISHED\n");

    // strcpy(data.cmd, "cd awffwan");
    // executeCommand(toShell, fromShell, &data, false);
    // printf("COMMAND:\t%s\n", data.cmd);
    // printf("OUTPUT:\t\t%s\n", data.out);
    // printf("RETURN C.:\t%s\n\n", data.returnC);

    // strcpy(data.cmd, "wc");
    // executeCommand(toShell, fromShell, &data, true);
    // printf("COMMAND:\t%s\n", data.cmd);
    // printf("OUTPUT:\t\t%s\n", data.out);
    // printf("RETURN C.:\t%s\n\n", data.returnC);

    // strcpy(data.cmd, "pwd");
    // executeCommand(toShell, fromShell, &data, true);
    // printf("COMMAND:\t%s\n", data.cmd);
    // printf("OUTPUT:\t\t%s\n", data.out);
    // printf("RETURN C.:\t%s\n\n", data.returnC);

    close(fromShell);
    close(toShell);
    unlink(TO_SHELL_FIFO);
    unlink(FROM_SHELL_FIFO);
    waitpid(shellID, NULL, 0); /* NEED ERROR CHECKING HERE */
    /* sending data to logger */
    /*
    fdFIFO = open(myFifo, O_WRONLY);
    write(fdFIFO, "TYPE", strlen("TYPE") + 1);
    write(fdFIFO, sett.cmd, strlen(sett.cmd) + 1);
    write(fdFIFO, sett.cmd, strlen(sett.cmd) + 1);
    write(fdFIFO, outBuff, strlen(outBuff) + 1);
    write(fdFIFO, retCode, strlen(retCode) + 1);
    kill(loggerID, SIGCONT);
    close(fdFIFO);
    */
    return EXIT_SUCCESS;
}
