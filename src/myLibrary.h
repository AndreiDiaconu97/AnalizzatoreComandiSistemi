#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_

typedef int bool;
#define true 1
#define false 0

//TODO(implement some MACROs)
#define PATH_S 512
#define BUFF_S 80
#define EXTNAME "../tmp/"

bool readArguments(char argc, char **argv, char *outfile, char *errfile, int *maxLen, bool *returnCode);
int loggerIsRunning(char *loggerIDfile, char *buffer, int *pfdID);
void removeFifo(char *fifoPath);
void logger(int argc, char *argv[]);

#endif // MYLIBRARY_H_