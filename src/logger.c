#include "myLibrary.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void printTxt(char **inputs, settings *s);
void usr1_handler(int sig);

/* enum for accessing to different parts of the string packet */
enum received_data {
    START,
    END,
    DURATION,
    TYPE,
    CMD,
    SUB_CMD,
    OUT,
    CODE
};

/**
 * this function is meant to run in a separate process as a daemon in background;
 * it loops indefinitely and blocks if there is no data in the fifo used;
 * if new data is received it proceeds to read only one "packet" at time.
 * -
 * Packets received begin with a string representing an integer,
 * this string tells the exact amount of data to read for the given packet, thus,
 * there is no risk to read unexpected data.
 * -
 * Even if packet is safely read, it must have a precise structure in order to
 * successfully extract data from it.
 * -
 * Packet structure is directly connnected to the 'Pack' struct.
 **/
int myFifo; /* needed to usr1_handler() */
void logger(settings *s) {
    /* logger is closed with a custom signal */
    signal(SIGUSR1, usr1_handler);
    char buffer[2 * s->maxCmd + 4 * PK_T + PK_O + PK_R]; /* must contain data received for a single comand */
    char *inputs[s->packFields];                         /* used to reference different parts of the buffer */
    char size[65];                                       /* not too big, must contain a string representing one integer */
    int count = -1;

    /* open FIFO in read/write mode so it blocks when no data is received */
    if ((myFifo = open(HOME TEMP_DIR LOGGER_FIFO_F, O_RDWR, 0777)) == -1) {
        perror("Logger: opening fifo");
        exit(EXIT_FAILURE);
    }

    /* create log folder if non-existant and check for errors */
    if (mkdir(HOME LOG_DIR, 0777) && errno != EEXIST) {
        printf("Error while trying to create %s folder\n", CONFIG_DIR);
    }
    /* open/create log file and move pipe to stdout */
    char logFile[PATH_S];
    strcpy(logFile, HOME LOG_DIR);
    strcat(logFile, s->logF);

    int myLog;
    if ((myLog = open(logFile, O_WRONLY | O_APPEND | O_CREAT, 0777)) == -1) {
        perror("Opening log file");
        exit(EXIT_FAILURE);
    }
    dup2(myLog, STDOUT_FILENO);
    close(myLog);

    ///* actual program *///
    while (1) {
        /* read next data-packet size */
        size[0] = '\0';
        do {
            read(myFifo, buffer, 1);

            /* exit condition */
            /*
            if (!strncmp(buffer, "!", 1)) {
                remove(HOME TEMP_DIR LOG_PID_F);
                remove(HOME TEMP_DIR LOGGER_FIFO_F);
                exit(EXIT_SUCCESS);
            }
            */
            strncat(size, buffer, 1);
        } while (*buffer != '\0');

        count = read(myFifo, buffer, atoi(size));

        /* analysing data */
        int inTmp = 0;
        int i;
        inputs[inTmp++] = buffer;
        for (i = 0; i < count; i++) {
            if (buffer[i] == '\0') {
                inputs[inTmp++] = &buffer[i + 1];
            }
        }

        printTxt(inputs, s);
    }
}

/**
 * Logger process is "safely" killed using custom signal SIGUSR1.
 **/
void usr1_handler(int sig) {
    //write(myFifo, "!", strlen("!"));
    remove(HOME TEMP_DIR LOG_PID_F);
    remove(HOME TEMP_DIR LOGGER_FIFO_F);
    exit(EXIT_SUCCESS);
}

void printTxt(char **inputs, settings *s) {
    printf("ID:\t\t%s\n", "1.1.1");
    printf("STARTED:\t%s\n", inputs[START]);
    printf("ENDED:\t\t%s\n", inputs[END]);
    printf("DURATION:\t%ss\n", inputs[DURATION]);
    printf("TYPE:\t\t%s\n", inputs[TYPE]);
    printf("COMMAND:\t%s\n", inputs[CMD]);
    printf("SUBCOMMAND:\t%s\n", inputs[SUB_CMD]);
    printf("OUTPUT:\n\n%s\n\n", inputs[OUT]);
    printf("LENGTH:\t%ld\n", strlen(inputs[OUT]));
    if (s->code) {
        printf("RETURN CODE: %s\n", inputs[CODE]);
    }
    printf("---------------------------------------------------\n");
}
