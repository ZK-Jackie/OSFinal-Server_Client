# Common library


##############################
#     PART 1   Variables     #
##############################
# Target directory
TARGET_DIR = ../make-target
TARGET = $(TARGET_DIR)/common/libcommon.a
# Source files
SRC = $(wildcard ./src/*.c)
# Object files
OBJS_DIR = ../make-target/common/obj
OBJS = $(patsubst ./src/%.c,$(OBJS_DIR)/%.o,$(SRC))


# Compiler flags
CFLAGS = -Wall -g -O2 -I./include

# Compile
$(TARGET): create_dirs $(OBJS)
	ar rcs $(TARGET) $(OBJS)
# Default target
create_dirs:
	mkdir -p $(TARGET_DIR)
	mkdir -p $(OBJS_DIR)
# Object files
$(OBJS_DIR)/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
# Clean
clean:
	rm -f $(LIB_OBJS) $(TARGET)
