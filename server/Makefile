# Simple Web Client project


##############################
#     PART 1   Variables     #
##############################
# Target directory
TARGET_DIR = ../make-target
TARGET = $(TARGET_DIR)/SERVER	# Executable name
# Source files
SRC = $(wildcard ./src/*.c)	# Source files
# Object files
OBJS = $(SRC:.c=.o)			# Object files
# Libraries
LIBS = -L../common -lcommon -lpthread	# Common libraries


##############################
#     PART 2   Commands      #
##############################
# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -g -O2 -I./include -I../common/include
# -Wall: Show all warnings
# -g: Debugging information
# -O2: Optimize code
# -I: Include directory


##############################
#     PART 3   Targets       #
##############################
# Compile
$(TARGET): create_dir $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
# Default target
create_dir:
	mkdir -p $(TARGET_DIR)
# Object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
# Clean
clean:
	rm -f $(OBJS) $(TARGET)