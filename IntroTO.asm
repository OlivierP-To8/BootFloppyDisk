(main)IntroTO.asm

    org $6600

PUTC    equ $E803   * Affichage d'un caractère

    leax STR_INTRO,PCR
Intro_putc
    ldb ,X+
    beq Intro_end
    jsr PUTC
    bra Intro_putc
Intro_end

    lda #$ff
Wait1
    ldb #$ff
Wait2
    decb
    bne Wait2
    deca
    bne Wait1

    rts

STR_INTRO FCB $14          * effacer le curseur de l'écran
          FCB $1B,$47      * couleur forme blanc
          FCB $1B,$50      * couleur fond  noir
          FCB $1B,$60      * couleur tour  noir
          FCB $0C          * effacement de la fenêtre
          FCB $1F,$40,$40  * positionnement du curseur
          FCB $0D          * retour en debut de ligne courante
          FCS "Fire effect ported by OlivierP-To8"

    end $6600

