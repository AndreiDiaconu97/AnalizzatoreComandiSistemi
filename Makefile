#USAGE: just put your source-files folder in the same directory as this Makefile, then follow instructions.

### MACRO ###
ABS_P := $(shell pwd)# project absolute path
SRC_DIR := src
BIN_DIR := bin
TMP_DIR := temp
EXEC := run# executable name

## program exclusive ##
CFG_DIR := config
LOG_DIR := logs

## compiling phase ##
CC := gcc
#MACRO have to be converted in string format, thus encapsulation with \" is used
CFLAGS = -std=gnu90
CFLAGS += -D ABS_P="\"$(ABS_P)\""
CFLAGS += -D TEMP_DIR="\"/$(TMP_DIR)/\""
CFLAGS += -D CFG_DIR="\"/$(CFG_DIR)/\""
CFLAGS += -D LOG_DIR="\"/$(LOG_DIR)/\""

## managed files ##
srcs := $(wildcard $(SRC_DIR)/*.c)# source files
objs := $(srcs:$(SRC_DIR)%.c=$(TMP_DIR)%.o)# one '.o' for every '.c' file
deps := $(wildcard $(SRC_DIR)/*.h)# header files

.PHONY: DEFAULT help h build b clean c

### RULES ###
DEFAULT:
	@ echo "use 'help' or 'h' for info"

help h:
	@ echo "/---------------------------------------------------------------------------/"
	@ echo "|---------------------------------------------------------------------------------------------/"
	@ echo "| STATISTICHE COMANDI E ANALISI - makefile                                                    |"
	@ echo "|---------------------------------------------------------------------------------------------\\"
	@ echo "\\---------------------------------------------------------------------------\\"
	@ echo ""
	@ echo "-----------------------------------------------------------------------------------------------"
	@ echo "COMMAND LIST"
	@ echo "command\t| c :\tdescription"
	@ echo "-----------------------------------------------------------------------------------------------"
	@ echo "help\t| h :\tLists usable commands"
	@ echo "build\t| b :\tSets up project structure, files and executable"
	@ echo "clean\t| c :\tCancels every file and folder created with [build | b] command"
	@ echo "\n"
	@ echo "-----------------------------------------------------------------------------------------------"
	@ echo "CUSTOMIZABLE MACRO LIST"
	@ echo "COMMAND\t<value>\t: description" 
	@ echo "-----------------------------------------------------------------------------------------------"
	@ echo "EXEC\t: executable file name"
	@ echo "CC\t: compiler"
	@ echo ""
	@ echo "ABS_P\t: absolute path of the project"
	@ echo "SRC_DIR\t: source folder name"
	@ echo "BIN_DIR\t: name of folder containing executables"
	@ echo "TMP_DIR\t: temporary files folder name"
	@ echo "CFG_DIR\t: configuration files folder name"
	@ echo "LOG_DIR\t: log files folder name"
	@ echo "-----------------------------------------------------------------------------------------------"
	@ echo "/* PS: directory name can include relative path, but must not include starting dot */"
	@ echo ""

##
# Deletes executable and *.o files of the project.
# -
# WARNING: bin folder is deleted with all its contents so be careful not to put personal files in it.
# -
# Only *.o files in temp folder will be removed, the folder itself will not be deleted as it
# can still contain temporary files of running logger.
##
clean c:
	@ rm -f $(BIN_DIR)/$(EXEC) $(TMP_DIR)/*.o
	@ rm -rf $(BIN_DIR)
	@ echo "Project cleaned"
	

build b: $(BIN_DIR)/$(EXEC)
# link *.o files after checking for folders and *.c timestamps
$(BIN_DIR)/$(EXEC) : $(BIN_DIR) $(TMP_DIR) $(objs)
	@ echo "\nLinking *.o files..."
	@ $(CC) -o $@ $(objs)
	@ echo "Executable is ready, path:'$(ABS_P)'"

# creating folders
$(BIN_DIR) $(TMP_DIR):
	@ mkdir -p $@
	@ echo "Folder '$@' created."

# compile sources checking for dependencies timestamps too
$(TMP_DIR)/%.o : $(SRC_DIR)/%.c $(deps)
	@ $(CC) $(CFLAGS) -o $@ -c $<
	@ echo "./$@ \tupdated/created"



##################
### CLEAN INFO ###
################################################################################################################################ \
rm -rf /path/to/directory \
	To remove the folder with all its contents(including all interior folders): \
\
rm -rf /path/to/directory/* \
	To remove all the contents of the folder(including all interior folders) but not the folder itself: \
\
rm -f /path/to/directory/* \
	To remove all the "files" from inside a folder(not removing interior folders): \
\
Where: \
	rm - stands for "remove" \
	-f - stands for "force" which is helpful when you don't want to be asked/prompted if you want to remove an archive, for example. \
	-r - stands for "recursive" which means that you want to go recursively down every folder and remove everything. \
################################################################################################################################ \
