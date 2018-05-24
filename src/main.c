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
//#include <time.h>
#include <unistd.h>

/* handler for SIGUSR1 signal used for father-shell communication */
bool proceed = false;
void unpauser(int sig) {
    proceed = true;
}

int main(int argc, char *argv[]) {
    printf("---- STATISTICHE COMANDI E ANALISI ----------------------------------------------------------------------\n\n");

    /* user-settings container */
    settings sett;
    initSettings(&sett);

    bool updateSettings = false;
    /* read and check user arguments */
    if (!readArguments(argc, argv, &sett, &updateSettings)) {
        printf("CLOSING PROGRAM\n");
        exit(EXIT_FAILURE);
    }
    printInfo(&sett);

    /* processes IDs */
    int fatherID = getpid();
    int loggerID;
    int shellID;

    /******************************************************************************/
    /******************************** LOGGER SETUP ********************************/
    /******************************************************************************/
    int pidFD;
    bool needNew = false;
    char logIDbuffer[PID_S];

    /* search for existing logger before any forking */
    pidFD = open(HOME TEMP_DIR LOG_PID_F, O_RDONLY, 0777);
    if (pidFD == -1) {
        needNew = true;
    } else {
        int pIDSize = read(pidFD, logIDbuffer, sizeof logIDbuffer);
        loggerID = atoi(logIDbuffer);
        if (getpgid(loggerID) < 0) {
            needNew = true;
        } else {
            int fifoFD = open(HOME TEMP_DIR LOGGER_FIFO_F, O_RDWR, 0777);
            if (fifoFD == -1) {
                needNew = true;
                kill(loggerID, SIGKILL);
            }
            close(fifoFD);
        }
    }
    close(pidFD);

    /** Update settings file and logger if found user arguments.
     * (deas not check for differences in settings file, always replace)
    **/
    if (updateSettings) {
        saveSettings(&sett);
        if (!needNew) {
            printf("Restarting logger process in order to apply latest settings...\n");
            /* restart logger */
            killLogger(loggerID);
            needNew = true;
            printf("logger restarted\n");
        }
        printf("\n");
    }

    /* close program if command is empty */
    if (strcmp(sett.cmd, "") == 0) {
        printf("No command found, closing...\n");
        exit(EXIT_FAILURE);
    }

    /* 'kill' special command check (kills logger) */
    if (!strcmp(sett.cmd, "kill")) {
        if (needNew) {
            printf("no logger\n");
        } else {
            killLogger(loggerID);
        }
        exit(EXIT_SUCCESS);
    }

    if (!needNew) {
        if (open(HOME LOG_DIR LOG_F, O_RDONLY, 0777) == -1) {
            // log file not found, restart logger
            printf("Log file not found, creating new one\n");
            killLogger(loggerID);
            needNew = true;
        }
    }

    /* fork for logger if no existing one is found */
    if (needNew) {
        printf("Initialising new logger process\n\n");

        /* reset fifo file */
        remove(HOME TEMP_DIR LOGGER_FIFO_F);
        mkfifo(HOME TEMP_DIR LOGGER_FIFO_F, 0777);

        /* fork for logger */
        if ((loggerID = fork()) < 0) {
            perror("logger fork");
            exit(EXIT_FAILURE);
        }
        if (loggerID == 0) { //child
            /* logger must be leader of its own group in order to be a daemon */
            setsid();

            /* logger call with appropriate arguments */
            logger(&sett);
            printf("Logger error\n");
            exit(EXIT_FAILURE);
        } else { //father
            /* saving childID on file */
            sprintf(logIDbuffer, "%d", loggerID);
            pidFD = open(HOME TEMP_DIR LOG_PID_F, O_WRONLY | O_TRUNC | O_CREAT, 0777);
            if (write(pidFD, logIDbuffer, strlen(logIDbuffer) + 1) == -1) {
                perror("Saving result");
                exit(EXIT_FAILURE);
            }
            close(pidFD);
        }
    } else {
        printf("Existing logger found\n\n");
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

        /* open shell just once per program */
        char *arguments[] = {"sh", 0};
        execvp(arguments[0], arguments);

        perror("shell process");
        exit(EXIT_FAILURE);
    }

    /******************************************************************************/
    /********************************* MAIN PROGRAM *******************************/
    /******************************************************************************/
    printf("Father ID: %d\n", fatherID);
    printf("Logger ID: %d\n", loggerID);
    printf("Shell  ID: %d\n", shellID);

    close(toShell[0]);
    close(fromShell[1]);
    signal(SIGUSR1, unpauser);

    Pk data;
    strcpy(data.origCmd, sett.cmd);

    printf("\nCommand: %s\n", data.origCmd);

    /* command factorization */
    int sx = 0;
    int dx = 0;
    bool lastIsPipe = false;
    bool currentIsPipe = false;
    bool needToExec = false;
    int open_p = 0;
    for (dx = 0; dx <= strlen(data.origCmd); dx++) {
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

        if (needToExec && open_p == 0) {
            segmentcpy(data.cmd, data.origCmd, sx, dx - 1);
            executeCommand(toShell[1], fromShell[0], &data, lastIsPipe, &proceed);
            sendData(&data, &sett);
            sx = dx + 1;
            lastIsPipe = currentIsPipe;
        }
        needToExec = false;
    }

    printf("\nFINISHED\n");

    /* close opened pipes */
    close(toShell[1]);
    close(fromShell[0]);
    return EXIT_SUCCESS;
}