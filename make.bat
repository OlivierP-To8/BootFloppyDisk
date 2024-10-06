tools\c6809.exe -bl IntroMO.asm INTROMO.BIN
tools\c6809.exe -bl DemoMO.asm DEMOMO.BIN
tools\c6809.exe -bl IntroTO.asm INTROTO.BIN
tools\c6809.exe -bl DemoTO.asm DEMOTO.BIN
tools\c6809.exe -bl Fire.asm Fire.BIN
tools\c6809.exe -bl BootMOTO.asm BOOTMOTO.BIN
tools\fdfs.exe -addBL BootMO.fd BOOTMOTO.BIN INTROMO.BIN DEMOMO.BIN FIRE.BIN
tools\fdfs.exe -addBL BootTO.fd BOOTMOTO.BIN INTROTO.BIN DEMOTO.BIN FIRE.BIN
copy /B BootTO.fd + BootMO.fd BootMOTO.fd
pause
