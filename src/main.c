#include "myLibrary.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
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
    //printInfo(&sett);

    /* processes IDs */
    int fatherID = getpid();
    int loggerID = -1;
    int shellID = -1;

    /******************************************************************************/
    /******************************** LOGGER SETUP ********************************/
    /******************************************************************************/
    int pidFD;
    bool needNew = false;
    char logIDbuffer[PID_S];

    /* search for existing logger before any forking */
    pidFD = open(HOME TEMP_DIR LOG_PID_F, O_RDONLY, 0777);
    if (pidFD == -1) {
        /* logger process id file not found */
        needNew = true;
    } else {
        /* existing file found, now read */
        int pIDSize = read(pidFD, logIDbuffer, sizeof logIDbuffer);
        logIDbuffer[pIDSize] = '\0';
        loggerID = atoi(logIDbuffer);
        if (getpgid(loggerID) < 0) {
            /* logger non existent */
            needNew = true;
        } else {
            /* check fifo accessibility */
            int fifoFD = open(HOME TEMP_DIR LOGGER_FIFO_F, O_RDWR, 0777);
            if (fifoFD == -1) {
                needNew = true;
                kill(loggerID, SIGKILL);
            }
            close(fifoFD);
        }
    }
    close(pidFD);

    /* 'kill' special command check (kills logger) */
    if (sett.needKill == true) {
        if (updateSettings) {
            saveSettings(&sett);
        }
        if (needNew) {
            printf("no logger\n");
        } else {
            killLogger(loggerID);
        }
        exit(EXIT_SUCCESS);
    }

    /* update settings file and logger if needed */
    if (updateSettings) {
        saveSettings(&sett);
        printf("Settings will be applied with logger restart\n");
        printf("To restart logger use -k=true and type again command\n\n");
    }

    /* close program if command is empty */
    if (strcmp(sett.cmd, "") == 0) {
        printf("No command found, closing...\n");
        exit(EXIT_FAILURE);
    }

    if (!needNew) {
        /* check for log file existence */
        char logFile[2048];
        sprintf(logFile, "%s%s", HOME LOG_DIR, sett.logF);
        if (open(logFile, O_RDONLY, 0777) == -1) {
            printf("Log file not found\n");
            killLogger(loggerID);
            needNew = true;
        }
    }

    /* fork for logger if no existing one is found */
    if (needNew) {
        printf("Started new logger process\n\n");

        /* try to create fifo files (error managing is not a problem here) */
        if ((mkfifo(HOME TEMP_DIR LOGGER_FIFO_F, 0777) == -1) && (errno != EEXIST)) {
            perror(LOGGER_FIFO_F "creation");
            exit(EXIT_FAILURE);
        }

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
            if (write(pidFD, logIDbuffer, strlen(logIDbuffer)) == -1) {
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

        /* s shell just once per program */
        char *arguments[] = {"sh", 0};
        execvp(arguments[0], arguments);

        perror("shell process");
        exit(EXIT_FAILURE);
    }

    /******************************************************************************/
    /********************************* MAIN PROGRAM *******************************/
    /******************************************************************************/
    showSettings(&sett);
    printf("Father ID: %d\n", fatherID);
    printf("Logger ID: %d\n", loggerID);
    printf("Shell  ID: %d\n", shellID);
    printf("\nCommand: %s\n", sett.cmd);
    printf("Output:\n\n");

    signal(SIGUSR1, unpauser);
    close(toShell[0]);
    close(fromShell[1]);

    /* initialize Packet struct */
    Pk data;
    strcpy(data.origCmd, sett.cmd);
    char tmpID[50];

    sprintf(tmpID, "%d", fatherID);
    strcpy(data.fatherID, tmpID);

    sprintf(tmpID, "%d", shellID);
    strcpy(data.shellID, tmpID);

    ////////////////////
    char superstring[PIPE_BUF];
    int pos;
    for (pos = 0; pos < 10; pos++) {
        superstring[pos] = '\0';
    }
    superstring[pos - 1] = '?';

    /* if command has && or ||, no subcommand splitting is performed */
    if (strstr(data.origCmd, "&&") || strstr(data.origCmd, "||")) {
        strcpy(data.cmd, data.origCmd);
        executeCommand(toShell[1], fromShell[0], &data, false, &proceed);
        pos += appendPack(&data, &sett, superstring + pos);
        printf("%s\n", data.out);
    } else {
        /* command factorization */
        int sx = 0;
        int dx = 0;
        bool lastIsPipe = false;
        bool currentIsPipe = false;
        bool needToExec = false;
        int open_p = 0;
        int subID = 1;
        char tmpID[50];
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

                // copy subID to pack
                sprintf(tmpID, "%d", subID++);
                strcpy(data.cmdID, tmpID);

                executeCommand(toShell[1], fromShell[0], &data, lastIsPipe, &proceed);
                pos += appendPack(&data, &sett, superstring + pos);
                sx = dx + 1;
                lastIsPipe = currentIsPipe;

                /* print output on screen only if command is not piped */
                if (!lastIsPipe) {
                    printf("%s\n", data.out);
                }
            }
            needToExec = false;
        }
    }
    /* write superstring size at the beginning of the superstring */
    char stringSizeStr[10];
    sprintf(stringSizeStr, "%d", pos - 10);
    strcpy(superstring, stringSizeStr);

    /* send superstring */
    int loggerFd;
    if ((loggerFd = open(HOME TEMP_DIR LOGGER_FIFO_F, O_WRONLY)) == -1) {
        perror("Opening logger fifo");
        exit(EXIT_FAILURE);
    }
    write(loggerFd, superstring, pos);
    close(loggerFd);

    printf("\nFINISHED\n");
    /* close opened pipes */
    close(toShell[1]);
    close(fromShell[0]);
    return EXIT_SUCCESS;
}