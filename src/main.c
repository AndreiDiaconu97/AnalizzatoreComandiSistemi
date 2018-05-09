#include "myLibrary.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int maxLen = BUFF_S;
    bool returnCode = false;
    char outfile[BUFF_S] = "change this";
    char errfile[BUFF_S] = "change this";

    if (!readArguments(argc, argv, outfile, errfile, &maxLen, &returnCode)) {
        printf("CLOSING PROGRAM\n");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", outfile);

    return EXIT_SUCCESS;
}