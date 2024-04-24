all: fdfs c6809

clean:
	-rm fdfs c6809

fdfs: fdfs.c
	gcc -o $@ $@.c

fdfs.exe: fdfs.c
	i686-w64-mingw32-gcc -o $@ fdfs.c

c6809: c6809.c
	gcc -o $@ $@.c

c6809.exe: c6809.c
	i686-w64-mingw32-gcc -o $@ c6809.c

