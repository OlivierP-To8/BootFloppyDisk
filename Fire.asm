* Thomson MO/TO Fire effect by OlivierP-To8
* May 2024
* Based on https://fabiensanglard.net/doom_fire_psx/

(main)Fire.asm

BUFW    equ 10

    ORG $8000       * org compatible with MO and TO

    PSHS A,B,X,Y

    ORCC #80        * don't interrupt

    LDA >$FFF2      * Test if MO ($F0) or TO ($70)
    BLT INITMO
    BSR INITTO


********************************************************************************
* Effacement écran
********************************************************************************
DEBUT
    * commutation du bit de couleur (C0 a 0)
    LDX PRC
    LDA ,X
    ANDA #$FE
    STA ,X

COUL_EFF
    LDD FEU_LEVELMO * couleur fond
    BSR EFF

    * commutation du bit de forme (C0 a 1)
    LDX PRC
    LDA ,X
    ORA #$01
    STA ,X

    LDD #$3333      * alternance forme / fond
    BSR EFF

    * commutation du bit de couleur (C0 a 0)
    LDX PRC
    LDA ,X
    ANDA #$FE
    STA ,X

    BSR BUFFER_DEBUT


********************************************************************************
* Boucle principale
********************************************************************************
BOUCLE
    BSR METTRE_FEU
    BRA BOUCLE

    PULS A,B,X,Y

    RTS


********************************************************************************
* Constantes MO/TO
********************************************************************************
PRC         FDB $A7C0
VRAM_DEB    FDB $0000
VRAM_FIN    FDB $1F40

INITMO
    LDD #FEU_LEVELMO
    STD COUL_EFF+1
    STD BUFFER_DEBUT+1
    STD AFF_FEU_LIGNE-2
    BRA DEBUT

INITTO
    LDD #$E7C3
    STD PRC

    LDD #$4000
    STD VRAM_DEB

    LDD #$5F40
    STD VRAM_FIN
    LDB AFF_FEU_LIGNE-6
    ADDB #$40
    STB AFF_FEU_LIGNE-6

    LDD #FEU_LEVELTO
    STD COUL_EFF+1
    STD BUFFER_DEBUT+1
    STD AFF_FEU_LIGNE-2
    RTS


********************************************************************************
* Effacement de l'écran
********************************************************************************
EFF
    LDY VRAM_DEB
EFF_RAM
    STD ,Y+
    CMPY VRAM_FIN
    BLO EFF_RAM
    RTS


********************************************************************************
* Initialisation du buffer avec une ligne blanche en bas de l'écran
********************************************************************************
BUFFER_DEBUT
    LDD FEU_LEVELMO   * noir
    LDY #BUFFER
EFF_LIGNE_BUFFER
    STD ,Y++
    CMPY #BUFFER_FIN-BUFW
    BLO EFF_LIGNE_BUFFER

    LDA #(FEU_LEVELMO_FIN-FEU_LEVELMO)
EFF_DERLIGNE_BUFFER
    STA ,Y+
    CMPY #BUFFER_FIN
    BLO EFF_DERLIGNE_BUFFER
    RTS


********************************************************************************
* Mettre le feu
********************************************************************************
METTRE_FEU
    LDU #BUFFER+BUFW
    LDY #RANDOM
FEU_LIGNE
    PULU A
    TSTA
    BEQ RNDA_POSITIF
    SUBA ,Y+
    TSTA
    BGE RNDA_POSITIF
    LDA #$00
RNDA_POSITIF
    CMPY #RANDOM_FIN
    BLO FEU_NEXT
    LDY #RANDOM
FEU_NEXT
    LEAU -BUFW,U
    PSHU A
    LEAU BUFW+1,U
    CMPU #BUFFER_FIN
    BLO FEU_LIGNE

    * le calcul est fait dans un buffer de 10 colonnes
    * l'affichage est répété 4 fois (pour les 40 octets par ligne)
    LDU #BUFFER+BUFW
    LDX #$1F40-(BUFFER_FIN-BUFFER)*(40/BUFW)
    LDY #FEU_LEVELMO
AFF_FEU_LIGNE
    PULU D
    LDA A,Y
    LDB B,Y
    STD ,X
    STD 10,X
    STD 20,X
    STD 30,X
    PULU D
    LDA A,Y
    LDB B,Y
    STD 2,X
    STD 12,X
    STD 22,X
    STD 32,X
    PULU D
    LDA A,Y
    LDB B,Y
    STD 4,X
    STD 14,X
    STD 24,X
    STD 34,X
    PULU D
    LDA A,Y
    LDB B,Y
    STD 6,X
    STD 16,X
    STD 26,X
    STD 36,X
    PULU D
    LDA A,Y
    LDB B,Y
    STD 8,X
    STD 18,X
    STD 28,X
    STD 38,X
    LEAX 40,X
    CMPU #BUFFER_FIN
    BLO AFF_FEU_LIGNE
    RTS


FEU_LEVELMO       * couleur selon la hauteur
    FCB $00,$00,$10,$01
    FCB $10,$01,$10,$01,$10,$01,$10,$01,$10,$01
    FCB $19,$91,$19,$91,$19,$91,$19,$91,$19,$91
    FCB $F9,$9F,$F9,$9F,$F9,$9F,$F9,$9F,$F9,$9F
    FCB $F3,$3F,$F3,$3F,$F3,$3F,$F3,$3F,$F3,$3F
    FCB $B3,$3B,$B3,$3B,$B3,$3B,$B3,$3B,$B3,$3B
    FCB $B7,$7B,$B7,$7B,$B7,$7B,$B7,$7B,$B7,$7B
    FCB $77,$77,$77,$77
FEU_LEVELMO_FIN

FEU_LEVELTO       * couleur selon la hauteur
    FCB $C0,$C0,$C8,$C1
    FCB $C8,$C1,$C8,$C1,$C8,$C1,$C8,$C1,$C8,$C1
    FCB $49,$89,$49,$89,$49,$89,$49,$89,$49,$89
    FCB $39,$0F,$39,$0F,$39,$0F,$39,$0F,$39,$0F
    FCB $BB,$5F,$BB,$5F,$BB,$5F,$BB,$5F,$BB,$5F
    FCB $9B,$5B,$9B,$5B,$9B,$5B,$9B,$5B,$9B,$5B
    FCB $9F,$7B,$9F,$7B,$9F,$7B,$9F,$7B,$9F,$7B
    FCB $FF,$FF,$FF,$FF
FEU_LEVELTO_FIN


RANDOM
    FCB 1,2,1,1,1,1,1,2,1,2,1,2,1,2,2,1
    FCB 2,1,2,1,1,2,2,2,2,2,2,1,1,1,1,1
    FCB 2,2,2,2,1,1,2,1,1,2,2,2,1,2,2,1
    FCB 2,1,1,1,1,1,2,1,2,2,2,2,1,2,1,1
    FCB 1,2,2,1,2,2,1,2,1,1,2,1,2,2,1,1
    FCB 1,1,1,1,2,1,1,1,2,1,1,1,1,2,1,2
    FCB 1,2,1,1,1,1,1,2,1,2,1,2,1,2,2,1
    FCB 2,2,1,2,1,1,2,1,1,2,1,1,2,1,1,2
    FCB 2,2,2,2,1,1,2,2,2,2,2,2,2,1,2,1
    FCB 1,2,2,1,1,1,1,1,2,1,2,1,2,2,1,1
    FCB 1,1,2,1,2,2,2,2,2,2,2,1,1,1,1,2
    FCB 1,1,2,1,1,1,2,1,1,2,2,1,1,2,2,2
RANDOM_FIN


BUFFER
    RMB BUFW*(FEU_LEVELMO_FIN-FEU_LEVELMO)
BUFFER_FIN

    END $8000
