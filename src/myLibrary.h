#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_

/* implementing boolean data type */
typedef int bool;
#define true 1
#define false 0

/* USER MACRO */
#define PATH_S 512
#define BUFF_S 50
#define EXTNAME "../tmp/"
//TODO(implement some MACROs)

/* settings number and position */
#define SET_N 5
#define outF 0
#define errF 1
#define logF 2
#define maxL 3
#define code 4

/* utility functions */
void initSettings(char settings[SET_N][2][PATH_S]);
bool readArguments(int argc, char **argv, char settings[SET_N][2][PATH_S]);
bool evaluateCommand(char settings[SET_N][2][PATH_S], char *arg, char *val);
void showSettings(char settings[SET_N][2][PATH_S]);
void loggerIsRunning(int *fdID, int *loggerID, char *loggerIDfile);
void runCommand(char *cmd, int fd);
void removeFifo(char *fifoPath);
char *getcTime();
void rmNewline(char *str);

/* logger-starting function */
void logger(int argc, char *argv[]);

#endif