CC = gcc
CFLAGS = -std=c99 -pedantic -Wall
LDFLAGS =

BUILD_DIR = build
TARGET_1 = $(BUILD_DIR)/task-1
TARGET_2 = $(BUILD_DIR)/task-2
TARGET_3 = $(BUILD_DIR)/task-3
TARGET_4 = $(BUILD_DIR)/task-4
TARGET_5 = $(BUILD_DIR)/task-5
TARGET_6 = $(BUILD_DIR)/task-6
TARGET_7 = $(BUILD_DIR)/task-7
TARGET_8 = $(BUILD_DIR)/task-8
SRCS_1 = task-1.c
SRCS_2 = task-2.c
SRCS_3 = task-3.c
SRCS_4 = task-4.c
SRCS_5 = task-5.c
SRCS_6 = task-6.c
SRCS_7 = task-7.c
SRCS_8 = task-8.c
OBJS_1 = $(SRCS_1:%.c=$(BUILD_DIR)/%.o)
OBJS_2 = $(SRCS_2:%.c=$(BUILD_DIR)/%.o)
OBJS_3 = $(SRCS_3:%.c=$(BUILD_DIR)/%.o)
OBJS_4 = $(SRCS_4:%.c=$(BUILD_DIR)/%.o)
OBJS_5 = $(SRCS_5:%.c=$(BUILD_DIR)/%.o)
OBJS_6 = $(SRCS_6:%.c=$(BUILD_DIR)/%.o)
OBJS_7 = $(SRCS_7:%.c=$(BUILD_DIR)/%.o)
OBJS_8 = $(SRCS_8:%.c=$(BUILD_DIR)/%.o)
.PHONY: all clean run_1 run_2 run_3 run_4 run_5 run_6 run_7 run_8

all: $(TARGET_1) $(TARGET_2) $(TARGET_3) $(TARGET_4) $(TARGET_5) $(TARGET_6) $(TARGET_7) $(TARGET_8)

$(TARGET_1): $(OBJS_1)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET_2): $(OBJS_2)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET_3): $(OBJS_3)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET_4): $(OBJS_4)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET_5): $(OBJS_5)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET_6): $(OBJS_6)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET_7): $(OBJS_7)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET_8): $(OBJS_8)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run_1: $(TARGET_1)
	./$(TARGET_1)

run_2: $(TARGET_2)
	./$(TARGET_2)

run_3: $(TARGET_3)
	./$(TARGET_3)

run_4: $(TARGET_4)
	./$(TARGET_4)

run_5: $(TARGET_5)
	./$(TARGET_5)

run_6: $(TARGET_6)
	./$(TARGET_6) $(ARGV)

run_7: $(TARGET_7)
	./$(TARGET_7) $(ARGV)

run_8: $(TARGET_8)
	./$(TARGET_8) $(ARGV)	

clean:
	rm -rf $(BUILD_DIR)
