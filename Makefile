CC=g++
LFLAGS = -lGL -lGLEW -lglfw -lGLU
CFLAGS = -std=c++11 -c

OS = $(shell uname)

ifeq ($(OS), Darwin)
all:
		$(CC) -g  craytracer main.cpp -lGLEW -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
else
all:
		$(CC) $(LFLAGS) $(CFLAGS )-g -o firstgame main.cpp ship.cpp imgui/imgui_impl_glfw_gl3.cpp imgui/imgui.cpp imgui/imgui_draw.cpp
endif
