CFLAGS += -Wall $(shell pkg-config --cflags sdl2)

LIBS = $(shell pkg-config --libs sdl2) -lSDL2_gfx

CC ?= gcc

TARGET = led_fan

all:: $(TARGET)

led_fan: led_fan.o
	$(CC) $^ -o $@ $(LIBS)

led_fan.o: led_fan.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean::
	-rm -f $(TARGET) *.o
