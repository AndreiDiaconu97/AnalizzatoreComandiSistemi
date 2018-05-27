#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_
#endif

/* boolean data type */
typedef int bool;
#define true 1
#define false 0

/* ------------------------------------------------------------------------------ */
/** following macro can be used together for pathing in the specified form:
 * -
 * HOME XXX_DIR XXX_F
 * -
 * where:
 *  HOME is the path (absolute or relative) of the project's main directory
 *  XXX_DIR is a project directory name
 *  XXX_F is a project file name
 **/

/* leave "." for relative path or assign an absolute path */
#ifndef HOME
#define HOME "."
#endif

/* default directories */
#ifndef LOG_DIR
#define LOG_DIR "/logs/"
#endif

#ifndef CONFIG_DIR
#define CONFIG_DIR "/config/"
#endif

#ifndef TEMP_DIR
#define TEMP_DIR "/temp/"
#endif

/* default file names */
#ifndef LOG_F
#define LOG_F "log.txt"
#endif

#ifndef LOG_PID_F
#define LOG_PID_F "loggerPid"
#endif

#ifndef LOGGER_FIFO_F
#define LOGGER_FIFO_F "logger.fifo"
#endif

#ifndef ID_COUNT_F
#define ID_COUNT_F "idCount"
#endif

#ifndef SETTINGS_F
#define SETTINGS_F "user_settings.txt"
#endif
/* ------------------------------------------------------------------------------ */

/* string lenghts */
#define PATH_S 512
#define CMD_S 512 /* given command max length */
#define OUT_S 512 /* how much of the command output to save on log */
#define PID_S 10  /* logger process ID max size */

/* MACRO for Pack struct */
#include <limits.h>
#define PK_FIELD_NUM 11
#define PK_LITTLE 50
#define PK_BIG PIPE_BUF - (PK_FIELD_NUM * PK_LITTLE)
/**
 * Only one istance of this struct is used;
 * contains all the needed information about the last analysed command
 **/
typedef struct Pack {
    /* manages commands for which shell returns no output */
    bool noOut;

    /* fields actually sent to logger */
    char cmdID[PK_LITTLE];
    char shellID[PK_LITTLE];
    char fatherID[PK_LITTLE];
    char *beginDate;      /* must be less than PK_LITTLE */
    char *completionDate; /* must be less than PK_LITTLE */
    char duration[PK_LITTLE];
    char origCmd[CMD_S];
    char outType[PK_LITTLE];
    char cmd[CMD_S];
    char out[PK_BIG];
    char returnC[PK_LITTLE];
} Pk;

/** 
 * user settings container:
 * everything in this struct can be customised by user using supported arguments
 **/
typedef struct Settings {
    char logF[PATH_S];
    char cmd[CMD_S];

    char printStyle[20];
    bool printInfo;
    bool code;
    bool needKill;
    int maxOut;

} settings;

/* ------------------------------------------------------------------------------ */
/**
 * List of globally available functions
 **/

/* arguments.c */
bool readArguments(int argc, char **argv, settings *s, bool *updateSettings);
bool evaluateCommand(settings *s, char *arg, char *val);

/* settings.c */
void initSettings(settings *s);
void defaultSettings(settings *s);
bool saveSettings(settings *s);
bool loadSettings(settings *s);
void showSettings(settings *s);
void printInfo(settings *s);

/* utility.c */
void segmentcpy(char *dst, char *src, int from, int to);
int appendPack(Pk *data, settings *s, char *superstring);
void executeCommand(int toShell, int fromShell, Pk *data, bool piping, bool *proceed);
void killLogger(int loggerID);
char *getcTime();

/* logger.c */
void logger(settings *s);