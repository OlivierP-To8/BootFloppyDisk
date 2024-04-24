(main)IntroMO.asm

    org $2600

PUTC    equ $02   * Affichage d'un caractère

    leax STR_INTRO,PCR
Intro_putc
    ldb ,X+
    beq Intro_end
    swi
    fcb PUTC
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

STR_INTRO FCB $0D       * retour en début de ligne courante
          FCB $0A       * descente d'une ligne
          FCS "Introduction"

    end $2600

