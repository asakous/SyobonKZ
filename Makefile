CFLAGS  := $(shell sdl-config --cflags) \
           $(shell pkg-config --cflags SDL_gfx SDL_ttf SDL_mixer SDL_image)
LDFLAGS := $(shell sdl-config --libs) \
           $(shell pkg-config --libs SDL_gfx SDL_ttf SDL_mixer SDL_image)

OBJS := main.o loadg.o DxLib.o

SyobonAction: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o SyobonAction

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o SyobonAction

.PHONY: clean