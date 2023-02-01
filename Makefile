# Windows variables.
WIN_USR := dev
WIN_IP := 172.16.140.129

# SSH variables.

SSH_KEY := $${HOME}/.ssh/id_ed25519_win

# Project variables.
SRC_DIR := src
OBJ_DIR := obj
BUILD_DIR := build

EXECUTABLE := test

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

CC := x86_64-w64-mingw32-gcc
CFLAGS := -g -Wall -Wextra -pedantic

# Begin targets.

.PHONY: all clean

all: | exe push run

run: 
	ssh -i $(SSH_KEY) $(WIN_USR)@$(WIN_IP) C:/Users/$(WIN_USR)/Desktop/$(EXECUTABLE).exe

push: 
	scp -i $(SSH_KEY) ./build/$(EXECUTABLE).exe $(WIN_USR)@$(WIN_IP):C:/Users/$(WIN_USR)/Desktop/$(EXECUTABLE).exe

exe: $(BUILD_DIR)/$(EXECUTABLE)

$(BUILD_DIR)/$(EXECUTABLE): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/*.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR) $(BUILD_DIR):
	mkdir $@

clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)


