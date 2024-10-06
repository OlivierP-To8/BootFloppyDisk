(main)DemoMO.asm

    org $2600

PUTC    equ $02   * Affichage d'un caractère

    leax STR_DEMO,PCR
Demo_putc
    ldb ,X+
    beq Demo_end
    swi
    fcb PUTC
    bra Demo_putc
Demo_end

    lda #$ff
Wait1
    ldb #$ff
Wait2
    decb
    bne Wait2
    deca
    bne Wait1

    rts

STR_DEMO  FCB $0D       * retour en début de ligne courante
          FCB $0A       * descente d'une ligne
          FCS "Running on MO"

    end $2600

