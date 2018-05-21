#USAGE: just put your source-files folder in the same directory as this Makefile, then follow instructions.

### MACRO ###
ABS_P := \"$(shell pwd)\" #project absolute path
SRC := src
BIN := bin
TMP := temp

CC := gcc
CFLAGS := -std=gnu90 -D ABS_P=$(ABS_P)

exec := run #executable name
srcs := $(wildcard $(SRC)/*.c) # source files
objs := $(srcs:$(SRC)%.c=$(TMP)%.o) #one '.o' for every '.c' file
deps := $(wildcard $(SRC)/*.h) # header files


.PHONY: DEFAULT help h build b clean c
### RULES ###
DEFAULT:
	@ echo "type 'help' or 'h' for info"
	@ echo $(ABS)

help h:
	@ echo "INFO [command]: description"
	@ echo "--------------------------------------------------------------------------------"
	@ echo "[help  | h] :  Lists usable commands."
	@ echo "[build | b] : Sets up project structure, files and executable."
	@ echo "[clean | c] : Cancels every file and folder created with [build | b] command."
	@ echo "\n"
	@ echo "customizable MACRO list:"
	@ echo "--------------------------------------------------------------------------------"
	@ echo "CC  : compiler"
	@ echo "SRC : source folder relative path"
	@ echo "BIN : relative path of folder containing executables"
	@ echo "TMP : temporary files folder relative path"
	@ echo "(relative path must include the name of the element itself)"

clean c:
	@ rm -f $(BIN)/$(exec) $(TMP)/*.o #$(objs) specialised alternative	(careful about foreign files)
	@ rm -rf $(BIN) #$(TMP) "temp" folder may still be in use by the logger
	@ echo "Project cleaned"
	

build b: $(BIN)/$(exec)

$(BIN)/$(exec) : $(BIN) $(TMP) $(objs) #first checks folders
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
