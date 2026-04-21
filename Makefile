CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
TARGET  = data_dictionary
SRCS    = main.c data_dictionary.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) *.bin

.PHONY: all clean
