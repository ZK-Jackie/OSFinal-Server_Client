# Simple Web Client project
TARGET = ./src/main.c
OBJS += $(TARGET).o

# Compiler
cc = gcc

# Compiler flags
CFLAGS = -Wall -g -O2

# Compile
$(TARGET): $(OBJS)
	$(cc) $(CFLAGS) -o $(TARGET) $(OBJS)

$(OBJS) : $(TARGET)
	$(cc) $(CFLAGS) -c $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)