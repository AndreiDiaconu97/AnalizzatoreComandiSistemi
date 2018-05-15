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

    printf("COMMAND: %s\n", sett.cmd);

    /* program variables */
    int loggerID = 0, shellID, fdID;
    char loggerIDfile[PATH_S] = "loggerPid.txt";
    char buffer[sett.maxBuff];

    /* create FIFO file if it does not exist */
    char *myFifo = "/tmp/myfifo";
    mkfifo(myFifo, 0666);

    /******************************************************************************/
    /******************************** LOGGER SETUP ********************************/
    /******************************************************************************/

    /* search for existing logger before any forking */
    loggerIsRunning(&fdID, &loggerID, loggerIDfile);

    /* fork for logger if no existing one is found */
    if (loggerID == 0) {
        if ((loggerID = fork()) < 0) {
            perror("logger fork() result");
            exit(EXIT_FAILURE);
        }
        if (loggerID == 0) {
            /* logger must be leader of its own group in order to be a daemon */
            setsid();

            /* logger call with appropriate arguments */
            char *args[3] = {sett.logF, loggerIDfile, myFifo};
            logger(args);
            exit(EXIT_SUCCESS);
        } else {
            /* if there is not logger found save childID on file */
            printf("Saving actual logger ID\n");
            sprintf(buffer, "%d", loggerID);
            if (write(fdID, buffer, strlen(buffer)) == -1) {
                perror("Saving result");
            }
            close(fdID);
        }
    }

    /******************************************************************************/
    /********************************* MAIN PROGRAM *******************************/
    /******************************************************************************/
    printf("Father: %d\n", getpid());
    printf("Logger ID: %d\n", loggerID);

    /* 'kill' special command check (only for debugging) */
    if (!strcmp(sett.cmd, "kill")) {
        int fdFIFO = open(myFifo, O_WRONLY); //open one end of the pipe
        kill(loggerID, SIGUSR1);
        kill(loggerID, SIGCONT);
        waitpid(loggerID, NULL, 0);
        close(fdFIFO);
        printf("Daemon killed\n");
        exit(EXIT_SUCCESS);
    }

    int toChild[2];
    int toParent[2];
    pipe(toChild);
    pipe(toParent);

    if ((shellID = fork()) < 0) {
        perror("shell fork() result");
        exit(EXIT_FAILURE);
    }

    if (!shellID) {
        char tmpCmd[100];
        char *template[] = {"/bin/sh", "-c", tmpCmd, 0};
        char *returnStatus = "; echo $?";
        /* CAREFUL!: $? returns exit code of LAST command */

        dup2(toParent[1], STDOUT_FILENO);
        dup2(toParent[1], STDERR_FILENO);
        /* close pipes fds */
        close(toChild[1]);
        close(toParent[1]);
        close(toParent[0]);

        /* waiting input from pipe */
        read(toChild[0], tmpCmd, sizeof tmpCmd);
        strcat(tmpCmd, returnStatus);
        close(toChild[0]);
        //close(STDERR_FILENO);

        /* system shell invocation with given command */
        execv(template[0], template);
    } else {
        int count;
        char outBuff[sett.maxOut];
        char *retCode;
        char *errDesc;

        /* close fds not required by parent */
        close(toChild[0]);
        close(toParent[1]);

        /* write to child’s stdin */
        write(toChild[1], sett.cmd, strlen(sett.cmd) + 1);

        /* wait for child to read from pipe and generate output */
        wait(NULL);

        /* read from child’s stdout */
        count = read(toParent[0], outBuff, sett.maxOut - 1);
        if (count < 0) {
            perror("IO Error\n");
            exit(EXIT_FAILURE);
        } else if (count == sett.maxOut - 1) { /* checki this code */
            rmNewline(outBuff);
        }
        outBuff[count] = '\0';

        /* split command output from return code */
        retCode = cmdOutSplitReturnCode(outBuff, retCode);

        /* error description from error code */
        //errDesc = strerror(atoi(retCode));

        close(toChild[1]);
        close(toParent[0]);

        /* sending data to logger */
        int fdFIFO = open(myFifo, O_WRONLY);
        write(fdFIFO, "TYPE", strlen("TYPE") + 1);
        write(fdFIFO, sett.cmd, strlen(sett.cmd) + 1);
        write(fdFIFO, outBuff, strlen(outBuff) + 1);
        write(fdFIFO, retCode, strlen(retCode) + 1);
        kill(loggerID, SIGCONT);
        close(fdFIFO);
    }
    return EXIT_SUCCESS;
}
