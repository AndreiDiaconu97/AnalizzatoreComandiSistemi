#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_
#endif

/* boolean data type */
typedef int bool;
#define true 1
#define false 0

/* leave "." for relative path or assign an absolute path (through complier) */
#ifndef ABS_P
#define ABS_P "."
#endif

/* files */
#ifndef LOG_F
#define LOG_F "log.txt"
#endif

#define LOG_FILE_P ABS_P "/logs/"

/* macro not meant for user */
#define LOG_PID_F ABS_P "/temp/loggerPid.txt"
#define LOGGER_FIFO ABS_P "/temp/loggerFifo"
#define SETTINGS_F ABS_P "/config/user_settings.txt"

/* MACRO for string lenghts */
#define PATH_S 512
#define CMD_S 124 /* given command max length */
#define OUT_S 124 /* how much of the command output to save on log */
#define PID_S 10  /* logger process ID max size */

/* MACRO for Pack struct */
#define PK_T 24
#define PK_O 2048
#define PK_R 10
/**
 * Only one istance of this struct is used;
 * contains all the needed information about the last analysed command
 **/
typedef struct Pack {
    /* manages commands for which shell returns no output */
    bool noOut;

    char origCmd[CMD_S];
    char outType[PK_T];
    char cmd[CMD_S];
    char out[PK_O];
    char returnC[PK_R];
} Pk;

/** user settings container:
 * everything in this struct can be customised by user using supported arguments
 **/
typedef struct Settings {
    char logF[PATH_S];
    char cmd[CMD_S];

    bool printInfo;
    bool code;

    int maxCmd;
    int maxOut;

    /* data hidden from user */
    int packFields;
} settings;


/* argumentsUtility.c */
void initSettings(settings *s);
void saveSettings(settings *s);
void loadSettings(settings *s);
bool readArguments(int argc, char **argv, settings *s, bool* updateSettings);
bool evaluateCommand(settings *s, char *arg, char *val);
void showSettings(settings *s);
void printInfo(settings *s);

/* utility.c */
void segmentcpy(char *dst, char *src, int from, int to);
void sendData(Pk *data);
void executeCommand(int toShell, int fromShell, Pk *data, bool piping, bool *proceed);
char *getcTime();

/* logger.c */
void logger(settings *s);