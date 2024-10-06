/*
 *  c6809 version 1.0.0
 *  copyright (c) 2024 François Mouret
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SCAN_DEPEND
    #include <stdio.h>
    #include <string.h>
    #include <ctype.h>
#endif

#include "defs.h"
#include "error.h"
#include "mark.h"
#include "arg.h"
#include "symbol.h"
#include "assemble.h"
#include "source.h"
#include "output.h"
#include "output/bin.h"
#include "output/asm.h"
#include "output/console.h"
#include "output/lst.h"
#include "output/html.h"
#include "output/thomson.h"
#include "directiv/blank.h"
#include "directiv/comment.h"
#include "directiv/end.h"
#include "directiv/fcx.h"
#include "directiv/includ.h"
#include "directiv/if.h"
#include "directiv/macro.h"
#include "directiv/mo.h"
#include "directiv/opt.h"
#include "directiv/org.h"
#include "directiv/page.h"
#include "directiv/print.h"
#include "directiv/rmx.h"
#include "directiv/setdp.h"
#include "directiv/setequ.h"
#include "directiv/title.h"
#include "instruct/inherent.h"
#include "instruct/indexed.h"
#include "instruct/immediat.h"
#include "instruct/bra.h"
#include "instruct/lbra.h"
#include "instruct/pushpull.h"
#include "instruct/tfrexg.h"

#if 0
    #define DO_PRINT  1      /* if output wanted */
    #include <stdlib.h>

    static int display_assemble_lines = 1;  /* 0:errors 1:print lines */
    struct ASS_ERR_LIST {
        int addr;
        struct ASS_ERR_LIST /*@null@*/*next;
    };
    struct ASS_ERR_LIST /*@only@*//*@null@*/*ass_err_list = NULL;
    struct ASS_ERR_LIST **ass_err_list_next;
#endif

enum {
    LOCK_NONE = 0,
    LOCK_IF,
    LOCK_MACRO
};

#define MAX_LABEL    6
struct INSTRUCTION_LIST {
    char  name[6];      /* Nom de l'instruction */
    int   code;         /* Code d'instruction */
    int   cycles;       /* Base du nombre de cycles */
    int   has_operand;  /* Présence d'une opérande */
    void  /*@null@*/(*prog)(void);  /* Programme d'assemblage */
};

struct DIRECTIVE_LIST {
     char  name[7];     /* Nom de la directive */
     int   lock;        /* Si l'assemblage est forcé */
     int   has_operand; /* Présence d'une opérande */
     void  /*@null@*/(*prog)(void);  /* Programme d'assemblage */
};

static const struct INSTRUCTION_LIST instruction[] = {
     { "ABX"  , 0x003A,  3, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*  1*/
     { "ADCA" , 0x0089,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*  2*/
     { "ADCB" , 0x00C9,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*  3*/
     { "ADDA" , 0x008B,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*  4*/
     { "ADDB" , 0x00CB,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*  5*/
     { "ADDD" , 0x00C3,  6, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*  6*/
     { "ANDA" , 0x0084,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*  7*/
     { "ANDB" , 0x00C4,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*  8*/
     { "ANDCC", 0x001C,  3, ASSEMBLE_OPER_YES, immediat_Assemble   }, /*  9*/
     { "ASLA" , 0x0048,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 10*/
     { "ASLB" , 0x0058,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 11*/
     { "ASL"  , 0x0008,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /* 12*/
     { "ASRA" , 0x0047,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 13*/
     { "ASRB" , 0x0057,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 14*/
     { "ASR"  , 0x0007,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /* 15*/
     { "BITA" , 0x0085,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 16*/
     { "BITB" , 0x00C5,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 17*/
     { "BRA"  , 0x0020,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 18*/
     { "BRN"  , 0x0021,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 19*/
     { "BHI"  , 0x0022,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 20*/
     { "BLS"  , 0x0023,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 21*/
     { "BCC"  , 0x0024,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 22*/
     { "BHS"  , 0x0024,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 23*/
     { "BCS"  , 0x0025,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 24*/
     { "BLO"  , 0x0025,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 25*/
     { "BNE"  , 0x0026,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 26*/
     { "BEQ"  , 0x0027,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 27*/
     { "BVC"  , 0x0028,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 28*/
     { "BVS"  , 0x0029,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 29*/
     { "BPL"  , 0x002A,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 30*/
     { "BMI"  , 0x002B,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 31*/
     { "BGE"  , 0x002C,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 32*/
     { "BLT"  , 0x002D,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 33*/
     { "BGT"  , 0x002E,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 34*/
     { "BLE"  , 0x002F,  3, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 35*/
     { "BSR"  , 0x008D,  7, ASSEMBLE_OPER_YES, bra_Assemble        }, /* 36*/
     { "CLRA" , 0x004F,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 37*/
     { "CLRB" , 0x005F,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 38*/
     { "CLR"  , 0x000F,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /* 39*/
     { "CMPA" , 0x0081,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 40*/
     { "CMPB" , 0x00C1,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 41*/
     { "CMPD" , 0x1083,  7, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 42*/
     { "CMPS" , 0x118C,  7, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 43*/
     { "CMPU" , 0x1183,  7, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 44*/
     { "CMPX" , 0x008C,  6, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 45*/
     { "CMPY" , 0x108C,  7, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 46*/
     { "COMA" , 0x0043,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 47*/
     { "COMB" , 0x0053,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 48*/
     { "COM"  , 0x0003,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /* 49*/
     { "CWAI" , 0x003C, 20, ASSEMBLE_OPER_YES, immediat_Assemble   }, /* 50*/
     { "DAA"  , 0x0019,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 51*/
     { "DECA" , 0x004A,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 52*/
     { "DECB" , 0x005A,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 53*/
     { "DEC"  , 0x000A,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /* 54*/
     { "EORA" , 0x0088,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 55*/
     { "EORB" , 0x00C8,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 56*/
     { "EXG"  , 0x001E,  8, ASSEMBLE_OPER_YES, tfrexg_Assemble     }, /* 57*/
     { "INCA" , 0x004C,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 58*/
     { "INCB" , 0x005C,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 59*/
     { "INC"  , 0x000C,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /* 60*/
     { "JMP"  , 0x000E,  3, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /* 61*/
     { "JSR"  , 0x009D,  7, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 62*/
     { "LBRA" , 0x0016,  5, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 63*/
     { "LBRN" , 0x1021,  5, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 64*/
     { "LBHI" , 0x1022,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 65*/
     { "LBLS" , 0x1023,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 66*/
     { "LBCC" , 0x1024,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 67*/
     { "LBHS" , 0x1024,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 68*/
     { "LBCS" , 0x1025,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 69*/
     { "LBLO" , 0x1025,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 70*/
     { "LBNE" , 0x1026,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 71*/
     { "LBEQ" , 0x1027,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 72*/
     { "LBVC" , 0x1028,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 73*/
     { "LBVS" , 0x1029,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 74*/
     { "LBPL" , 0x102A,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 75*/
     { "LBMI" , 0x102B,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 76*/
     { "LBGE" , 0x102C,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 77*/
     { "LBLT" , 0x102D,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 78*/
     { "LBGT" , 0x102E,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 79*/
     { "LBLE" , 0x102F,  6, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 80*/
     { "LBSR" , 0x0017,  9, ASSEMBLE_OPER_YES, lbra_Assemble       }, /* 81*/
     { "LDA"  , 0x0086,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 82*/
     { "LDB"  , 0x00C6,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 83*/
     { "LDD"  , 0x00CC,  5, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 84*/
     { "LDS"  , 0x10CE,  6, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 85*/
     { "LDU"  , 0x00CE,  5, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 86*/
     { "LDX"  , 0x008E,  5, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 87*/
     { "LDY"  , 0x108E,  6, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /* 88*/
     { "LEAS" , 0x0032,  4, ASSEMBLE_OPER_YES, indexed_AssembleLea }, /* 89*/
     { "LEAU" , 0x0033,  4, ASSEMBLE_OPER_YES, indexed_AssembleLea }, /* 90*/
     { "LEAX" , 0x0030,  4, ASSEMBLE_OPER_YES, indexed_AssembleLea }, /* 91*/
     { "LEAY" , 0x0031,  4, ASSEMBLE_OPER_YES, indexed_AssembleLea }, /* 92*/
     { "LSLA" , 0x0048,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 93*/
     { "LSLB" , 0x0058,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 94*/
     { "LSL"  , 0x0008,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /* 95*/
     { "LSRA" , 0x0044,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 96*/
     { "LSRB" , 0x0054,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 97*/
     { "LSR"  , 0x0004,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /* 98*/
     { "MUL"  , 0x003D, 11, ASSEMBLE_OPER_NO , inherent_Assemble   }, /* 99*/
     { "NEGA" , 0x0040,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*100*/
     { "NEGB" , 0x0050,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*101*/
     { "NEG"  , 0x0000,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /*102*/
     { "NOP"  , 0x0012,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*103*/
     { "ORA"  , 0x008A,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*104*/
     { "ORB"  , 0x00CA,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*105*/
     { "ORCC" , 0x001A,  3, ASSEMBLE_OPER_YES, immediat_Assemble   }, /*106*/
     { "PSHS" , 0x0034,  5, ASSEMBLE_OPER_YES, pushpull_AssembleS  }, /*107*/
     { "PSHU" , 0x0036,  5, ASSEMBLE_OPER_YES, pushpull_AssembleU  }, /*108*/
     { "PULS" , 0x0035,  5, ASSEMBLE_OPER_YES, pushpull_AssembleS  }, /*109*/
     { "PULU" , 0x0037,  5, ASSEMBLE_OPER_YES, pushpull_AssembleU  }, /*110*/
     { "ROLA" , 0x0049,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*111*/
     { "ROLB" , 0x0059,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*112*/
     { "ROL"  , 0x0009,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /*113*/
     { "RORA" , 0x0046,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*114*/
     { "RORB" , 0x0056,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*115*/
     { "ROR"  , 0x0006,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /*116*/
     { "RTI"  , 0x003B, 15, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*117*/
     { "RTS"  , 0x0039,  5, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*118*/
     { "SBCA" , 0x0082,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*119*/
     { "SBCB" , 0x00C2,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*110*/
     { "SEX"  , 0x001D,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*121*/
     { "STA"  , 0x0097,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*122*/
     { "STB"  , 0x00D7,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*123*/
     { "STD"  , 0x00DD,  5, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*124*/
     { "STS"  , 0x10DF,  6, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*125*/
     { "STU"  , 0x00DF,  5, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*126*/
     { "STX"  , 0x009F,  5, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*127*/
     { "STY"  , 0x109F,  6, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*128*/
     { "SUBA" , 0x0080,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*129*/
     { "SUBB" , 0x00C0,  4, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*130*/
     { "SUBD" , 0x0083,  6, ASSEMBLE_OPER_YES, indexed_AssembleAll }, /*131*/
     { "SWI"  , 0x003F, 19, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*132*/
     { "SWI2" , 0x103F, 20, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*133*/
     { "SWI3" , 0x113F, 20, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*134*/
     { "SYNC" , 0x0013,  4, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*135*/
     { "TFR"  , 0x001F,  6, ASSEMBLE_OPER_YES, tfrexg_Assemble     }, /*136*/
     { "TSTA" , 0x004D,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*137*/
     { "TSTB" , 0x005D,  2, ASSEMBLE_OPER_NO , inherent_Assemble   }, /*138*/
     { "TST"  , 0x000D,  6, ASSEMBLE_OPER_YES, indexed_AssembleNot }, /*139*/
     { ""     , 0x0000,  0, ASSEMBLE_OPER_NO , NULL                }  /*Fin*/
};

static const struct DIRECTIVE_LIST directive[] = {
     { "CALL"  , LOCK_NONE , ASSEMBLE_OPER_YES, mo_AssembleCALL       }, /* 0*/
     { "ECHO"  , LOCK_NONE , ASSEMBLE_OPER_NO , print_AssembleECHO    }, /* 1*/
     { "ELSE"  , LOCK_IF   , ASSEMBLE_OPER_NO , if_AssembleELSE       }, /* 2*/
     { "END"   , LOCK_NONE , ASSEMBLE_OPER_NO , end_AssembleEND       }, /* 3*/
     { "ENDC"  , LOCK_IF   , ASSEMBLE_OPER_NO , if_AssembleENDC       }, /* 4*/
     { "ENDIF" , LOCK_IF   , ASSEMBLE_OPER_NO , if_AssembleENDIF      }, /* 5*/
     { "ENDM"  , LOCK_MACRO, ASSEMBLE_OPER_NO , macro_AssembleENDM    }, /* 6*/
     { "EQU"   , LOCK_NONE , ASSEMBLE_OPER_YES, setequ_AssembleEQU    }, /* 7*/
     { "FCB"   , LOCK_NONE , ASSEMBLE_OPER_YES, fcx_AssembleFCB       }, /* 8*/
     { "FCC"   , LOCK_NONE , ASSEMBLE_OPER_NO , fcx_AssembleFCC       }, /* 9*/
     { "FCN"   , LOCK_NONE , ASSEMBLE_OPER_NO , fcx_AssembleFCN       }, /*10*/
     { "FCS"   , LOCK_NONE , ASSEMBLE_OPER_NO , fcx_AssembleFCS       }, /*11*/
     { "FDB"   , LOCK_NONE , ASSEMBLE_OPER_YES, fcx_AssembleFDB       }, /*12*/
     { "GOTO"  , LOCK_NONE , ASSEMBLE_OPER_YES, mo_AssembleGOTO       }, /*13*/
     { "IF"    , LOCK_IF   , ASSEMBLE_OPER_YES, if_AssembleIF         }, /*14*/
     { "IFEQ"  , LOCK_IF   , ASSEMBLE_OPER_YES, if_AssembleIFEQ       }, /*15*/
     { "IFGE"  , LOCK_IF   , ASSEMBLE_OPER_YES, if_AssembleIFGE       }, /*16*/
     { "IFGT"  , LOCK_IF   , ASSEMBLE_OPER_YES, if_AssembleIFGT       }, /*17*/
     { "IFLE"  , LOCK_IF   , ASSEMBLE_OPER_YES, if_AssembleIFLE       }, /*18*/
     { "IFLT"  , LOCK_IF   , ASSEMBLE_OPER_YES, if_AssembleIFLT       }, /*19*/
     { "IFNE"  , LOCK_IF   , ASSEMBLE_OPER_YES, if_AssembleIFNE       }, /*20*/
     { "INCBIN", LOCK_NONE , ASSEMBLE_OPER_NO , includ_AssembleINCBIN }, /*21*/
     { "INCDAT", LOCK_NONE , ASSEMBLE_OPER_NO , includ_AssembleINCDAT }, /*22*/
     { "INCLUD", LOCK_NONE , ASSEMBLE_OPER_NO , includ_AssembleINCLUD }, /*23*/
     { "MACRO" , LOCK_MACRO, ASSEMBLE_OPER_NO , macro_AssembleMACRO   }, /*24*/
     { "OPT"   , LOCK_NONE , ASSEMBLE_OPER_YES, opt_AssembleOPT       }, /*25*/
     { "ORG"   , LOCK_NONE , ASSEMBLE_OPER_YES, org_AssembleORG       }, /*26*/
     { "PAGE"  , LOCK_NONE , ASSEMBLE_OPER_NO , page_AssemblePAGE     }, /*27*/
     { "PRINT" , LOCK_NONE , ASSEMBLE_OPER_NO , print_AssemblePRINT   }, /*28*/
     { "RMB"   , LOCK_NONE , ASSEMBLE_OPER_YES, rmx_AssembleRMB       }, /*29*/
     { "RMD"   , LOCK_NONE , ASSEMBLE_OPER_YES, rmx_AssembleRMD       }, /*30*/
     { "SET"   , LOCK_NONE , ASSEMBLE_OPER_YES, setequ_AssembleSET    }, /*31*/
     { "SETDP" , LOCK_NONE , ASSEMBLE_OPER_YES, setdp_AssembleSETDP   }, /*32*/
     { "STOP"  , LOCK_NONE , ASSEMBLE_OPER_NO , mo_AssembleSTOP       }, /*33*/
     { "TITLE" , LOCK_NONE , ASSEMBLE_OPER_NO , title_AssembleTITLE   }, /*34*/
     { ""      , LOCK_NONE , ASSEMBLE_OPER_NO , blank_Assemble        }, /*35*/
     { ""      , LOCK_NONE , ASSEMBLE_OPER_NO , NULL                  }  /*Fin*/
};

struct ASSEMBLE_STRUCT assemble;



#ifdef DO_PRINT
/* sub_ass_err_list:
 *  Libère un élément de la liste d'adresses.
 */
static void sub_ass_err_list (void)
{
    struct ASS_ERR_LIST /*@null@*/*next = NULL;

    if (ass_err_list != NULL)
    {
        next = ass_err_list->next;
        free (ass_err_list);
        ass_err_list = next;
    }
}



/* free_ass_err_list:
 *  Libère la liste d'adresses.
 */
static void free_ass_err_list (void)
{
    while (ass_err_list != NULL)
    {
        sub_ass_err_list ();
    }
}



/* add_ass_err_list:
 *  Ajoute une adresse dans la liste.
 */
static void add_ass_err_list (void)
{
    struct ASS_ERR_LIST *list = NULL;

    list = malloc (sizeof (struct ASS_ERR_LIST));
    if (list != NULL)
    {
        list->addr = org_Get ();
        list->next = NULL;
        if (ass_err_list == NULL)
        {
            ass_err_list_next = &ass_err_list;
        }
    
        *ass_err_list_next = list;
        ass_err_list_next = &list->next;
    }
}



/* display_assemble_error:
 *  Affiche les lignes d'assemblage.
 */
static void display_assemble_error (void)
{
    if (display_assemble_lines == 0)
    {
        switch (assemble.pass)
        {
            case ASSEMBLE_PASS1:
                add_ass_err_list ();
                break;

            case ASSEMBLE_PASS2:
                if (ass_err_list != NULL)
                {
                    if (org_Get () != ass_err_list->addr)
                    {
                        (void)printf (
                            "--- LOCATION ERROR: Line %d (%d) " \
                            "$%04x!=$%04x '%s'\n",
                            assemble.count, assemble.line,
                            (uint)org_Get () & 0xffff,
                            (uint)ass_err_list->addr & 0xffff,
                            assemble.buf);
                        free_ass_err_list ();  /* Seulement première erreur */
                    }

                    sub_ass_err_list ();                

                    if ((ass_err_list != NULL)
                     && (ass_err_list->next == NULL))
                    {
                        free_ass_err_list ();
                    }
                }
                break;
        }
    }
    else
    {
        (void)printf ("%8d %8d %04x '%s'\n",
            assemble.count, assemble.line, 
            (uint)org_Get () & 0xffff, assemble.buf);
        (void)fflush (stdout);
    }
}
#endif



/* init_assemble:
 *  Initialise l'assemblage.
 */
static void init_assemble (void)
{
    assemble.lock = 0;
    assemble.count = 0;
}



/* get_instruction_code:
 *  Récupère le code de l'instruction.
 */
static int get_instruction_code (char *name)
{
    int i;
    int code = -1;
    char *upper_name = arg_Upper (name);

    for (i = 0; (code == -1) && (instruction[i].prog != NULL); i++)
    {
        code = (strcmp (upper_name, instruction[i].name) == 0) ? i : code;
    }

    return code;
}



/* get_directive_code:
 *  Vérifie si le nom est une directive.
 */
static int get_directive_code (char *name)
{
    int i;
    int code = -1;
    char *upper_name = arg_Upper (name);

    for (i = 0; (code == -1) && (directive[i].prog != NULL); i++)
    {
        code = (strcmp (upper_name, directive[i].name) == 0) ? i : code;
    }

    return code;
}


    
/* execute_command:
 *  Execute l'instruction/directive/macro.
 */
static void execute_command (void)
{
    int i;

    if ((i = get_directive_code (assemble.command)) >= 0)
    {
        if (((directive[i].lock == LOCK_NONE) && (assemble.lock == 0))
         || ((directive[i].lock == LOCK_IF)
          && ((assemble.lock & ~ASSEMBLE_LOCK_IF) == 0))
         || ((directive[i].lock == LOCK_MACRO)
          && ((assemble.lock & ~ASSEMBLE_LOCK_MACRO) == 0)))
        {
            if (directive[i].prog != NULL)
            {
                (*directive[i].prog)();
            }
            else
            {
                (void)error_Internal (__FILE__, __LINE__, __func__);
            }
        }
    }
    else
    if (assemble.lock == 0)
    {
        if ((i = get_instruction_code (assemble.command)) >= 0)
        {
            assemble_Label (SYMBOL_TYPE_LABEL, org_Get());
            bin.buf[0] = (char)((uint)instruction[i].code >> 8);
            bin.buf[1] = (char)instruction[i].code;
            mark_AddCycle (instruction[i].cycles);
            if (instruction[i].prog != NULL)
            {
                (*instruction[i].prog)();
            }
            else
            {
                (void)error_Internal (__FILE__, __LINE__, __func__);
            }
        }
        else
        {
            macro_Call ();
        }
    }
}



/* assemble_code_line:
 *  Assemble la ligne de code.
 */
static void assemble_code_line (void)
{
    int style;

    /* Expanse la ligne de macro */
    macro_Expansion ();

    /* Lit l'étiquette */
    *assemble.label = '\0';
    *assemble.command = '\0';
    if (*assemble.ptr == '*')
    {
        arg_CaseOn ();
        style = ARG_STYLE_COMMENT;
        (void)arg_ReadField (&assemble.ptr, style);
        arg_CaseOff ();
    }
    else
    {
        style = ARG_STYLE_LABEL;
        if (arg_ReadField (&assemble.ptr, style) != ARG_END)
        {
            strncat (assemble.label, arg.str, MAX_STRING);
        }
    }

    arg_SkipSpaces (&assemble.ptr);

    /* Lit instruction/directive/macro */
    if ((style != ARG_STYLE_COMMENT)
     && (arg_IsEnd (*assemble.ptr) == 0))
    {
        if (*assemble.ptr == '*')
        {
            arg_CaseOn ();
            (void)arg_ReadField (&assemble.ptr, ARG_STYLE_COMMENT);
            arg_CaseOff ();
        }
        else
        {
            (void)arg_ReadField (&assemble.ptr, ARG_STYLE_NONE);
            style = (get_instruction_code (arg.str) >= 0)
                         ? ARG_STYLE_INSTRUCTION :
                    (get_directive_code (arg.str) >= 0)
                         ? ARG_STYLE_DIRECTIVE : ARG_STYLE_MACRO_CALL;
            arg_SetStyle (style);
            strncat (assemble.command, arg.str, MAX_STRING);
        }
    }

    arg_SkipSpaces (&assemble.ptr);

    /* Execute instruction/directive/macro */
    if (style != ARG_STYLE_COMMENT)
    {
        execute_command ();
    }
}



/* assemble_line:
 *  Assemble la ligne.
 */
static void assemble_line (void)
{
    switch (*assemble.ptr)
    {
        case '\0': break;

        case '(': if (assemble.lock == 0)
                  {
                      mark_Read ();
                  }
                  break;

        case '/': comment_Assemble (); break;

        default:  if ((assemble.lock & ASSEMBLE_LOCK_COMMENT) == 0)
                  {
                      assemble_code_line ();
                  }
                  break;
    }
}



/* new_extension_name:
 *  Renvoie une copie du nom de fichier avec une nouvelle extension.
 */
static char /*@dependent@*/*new_extension_name (char *name, char *extension)
{
    static char path_name[MAX_STRING+1];
    char *p = path_name;

    p[0] = '\0';
    strncat (p, name, MAX_STRING);
    if ((p = strrchr (path_name, '.')) == NULL)
    {
        p = path_name + strlen (path_name);
    }

    strcpy (p, extension);

    return path_name;
}



/* check_end:
 *  Génère les erreurs à la fin de la compilation.
 */
static void check_end (void)
{
    if ((end_IsEnd() != 0)
     || ((assemble.text != NULL) && (assemble.text->next == NULL)))
    {
        comment_EndError ();
        macro_EndError ();
        if_EndError ();
    }
}



/* open_line_pas:
 *  Ouvre une passe pour une ligne.
 */
static void start_line_pass (void)
{
    mark_Reset ();
    error_Clear ();
    bin_Clear ();
    symbol_Clear ();
    rmx_SetSize (0);
}



/* close_line_pas:
 *  Ferme une passe pour une ligne.
 */
static void end_line_pass (void)
{
    arg_ReadClose ();
    bin_Flush ();
    error_Print ();
    output_PrintCode ();
    bin_MoveOrg ();
    arg_Close ();
}



/* assemble_pass:
 *  Effectue un pass d'assemblage.
 */
static int assemble_pass (char *fass, char *fbin, char *pass)
{
    int err = 0;
    int stop = 0;

    /* Charge le fichier maître */
    err = includ_Load (fass);
    if (err == 0)
    {
        lst_Open (new_extension_name (fass, ".lst"));
        htm_Open (new_extension_name (fass, ".html"), fass);
        bin_Open ((fbin[0] == '\0') ? new_extension_name (fass, ".BIN") : fbin);
        thm_PrintSource (pass);

        arg_Init ();
        init_assemble ();
        end_Init ();
        org_Set (0x0000);
        setdp_Set (0x00);
        opt_Reset ();
        if_Init ();
        mark_Init ();
        macro_Init ();
        error_Clear ();

        /* Boucle d'assemblage */
        while ((end_IsEnd() == 0) && (stop == 0) && (error_IsFatal () == 0))
        {
            /* Ouvre la ligne */
            arg_Reset ();
            bin_Reset ();
            start_line_pass ();

            /* Charge la ligne et prévoit la suivante */
            assemble.buf[0] = '\0';
            assemble.ptr = assemble.buf;
            if ((assemble.text != NULL)
             && (assemble.text->str != NULL))
            {
                strncat (assemble.buf, assemble.text->str, MAX_STRING);
                if (assemble.text != NULL)
                {
                    /*@-onlytrans@*/
                    assemble.next_text = assemble.text->next;
                    /*@+onlytrans@*/
                }
                assemble.next_line = assemble.line + 1;
            }
#ifdef DO_PRINT
            display_assemble_error ();
#endif
            /* Assemble la ligne */
            output_SetCode (OUTPUT_CODE_COMMENT);
            assemble_line ();
            check_end ();

            /* Ferme la ligne */
            end_line_pass ();

            /* Ligne suivante */
            assemble.text = assemble.next_text;
            assemble.line = assemble.next_line;
            assemble.count += 1;
            stop = includ_Next ();  /* Retour d'include si nécessaire */
        }

        /* Erreur si pas END dans ASSEMBLER */
        if ((end_IsEnd() == 0)
         && (assemble.soft < ASSEMBLER_MACRO))
        {
            error_Assembler ((is_fr) ? "{-}directive END obligatoire"
                                     : "{-}END directive required");
        }

        if_Close ();
        macro_CloseCall ();
        error_Close ();
    }

    includ_Close ();

    return err;
}


/* ------------------------------------------------------------------------- */


/* assemble_Init:
 *  Initialise le module.
 */
void assemble_Init (void)
{
    assemble.pass = ASSEMBLE_SCAN;
    assemble_SetMacroAssembler ();
}



/* assemble_FlushDisplay:
 *  Flushe l'assemblage.
 */
void assemble_Flush (int style)
{
    output_SetCode (style);
    end_line_pass ();
    start_line_pass ();
}



/* assemble_Label:
 *  Enregistre l'étiquette.
 */
void assemble_Label (int type, int value)
{
    int i;
    int err = 0;
    int max = (assemble.soft < ASSEMBLER_MACRO) ? MAX_LABEL : MAX_ARG;
    struct SYMBOL_LIST *symbol;

    if (assemble.label[0] != '\0')
    {
        /* Erreur si caractère non autorisé */
        for (i=0; assemble.label[i] != '\0'; i++)
        {
            if (arg_IsAlnum (assemble.label[i]) == 0)
            {
                err = error_Error ((is_fr) ? "{l}etiquette incorrecte"
                                           : "{l}bad label");
            }
            
            if ((assemble.soft < ASSEMBLER_MACRO)
              && (assemble.label[i] == '_'))
            {
                error_Assembler ((is_fr) ? "{Ll}'_' non supporté"
                                         : "{Ll}'_' not supported");
                err = -1;
            }
        }

        /* Erreur si étiquette trop longue */
        if (i > max)
        {
            error_Assembler ((is_fr)
                ? "{Ll}étiquette trop longue (%d car. max)"
                : "{Ll}label too long (%d chars max)", max);
        }

        /* Erreur si nom réservé */
        if ((get_instruction_code (assemble.label) >= 0)
         || (get_directive_code (assemble.label) >= 0)
         || (arg_GetRegisterCode (assemble.label) >= 0))
        {
            (void)error_Error ((is_fr)?"{Ll}symbole réservé"
                                      :"{Ll}reserved symbol");
        }
        else
        /* Etiquette correcte */
        if (err == 0)
        {
            symbol = symbol_Do (assemble.label, value, type);
            if ((symbol != NULL)
             && (symbol->prm != NULL))
            {
                arg_SetLabelStyle (
                    (type == SYMBOL_TYPE_LABEL) ? ARG_STYLE_LABEL_NAME :
                    (type == SYMBOL_TYPE_EQU) ? ARG_STYLE_EQU_NAME :
                    (type == SYMBOL_TYPE_SET) ? ARG_STYLE_SET_NAME :
                    ARG_STYLE_MACRO_NAME);

                switch (symbol->prm->error)
                {
                    case SYMBOL_ERROR_NOT_DEFINED:
                        (void)error_Error ((is_fr)
                            ? "{Ll}symbole inconnu"
                            : "{Ll}unknown symbol", assemble.label);
                        break;

                    case SYMBOL_ERROR_MULTIPLY_DEFINED:
                        (void)error_Error ((is_fr)
                            ? "{Ll}définition multiple"
                            : "{Ll}multiple definition", assemble.label);
                        break;

                    case SYMBOL_ERROR_LONE:
                        error_Warning ((is_fr)
                            ? "{Ll}symbole unique"
                            : "{Ll}lone symbol", assemble.label);
                        break;
                }
            }
        }
    }
}



/* assemble_HasOperand:
 *  Retourne le flag pour l'existence de l'opérande (création ASM).
 */
int assemble_HasOperand (char *name)
{
    int i;
    int has_operand = -1;

    if ((i = get_instruction_code (name)) >= 0)
    {
        has_operand = instruction[i].has_operand;
    }
    else
    if ((i = get_directive_code (name)) >= 0)
    {
        has_operand = directive[i].has_operand;
    }

    return has_operand;
}



/* assemble_Do:
 *  Effectue tous les pass d'assemblage.
 */
void assemble_Do (char *fass, char *fbin)
{
    arg_SetCreateList (1);
    assemble.pass = ASSEMBLE_PASS1;
    if ((assemble_pass (fass, fbin, "Pass1") == 0)
     && (error_IsFatal () == 0))
    {
        assemble.pass = ASSEMBLE_PASS2;
        if ((assemble_pass (fass, fbin, "Pass2") == 0)
         && (error_IsFatal () == 0))
        {
            if (opt_GetUsr (OPT_WS) != 0)
            {
                symbol_DisplayList ();
            }

            arg_SetCreateList (0);
            asm_Create (fass);
        }
    }

    bin_WClose ();
    lst_Close ();
    htm_Close ();
    symbol_Close ();
    macro_Close ();
    source_Close ();
}



/* assemble_SetC6809:
 *  Demande une simulation d'assemblage pour C6809.
 */
void assemble_SetC6809 (void)
{
    assemble.soft = ASSEMBLER_C6809;
}



/* assemble_SetMacroAssembler:
 *  Demande une simulation d'assemblage pour MacroAssembler.
 */
void assemble_SetMacroAssembler (void)
{
    assemble.soft = ASSEMBLER_MACRO;
}



/* assemble_SetAssemblerTo:
 *  Demande une simulation d'assemblage pour Assembler sur TO.
 */
void assemble_SetAssemblerTo (void)
{
    assemble.soft = ASSEMBLER_TO;
}



/* assemble_SetAssemblerMo:
 *  Demande une simulation d'assemblage pour Assembler sur MO.
 */
void assemble_SetAssemblerMo (void)
{
    assemble.soft = ASSEMBLER_MO;
}

