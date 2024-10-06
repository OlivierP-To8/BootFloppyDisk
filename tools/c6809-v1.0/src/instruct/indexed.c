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
    #include <ctype.h>
#endif

#include "defs.h"
#include "assemble.h"
#include "error.h"
#include "output.h"
#include "arg.h"
#include "mark.h"
#include "eval.h"
#include "symbol.h"
#include "directiv/setdp.h"
#include "directiv/org.h"
#include "output/bin.h"

#define BIT_NO_MODE     0
#define BIT_IMMEDIATE   (1<<0)
#define BIT_LEA         (1<<1)
#define BIT_EXTENDED    (1<<2)
#define BIT_DIRECT      (1<<3)
#define BIT_INDIRECT    (1<<4)

#define SIZE_0_BIT    0
#define SIZE_5_BITS   5
#define SIZE_8_BITS   8
#define SIZE_16_BITS  16

static int immediate_size[16] = {
    SIZE_8_BITS,  /* x0 */
    SIZE_8_BITS,  /* x1 */
    SIZE_8_BITS,  /* x2 */
    SIZE_16_BITS, /* x3 */
    SIZE_8_BITS,  /* x4 */
    SIZE_8_BITS,  /* x5 */
    SIZE_8_BITS,  /* x6 */
    SIZE_0_BIT,   /* x7 */
    SIZE_8_BITS,  /* x8 */
    SIZE_8_BITS,  /* x9 */
    SIZE_8_BITS,  /* xa */
    SIZE_8_BITS,  /* xb */
    SIZE_16_BITS, /* xc */
    SIZE_0_BIT,   /* xd */
    SIZE_16_BITS, /* xe */
    SIZE_0_BIT    /* xf */
};



/* optimize_can_be_direct_mode:
 *  Helper pour erreur.
 */
static void optimize_can_be_direct_mode (int value)
{
    if ((value&0xff00) == (setdp_Get()*256))
    {
        error_Optimize ((is_fr)?"{a}mode direct possible (DP=$%02X)"
                               :"{a}could be direct mode (DP=$%02X)",
                                setdp_Get());
    }
}



/* error_8_bits_out_of_range:
 *  Helper pour erreur.
 */
static void error_8_bits_out_of_range (int value)
{
    if ((value < -128) || (value > 127))
    {
        (void)error_Error ((is_fr)?"{o}offset hors champ"
                                  :"{o}offset out of range");
    }
}



/* optimize_can_be_8_bits:
 *  Helper pour erreur.
 */
static void optimize_can_be_8_bits (int value)
{
    if ((value >= -128) && (value <= 127))
    {
        error_Optimize ((is_fr) ? "{a}offset 8 bits possible"
                                : "{a}could be 8 bits offset");
    }
}



/* adjust_post_code:
 *  Ajuste le post-code selon le mode d'adressage.
 */
static void adjust_post_code (int mode)
{
    if ((mode & BIT_LEA) == 0)
    {
        bin.buf[1] += ((mode & BIT_IMMEDIATE) != 0) ? '\x10' : '\x60';
    }
}



/* set_register_code:
 *  Actualise le code du registre.
 */
static void set_register_code (int rcode)
{
    bin.buf[2] |= (char)(((rcode & 0xff) - 1) * 0x20);
}



/* indexed_with_offset_and_pcr:
 *  Assemble un adressage indexé avec offset et PCR.
 */
static void indexed_with_offset_and_pcr (int value, int mode)
{
    int size;

    value -= org_Get() + 3 + ((bin.buf[0] == '\0') ? 0 : 1);
    size = (((symbol_Undefined () == 0)
         && ((mode & BIT_EXTENDED) == 0)
         && (value >= -128) && (value <= 127))
         || ((mode & BIT_DIRECT) != 0)) ? SIZE_8_BITS : SIZE_16_BITS;

    switch (size)
    {
        case SIZE_8_BITS :
            output_SetCode (OUTPUT_CODE_3_FOR_3);
            mark_AddPlus (1);
            bin.buf[2] = '\x8c';
            bin.buf[3] = (char)value;
            error_8_bits_out_of_range (value);
            break;

        case SIZE_16_BITS :
            optimize_can_be_8_bits (value);
            value--;
            output_SetCode (OUTPUT_CODE_3_FOR_4);
            mark_AddPlus (5);
            bin.buf[2] = '\x8d';
            bin.buf[3] = (char)((value&0xff00)/256);
            bin.buf[4] = (char)value;
            break;
    }
}



/* indexed_with_plus:
 *  Assemble un adressage indexé avec incrémentation positive.
 */
static void indexed_with_plus (int mode)
{
    mark_AddPlus (0);
    bin.buf[2] = '\x84';
    if (*assemble.ptr == '+')
    {
        arg_ReadChar (&assemble.ptr, ARG_STYLE_SIGN);
        mark_AddPlus (2);
        bin.buf[2] = '\x80';
        if (*assemble.ptr == '+')
        {
            arg_ReadChar (&assemble.ptr, ARG_STYLE_SIGN);
            mark_AddPlus (1);
            bin.buf[2] = '\x81';
        }
        else
        if ((mode & BIT_INDIRECT) != 0)
        {
            (void)error_Error ((is_fr) ? "{[a+}mode d'adressage incorrect"
                                       : "{[a+}bad addressing mode");
        }
    }
}



/* indexed_with_minus:
 *  Assemble un adressage indexé avec incrémentation négative.
 */
static void indexed_with_minus (int *rcode, int mode)
{
    int code;

    mark_AddPlus (2);
    bin.buf[2] = '\x82';
    if (*assemble.ptr == '-')
    {
        arg_ReadChar (&assemble.ptr, ARG_STYLE_SIGN);
        mark_AddPlus (1);
        bin.buf[2] = '\x83';
    }
    else
    if ((mode & BIT_INDIRECT) != 0)
    {
        (void)error_Error ((is_fr) ? "{[a+}mode d'adressage incorrect"
                                   : "{[a+}bad addressing mode");
    }


    code = arg_Read (&assemble.ptr, ARG_STYLE_REGISTER);
    switch (code)
    {
        case ARG_X:
        case ARG_Y:
        case ARG_U:
        case ARG_S:   *rcode = code; break;

        case ARG_END: (void)error_MissingRegister (); break;

        default:      (void)error_BadRegister (); break;
    }
}



/* indexed_without_offset:
 *  Assemble un adressage indexé sans offset.
 */
static void indexed_without_offset (int mode)
{
    int rcode = 0;
    int code;

    adjust_post_code (mode);
    (void)arg_Read (&assemble.ptr, ARG_STYLE_SEPARATOR); /* Passe virgule */
    code = arg_Read (&assemble.ptr, ARG_STYLE_REGISTER);

    switch (code)
    {
        case ARG_SIGN:
            if (*arg.str == '-')
            {
                indexed_with_minus (&rcode, mode);
            }
            else
            {
                (void)error_Error ((is_fr)?"{+}devrait être '-'"
                                          :"{+}should be '-'");
            }
            break;

        case ARG_X:
        case ARG_Y:
        case ARG_U:
        case ARG_S:   rcode = code; indexed_with_plus (mode); break;

        case ARG_END: (void)error_MissingRegister (); break;

        default:      (void)error_BadRegister (); break;
    }

    set_register_code (rcode);
}



/* indexed_with_register:
 *  Assemble un adressage indexé avec registre.
 */
static void indexed_with_register (int mode)
{
    int code;
    int rcode;

    adjust_post_code (mode);
    code = arg_Read (&assemble.ptr, ARG_STYLE_REGISTER);

    if (((mode & BIT_DIRECT) != 0)
     || ((mode & BIT_EXTENDED) != 0))
    {
        (void)error_BadAddressingMode ();
    }

    /* On est sûr qu'il s'agit de A, B ou D */
    bin.buf[2] = (code == ARG_A) ? '\x86' : (code == ARG_B) ? '\x85' : '\x8b';
    mark_AddPlus ((code == ARG_D) ? 4 : 1);

    (void)arg_Read (&assemble.ptr, ARG_STYLE_SEPARATOR);  /* Passe virgule */
    rcode = arg_Read (&assemble.ptr, ARG_STYLE_REGISTER);
    switch (rcode)
    {
        case ARG_X:
        case ARG_Y:
        case ARG_U:
        case ARG_S:   set_register_code (rcode); break;

        case ARG_END: (void)error_MissingRegister (); break;

        default:      (void)error_BadRegister (); break;
    }

    output_SetCode (OUTPUT_CODE_2_FOR_2);
}



/* immediate:
 *  Assemble un adressage immédiat.
 */
static void immediate (int mode)
{
    int value = 0;

    mark_AddCycle (-2);

    if ((mode & BIT_IMMEDIATE) == 0)
    {
        (void)error_BadAddressingMode ();
    }

    (void)eval_Do (&assemble.ptr, &value);

    switch (immediate_size [(int)bin.buf[1]&0xf])
    {
        case SIZE_8_BITS :
            output_SetCode (OUTPUT_CODE_2_FOR_2);
            bin.buf[2] = (char)value;
            if ((value < -256) || (value > 255))
            {
                (void)error_OperandOutOfRange ();
            }
            break;

        case SIZE_16_BITS :
            output_SetCode (OUTPUT_CODE_2_FOR_3);
            bin.buf[2] = (char)((value&0xff00)/256);
            bin.buf[3] = (char)value;
            break;

        default : (void)error_BadAddressingMode (); break;
    }
}



/* indexed_with_offset_and_register:
 *  Assemble un adressage indexé avec offset et registre.
 */
static void indexed_with_offset_and_register (int value, int mode, int rcode)
{
    int size = ((symbol_Undefined () == 0) && ((mode & BIT_EXTENDED) == 0))
           ? (((mode & BIT_INDIRECT) == 0)
             && (value >= -16) && (value <= 15)) ? SIZE_5_BITS :
             ((value >= -128) && (value <= 127)) ? SIZE_8_BITS : SIZE_16_BITS
           : ((mode & BIT_DIRECT) != 0) ? SIZE_8_BITS : SIZE_16_BITS;

    switch (size)
    {
        case SIZE_5_BITS:
            output_SetCode (OUTPUT_CODE_2_FOR_2);
            mark_AddPlus (1);
            bin.buf[2] = (char)(0x00 | (value&0x1f));
            break;

        case SIZE_8_BITS:
            output_SetCode (OUTPUT_CODE_3_FOR_3);
            mark_AddPlus (1);
            bin.buf[2] = '\x88';
            bin.buf[3] = (char)value;
            error_8_bits_out_of_range (value);
            break;

        case SIZE_16_BITS:
            output_SetCode (OUTPUT_CODE_3_FOR_4);
            mark_AddPlus (4);
            bin.buf[2] = '\x89';
            bin.buf[3] = (char)((value&0xff00)/256);
            bin.buf[4] = (char)value;
            break;
    }

    if ((value == 0)
     && (size > SIZE_0_BIT)
     && ((mode & BIT_INDIRECT) == 0))
    {
        error_Optimize ((is_fr) ? "{a}offset 0 bit possible"
                                : "{a}could be 0 bit offset");
    }
    else
    if ((value >= -16) && (value <= 15)
     && (size > SIZE_5_BITS)
     && ((mode & BIT_INDIRECT) == 0))
    {
        error_Optimize ((is_fr) ? "{a}offset 5 bits possible"
                                : "{a}could be 5 bits offset");
    }
    else
    if (size > SIZE_8_BITS)
    {
        optimize_can_be_8_bits (value);
    }

    set_register_code (rcode);
}



/* direct_or_extended:
 *  Assemble un adressage direct ou étendu.
 */
static void direct_or_extended (int value, int mode)
{
    int size = (((symbol_Undefined () == 0)
             && ((mode & BIT_INDIRECT) == 0)
             && ((mode & BIT_EXTENDED) == 0)
             && ((value&0xff00) == (setdp_Get()*256)))
             || ((mode & BIT_DIRECT) != 0)) ? SIZE_8_BITS : SIZE_16_BITS;

    switch (size)
    {
        case SIZE_8_BITS:
            output_SetCode (OUTPUT_CODE_2_FOR_2);
            bin.buf[2] = (char)value;
            if ((value&0xff00) != (setdp_Get()*256))
            {
                (void)error_BadDp ();
            }
            if ((mode & BIT_INDIRECT) != 0)
            {
                (void)error_BadAddressingMode ();
            }
            break;

        case SIZE_16_BITS:
            if ((mode & BIT_INDIRECT) == 0)
            {
                mark_AddCycle (1);
                output_SetCode (OUTPUT_CODE_2_FOR_3);
                bin.buf[1] |= ((mode & BIT_IMMEDIATE) != 0) ? '\x30' : '\x70';
                bin.buf[2] = (char)((value&0xff00)/256);
                bin.buf[3] = (char)value;
                optimize_can_be_direct_mode (value);
            }
            else
            {
                output_SetCode (OUTPUT_CODE_3_FOR_4);
                adjust_post_code (mode);
                mark_AddPlus (2);
                bin.buf[2] = '\x9f';
                bin.buf[3] = (char)((value&0xff00)/256);
                bin.buf[4] = (char)value;
            }
            break;
    }

    if ((mode & BIT_LEA) != 0)
    {
        (void)error_Error ((is_fr)?"{o}opérande incorrecte"
                                  :"{o}bad operand");
    }
}



/* with_offset_and_register:
 *  Assemble un adressage avec offset et registre.
 */
static void with_offset_and_register (int value, int mode)
{
    int rcode;

    adjust_post_code (mode);
    rcode = arg_Read (&assemble.ptr, ARG_STYLE_REGISTER);
    switch (rcode)
    {
        case ARG_PCR: indexed_with_offset_and_pcr (value, mode); break;

        case ARG_X:
        case ARG_Y:
        case ARG_U:
        case ARG_S: indexed_with_offset_and_register (value,mode,rcode); break;

        case ARG_END: (void)error_MissingRegister (); break;

        default: (void)error_BadRegister (); break;
    }
}



/* with_offset:
 *  Assemble un adressage avec offset.
 */
static void with_offset (int mode)
{
    int value = 0;

    (void)eval_Do (&assemble.ptr, &value);
    switch (*assemble.ptr)
    {
        case ']': direct_or_extended (value, mode); break;

        case ',': (void)arg_Read (&assemble.ptr, ARG_STYLE_SEPARATOR);
                  with_offset_and_register (value, mode);
                  break;

        default:  if (arg_Read (&assemble.ptr, ARG_STYLE_NONE) == ARG_END)
                  {
                      direct_or_extended (value, mode);
                  }
                  else
                  {
                      (void)error_Error ((is_fr)?"{+}erreur d'opérande"
                                                :"{+}operand error");
                  }
                  break;
    }
}



/* assemble_instruction:
 *  Assemble l'instruction (sauf immédiat).
 */
static void assemble_all_but_immediate (int mode)
{
    if (*assemble.ptr == '[') 
    {
        mode |= BIT_INDIRECT;
        (void)arg_Read (&assemble.ptr, ARG_STYLE_BRACKET_ON);
    }

    bin.buf[1] |= ((bin.buf[1] & '\x80') != '\x00') ? '\x10' : '\x00';

    /* Assemble selon le début d'opérande */
    if (((arg_ToUpper (*assemble.ptr) == 'A')
      || (arg_ToUpper (*assemble.ptr) == 'B')
      || (arg_ToUpper (*assemble.ptr) == 'D'))
     && (*(assemble.ptr+1) == ','))
    {
        indexed_with_register (mode);
    }
    else
    if (*assemble.ptr == ',')
    {
        indexed_without_offset (mode);
    }
    else
    {
        with_offset (mode);
    }

    /* Vérifie si mode indirect (']' attendu) */
    if ((mode & BIT_INDIRECT) != 0)
    {
        mark_AddPlus (3);
        bin.buf[2] |= '\x10';

        if (arg_Read (&assemble.ptr, ARG_STYLE_BRACKET_OFF) == ARG_END)
        {
            (void)error_Error ((is_fr)?"{[}']' manquant":"{[}missing ']'");
        }
        else
        if (*arg.str != ']')
        {
            (void)error_Error ((is_fr)?"{[}']' attendu" :"{[}']' expected");
        }
    }

    if (arg_Read (&assemble.ptr, ARG_STYLE_NONE) != ARG_END)
    {
        (void)error_Error ((is_fr)?"{+}fin d'opérande attendue"
                                  :"{+}end of operand expected");
    }
}



/* assemble_instruction:
 *  Assemble l'instruction.
 */
static void assemble_instruction (int mode)
{
    bin.buf[2] = '\x00';
    output_SetCode (OUTPUT_CODE_2_FOR_2);

    /* Sélectionne le type d'adressage */
    switch (*assemble.ptr)
    {
        case '#':
            (void)arg_Read (&assemble.ptr, ARG_STYLE_ADDRESSING);
            immediate (mode);
            break;

        case '<':
            (void)arg_Read (&assemble.ptr, ARG_STYLE_ADDRESSING);
            assemble_all_but_immediate (mode | BIT_DIRECT);
            break;

        case '>':
            (void)arg_Read (&assemble.ptr, ARG_STYLE_ADDRESSING);
            assemble_all_but_immediate (mode | BIT_EXTENDED);
            break;

        default:
            assemble_all_but_immediate (mode);
            break;
    }
}


/* ------------------------------------------------------------------------- */


/* indexed_AssembleAll:
 *  Assemble une instruction dans tous les adressages.
 */
void indexed_AssembleAll (void)
{
    assemble_instruction (BIT_IMMEDIATE);
}



/* indexed_AssembleNot:
 *  Assemble une instruction dans tous les adressages sauf immédiat.
 */
void indexed_AssembleNot (void)
{
    assemble_instruction (BIT_NO_MODE);
}



/* indexed_AssembleLea:
 *  Assemble une instruction de type LEAx.
 */
void indexed_AssembleLea (void)
{
    assemble_instruction (BIT_LEA);
}

