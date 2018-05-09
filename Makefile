#USAGE: just put your source-files folder in the same directory as this Makefile, then follow instructions.

### MACRO ###
CC := gcc
SRC := src
BIN := bin
TMP := tmp

exec := run
srcs := $(wildcard $(SRC)/*.c) # source files
objs := $(srcs:$(SRC)%.c=$(TMP)%.o) #one '.o' for every '.c' file
deps := $(wildcard $(SRC)/*.h) # header files


.PHONY: DEFAULT help h build b clean c
### RULES ###
DEFAULT:
	@echo "type 'help' or 'h' for info"

help h:
	@echo "INFO [command]: description"
	@echo "--------------------------------------------------------------------------------"
	@echo "[help  | h] :  Lists usable commands."
	@echo "[build | b] : Sets up project structure, files and executable."
	@echo "[clean | c] : Cancels every file and folder created with [build | b] command."
	@echo "\n"
	@echo "customizable MACRO list:"
	@echo "--------------------------------------------------------------------------------"
	@echo "CC  : compiler"
	@echo "SRC : source folder relative path"
	@echo "BIN : relative path of folder containing executables"
	@echo "TMP : temporary files folder relative path"
	@echo "(relative path must include the name of the element itself)"

clean c:
	@rm -f $(BIN)/$(exec) $(objs)
	@rmdir $(BIN) $(TMP)
	@echo "Project cleaned"

build b: $(BIN)/$(exec)

.$(BIN)/$(exec) : $(BIN) $(TMP) $(objs) #first checks folders
	@echo "\nLinking *.o files..."
	@$(CC) -o $@ $(objs)
	@echo "Executable is ready, path:'./$@'"

.$(BIN) $(TMP): 
	@mkdir -p $@
	@echo "Folder '$@' created."

.$(TMP)/%.o : $(SRC)/%.c $(deps)
	@$(CC) -o $@ -c $<
	@echo "./$@ updated"

