* Thomson Floppy Disk Boot Loader by OlivierP-To8
* April 2024
* Based on https://github.com/OlivierP-To8/InufutoPorts/blob/main/Thomson/AUTO.BOOT/Floppy.asm

* Must be used with a floppy created by fdfs
* Track 20 Sector 1 contains the name of the floppy, fdfs adds:
* - Address where to load file at position 12 & 13 (2 bytes)
* - Track + sector min + sector max at next positions (3 bytes)
* - 0xff end of file to load
* - Address where to exec loaded file at next positions (2 bytes)


(main)BootTO.asm

    org $6200

* Page 0 moniteur [$6000-$60FF] (Stack [$608B-$60CC])
* Page 0 extra-moniteur [$6100-$62FF]
* Free : [$6300-$DFFF]

Buffer_ equ $6300

DKOPC   equ $6048   * Commande du contrôleur de disque
DKDRV   equ $6049   * Numéro du disque (0 à 3)
DKTRK   equ $604B   * Numéro de piste (0 à 39 ou 79)
DKSEC   equ $604C   * Numéro de secteur (1 à 16)
DKSTA   equ $604E   * Etat du contrôleur de disquettes
DKBUF   equ $604F   * Pointeur de la zone tampon d'I/O disque (256 octets max)
TSTRST  equ $60FE   * Sémaphore de demarrage à chaud ou à froid
DKCO    equ $E82A   * Contrôleur de disque
MENU    equ $E82D   * Retour au menu principal

    SETDP $60

    * set S (system stack)
    lds #$60CC

    * set DP (direct page) register
    lda #$60
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

    * back to boot menu if X is $ffff
    ldx #$0000          * value different than $A55A
    stx TSTRST          * cold reset
    jmp MENU

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

