CC        := gcc
CFLAGS    := -std=c99 -pedantic -Wall
BUILD_DIR := build
BINARIES  := tcp-server tcp-client udp-server udp-client palindrome-server
OUT       := $(addprefix $(BUILD_DIR)/,$(BINARIES))

.PHONY: all clean $(BINARIES)

all: $(OUT)

define BINARY_RULE
$(1): $(BUILD_DIR)/$(1)
endef
$(foreach b,$(BINARIES),$(eval $(call BINARY_RULE,$(b))))

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
