BIN := tuya_switch_comm

CFLAGS := -Wall -Wextra -std=c99 -g

BIN_DIR := ./bin
BLD_DIR := ./build
INC_DIR := ./inc
LIB_DIR := ./lib
SRC_DIR := ./src
TUYA_SDK_DIR := ../tuya-iot-core-sdk-main

SO_INCLUDES := \
-I$(TUYA_SDK_DIR)/utils\
-I$(TUYA_SDK_DIR)/projects/tuya_switch_comm\
-I$(TUYA_SDK_DIR)/interface\
-I$(TUYA_SDK_DIR)/include

SRC := $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ := $(patsubst $(SRC_DIR)%.c,$(BLD_DIR)%.o,$(SRC))

#full path
lib_names = \
$(TUYA_SDK_DIR)/build/lib/libutils_modules.so\
$(TUYA_SDK_DIR)/build/lib/liblink_core.so\
$(TUYA_SDK_DIR)/build/lib/libmiddleware_implementation.so\
$(TUYA_SDK_DIR)/build/lib/libplatform_port.so
lib_names2 := $(lib_names:*lib%.so=%) #just the name
lib_names3 := $(lib_names:*lib%.so=-l%) #add an -l flag

.PHONY: all
all:$(BLD_DIR)/$(BIN_DIR)/$(BIN)

$(BLD_DIR)/$(BIN_DIR)/$(BIN): builddeps
	$(CC) -I $(INC_DIR) -o $(BLD_DIR)/$(BIN_DIR)/$(BIN) $(OBJ) -L$(BLD_DIR)/lib $(lib_names3)

$(OBJ): $(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -I $(INC_DIR) $(SO_INCLUDES) -c ./$< -o ./$@

.PHONY: builddeps
builddeps:
	mkdir -p $(BLD_DIR)/$(BIN_DIR)
	mkdir -p $(TUYA_SDK_DIR)/$(BLD_DIR)
	cd $(TUYA_SDK_DIR)/$(BLD_DIR) && cmake .. && $(MAKE)
	cd $(SRC_DIR) && make

.PHONY: install
install:$(BLD_DIR)/$(BIN_DIR)/$(BIN)
	cp -f $(lib_names) /usr/lib

.PHONY: uninstall
uninstall:
	$(RM) -f $(lib_names:$(BLD_DIR)/$(LIB:./%=%)/%.so=/usr/lib/%.so)

.PHONY: clean
clean:
	$(RM) -r $(BLD_DIR)
#	$(RM) -r $(TUYA_SDK_DIR)/$(BLD_DIR)

.PHONY: cleanSDK
cleanSDK:
	$(RM) -r $(BLD_DIR)
	$(RM) -r $(TUYA_SDK_DIR)/$(BLD_DIR)