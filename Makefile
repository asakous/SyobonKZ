CFLAGS:= `pkg-config --cflags sdl sdl_gfx sdl_ttf sdl_mixer sdl_image`
LDFLAGS:= `pkg-config --libs sdl sdl_gfx sdl_ttf sdl_mixer sdl_image`

SyobonAction:main.o loadg.o DxLib.o
	$(CXX) $(LDFLAGS) main.o loadg.o DxLib.o -o SyobonAction

%.o : %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm *.o
