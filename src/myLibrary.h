#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_

/* implementing boolean data type */
typedef int bool;
#define true 1
#define false 0

/* MACRO for string lenghts */
#define PATH_S 512
#define CMD_S 124 /* given command max length */
#define OUT_S 124 /* how much of the command output to save on log */

#define LOG_FILE "log.txt"
#define LOG_PID_F "loggerPid.txt"
#define LOGGER_FIFO "/tmp/temp/loggerFifo"
#define LOGGER_QUEUE "/tmp/temp/loggerqueue"

/* settings container */
typedef struct Settings {
    char *logF;
    char *cmd;
    int maxCmd;
    int maxOut;
    int maxBuff;
    bool code;
} settings;

#define PK_T 24
#define PK_O 2048
#define PK_R 10

typedef struct Pack {
    bool noOut;

    char origCmd[CMD_S];
    char outType[PK_T];

    char cmd[CMD_S];
    char out[PK_O];
    char returnC[PK_R];
} Pk;

/* argumentsUtility.c */
void initSettings(settings *s);
bool readArguments(int argc, char **argv, settings *s);
bool evaluateCommand(settings *s, char *arg, char *val);
void showSettings(settings *s);

/* utility.c */
void segmentcpy(char *dst, char *src, int from, int to);
void sendData(Pk *data);
void executeCommand(int toShell, int fromShell, Pk *data, bool piping);
char *getcTime();
void rmNewline(char *str);

/* logger.c */
void logger(char *argv[]);

#endif