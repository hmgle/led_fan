CFLAGS += -Wall `pkg-config sdl2 --cflags`
LIBS = `pkg-config sdl2 --libs`
CC ?= gcc

TARGET = led_fan

all: $(TARGET)

led_fan: led_fan.o
	$(CC) $^ -o $@ $(LIBS)

led_fan.o: led_fan.c
	$(CC) -c $^ -o $@ $(CFLAGS)
