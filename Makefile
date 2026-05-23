# Variables
CXX = g++
SRCS = main.cpp game.cpp player.cpp renderer.cpp dialogue.cpp map.cpp 

# Linux Output and Flags
LINUX_OUT = game
LINUX_FLAGS = -lraylib

# Windows Output and Flags
WIN_OUT = game.exe
WIN_FLAGS = -I/c/raylib/raylib/include -L/c/raylib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static-libgcc -static-libstdc++

# Auto-detect OS
ifeq ($(OS), Windows_NT)
    TARGET = windows
else
    TARGET = linux
endif

all: $(TARGET)

linux:
	$(CXX) $(SRCS) -o $(LINUX_OUT) $(LINUX_FLAGS)

windows:
	$(CXX) $(SRCS) -o $(WIN_OUT) $(WIN_FLAGS)

clean:
	rm -f $(LINUX_OUT) $(WIN_OUT)
