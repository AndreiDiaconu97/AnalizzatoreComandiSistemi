#ifndef MYLIBRARY_H_
#define MYLIBRARY_H_

typedef int bool;
#define true 1
#define false 0

//TODO(implement some MACROs)
#define BUFF_S 80
#define EXTNAME "../tmp/"

bool readArguments(char argc, char **argv, char *outfile, char *errfile, int *maxLen, bool *returnCode);
void logger(int argc, char *argv[]);

#endif // MYLIBRARY_H_