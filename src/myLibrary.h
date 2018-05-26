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
#define LOG_PID_F "loggerPid.txt"
#endif

#ifndef LOGGER_FIFO_F
#define LOGGER_FIFO_F "loggerFifo"
#endif

#ifndef SETTINGS_F
#define SETTINGS_F "user_settings.txt"
#endif
/* ------------------------------------------------------------------------------ */

/* string lenghts */
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

    char *beginDate;      /* should be less than PK_T */
    char *completionDate; /* should be less than PK_T */
    char duration[PK_T];
    char origCmd[CMD_S];
    char outType[PK_T];
    char cmd[CMD_S];
    char out[PK_O];
    char returnC[PK_R];
} Pk;

/** 
 * user settings container:
 * everything in this struct can be customised by user using supported arguments
 **/
typedef struct Settings {
    char logF[PATH_S];
    char cmd[CMD_S];

    bool printInfo;
    bool code;
    int maxOut;

    /* intended to be used only by logger, user should not touch */
    int packFields;
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
void sendData(Pk *data, settings *s);
void executeCommand(int toShell, int fromShell, Pk *data, bool piping, bool *proceed);
void killLogger(int loggerID);
char *getcTime();

/* logger.c */
void logger(settings *s);