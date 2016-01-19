all:
	gcc -g -o prog prog.c `sdl2-config --cflags --libs`
