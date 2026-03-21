CC        := gcc
CFLAGS    := -std=c99 -pedantic -Wall
BUILD_DIR := build
BINARIES  := tcp-server tcp-client udp-server udp-client
OUT       := $(addprefix $(BUILD_DIR)/,$(BINARIES))

.PHONY: all clean $(BINARIES)

all: $(OUT)

tcp-server: $(BUILD_DIR)/tcp-server
tcp-client: $(BUILD_DIR)/tcp-client
udp-server: $(BUILD_DIR)/udp-server
udp-client: $(BUILD_DIR)/udp-client

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
