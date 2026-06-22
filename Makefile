CFLAGS  := $(shell pkg-config --cflags sdl2 SDL2_gfx SDL2_ttf SDL2_mixer SDL2_image)
LDFLAGS := $(shell pkg-config --libs sdl2 SDL2_gfx SDL2_ttf SDL2_mixer SDL2_image)

OBJS := main.o loadg.o DxLib.o

SyobonAction: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o SyobonAction

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o SyobonAction

.PHONY: clean