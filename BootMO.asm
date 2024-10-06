* Thomson Floppy Disk Boot Loader by OlivierP-To8
* April 2024
* Based on https://github.com/OlivierP-To8/InufutoPorts/blob/main/Thomson/AUTO.BOOT/Floppy.asm

* Must be used with a floppy created by fdfs
* Track 20 Sector 1 contains the name of the floppy, fdfs adds:
* - Address where to load file at position 12 & 13 (2 bytes)
* - Track + sector min + sector max at next positions (3 bytes)
* - 0xff end of file to load
* - Address where to exec loaded file at next positions (2 bytes)


(main)BootMO.asm

    org $2200

* Page 0 moniteur [$2000-$20FF] (Stack [$2087-$20CC])
* Page 0 extra-moniteur [$2100-$22FF]
* Free : [$2300-$9FFF]

Buffer_ equ $2300

DKOPC   equ $2048   * Commande du contrôleur de disque
DKDRV   equ $2049   * Numéro du disque (0 à 3)
DKTRK   equ $204B   * Numéro de piste (0 à 39 ou 79)
DKSEC   equ $204C   * Numéro de secteur (1 à 16)
DKSTA   equ $204E   * Etat du contrôleur de disquettes
DKBUF   equ $204F   * Pointeur de la zone tampon d'I/O disque (256 octets max)
TSTRST  equ $20FE   * Sémaphore de demarrage à chaud ou à froid
DKCO    equ $26     * Contrôleur de disque
DKBOOT  equ $28     * Lancement du boot

    SETDP $20

    * set S (system stack)
    lds #$20CC

    * set DP (direct page) register
    lda #$20
    tfr a,dp

    * drive 0
    lda #$00
    sta <DKDRV

    * load track 20 sector 1 in buffer
    ldd #$1401
    ldx #Buffer_
    jsr ReadSector_
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
    jsr ReadSector_
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

    * reboot if X is $ffff
    swi
    fcb DKBOOT

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
    swi
    fcb DKCO

    puls a,b
    rts

