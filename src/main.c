#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "myLibrary.h"

int main(int argc, char *argv[]) {
    prova();
    printf("ciao\n");

    return 0;
}