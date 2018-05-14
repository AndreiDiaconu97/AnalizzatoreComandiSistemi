#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_

/* implementing boolean data type */
typedef int bool;
#define true 1
#define false 0

/* USER MACRO */
#define PATH_S 512
#define BUFF_S 512
#define OUT_S 124 //how much text to save on log
#define EXTNAME "../tmp/"
//TODO(implement some MACROs)

/* settings number and position */
#define SET_N 6
#define shCmd 0
#define outF 1
#define errF 2
#define logF 3
#define maxL 4
#define code 5

/* utility functions */
void initSettings(char settings[SET_N][2][PATH_S]);
bool readArguments(int argc, char **argv, char settings[SET_N][2][PATH_S]);
bool evaluateCommand(char settings[SET_N][2][PATH_S], char *arg, char *val);
void showSettings(char settings[SET_N][2][PATH_S]);
void loggerIsRunning(int *fdID, int *loggerID, char *loggerIDfile);
void runCommand(char *cmd, int fd);
void removeFile(char *filePath);
char *getcTime();
void rmNewline(char *str);

/* logger-starting function */
void logger(char *argv[]);

#endif