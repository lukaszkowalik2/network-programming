CC        := gcc
CFLAGS    := -std=c99 -pedantic -Wall
BUILD_DIR := build

.PHONY: all clean

all: $(BUILD_DIR)/palindrome-server $(BUILD_DIR)/palindrome-client

$(BUILD_DIR)/palindrome-server: palindrome-server.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/palindrome-server $<

$(BUILD_DIR)/palindrome-client: palindrome-client.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/palindrome-client $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
