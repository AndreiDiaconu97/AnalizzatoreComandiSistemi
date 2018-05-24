#include "myLibrary.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <syslog.h>
//#include <time.h>
#include <unistd.h>

void initSettings(settings *s) {
    strcpy(s->cmd, "");
    s->packFields = 5;
    s->printInfo = false;

    /* check for existing user configuration */
    if (!loadSettings(s)) {
        printf("restoring file...\n");
        resetSettings(s);
    }
}

void resetSettings(settings *s) {
    /* restore default settings */
    strcpy(s->logF, LOG_F);

    s->maxCmd = CMD_S;
    s->maxOut = OUT_S;
    s->code = true;

    /* save default settings */
    if (!saveSettings(s)) {
        exit(EXIT_FAILURE);
    } else {
        printf("Restored default settings\n");
    }
}

bool loadSettings(settings *s) {
    FILE *settFp;
    char *line = NULL;
    char *value;
    size_t len = 0;

    bool result = true;
    bool readError = false;

    if (settFp = fopen(HOME CONFIG_DIR SETTINGS_F, "r")) {
        /* discard first line */
        if (getline(&line, &len, settFp) == -1) {
            readError = true;
        }

        /* get log name */
        if (getline(&line, &len, settFp) == -1) {
            readError = true;
        } else {
            if ((value = strchr(line, '#')) == NULL) {
                printf("Corrupted settings file\n");
                exit(EXIT_FAILURE);
            }
            value += 2; /* move to value position */
            strncpy(s->logF, value, strlen(value) - 1);
        }

        /* get max command length */
        if (getline(&line, &len, settFp) == -1) {
            readError = true;
        } else {
            if ((value = strchr(line, '#')) == NULL) {
                printf("Corrupted settings file\n");
                exit(EXIT_FAILURE);
            }
            value += 2;                      /* move to value position */
            value[strlen(value) - 1] = '\0'; /* remove ending '\n' */
            if ((s->maxCmd = atoi(value)) == 0) {
                printf("maxCmd: invalid value");
                exit(EXIT_FAILURE);
            }
        }

        /* get max output length */
        if (getline(&line, &len, settFp) == -1) {
            readError = true;
        } else {
            if ((value = strchr(line, '#')) == NULL) {
                printf("Corrupted settings file\n");
                exit(EXIT_FAILURE);
            }
            value += 2;                      /* move to value position */
            value[strlen(value) - 1] = '\0'; /* remove ending '\n' */
            if ((s->maxOut = atoi(value)) == 0) {
                printf("maxOut: invalid value");
                exit(EXIT_FAILURE);
            }
        }

        /* get return code bool */
        if (getline(&line, &len, settFp) == -1) {
            readError = true;
        } else {
            if ((value = strchr(line, '#')) == NULL) {
                printf("Corrupted settings file\n");
                exit(EXIT_FAILURE);
            }
            value += 2; /* move to value position */
            if (!strcmp(value, "true\n")) {
                s->code = true;
            } else if (!strcmp(value, "false\n")) {
                s->code = false;
            } else {
                printf("code: Invalid value\n");
                exit(EXIT_FAILURE);
            }
        }

        fclose(settFp);
        if (line) {
            free(line);
        }
        if (readError) {
            printf("loading settings: reading error\n");
            result = false;
        }
        printf("settings loaded\n");
    } else {
        perror("loading settings from file");
        result = false;
    }
    return result;
}

bool saveSettings(settings *s) {
    int settFd;
    char num[20];
    bool result = true;

    /* create config folder if non-existant and check for errors */
    if (mkdir(HOME CONFIG_DIR, 0777) && errno != EEXIST) {
        printf("Error while trying to create %s folder\n", CONFIG_DIR);
    }
    /* create config file in non-existant and save data from a settings struct */
    if ((settFd = open(HOME CONFIG_DIR SETTINGS_F, O_WRONLY | O_TRUNC | O_CREAT, 0777)) != -1) {
        write(settFd, "---- USER SETTINGS --------------------------\n", 46);

        write(settFd, "LOG_NAME#\t", strlen("LOG_NAME#\t"));
        write(settFd, s->logF, strlen(s->logF));
        write(settFd, "\n", strlen("\n"));

        write(settFd, "MAX_CMD_LENGTH#\t", strlen("MAX_CMD_LENGTH#\t"));
        sprintf(num, "%d\n", s->maxCmd);
        write(settFd, num, strlen(num));

        write(settFd, "MAX_OUT_LENGTH#\t", strlen("MAX_OUT_LENGTH#\t"));
        sprintf(num, "%d\n", s->maxOut);
        write(settFd, num, strlen(num));

        write(settFd, "RETURN_CODE#\t", strlen("RETURN_CODE#\t"));
        if (s->code) {
            write(settFd, "true", strlen("true"));

        } else {
            write(settFd, "false", strlen("false"));
        }
        write(settFd, "\n", strlen("\n"));

        close(settFd);
        printf("settings saved\n");
    } else {
        perror("saving settings");
        result = false;
    }
    return result;
}

/* must return true */
bool readArguments(int argc, char **argv, settings *s, bool *updateSettings) {
    bool allValid = true;
    char *tmpArg, *tmpVal;

    /* read program arguments */
    int i;
    for (i = 1; i < argc; i++) {
        /* help command found */
        if ((strcmp(argv[i], "help") == 0) || (strcmp(argv[i], "h") == 0)) {
            s->printInfo = true;
        }
        /* command found */
        else if (strncmp(argv[i], "-", 1) != 0) {
            strcpy(s->cmd, argv[i]);
        } else { /* user argument found */
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
            if (allValid) {
                /* if no invalid command is found until now, proceed evaluating actual argument */
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
    } else if ((!strcmp(arg, "--maxCmd")) || (!strcmp(arg, "-mc"))) {
        if (!strncmp(val, "-", 1)) {
            printf("Command '%s': invalid negative number\n", arg);
        } else if (atoi(val)) { //checking for integer value
            s->maxCmd = atoi(val);
            result = true;
        } else {
            printf("Command '%s': couldn't parse value '%s'\n", arg, val);
        }
    } else if ((!strcmp(arg, "--maxOutput")) || (!strcmp(arg, "-mo"))) {
        if (!strncmp(val, "-", 1)) {
            printf("Command '%s': invalid negative number\n", arg);
        } else if (atoi(val)) { //checking for integer value
            s->maxOut = atoi(val);
            result = true;
        } else {
            printf("Command '%s': couldn't parse value '%s'\n", arg, val);
        }
    } else if ((!strcmp(arg, "--code")) || (!strcmp(arg, "-c"))) {
        result = true;
        if (!strcmp(val, "true")) {
            s->code = true;
        } else if (!strcmp(val, "false")) {
            s->code = false;
        } else {
            printf("Argument:%s - Invalid input:%s\n", arg, val);
            result = false;
        }
    } else if ((!strcmp(arg, "--help")) || (!strcmp(arg, "-h"))) {
        result = true;
        if (!strcmp(val, "true")) {
            s->printInfo = true;
        } else if (!strcmp(val, "false")) {
            s->printInfo = false;
        } else {
            printf("Argument:%s - Invalid input:%s\n", arg, val);
            result = false;
        }
    } else if ((!strcmp(arg, "--default")) || (!strcmp(arg, "-d"))) {
        result = true;
        if (!strcmp(val, "true")) {
            resetSettings(s);
        } else if (!strcmp(val, "false")) {
        } else {
            printf("Argument:%s - Invalid input:%s\n", arg, val);
            result = false;
        }
    } else {
        printf("Invalid argument found:'%s=%s'\n", arg, val);
    }
    return result;
}

void showSettings(settings *s) {
    printf("--- SETTINGS ---------------------------\n");
    printf("logfile:\t%s\n", s->logF); // strrchr(s->logF, '/')
    printf("return code:\t%s\n", s->code ? "true" : "false");
    printf("command max length:\t%d\n", s->maxCmd);
    printf("output max length:\t%d\n", s->maxOut);
    printf("----------------------------------------\n\n");
}

void printInfo(settings *s) {
    printf("---------------------------------------------------------------------------------------------------------\n");
    if (s->printInfo) {
        printf("List of compatible arguments:\n");
        printf("-log\t| --logfile\t:\t<str>\tlog file name (NOT USABLE NOW)\n");
        printf("-h\t| --help\t:\t<bool>\tset true in order to display arguments list\n");
        printf("-d\t| --default\t:\t<bool>\tsettings factory reset\n");
        printf("-c\t| --code\t:\t<bool>\tprints command/subcommand return code on log file\n");
        printf("-mc\t| --maxCmd\t:\t<int>\tmaximum input command length\n");
        printf("-mo\t| --maxOutput\t:\t<int>\tmaximum length for command/subcommand output result on log file\n");
        printf("\n");
        printf("kill (special argument) :\tcloses logger process if existing\n");
    } else {
        printf(" - for info about program usage, type h or help in the arguments line\n");
    }
    printf("---------------------------------------------------------------------------------------------------------\n\n");
}