# Common library
LIB_SRC = $(wildcard ./src/*.c)
LIB_OBJS = $(LIB_SRC:.c=.o)
LIB_TARGET = libcommon.a

# Compiler flags
CFLAGS = -Wall -g -O2 -I./include

# Compile library
$(LIB_TARGET): $(LIB_OBJS)
	ar rcs $(LIB_TARGET) $(LIB_OBJS)
