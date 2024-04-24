(main)DemoMO.asm

    org $2600

PUTC    equ $02   * Affichage d'un caract�re

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

STR_DEMO  FCB $0D       * retour en d�but de ligne courante
          FCB $0A       * descente d'une ligne
          FCS "DEMO"

    end $2600

