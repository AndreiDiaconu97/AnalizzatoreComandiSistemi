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
    char *command = settings[shCmd][1];
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
    int loggerID, shellID, fdID = 0;
    char loggerIDfile[PATH_S] = "loggerPid.txt";
    char buffer[BUFF_S];

    /* create FIFO file if needed */
    char *myFifo = "/tmp/myfifo";
    if (mkfifo(myFifo, 0666)) {
        perror("FIFO");
    }

    /**********************************************************/
    /********************** LOGGER SETUP **********************/
    /***************************************************+******/
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
            //printf("Child:%d\n", getpid());
            char *args[3] = {logfile, loggerIDfile, myFifo};
            logger(args); //logger process
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

    /**********************************************************/
    /********************** MAIN PROGRAM **********************/
    /***************************************************+******/

    // pipes for parent to write and read
    int toChild[2];
    int toParent[2];
    pipe(toChild);
    pipe(toParent);

    /* closes logger with customm SIGUSR1 */
    if (!strcmp(command, "kill")) {
        int fdFIFO = open(myFifo, O_WRONLY); //open one end of the pipe
        kill(loggerID, SIGUSR1);
        kill(loggerID, SIGCONT);
        waitpid(loggerID, NULL, 0);
        printf("Daemon killed\n");
        close(fdFIFO);
        exit(EXIT_SUCCESS);
    }

    if ((shellID = fork()) < 0) {
        perror("shell fork() result");
        exit(EXIT_FAILURE);
    }
    if (!shellID) {
        char tmpCmd[100];
        char *template[] = {"/bin/sh", "-c", tmpCmd, 0};
        char *returnStatus = "; echo $?";

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
        printf("Father: %d\n", getpid());
        printf("Logger ID: %d\n", loggerID);
        int count;
        char outBuff[OUT_S];
        char *errCode;
        char *errDesc;

        /* close fds not required by parent */
        close(toChild[0]);
        close(toParent[1]);

        // Write to child’s stdin
        write(toChild[1], command, strlen(command) + 1);

        /*wait for child to read from pipe and generate output */
        wait(NULL);

        // Read from child’s stdout
        count = read(toParent[0], outBuff, OUT_S - 1);
        if (count < 0) {
            perror("IO Error\n");
            exit(EXIT_FAILURE);
        } else if (count == OUT_S - 1) {
            outBuff[OUT_S - 1] = '\0';
        }
        /* splitting stderr output from stdout */
        outBuff[count] = '\0';
        outBuff[strlen(outBuff) - 1] = '\0';
        errCode = strrchr(outBuff, '\n');
        *errCode = '\0';
        errCode++;

        /* data printf() test */
        errDesc = strerror(atoi(errCode));
        printf("%s\n", errCode);
        printf("ERR:%s\n", errCode);
        printf("DESC:%s\n", errDesc);

        close(toChild[1]);
        close(toParent[0]);

        int fdFIFO = open(myFifo, O_WRONLY); //open one end of the pipe
        write(fdFIFO, "TYPE", strlen("TYPE") + 1);
        write(fdFIFO, command, strlen(command) + 1);
        write(fdFIFO, outBuff, strlen(outBuff) + 1);
        write(fdFIFO, errCode, strlen(errCode) + 1);
        kill(loggerID, SIGCONT);

        close(fdFIFO);
    }

    /* father process */
    /*
    printf("Father: %d\n", getpid());
    printf("Logger ID: %d\n", loggerID);
    char *prova = "zero";
    int fdFIFO;

    sprintf(buffer, "%s; echo $?", command);
    FILE *fp = popen(buffer, "r");

    fread(buffer, 1, BUFF_S, fp);
    buffer[strlen(buffer) - 1] = '\0';

    printf("%s\n", buffer);

    kill(loggerID, SIGCONT);
    fdFIFO = open(myFifo, O_WRONLY); //open one end of the pipe
    write(fdFIFO, "TYPE", strlen("TYPE") + 1);
    write(fdFIFO, command, strlen(command) + 1);
    write(fdFIFO, buffer, strlen(buffer) + 1);
    write(fdFIFO, "2", strlen("2") + 1);

    close(fdFIFO);
    */

    /*
    printf("\nREADY:\n");
    while (strcmp(buffer, "quit") != 0) {
        printf(">>");
        getline(&buffer, &buffS, stdin);
        rmNewline(buffer);

        if (strcmp(buffer, "kill") == 0) {
            write(fdFIFO, buffer, strlen(buffer) + 1);
            kill(loggerID, SIGCONT);
        } else {
            //runCommand("ls", fdFIFO);
            write(fdFIFO, "TYPE", strlen("TYPE") + 1);
            write(fdFIFO, command, strlen(command) + 1);
            write(fdFIFO, buffer, strlen(buffer) + 1);
            write(fdFIFO, "2", strlen("2") + 1);
            //pause();
            //sleep(1);
            //write(fdFIFO, prova, sizeof prova);
        }
        close(fdFIFO);
    }
    */
    //free(buffer);
    return EXIT_SUCCESS;
}
