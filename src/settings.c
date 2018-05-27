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
char *readNextSetting(char *line, size_t *len, FILE *settFp, bool *result);
/* ------------------------------------------------------------------------------ */

void initSettings(settings *s) {
    strcpy(s->cmd, "");
    s->printInfo = false;
    s->needKill = false;

    /* check for existing user configuration */
    if (!loadSettings(s)) {
        printf("restoring file...\n");
        defaultSettings(s);
    }
}

void defaultSettings(settings *s) {
    /* restore default settings */
    strcpy(s->logF, LOG_F);

    s->maxOut = OUT_S;
    s->code = true;
    strcpy(s->printStyle, "TXT");

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

    if (settFp = fopen(HOME CONFIG_DIR SETTINGS_F, "r")) {
        /* discard first line */
        if (getline(&line, &len, settFp) == -1) {
            result = false;
        }

        /* log name */
        value = readNextSetting(line, &len, settFp, &result);
        strcpy(s->logF, value);

        /* print style */
        value = readNextSetting(line, &len, settFp, &result);
        strcpy(s->printStyle, value);

        /* max output length */
        value = readNextSetting(line, &len, settFp, &result);
        if ((s->maxOut = atoi(value)) == 0) {
            printf("maxOut: invalid value\n");
            exit(EXIT_FAILURE);
        }

        /* return code */
        value = readNextSetting(line, &len, settFp, &result);
        if (!strcmp(value, "true")) {
            s->code = true;
        } else if (!strcmp(value, "false")) {
            s->code = false;
        } else {
            printf("code: Invalid value\n");
            exit(EXIT_FAILURE);
        }

        if (!result) {
            printf("loading settings: reading error\n");
        } else {
            printf("settings loaded\n");
        }
        if (line) {
            free(line);
        }
        fclose(settFp);
    } else {
        perror("loading settings from file");
        result = false;
    }
    return result;
}

char *readNextSetting(char *line, size_t *len, FILE *settFp, bool *result) {
    char *value;
    /* get return code bool */
    if (getline(&line, len, settFp) == -1) {
        result = false;
    } else {
        if ((value = strchr(line, '#')) == NULL) {
            printf("Corrupted settings file\n");
            exit(EXIT_FAILURE);
        }
        value += 2;                      /* move to value position */
        value[strlen(value) - 1] = '\0'; /* remove ending '\n' */
    }
    return value;
}

bool saveSettings(settings *s) {
    int settFd;
    char num[20];
    bool result = true;

    /* create config folder if non-existent and check for errors */
    if (mkdir(HOME CONFIG_DIR, 0777) && errno != EEXIST) {
        printf("Error while trying to create %s folder\n", CONFIG_DIR);
    }

    /* create config file in non-existent and save data from a settings struct */
    if ((settFd = open(HOME CONFIG_DIR SETTINGS_F, O_WRONLY | O_TRUNC | O_CREAT, 0777)) != -1) {
        write(settFd, "---- USER SETTINGS --------------------------\n", 46);

        write(settFd, "LOG_NAME#\t", strlen("LOG_NAME#\t"));
        write(settFd, s->logF, strlen(s->logF));
        write(settFd, "\n", strlen("\n"));

        write(settFd, "PRINT_STYLE#\t", strlen("PRINT_STYLE#\t"));
        write(settFd, s->printStyle, strlen(s->printStyle));
        write(settFd, "\n", strlen("\n"));

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

void showSettings(settings *s) {
    printf("--- SETTINGS ---------------------------\n");
    printf("logfile:\t%s\n", s->logF);
    printf("print style:\t%s\n", s->printStyle);
    printf("return code:\t%s\n", s->code ? "true" : "false");
    printf("max out len:\t%d\n", s->maxOut);
    printf("----------------------------------------\n\n");
}

void printInfo(settings *s) {
    printf("---------------------------------------------------------------------------------------------------------\n");
    if (s->printInfo) {
        printf("List of compatible arguments:\n");
        printf("-l\t| --logfile\t:\t<str>\tlog file name\n");
        printf("-h\t| --help\t:\t<bool>\tset true in order to display arguments list\n");
        printf("-k\t| --kill\t:\t<bool>\tkills logger process");
        printf("-d\t| --default\t:\t<bool>\tfactory reset\n");
        printf("-c\t| --code\t:\t<bool>\tprints command/subcommand return code on log file\n");
        printf("-m\t| --maxOutput\t:\t<int>\tmaximum length for command/subcommand output result on log file\n");
        printf("-p\t| --printStyle\t:\t<string>\tlog formatting (TXT or CSV)\n");
        printf("\n");
        printf("kill (special argument) :\tcloses logger process if existing\n");
    } else {
        printf(" - for info about program usage, type h or help in the arguments line\n");
    }
    printf("---------------------------------------------------------------------------------------------------------\n\n");
}