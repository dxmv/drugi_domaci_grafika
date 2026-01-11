CC = gcc
IN = main.c src/main_state.c src/vertex.c src/terrain.c src/glad/glad.c src/camera.c src/noise.c src/texture.c src/tree.c
OUT = main.out
CFLAGS = -Wall -DGLFW_INCLUDE_NONE
LFLAGS = -L/opt/homebrew/opt/glfw/lib -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -lm
IFLAGS = -I. -I./include

.SILENT all: clean build run

clean:
	rm -f $(OUT)

build: $(IN) include/main_state.h include/stb_image.h 
	$(CC) $(IN) -o $(OUT) $(CFLAGS) $(LFLAGS) $(IFLAGS)

run: $(OUT)
	./$(OUT)
