#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_
#endif

/* leave "." for relative path or assign an absolute path */
#if !defined(ABS_P)
#define ABS_P "."
#endif

/* boolean data type */
typedef int bool;
#define true 1
#define false 0

/* MACRO for string lenghts */
#define PATH_S 512
#define CMD_S 124 /* given command max length */
#define OUT_S 124 /* how much of the command output to save on log */
#define PID_S 10  /* logger process ID max size */

/* file paths */
#define LOG_FILE ABS_P "/logs/log.txt"
#define LOG_PID_F ABS_P "/temp/loggerPid.txt"
#define LOGGER_FIFO ABS_P "/temp/loggerFifo"

/* MACRO for Pack struct */
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

/* settings container */
typedef struct Settings {
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
void segmentcpy(char *dst, char *src, int from, int to);
void sendData(Pk *data);
void executeCommand(int toShell, int fromShell, Pk *data, bool piping, bool *proceed);
char *getcTime();

/* logger.c */
void logger(char *argv[]);