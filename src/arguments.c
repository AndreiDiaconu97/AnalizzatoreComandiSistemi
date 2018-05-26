#include "myLibrary.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* file specific functions */
/* ------------------------------------------------------------------------------ */
bool checkCommandBool(bool *setting, char *arg, char *val);
bool checkCommandInt(int *setting, char *arg, char *val);
/* ------------------------------------------------------------------------------ */

/* must return true */
bool readArguments(int argc, char **argv, settings *s, bool *updateSettings) {
    bool allValid = true;
    char *tmpArg, *tmpVal;

    /* read program arguments */
    int i;
    for (i = 1; i < argc; i++) {
        /* special command help/h must not be analised further */
        if ((strcmp(argv[i], "help") == 0) || (strcmp(argv[i], "h") == 0)) {
            s->printInfo = true;
        }
        /* shell command found */
        else if (strncmp(argv[i], "-", 1) != 0) {
            strcpy(s->cmd, argv[i]);
        }
        /* typical arguments have a value to be read */
        else {
            *updateSettings = true;
            /* search for '=', if not found search for ' ' separator */
            tmpVal = strchr(argv[i], '=');
            if (tmpVal == NULL) {
                tmpVal = strchr(argv[i], ' ');
                if (tmpVal == NULL) {
                    printf("Separator not found\n");
                    allValid = false;
                }
            }
            /* if no invalid command is found until now, proceed evaluating actual argument */
            if (allValid) {
                /* split argument name from value */
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
    return allValid;
}

bool evaluateCommand(settings *s, char *arg, char *val) {
    bool result = false;

    if ((!strcmp(arg, "--logfile")) || (!strcmp(arg, "-log"))) {
        strcpy(s->logF, val);
        result = true;
    } else if ((!strcmp(arg, "--maxOutput")) || (!strcmp(arg, "-mo"))) {
        result = checkCommandInt(&s->maxOut, arg, val);
    } else if ((!strcmp(arg, "--code")) || (!strcmp(arg, "-c"))) {
        result = checkCommandBool(&s->code, arg, val);
    } else if ((!strcmp(arg, "--help")) || (!strcmp(arg, "-h"))) {
        result = checkCommandBool(&s->printInfo, arg, val);
    } else if ((!strcmp(arg, "--default")) || (!strcmp(arg, "-d"))) {
        bool *reset;
        result = checkCommandBool(reset, arg, val);
        if (reset) {
            defaultSettings(s);
        }
    } else {
        printf("Invalid argument found:'%s=%s'\n", arg, val);
    }
    return result;
}

bool checkCommandBool(bool *setting, char *arg, char *val) {
    bool result = true;
    if (!strcmp(val, "true")) {
        *setting = true;
    } else if (!strcmp(val, "false")) {
        *setting = false;
    } else {
        printf("Argument:%s - Invalid input:%s\n", arg, val);
        result = false;
    }
    return result;
}

bool checkCommandInt(int *setting, char *arg, char *val) {
    bool result = false;
    if (!strncmp(val, "-", 1)) {
        printf("Command '%s': invalid negative number\n", arg);
    } else if (atoi(val)) {
        *setting = atoi(val);
        result = true;
    } else {
        printf("Command '%s': couldn't parse value '%s'\n", arg, val);
    }
    return result;
}