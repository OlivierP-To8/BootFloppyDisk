(main)IntroTO.asm

    org $6600

PUTC    equ $E803   * Affichage d'un caract�re

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

STR_INTRO FCB $0D       * retour en d�but de ligne courante
          FCB $0A       * descente d'une ligne
          FCS "Introduction"

    end $6600
