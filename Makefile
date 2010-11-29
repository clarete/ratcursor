all:
	gcc `pkg-config --libs --cflags x11` main.c -o ratcursor

clean:
	rm -rf ratcursor *~
