BIN_DIR = bin
SRC_DIR = src
UTILS_DIR = $(SRC_DIR)/utils
DEVICES_DIR = $(SRC_DIR)/devices
NETMAN_DIR = $(SRC_DIR)/netman

YAML_DYNLIB = yaml

TARGET = tinynet
DEBUG_TARGET = debug_tinynet

SRCS = $(wildcard $(NETMAN_DIR)/*.c) \
	   $(wildcard $(DEVICES_DIR)/*.c) \
	   $(wildcard $(UTILS_DIR)/addr/*.c) $(wildcard $(UTILS_DIR)/yaml/*.c) $(wildcard $(UTILS_DIR)/visualize/*.c) \
       main.c

CC = clang
CFLAGS = -Wall -Wextra -I.
DEBUG_FLAGS = -g3 -ggdb
LDFLAGS = -l$(YAML_DYNLIB)

all: clean $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(SRCS)
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Built target: $@"

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(BIN_DIR)/$(DEBUG_TARGET)

$(BIN_DIR)/$(DEBUG_TARGET): $(SRCS)
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Built debug target: $@"

run: all
	@./$(BIN_DIR)/$(TARGET)

run_debug:
	@cgdb ./$(BIN_DIR)/$(DEBUG_TARGET)

clean:
	@rm -rf $(BIN_DIR)
	@find . -name "*.o" -delete
	@echo "Cleaned up build files."

.PHONY: all clean run debug test run_debug
