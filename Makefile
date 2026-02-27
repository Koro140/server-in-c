CC := gcc
FLAGS := -Wall -Wextra

BUILD_DIR = ./build

COMMON_SRC := 

CLIENT_SRC := src/client.c
SERVER_SRC := src/server.c

CLIENT := $(BUILD_DIR)/client
SERVER := $(BUILD_DIR)/server

.PHONY: all clean

all: $(CLIENT) $(SERVER)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(CLIENT): $(CLIENT_SRC) | $(BUILD_DIR)
	$(CC) $(CLIENT_SRC) $(COMMON_SRC) $(FLAGS) -o $(CLIENT)

$(SERVER): $(SERVER_SRC) | $(BUILD_DIR)
	$(CC) $(SERVER_SRC) $(COMMON_SRC) $(FLAGS) -o $(SERVER)

clean:
	rm -rf $(BUILD_DIR)