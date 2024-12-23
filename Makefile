BIN_DIR = bin
SRC_DIR = src

FSM_DIR = $(SRC_DIR)/fsm
ALG_DIR = $(SRC_DIR)/alg
NET_DIR = $(SRC_DIR)/net
VIZ_DIR = $(SRC_DIR)/viz

DEVICES_DIR = $(NET_DIR)/devices
ADDR_DIR = $(NET_DIR)/addr

GVC_DYNIB = gvc 
YAML_DYNLIB = yaml
GRAPH_DYNLIB = cgraph

TARGET = tinynet
DEBUG_TARGET = debug_tinynet

SRCS = $(wildcard $(SRC_DIR)/*.c) \
	   $(wildcard $(VIZ_DIR)/*.c) \
	   $(wildcard $(NET_DIR)/*.c) \
	   $(wildcard $(ALG_DIR)/*.c) \
	   $(wildcard $(FSM_DIR)/*.c) \
	   $(wildcard $(DEVICES_DIR)/*.c) \
	   $(wildcard $(ADDR_DIR)/*.c) \
       main.c

CC = clang
CFLAGS = -Wall -Wextra -Wno-switch -I.
DEBUG_FLAGS = -g3 -ggdb
LDFLAGS = -l$(YAML_DYNLIB) -l$(GVC_DYNIB) -l$(GRAPH_DYNLIB) 

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

viz:
	@dot -Tpng -o ./conf/image/net.png ./conf/data/net.dot

vrun: all
	@./$(BIN_DIR)/$(TARGET)
	@dot -Tpng -o ./conf/image/net.png ./conf/data/net.dot

run_debug: debug
	@cgdb ./$(BIN_DIR)/$(DEBUG_TARGET)

clean:
	@rm -rf $(BIN_DIR)
	@find . -name "*.o" -delete
	@echo "Cleaned up build files."

.PHONY: all viz vrun run clean debug run_debug
