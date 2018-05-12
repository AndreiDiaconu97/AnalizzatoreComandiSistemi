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
    char settings[SET_N][2][PATH_S];
    initSettings(settings);

    /* mapping variables to settings-array */
    char *outfile = settings[outF][1];
    char *errfile = settings[errF][1];
    char *logfile = settings[logF][1];
    /* non char* variables don't auto-update */
    size_t buffS;
    bool returnCode;

    /* read and check user arguments */
    if (!readArguments(argc, argv, settings)) {
        printf("CLOSING PROGRAM\n");
        exit(EXIT_FAILURE);
    } else {
        buffS = (size_t)atoi(settings[maxL][1]);
        if (!(strcmp(settings[code][1], "true"))) {
            returnCode = true;
        } else {
            returnCode = false;
        }
    }

    showSettings(settings);

    /* program variables */
    int loggerID, fdID = 0;
    char loggerIDfile[PATH_S] = "loggerPid.txt";
    char *buffer = malloc(sizeof(char) * buffS);

    /* create FIFO file if needed */
    char *myFifo = "/tmp/myfifo";
    if (mkfifo(myFifo, 0666)) {
        perror("FIFO");
    }

    /* search for existing logger before any forking */
    loggerIsRunning(&fdID, &loggerID, loggerIDfile);

    /* fork for logger if no existing one is found */
    if (loggerID == 0) {
        if ((loggerID = fork()) < 0) {
            perror("FORK result");
            exit(EXIT_FAILURE);
        }
        if (loggerID == 0) {
            /* logger must be leader of its own group in order to be a daemon */
            setsid();
            //printf("Child:%d\n", getpid());
            char *argss[3] = {logfile, loggerIDfile, myFifo};
            logger(sizeof argss, argss); //logger process
            exit(EXIT_SUCCESS);
        } else {
            /* if there is not logger found save childID on file */
            printf("Saving actual logger ID\n");
            sprintf(buffer, "%d", loggerID);
            if (write(fdID, buffer, strlen(buffer)) == -1) { //writing actual running logger ID
                perror("Saving result");
            }
            close(fdID);
        }
    }

    /* father process */
    printf("Father: %d\n", getpid());
    printf("Logger ID: %d\n", loggerID);
    char *prova = "zero";
    int fdFIFO;

    //printf("If you read this means that child opened FIFO too.\n");
    printf("\nREADY:\n");
    while (strcmp(buffer, "quit")) {
        printf(">>");
        getline(&buffer, &buffS, stdin);
        rmNewline(buffer);

        kill(loggerID, SIGCONT);
        fdFIFO = open(myFifo, O_WRONLY); //open one end of the pipe

        if (!strcmp(buffer, "kill")) {
            write(fdFIFO, buffer, strlen(buffer) + 1);
            kill(loggerID, SIGCONT);
        } else {
            //runCommand("ls", fdFIFO);
            write(fdFIFO, buffer, strlen(buffer) + 1);
            write(fdFIFO, "CONAD", strlen("CONAD") + 1);
            write(fdFIFO, "2", strlen("2") + 1);
            write(fdFIFO, "data", strlen("data") + 1);
            //pause();
            //sleep(1);
            //write(fdFIFO, prova, sizeof prova);
        }
        close(fdFIFO);
    }
    return EXIT_SUCCESS;
}
