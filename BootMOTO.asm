* Thomson Floppy Disk Boot Loader by OlivierP-To8
* April 2024
* Based on https://github.com/OlivierP-To8/InufutoPorts/blob/main/Thomson/AUTO.BOOT/Floppy.asm

* Must be used with a floppy created by fdfs
* Track 20 Sector 1 contains the name of the floppy, fdfs adds:
* - Address where to load file at position 12 & 13 (2 bytes)
* - Track + sector min + sector max at next positions (3 bytes)
* - 0xff end of file to load
* - Address where to exec loaded file at next position (2 bytes)

(main)BootMOTO.asm

    SETDP $60

    org $2200       * MO org to be able to self modify code

* TO Page 0 moniteur [$6000-$60FF] (Stack [$608B-$60CC])
* MO Page 0 moniteur [$2000-$20FF] (Stack [$2087-$20CC])
* TO Page 0 extra-moniteur [$6100-$62FF]
* MO Page 0 extra-moniteur [$2100-$22FF]
* TO Free : [$6300-$DFFF]
* MO Free : [$2300-$9FFF]

Buffer_ equ $6300

DKOPC   equ $6048   * Commande du controleur de disque
DKDRV   equ $6049   * Numero du disque (0 a 3)
DKTRK   equ $604B   * Numero de piste (0 a 39 ou 79)
DKSEC   equ $604C   * Numero de secteur (1 a 16)
DKSTA   equ $604E   * Etat du controleur de disquettes
DKBUF   equ $604F   * Pointeur de la zone tampon d'I/O disque (256 octets max)
STACK   equ $60CC   * Pile systeme
DKCO    equ $E82A   * Controleur de disque

    lda $FFF2           * Test if MO ($F0) or TO ($70)
    bge BOOT

    lda #$20            * STACK for MO
    sta BOOT+2

    lda #$23            * Buffer
    sta BOOT+12
    sta BOOT+21

    ldd #$3F26          * swi $26
    std ReadSector_+12
    lda #$12            * nop
    sta ReadSector_+14

    * side 1 for MO (the TO9 internal drive can only read side 0)
    lda #$01
    sta <DKDRV

BOOT
    * set S (system stack)
    lds #STACK

    * set DP (direct page) register
    tfr s,d
    tfr a,dp

    * load track 20 sector 1 in buffer
    ldd #$1401
    ldx #Buffer_
    bsr ReadSector_
    tfr x,y
    leay 13,y           * Y points to first track and sector -1

    * read sectors until FileEnd
    ldx Buffer_+12      * X = address where to load file
Boot_file
    leax -5,x           * starts 5 bytes before address to ignore bin header

Boot_track
    leay 1,y            * Y points to track and first sector
    ldd ,y++            * AB = track/sector to read
    cmpa #$ff
    beq Boot_exec

Boot_loop
    bsr ReadSector_
    leax 255,x          * X = where to load next sector
    incb
    cmpb ,y
    bgt Boot_track
    bra Boot_loop

Boot_exec
    leay -1,y           * Y points to exec address
    ldx ,y++            * X = address where to exec file
    pshs y
    jsr ,x              * exec loaded file
    puls y
    ldx ,y+             * X = address where to exec file
    cmpx #$ffff
    bne Boot_file

    * loop when over
    bra BOOT

ReadSector_
    pshs a,b

    * track in A
    sta <DKTRK

    * sector in B
    stb <DKSEC

    * buffer address
    stx <DKBUF

    * read sector command
    lda #$02
    sta <DKOPC

    * call ROM
    jsr DKCO

    puls a,b
    rts

