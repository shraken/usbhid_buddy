CC = sdcc
INCLUDE_DIRS = -Iinc -I../lib/libbuddy
SOURCE_DIR = src
BUILD_DIR = build
EXECUTABLE = buddy

#SOURCES = src/main.c
#OBJECTS = $(SOURCES:.c=$(BUILD_DIR).rel)
#OBJECTS = $(patsubst %.c, %.rel, $(SOURCES))

SOURCES = $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS = $(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.rel,$(SOURCES))

CFLAGS = --model-large -c
CFLAGS += $(INCLUDE_DIRS)

LDFLAGS = --model-large
LDFLAGS += $(INCLUDE_DIRS)

all: directories $(BUILD_DIR)/$(EXECUTABLE)
    
directories: 
	mkdir -p $(BUILD_DIR)
	$(info $$SOURCES is [${SOURCES}])
	$(info $$OBJECTS is [${OBJECTS}])

$(BUILD_DIR)/$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@.ihx

$(OBJECTS): $(BUILD_DIR)/%.rel : $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf flash.jlink
	rm -rf $(BUILD_DIR)

JLINK_OPTS = -device EFM8UB20F64G -if c2 -speed 4000
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