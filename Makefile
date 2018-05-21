#USAGE: just put your source-files folder in the same directory as this Makefile, then follow instructions.

### MACRO ###
ABS_P := \"$(shell pwd)\"#project absolute path
SRC := src
BIN := bin
TMP := temp
EXEC := run#executable name

CC := gcc
CFLAGS := -std=gnu90 -D ABS_P=$(ABS_P)

srcs := $(wildcard $(SRC)/*.c) # source files
objs := $(srcs:$(SRC)%.c=$(TMP)%.o) #one '.o' for every '.c' file
deps := $(wildcard $(SRC)/*.h) # header files


.PHONY: DEFAULT help h build b clean c
### RULES ###
DEFAULT:
	@ echo "use 'help' or 'h' for info"

help h:
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
	@ echo "EXEC\t<$(EXEC)>\t: executable file name"
	@ echo "CC\t<$(CC)>\t: compiler"
	@ echo ""
	@ echo "ABS_P\t<$(ABS_P)>\t: absolute path of the project\
		\n\t(needed as string as it is used for C macro)"
	@ echo "SRC\t<$(SRC)>\t: source folder relative path"
	@ echo "BIN\t<$(BIN)>\t: relative path of folder containing executables"
	@ echo "TMP\t<$(BIN)>\t: temporary files folder relative path"
	@ echo "-----------------------------------------------------------------------------------------------"
	@ echo "/* relative path must include the name of the element itself */"

clean c:
	@ rm -f $(BIN)/$(EXEC) $(TMP)/*.o #$(objs) specialised alternative	(careful about foreign files)
	@ rm -rf $(BIN) #$(TMP) "temp" folder may still be in use by the logger
	@ echo "Project cleaned"
	

build b: $(BIN)/$(EXEC)

$(BIN)/$(EXEC) : $(BIN) $(TMP) $(objs) #first checks folders
	@ echo "\nLinking *.o files..."
	@ $(CC) -o $@ $(objs)
	@ echo "Executable is ready, path:'./$@'"

$(BIN) $(TMP): #creating folders
	@ mkdir -p $@
	@ echo "Folder '$@' created."

$(TMP)/%.o : $(SRC)/%.c $(deps)
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
