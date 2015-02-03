SDL2_CFLAGS = $(shell pkg-config sdl2 --cflags)
SDL2_IMAGE_CFLAGS = $(shell pkg-config SDL2_image --cflags) 
CFLAGS += -Wall $(SDL2_CFLAGS) $(SDL2_IMAGE_CFLAGS)
SDL2_LIBS = $(shell pkg-config sdl2 --libs)
SDL2_IMAGE_LIBS = $(shell pkg-config SDL2_image --libs) 
LIBS = $(SDL2_LIBS) $(SDL2_IMAGE_LIBS)

CC ?= gcc

TARGET = led_fan

all:: $(TARGET)

led_fan: led_fan.o
	$(CC) $^ -o $@ $(LIBS)

led_fan.o: led_fan.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean::
	-rm -f $(TARGET) *.o
