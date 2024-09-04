CC = gcc
CFLAGS = -Wall -g -std=c99
SRC_DIR = src
OBJ_DIR = obj
EXEC_DIR = bin

SOURCES = $(SRC_DIR)/slave.c $(SRC_DIR)/md5.c $(SRC_DIR)/vista.c $(SRC_DIR)/vistaLib.c

EXECUTABLES = $(patsubst $(SRC_DIR)/%.c, $(EXEC_DIR)/%, $(SOURCES))

all: $(EXECUTABLES)

$(EXEC_DIR)/slave: $(SRC_DIR)/slave.c
	$(CC) $(CFLAGS) -o $@ $<

$(EXEC_DIR)/md5: $(SRC_DIR)/md5.c
	$(CC) $(CFLAGS) -o $@ $<

$(EXEC_DIR)/vista: $(SRC_DIR)/vista.c $(SRC_DIR)/vistaLib.c
	$(CC) $(CFLAGS) -D _XOPEN_SOURCE=500 -o $@ $(SRC_DIR)/vista.c $(SRC_DIR)/vistaLib.c

$(EXEC_DIR):
	mkdir -p $(EXEC_DIR)

$(EXECUTABLES): | $(EXEC_DIR)

clean:
	rm -f $(EXECUTABLES)

.PHONY: all clean