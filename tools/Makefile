all: fdfs c6809

clean:
	make -C c6809-v1.0 clean
	-rm fdfs c6809

fdfs: fdfs.c
	gcc -o $@ $@.c

fdfs.exe: fdfs.c
	i686-w64-mingw32-gcc -o $@ fdfs.c

c6809:
	make -C c6809-v1.0 all
	cp c6809-v1.0/c6809 .

