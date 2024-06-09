# Simple Web Client project


##############################
#     PART 1   Variables     #
##############################
# Target info
TARGET_DIR = ../make-target
TARGET = $(TARGET_DIR)/CLIENT
# Source files
SRC = $(wildcard ./src/*.c)
# Object files
OBJS_DIR = ../make-target/client/obj
OBJS = $(patsubst ./src/%.c,$(OBJS_DIR)/%.o,$(SRC))
# Libraries
LIBS = -L$(TARGET_DIR)/common -lcommon -lpthread	# Common libraries


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
$(TARGET): create_dirs $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
# Default target
create_dirs:
	mkdir -p $(TARGET_DIR)
	mkdir -p $(OBJS_DIR)
# Object files
$(OBJS_DIR)/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
# Clean
clean:
	rm -f $(OBJS) $(TARGET)