main: directories init.rel gpio.rel millis.rel timers.rel uart.rel F3xx_USB0_Descriptor.rel \
	F3xx_USB0_InterruptServiceRoutine.rel F3xx_USB0_ReportHandler.rel F3xx_USB0_Standard_Requests.rel
	sdcc -Iinc --model-large src\main.c build\init.rel build\gpio.rel \
	build\millis.rel build\timers.rel build\uart.rel build\F3xx_USB0_Descriptor.rel \
	build\F3xx_USB0_InterruptServiceRoutine.rel build\F3xx_USB0_ReportHandler.rel \
	build\F3xx_USB0_Standard_Requests.rel -o build/

directories: 
	mkdir build

F3xx_USB0_Descriptor.rel: src\F3xx_USB0_Descriptor.c
	sdcc -Iinc --model-large -c src\F3xx_USB0_Descriptor.c -o build/

F3xx_USB0_InterruptServiceRoutine.rel: src\F3xx_USB0_InterruptServiceRoutine.c
	sdcc -Iinc --model-large -c src\F3xx_USB0_InterruptServiceRoutine.c -o build/

F3xx_USB0_ReportHandler.rel: src\F3xx_USB0_ReportHandler.c
	sdcc -Iinc --model-large -c src\F3xx_USB0_ReportHandler.c -o build/

F3xx_USB0_Standard_Requests.rel: src\F3xx_USB0_Standard_Requests.c
	sdcc -Iinc --model-large -c src\F3xx_USB0_Standard_Requests.c -o build/

gpio.rel: src\gpio.c inc\gpio.h
	sdcc -Iinc --model-large -c src\gpio.c -o build/

init.rel: src\init.c inc\init.h
	sdcc -Iinc --model-large -c src\init.c -o build/

millis.rel: src\millis.c inc\millis.h
	sdcc -Iinc --model-large -c src\millis.c -o build/

timers.rel: src\timers.c inc\timers.h
	sdcc -Iinc --model-large -c src\timers.c -o build/

uart.rel: src\uart.c inc\uart.h
	sdcc -Iinc --model-large -c src\uart.c -o build/

.PHONY: clean

clean:
	rm -rf flash.jlink
	rm -rf build

JLINK_OPTS = -device EFM8UB20F64G -if c2 -speed 4000
# flashing windows vs. posix
ifeq ($(OS),Windows_NT)
JLINK = JLink $(JLINK_OPTS)
else
JLINK = JLinkExe $(JLINK_OPTS)
endif # flashing windows vs. posix

flash: flash.jlink
	packihx build\main.ihx > build\main.hex
	$(JLINK) flash.jlink

flash.jlink:
	printf "erase\nloadbin build\main.hex,0\nexit\n" > flash.jlink