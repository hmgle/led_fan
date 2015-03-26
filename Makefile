HAS_SDL2_gfx = $(shell pkg-config SDL2_gfx 2> /dev/null; echo $$?)
ifneq ($(HAS_SDL2_gfx),0)
	SDL2_gfx_INC =
	SDL2_gfx_LIB = -lSDL2_gfx
else
	SDL2_gfx_INC = $(shell pkg-config --cflags SDL2_gfx)
	SDL2_gfx_LIB = $(shell pkg-config --libs SDL2_gfx)
endif
CFLAGS += -O2 -Wall $(shell pkg-config --cflags sdl2) $(SDL2_gfx_INC)
LIBS = $(shell pkg-config --libs sdl2) -lm $(SDL2_gfx_LIB)

CC ?= gcc

TARGET = led_fan

all:: $(TARGET)

led_fan: led_fan.o dotfont.o encoding_convert.o plane.o nyancat.o
	$(CC) $^ -o $@ $(LIBS)

led_fan.o: led_fan.c
	$(CC) -c $(CFLAGS) $^ -o $@

encoding_convert.o: encoding_convert.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean::
	-rm -f $(TARGET) *.o
