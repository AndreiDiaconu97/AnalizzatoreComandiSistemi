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

void initSettings(settings *s) {
    s->code = true;
    s->maxCmd = CMD_S;
    s->maxOut = OUT_S;
    s->maxBuff = s->maxCmd + s->maxOut + 50; /* temporary size */

    char *logfile = malloc(sizeof(char) * PATH_S);
    char *command = malloc(sizeof(char) * CMD_S);
    strcpy(logfile, "commands.log");
    strcpy(command, "");
    s->logF = logfile;
    s->cmd = command;
}

/* must return true */
bool readArguments(int argc, char **argv, settings *s) {
    bool allValid = true;
    char *tmpArg, *tmpVal;

    /* reading program arguments */
    for (int i = 1; i < argc; i++) {
        /* command argument found */
        if (strncmp(argv[i], "-", 1) != 0) {
            strcpy(s->cmd, argv[i]);
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
                allValid = evaluateCommand(s, tmpArg, tmpVal);
            }
        }
        if (!allValid) {
            break;
        }
    }
    s->maxBuff = s->maxCmd + s->maxOut + 50; /* adjust buffer size */
    return allValid;
}

bool evaluateCommand(settings *s, char *arg, char *val) {
    bool result = true;
    if ((!strcmp(arg, "--logfile")) || (!strcmp(arg, "-l"))) {
        strcpy(s->logF, val);
    } else if ((!strcmp(arg, "--maxCmd")) || (!strcmp(arg, "-mC"))) {
        if (!strncmp(val, "-", 1)) {
            printf("Command '%s': invalid negative number\n", arg);
            result = false;
        } else if (atoi(val)) { //checking for integer value
            s->maxCmd = atoi(val);
        } else {
            printf("Command '%s': couldn't parse value '%s'\n", arg, val);
            result = false;
        }
    } else if ((!strcmp(arg, "--maxOutput")) || (!strcmp(arg, "-mO"))) {
        if (!strncmp(val, "-", 1)) {
            printf("Command '%s': invalid negative number\n", arg);
            result = false;
        } else if (atoi(val)) { //checking for integer value
            s->maxOut = atoi(val);
        } else {
            printf("Command '%s': couldn't parse value '%s'\n", arg, val);
            result = false;
        }
    } else if ((!strcmp(arg, "--maxBuffer")) || (!strcmp(arg, "-mB"))) {
        if (!strncmp(val, "-", 1)) {
            printf("Command '%s': invalid negative number\n", arg);
            result = false;
        } else if (atoi(val)) { //checking for integer value
            s->maxBuff = atoi(val);
        } else {
            printf("Command '%s': couldn't parse value '%s'\n", arg, val);
            result = false;
        }
    } else if ((!strcmp(arg, "--code")) || (!strcmp(arg, "-c"))) {
        if (!strcmp(val, "true")) {
            s->code = true;
        } else if (!strcmp(val, "false")) {
            s->code = false;
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

void showSettings(settings *s) {
    printf("--- SETTINGS ---------------------------\n");
    printf("logfile: %s\n", s->logF);
    printf("return code: %s\n", s->code ? "true" : "false");
    printf("command max length: %d\n", s->maxCmd);
    printf("output max length: %d\n", s->maxOut);
    printf("----------------------------------------\n\n");
}