prog: prog.c
	$(CC) $(CFLAGS) -o prog prog.c `sdl2-config --cflags --libs`
