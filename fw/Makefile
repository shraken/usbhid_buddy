CC = sdcc
LIBBUDDY_PATH = ../lib/libbuddy
INCLUDE_DIRS = -Icode/include -Icode/include/drivers -I$(LIBBUDDY_PATH)/common
SOURCE_DIR = code/source
BUILD_DIR = build
EXECUTABLE = buddy

SOURCES = $(shell find $(SOURCE_DIR) -type f -name '*.c')
OBJECTS = $(patsubst $(SOURCE_DIR)/%.c, $(BUILD_DIR)/%.rel, $(SOURCES))

CFLAGS = --model-large --std-sdcc99 -c
CFLAGS += $(INCLUDE_DIRS)

LDFLAGS = --model-large --std-sdcc99
LDFLAGS += $(INCLUDE_DIRS)

all: directories $(BUILD_DIR)/$(EXECUTABLE)
    
directories: 
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/drivers
	$(info $$SOURCES is [${SOURCES}])
	$(info $$OBJECTS is [${OBJECTS}])

$(BUILD_DIR)/$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@.ihx

$(OBJECTS): $(BUILD_DIR)/%.rel : $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf flash.jlink
	rm -rf $(BUILD_DIR)

JLINK_OPTS = -device EFM8UB20F64G -if c2 -speed 200
# flashing windows vs. posix
ifeq ($(OS),Windows_NT)
JLINK = JLink $(JLINK_OPTS)
else
JLINK = JLinkExe $(JLINK_OPTS)
endif # flashing windows vs. posix

flash: flash.jlink
	packihx $(BUILD_DIR)/$(EXECUTABLE).ihx > $(BUILD_DIR)/$(EXECUTABLE).hex
	$(JLINK) flash.jlink

flash.jlink:
	printf "erase\nloadbin $(BUILD_DIR)/$(EXECUTABLE).hex,0\nexit\n" > flash.jlink