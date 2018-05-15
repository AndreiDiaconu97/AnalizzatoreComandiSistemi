#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_

/* implementing boolean data type */
typedef int bool;
#define true 1
#define false 0

/* MACRO for string lenghts */
#define PATH_S 512
#define CMD_S 124                  /* given command max length */
#define OUT_S 124                  /* how much of the command output to save on log */

#define EXTNAME "../tmp/"

/* settings container */
typedef struct settings {
    char *logF;
    char *cmd;
    int maxCmd;
    int maxOut;
    int maxBuff;
    bool code;
} settings;

/* utility functions */
void initSettings(settings *s);
bool readArguments(int argc, char **argv, settings *s);
bool evaluateCommand(settings *s, char *arg, char *val);
void showSettings(settings *s);
void loggerIsRunning(int *fdID, int *loggerID, char *loggerIDfile);
void removeFile(char *filePath);
char *getcTime();
void rmNewline(char *str);

/* logger-starting function */
void logger(char *argv[]);

#endif