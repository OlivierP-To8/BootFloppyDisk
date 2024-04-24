all: BootMO.fd BootTO.fd

clean:
	rm *.lst *.BIN *.fd
	make -C tools clean

fdfs: tools/fdfs
	make -C tools fdfs

c6809: tools/c6809
	make -C tools c6809


BOOTMO.BIN: tools/c6809 BootMO.asm
	tools/c6809 -bl BootMO.asm $@

INTROMO.BIN: tools/c6809 IntroMO.asm
	tools/c6809 -bl IntroMO.asm $@

DEMOMO.BIN: tools/c6809 DemoMO.asm
	tools/c6809 -bl DemoMO.asm $@

BootMO.fd: tools/fdfs BOOTMO.BIN INTROMO.BIN DEMOMO.BIN
	tools/fdfs -addBL $@ BOOTMO.BIN INTROMO.BIN DEMOMO.BIN


BOOTTO.BIN: tools/c6809 BootTO.asm
	tools/c6809 -bl BootTO.asm $@

INTROTO.BIN: tools/c6809 IntroTO.asm
	tools/c6809 -bl IntroTO.asm $@

DEMOTO.BIN: tools/c6809 DemoTO.asm
	tools/c6809 -bl DemoTO.asm $@

BootTO.fd: tools/fdfs BOOTTO.BIN INTROTO.BIN DEMOTO.BIN
	tools/fdfs -addBL $@ BOOTTO.BIN INTROTO.BIN DEMOTO.BIN

