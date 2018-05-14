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
    strcpy(settings[shCmd][0], "command");
    strcpy(settings[outF][0], "outfile");
    strcpy(settings[errF][0], "errfile");
    strcpy(settings[logF][0], "logfile");
    strcpy(settings[maxL][0], "maxLen");
    strcpy(settings[code][0], "code");

    /* DEFAULT values */
    strcpy(settings[shCmd][1], "");
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
    char *tmpArg, *tmpVal;

    /* reading program arguments */
    for (int i = 1; i < argc; i++) {
        /* command argument found */
        if (strncmp(argv[i], "-", 1) != 0) {
            strcpy(settings[shCmd][1], argv[i]);
        } else { /* find setting arguments */
            /* Arguments are in form '--setting=value',thus  */
            /* finding the '=' char gives the splitting point.    */
            tmpVal = strchr(argv[i], '=');
            if (tmpVal == NULL) {
                tmpVal = strchr(argv[i], ' ');
                if (tmpVal == NULL) {
                    printf("Separator not found\n");
                    allValid = false;
                }
            }
            if (allValid) {
                tmpArg = argv[i];
                tmpVal[0] = '\0';
                tmpVal++;
                allValid = evaluateCommand(settings, tmpArg, tmpVal);
            }
        }
        if (!allValid) {
            break;
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

void removeFile(char *filePath) {
    char rmFile[PATH_S] = "rm ";
    strcat(rmFile, filePath);
    system(rmFile);
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