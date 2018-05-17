#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_

/* implementing boolean data type */
typedef int bool;
#define true 1
#define false 0

/* MACRO for string lenghts */
#define PATH_S 512
#define CMD_S 124      /* given command max length */
#define OUT_S 124      /* how much of the command output to save on log */
#define MAX_OUT_S 2048 /* if command output is bigger than that return code briks */

#define LOG_PID_F "loggerPid.txt"
#define LOGGER_FIFO "/tmp/temp/loggerFifo"
#define TO_SHELL_FIFO "/tmp/temp/toShellFifo"
#define FROM_SHELL_FIFO "/tmp/temp/fromShellFifo"

/* settings container */
typedef struct settings {
    char *logF;
    char *cmd;
    int maxCmd;
    int maxOut;
    int maxBuff;
    bool code;
} settings;

/* argumentsUtility.c */
void initSettings(settings *s);
bool readArguments(int argc, char **argv, settings *s);
bool evaluateCommand(settings *s, char *arg, char *val);
void showSettings(settings *s);

/* utility.c */
void executeCommand(int toShell, int fromShell, char *command, char retCode[3], char *outBuff, int buffSize, int shellID);
void removeFile(char *filePath);
char *getcTime();
void rmNewline(char *str);

/* logger.c */
void logger(char *argv[]);

#endif