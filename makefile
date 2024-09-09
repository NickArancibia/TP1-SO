CC = gcc
CFLAGS = -Wall -g -std=c99
SRC_DIR = src

SOURCES = $(SRC_DIR)/slave.c $(SRC_DIR)/md5.c $(SRC_DIR)/vista.c

EXECUTABLES = $(patsubst $(SRC_DIR)/%.c, %, $(SOURCES))

all: $(EXECUTABLES)

slave: $(SRC_DIR)/slave.c
	$(CC) $(CFLAGS) -o $@ $<

md5: $(SRC_DIR)/md5.c
	$(CC) $(CFLAGS) -D_XOPEN_SOURCE=500 -o $@ $<

vista: $(SRC_DIR)/vista.c
	$(CC) $(CFLAGS)  -o $@ $(SRC_DIR)/vista.c $(SRC_DIR)/vistaLib.c

clean:
	rm -f $(EXECUTABLES)

.PHONY: all clean