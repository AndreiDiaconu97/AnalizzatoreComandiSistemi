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

    /* some program variables */
    int fdID;
    int loggerID = 0;
    int shellID;
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

    char retCode[3];
    char outBuff[sett.maxOut];
    char trashBuff[100];

    FILE *fp = popen(sett.cmd, "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    size_t len = fread(outBuff, sizeof(char), sett.maxOut - 1, fp);
    if (!len) {
        perror("reading output from FILE*");
        exit(EXIT_FAILURE);
    }

    /* limiting reading to outBuff size */
    if (len == sett.maxOut - 1) {
        outBuff[len] = '\0';
    } else { /* ending char is a '\n' */
        outBuff[len - 1] = '\0';
    }

    /* emptying pipe in order to be sure it is not used anymore */
    while (fread(trashBuff, sizeof(char), sizeof(trashBuff), fp)) {
        /** removing this cycle causes broken pipe error if pclose(fp) completes
         * before given command fully writes the output on pipe */
    }

    int tmpCode = WEXITSTATUS(pclose(fp));
    sprintf(retCode, "%d", tmpCode);

    /* sending data to logger */
    int fdFIFO = open(myFifo, O_WRONLY);
    write(fdFIFO, "TYPE", strlen("TYPE") + 1);
    write(fdFIFO, sett.cmd, strlen(sett.cmd) + 1);
    write(fdFIFO, outBuff, strlen(outBuff) + 1);
    write(fdFIFO, retCode, strlen(retCode) + 1);
    kill(loggerID, SIGCONT);
    close(fdFIFO);

    return EXIT_SUCCESS;
}
