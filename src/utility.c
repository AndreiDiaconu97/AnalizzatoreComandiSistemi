#include "myLibrary.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

void initSettings(char settings[SET_N][2][PATH_S]) {
    /* available settings */
    strcpy(settings[outF][0], "outfile");
    strcpy(settings[errF][0], "errfile");
    strcpy(settings[logF][0], "logfile");
    strcpy(settings[maxL][0], "maxLen");
    strcpy(settings[code][0], "code");

    /* DEFAULT values */
    strcpy(settings[outF][1], "change this");
    strcpy(settings[errF][1], "change this");
    strcpy(settings[logF][1], "output.log");
    sprintf(settings[maxL][1], "%d", BUFF_S);
    strcpy(settings[code][1], "true");
}

/**
 * must return true
 */
bool readArguments(int argc, char **argv, char settings[SET_N][2][PATH_S]) {
    bool allValid = true;
    bool argValid = true;
    char tmpArg[PATH_S];
    char tmpVal[PATH_S];
    /* reading program arguments */
    for (int i = 0; i < argc; i++) {
        strncpy(tmpArg, "", sizeof tmpArg);
        strncpy(tmpVal, "", sizeof tmpVal);
        /* find command arguments (commands begin with "--") */
        if (!strncmp(argv[i], "-", 1)) {
            /* Arguments are in form '--command=value',thus  */
            /* finding the '=' gives the splitting point.    */
            for (int t = 0; t < strlen(argv[i]); t++) {
                /* get command */
                if (argv[i][t] != '=') {
                    tmpArg[strlen(tmpArg)] = argv[i][t];
                    tmpArg[strlen(tmpArg)] = '\0';
                } else {
                    /* found the '=' char, now get value */
                    for (int x = t + 1; x < strlen(argv[i]); x++) {
                        tmpVal[strlen(tmpVal)] = argv[i][x];
                        tmpVal[strlen(tmpVal)] = '\0';
                    }
                    break; /* exit form command loop too */
                }
            }
            //printf("Arg:%s | Val:%s\n", tmpArg, tmpVal); //read check

            argValid = evaluateCommand(settings, tmpArg, tmpVal);
            if (allValid) { //result must stay false if only one argument is invalid
                allValid = argValid;
            }
        }
    }
    return allValid;
}

bool evaluateCommand(char settings[SET_N][2][PATH_S], char *arg, char *val) {
    bool result = true;
    if ((!strcmp(arg, "--outfile")) || (!strcmp(arg, "-o"))) {
        strcpy(settings[outF][1], val);
    } else if ((!strcmp(arg, "--errfile")) || (!strcmp(arg, "-e"))) {
        strcpy(settings[errF][1], val);
    } else if ((!strcmp(arg, "--logfile")) || (!strcmp(arg, "-l"))) {
        strcpy(settings[logF][1], val);
    } else if ((!strcmp(arg, "--maxLen")) || (!strcmp(arg, "-m"))) {
        if (!strncmp(val, "-", 1)) {
            printf("Command '%s': invalid negative number\n", arg);
            result = false;
        } else if ((atoi(val))) { //checking for integer value
            strcpy(settings[maxL][1], val);
        } else {
            printf("Command '%s': couldn't parse value '%s'\n", arg, val);
            result = false;
        }

    } else if ((!strcmp(arg, "--code")) || (!strcmp(arg, "-c"))) {
        if ((!strcmp(val, "true")) || (!strcmp(val, "false"))) {
            strcpy(settings[code][1], val);
        } else {
            printf("Argument:%s - Invalid input:%s\n", arg, val);
            result = false;
        }
    } else {
        printf("Invalid argument found:'%s=%s'\n", arg, val);
        result = false;
    }
    return result;
}

void showSettings(char settings[SET_N][2][PATH_S]) {
    printf("--- SETTINGS ---------------------------\n");
    for (int i = 0; i < 5; i++) {
        printf("%d) %s: %s\n", i, settings[i][0], settings[i][1]);
    }
    printf("----------------------------------------\n\n");
}

void loggerIsRunning(int *fdID, int *loggerID, char *loggerIDfile) {
    int pId_MaxLen = 10;
    char buffer[pId_MaxLen];

    /* if loggerID file not found, create one and write new logger ID */
    *fdID = open(loggerIDfile, O_RDWR);
    if (*fdID == -1) {
        perror("Opening logger ID file");
        if ((*fdID = open(loggerIDfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
            perror("Creating file");
            exit(EXIT_FAILURE);
        }
    } else {
        if (read(*fdID, buffer, pId_MaxLen)) { //if file is not empty
            printf("Found existing logger ID\n");
            read(*fdID, buffer, pId_MaxLen);
            *loggerID = atoi(buffer);
        }
    }
}

void runCommand(char *cmd, int fd) {
    FILE *fp;
    char buffer[PATH_S];

    /* Open the command for reading. */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        exit(1);
    }
    /* Read the output a line at a time - output it. */
    while (fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
        //printf("%s", buffer);
        write(fd, buffer, strlen(buffer));
    }
    pclose(fp);
}

void removeFifo(char *fifoPath) {
    char rmFifo[PATH_S] = "";
    strcat(rmFifo, "rm ");
    strcat(rmFifo, fifoPath);
    system(rmFifo);
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