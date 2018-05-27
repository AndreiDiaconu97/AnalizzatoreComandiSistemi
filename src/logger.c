#include "myLibrary.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum format_style {
    CSV,
    TXT
} f_style;

/* file specific functions */
/* ------------------------------------------------------------------------------ */
void printFormatted(char **Pk_fields, int commandID, f_style formatStyle);
void printCSVfields();
void printCSV(char **Pk_fields, int commandID);
void printTxt(char **Pk_fields, int commandID);
void usr1_handler(int sig);
/* ------------------------------------------------------------------------------ */

/* enum for accessing to different parts of the string packet */
enum received_data {
    CMD_ID,
    SENDER_PID,
    SHELL_PID,
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
    char *Pk_fields[PK_FIELD_NUM];                   /* used to reference different parts of the buffer */
    char size[65];

    f_style formatStyle;
    if (!strcmp(s->printStyle, "TXT")) {
        formatStyle = TXT;
    } else if (!strcmp(s->printStyle, "CSV")) {
        formatStyle = CSV;
    } else {
        printf("ERROR: invalid formatStyle\n");
        exit(EXIT_FAILURE);
    }

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
    bool needToPrintFields = false;
    f_style prevMode;
    if ((myLog = open(logFile, O_RDONLY | O_CREAT, 0777)) == -1) {
        // unable to create or open file => exit
        perror("Opening log file");
        close(myLog);
        exit(EXIT_FAILURE);
    } else {
        char readMode[11];
        int size = read(myLog, readMode, sizeof readMode);
        if (size <= 0) {
            // file empty
            //printf("FILE EMPTY\n");
            prevMode = formatStyle;
            if (formatStyle == CSV) {
                needToPrintFields = true;
            }
        } else {
            if (readMode[10] == ':') {
                //printf("TXT MODE\n");
                prevMode = TXT;
            } else {
                //printf("CSV MODE\n");
                prevMode = CSV;
            }
        }
    }
    close(myLog);

    if (prevMode != formatStyle) {
        myLog = open(logFile, O_TRUNC | O_CREAT, 0777);
        //printf("SVUOTATO\n");
        close(myLog);
        if (formatStyle == CSV) {
            needToPrintFields = true;
        }
    }

    myLog = open(logFile, O_WRONLY | O_APPEND, 0777);
    dup2(myLog, STDOUT_FILENO);
    close(myLog);

    if (needToPrintFields) {
        printCSVfields();
    }

    ///* actual program *///
    int commandID = 1;
    char commandIDbuff[50];
    int idFd = open(HOME TEMP_DIR ID_COUNT_F, O_RDONLY, 0777);
    if (idFd == -1) {
        close(idFd);
        idFd = open(HOME TEMP_DIR ID_COUNT_F, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (write(idFd, "1", strlen("1")) == -1) {
            perror("Saving new commandID");
            exit(EXIT_FAILURE);
        }
        close(idFd);
    } else {
        int size = read(idFd, commandIDbuff, sizeof commandIDbuff);
        commandIDbuff[size] = '\0';
        commandID = atoi(commandIDbuff);
    }

    int count;
    while (1) {
        /* read next data-packet size */
        size[0] = '\0';
        do {
            read(myFifo, buffer, 1);
            /* exit condition */
            if (!strncmp(buffer, "!", 1)) {
                idFd = open(HOME TEMP_DIR ID_COUNT_F, O_WRONLY, 0777);
                sprintf(commandIDbuff, "%d", commandID);
                write(idFd, commandIDbuff, strlen(commandIDbuff));
                exit(EXIT_SUCCESS);
            }
            strncat(size, buffer, 1);
        } while (*buffer != '\0');

        while (*buffer == '\0') {
            read(myFifo, buffer, 1);
        }

        count = read(myFifo, buffer, atoi(size));

        /* analysing data */
        int pos = 0;
        while (pos < count) {
            int field = 0;
            Pk_fields[field] = buffer + pos;
            //field++;
            while (field < PK_FIELD_NUM) {
                if (buffer[pos] == '\0') {
                    field++;
                    Pk_fields[field] = buffer + pos + 1;
                    //field++;
                }
                pos++;
            }
            printFormatted(Pk_fields, commandID, formatStyle);
        }

        commandID++;
    }
}


/**
 * used to close safely the logger process
 **/
void usr1_handler(int sig) {
    write(myFifo, "!", strlen("!") + 1);
}


void printFormatted(char **Pk_fields, int commandID, f_style formatStyle) {
    if (formatStyle == TXT) {
        printTxt(Pk_fields, commandID);
    } else if (formatStyle == CSV) {
        printCSV(Pk_fields, commandID);
    }
}


/**
 * prints first line in case a CSV formatted log is used
 **/
void printCSVfields() {
    printf("COMMAND ID,");
    printf("LOGGER PID,");
    printf("SENDER PID,");
    printf("SHELL PID,");
    printf("STARTED,");
    printf("ENDED,");
    printf("DURATION,");
    printf("TYPE,");
    printf("COMMAND,");
    printf("SUBCOMMAND,");
    printf("OUTPUT LENGTH,");
    printf("OUTPUT,");
    printf("RETURN CODE\n");
}


/**
 * prints on log using a CSV format style
 **/
void printCSV(char **Pk_fields, int commandID) {
    printf("\"%d.%s\",", commandID, Pk_fields[CMD_ID]);
    printf("\"%d\",", getpid());

    int field;
    for (field = 1; field < CODE; field++) {
        int character;
        printf("\"");
        for (character = 0; character < strlen(Pk_fields[field]); character++) {
            if (Pk_fields[field][character] == '"') {
                printf("\"\"");
            } else {
                printf("%c", Pk_fields[field][character]);
            }
        }
        printf("\",");
    }
    if(strcmp(Pk_fields[CODE], "false") != 0){
        printf("\"%s\"", Pk_fields[CODE]);
    } else {
        printf("\"N/A\"");
    }
    printf("\n");
}


/**
 * prints on log using a TXT format style
 **/
void printTxt(char **Pk_fields, int commandID) {
    printf("COMMAND ID:\t\t%d.%s\n", commandID, Pk_fields[CMD_ID]);
    printf("LOGGER PID:\t\t%d\n", getpid());
    printf("SENDER PID:\t\t%s\n", Pk_fields[SENDER_PID]);
    printf("SHELL PID:\t\t%s\n", Pk_fields[SHELL_PID]);
    printf("STARTED:\t\t%s\n", Pk_fields[START]);
    printf("ENDED:\t\t%s\n", Pk_fields[END]);
    printf("DURATION:\t\t%ss\n", Pk_fields[DURATION]);
    printf("TYPE:\t\t\t%s\n", Pk_fields[TYPE]);
    printf("COMMAND:\t\t%s\n", Pk_fields[CMD]);
    printf("SUBCOMMAND:\t\t%s\n", Pk_fields[SUB_CMD]);
    printf("OUTPUT LENGTH:\t%ld\n", strlen(Pk_fields[OUT]));
    printf("OUTPUT:\n\n%s\n\n", Pk_fields[OUT]);
    if (strcmp(Pk_fields[CODE], "false") != 0) {
        printf("RETURN CODE:\t%s\n", Pk_fields[CODE]);
    }
    printf("---------------------------------------------------\n");
}
