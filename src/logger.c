#include "myLibrary.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* file specific functions */
/* ------------------------------------------------------------------------------ */
void printTxt(char **inputs, char *id);
void usr1_handler(int sig);
/* ------------------------------------------------------------------------------ */

/* enum for accessing to different parts of the string packet */
enum received_data {
    SUB_ID,
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
int myFifo; /* needed for usr1_handler() */
void logger(settings *s) {
    /* logger is closed with a custom signal */
    signal(SIGUSR1, usr1_handler);
    char buffer[2 * CMD_S + 6 * PK_LITTLE + PK_BIG]; /* must contain data received from a single comand */
    char *inputs[PK_FIELDS];                         /* used to reference different parts of the buffer */
    char size[65];                                   /* not too big, must contain a string representing one integer */

    /* open FIFO in read/write mode so it blocks when no data is received */
    if ((myFifo = open(HOME TEMP_DIR LOGGER_FIFO_F, O_RDWR, 0777)) == -1) {
        perror("Logger: opening fifo");
        exit(EXIT_FAILURE);
    }

    /* id count is stored in a file */
    int idCountFd;
    if ((idCountFd = open(HOME TEMP_DIR ID_COUNT_F, O_RDWR, 0777)) == -1) {
        perror("Opening id counter file");

        printf("creating new one\n");
        idCountFd = open(HOME TEMP_DIR ID_COUNT_F, O_RDWR | O_TRUNC | O_CREAT, 0777);
        if (write(idCountFd, "0", strlen("0")) == -1) {
            perror("Saving count");
            exit(EXIT_FAILURE);
        }
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
    int count;
    int id;
    char idBuffer[32];
    while (1) {
        /* read next data-packet size */
        size[0] = '\0';
        do {
            read(myFifo, buffer, 1);
            /* exit condition */
            if (!strncmp(buffer, "!", 1)) {
                close(idCountFd);
                remove(HOME TEMP_DIR LOG_PID_F);
                exit(EXIT_SUCCESS);
            }
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

        lseek(idCountFd, 0L, SEEK_SET);
        count = read(idCountFd, idBuffer, sizeof idBuffer);
        idBuffer[count] = '\0';
        id = atoi(idBuffer) + 1;
        sprintf(idBuffer, "%d", id);
        lseek(idCountFd, 0L, SEEK_SET);
        write(idCountFd, idBuffer, strlen(idBuffer));

        printTxt(inputs, idBuffer);
    }
}

void usr1_handler(int sig) {
    write(myFifo, "!", strlen("!") + 1);
}

void printTxt(char **inputs, char *id) {
    printf("ID:\t\t%s.%s\n", id, inputs[SUB_ID]);
    printf("STARTED:\t%s\n", inputs[START]);
    printf("ENDED:\t\t%s\n", inputs[END]);
    printf("DURATION:\t%ss\n", inputs[DURATION]);
    printf("TYPE:\t\t%s\n", inputs[TYPE]);
    printf("COMMAND:\t%s\n", inputs[CMD]);
    printf("SUBCOMMAND:\t%s\n", inputs[SUB_CMD]);
    printf("OUTPUT:\n\n%s\n\n", inputs[OUT]);
    printf("LENGTH:\t\t%ld\n", strlen(inputs[OUT]));
    if (strcmp(inputs[CODE], "false") != 0) {
        printf("RETURN CODE: %s\n", inputs[CODE]);
    }
    printf("---------------------------------------------------\n");
}
