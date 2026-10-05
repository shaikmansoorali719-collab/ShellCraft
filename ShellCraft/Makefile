CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
SRC := $(wildcard src/*.c)
TARGET = shellcraft

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -lreadline -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
