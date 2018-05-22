#include "myLibrary.h"
//#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//#include <sys/types.h>
#include <sys/wait.h>
//#include <syslog.h>
//#include <time.h>
#include <unistd.h>

bool proceed = false;
void unpauser(int sig) {
    proceed = true;
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

    /* process IDs */
    int fatherID = getpid();
    int loggerID;
    int shellID;

    /******************************************************************************/
    /******************************** LOGGER SETUP ********************************/
    /******************************************************************************/
    int pidFD;
    bool needNew = false;
    char logIDbuffer[PID_S];

    /* search for existing logger   any forking */
    pidFD = open(LOG_PID_F, O_RDONLY, 0777);
    if (pidFD == -1) {
        needNew = true;
    } else {
        int pIDSize = read(pidFD, logIDbuffer, sizeof logIDbuffer);
        loggerID = atoi(logIDbuffer);
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

    /* 'kill' special command check (kills logger) */
    if (!strcmp(sett.cmd, "kill")) {
        if (needNew) {
            printf("No Daemon\n");
        } else {
            int loggerFd = open(LOGGER_FIFO, O_WRONLY);
            kill(loggerID, SIGUSR1);
            waitpid(loggerID, NULL, 0);
            close(loggerFd);
            printf("Daemon killed\n");
        }
        exit(EXIT_SUCCESS);
    }

    /* fork for logger if no existing one is found */
    if (needNew) {
        /* reset fifo file */
        remove(LOGGER_FIFO);
        mkfifo(LOGGER_FIFO, 0777);

        /* fork for logger */
        if ((loggerID = fork()) < 0) {
            perror("logger fork");
            exit(EXIT_FAILURE);
        }
        if (loggerID == 0) { //child
            /* logger must be leader of its own group in order to be a daemon */
            setsid();

            /* logger call with appropriate arguments */
            char *args[3] = {sett.logF, LOG_PID_F, LOGGER_FIFO};
            logger(args);
            printf("Logger error\n");
            exit(EXIT_FAILURE);
        } else { //father
            /* saving childID on file */
            printf("Saving actual logger ID\n");
            sprintf(logIDbuffer, "%d", loggerID);
            pidFD = open(LOG_PID_F, O_WRONLY | O_TRUNC | O_CREAT, 0777);
            if (write(pidFD, logIDbuffer, strlen(logIDbuffer) + 1) == -1) {
                perror("Saving result");
                exit(EXIT_FAILURE);
            }
            close(pidFD);
        }
    }

    /******************************************************************************/
    /********************************* SHELL SETUP ********************************/
    /******************************************************************************/
    int toShell[2];
    int fromShell[2];
    pipe(toShell);
    pipe(fromShell);

    /* shell process */
    if ((shellID = fork()) < 0) {
        perror("shell fork");
        exit(EXIT_FAILURE);
    }
    if (shellID == 0) {
        dup2(toShell[0], STDIN_FILENO);
        dup2(fromShell[1], STDOUT_FILENO);
        dup2(fromShell[1], STDERR_FILENO);

        close(toShell[0]);
        close(toShell[1]);
        close(fromShell[0]);
        close(fromShell[1]);

        char *arguments[] = {"sh", 0};
        execvp(arguments[0], arguments);
        perror("shell process");
        exit(EXIT_FAILURE);
    }

    /******************************************************************************/
    /********************************* MAIN PROGRAM *******************************/
    /******************************************************************************/
    printf("Father: %d\n", fatherID);
    printf("Logger ID: %d\n", loggerID);
    printf("ShellID: %d\n", shellID);

    close(toShell[0]);
    close(fromShell[1]);
    signal(SIGUSR1, unpauser);

    Pk data;
    strcpy(data.origCmd, sett.cmd);

    printf("\nFULL COMMAND: %s\n", data.origCmd);

    /* command factorization */
    int sx = 0;
    int dx = 0;
    bool lastIsPipe = false;
    bool currentIsPipe = false;
    bool needToExec = false;
    int open_p = 0;
    for (dx = 0; dx <= strlen(data.origCmd); dx++) { // pwd; ls'\0'
        switch (data.origCmd[dx]) {
        case '\0':
        case ';':
            currentIsPipe = false;
            needToExec = true;
            break;
        case '|':
            currentIsPipe = true;
            needToExec = true;
            break;
        case '(':
            open_p++;
            break;
        case ')':
            open_p--;
            break;
        default:
            break;
        }

        if (needToExec && open_p==0) {
            segmentcpy(data.cmd, data.origCmd, sx, dx - 1);
            executeCommand(toShell[1], fromShell[0], &data, lastIsPipe, &proceed);
            sendData(&data);
            sx = dx + 1;
            lastIsPipe = currentIsPipe;
        }
        needToExec = false;
    }

    printf("\nFINISHED\n");
    close(toShell[1]);
    close(fromShell[0]);
    return EXIT_SUCCESS;
}