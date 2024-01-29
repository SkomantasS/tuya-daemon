SRC := $(shell find -name '*.c')
OBJ := $(SRC:%.c=%.o)
INC_DIR := ../inc
BLD_DIR := ../build
CFLAGS := -Wall -Wextra -std=c99 -g

OBJ := $(OBJ:%.o=$(BLD_DIR)/%.o)

TUYA_SDK_DIR := ../../tuya-iot-core-sdk-main/

SO_INCLUDES := \
-I$(TUYA_SDK_DIR)/utils\
-I$(TUYA_SDK_DIR)/projects/tuya_switch_comm\
-I$(TUYA_SDK_DIR)/interface\
-I$(TUYA_SDK_DIR)/include

.PHONY: all
all: $(OBJ)

$(OBJ): $(BLD_DIR)/%.o: %.c
	mkdir -p $(BLD_DIR)
	$(CC) -I$(INC_DIR) $(SO_INCLUDES) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(OBJ)