/*
 *  c6809 version 1.0.3
 *  copyright (c) 2025 François Mouret
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

#ifndef C6809_ARG_H
#define C6809_ARG_H 1

#define REGS_PCR     0x100
#define REGS_CCDPPC  0x200
#define REGS_ABD     0x400
#define REGS_XYUS    0x800
/* ISREG = Tout registre sauf PCR */
#define ISREG        (REGS_CCDPPC|REGS_ABD|REGS_XYUS)

/* Les registres sont dans l'ordre pour TFR/EXG */
#define ARG_D        (0x00+REGS_ABD)
#define ARG_X        (0x01+REGS_XYUS)
#define ARG_Y        (0x02+REGS_XYUS)
#define ARG_U        (0x03+REGS_XYUS)
#define ARG_S        (0x04+REGS_XYUS)
#define ARG_PC       (0x05+REGS_CCDPPC)
#define ARG_A        (0x08+REGS_ABD)
#define ARG_B        (0x09+REGS_ABD)
#define ARG_CC       (0x0a+REGS_CCDPPC)
#define ARG_DP       (0x0b+REGS_CCDPPC)
#define ARG_PCR      (0x0f+REGS_PCR)
#define ARG_END      0
#define ARG_NUMERIC  0x10
#define ARG_ALPHA    0x11
#define ARG_SIGN     0x12
#define ARG_OK       0x13

#define FILTERED_ARG_LENGTH   12

enum {
    ARG_STYLE_SPACE = 0,
    ARG_STYLE_LABEL,
    ARG_STYLE_LABEL_NAME,
    ARG_STYLE_SET_NAME,
    ARG_STYLE_EQU_NAME,
    ARG_STYLE_MACRO_NAME,
    ARG_STYLE_ADDRESSING,
    ARG_STYLE_DIVISION,
    ARG_STYLE_PARENTHESIS_ON,
    ARG_STYLE_PARENTHESIS_OFF,
    ARG_STYLE_BRACKET_ON,
    ARG_STYLE_BRACKET_OFF,
    ARG_STYLE_BOLD,
    ARG_STYLE_RED,
    ARG_STYLE_BLUE,
    ARG_STYLE_GREY,
    ARG_STYLE_GREEN,
    ARG_STYLE_MAROON,
    ARG_STYLE_COMMENT,
    ARG_STYLE_THOMSON,
    ARG_STYLE_ADDRESS,
    ARG_STYLE_CODE,
    ARG_STYLE_VALUE,
    ARG_STYLE_ARG_LINK,
    ARG_STYLE_LABEL_LINK,
    ARG_STYLE_EQU_LINK,
    ARG_STYLE_SET_LINK,
    ARG_STYLE_MACRO_LINK,
    ARG_STYLE_LABEL_HREF,
    ARG_STYLE_EQU_HREF,
    ARG_STYLE_SET_HREF,
    ARG_STYLE_REGISTER,
    ARG_STYLE_SEPARATOR,
    ARG_STYLE_SIGN,
    ARG_STYLE_SYMBOL,
    ARG_STYLE_NONE,
    ARG_STYLE_OPTIMIZE,
    ARG_STYLE_ASSEMBLER,
    ARG_STYLE_WARNING,
    ARG_STYLE_ERROR,
    ARG_STYLE_FATAL,
    ARG_STYLE_WRONG,
    ARG_STYLE_INSTRUCTION,
    ARG_STYLE_DIRECTIVE,
    ARG_STYLE_MACRO_CALL,
    ARG_STYLE_MACRO_HREF
};

struct ARG_LIST {
    char   /*@null@*/*str;  /* Chaîne de l'argument */
    int    style;      /* Style de l'argument */
    int    error;      /* Error sur l'argument */
    int    column;     /* Colonne de l'argument */
    u16    value;      /* Valeur de l'argument */
    struct ARG_LIST /*@only@*//*@null@*/*next;  /* Argument suivant */
};

struct ARG_STRUCT {
    char   str[MAX_STRING+1];  /* Chaîne de l'argument courant */
    int    style;      /* Style de l'argument courant */
    int    error;      /* Erreur sur l'argument courant */
    int    column;     /* Colonne de l'argument courant */
    u16    value;      /* Valeur de l'argument courant */
    int    size;       /* Taille de l'argument courant */
    struct ARG_LIST /*@only@*//*@null@*/*list;  /* Liste des arguments */
};

extern struct ARG_STRUCT arg;

extern void  arg_Init (void);
extern void  arg_Reset (void);
extern void  arg_CaseOn (void);
extern void  arg_CaseOff (void);
extern void  arg_Close (void);
extern void  arg_SkipSpaces (char **p);
extern char  /*@dependent@*/*arg_FilteredChar (char c);
extern int   arg_GetRegisterCode (char *argument);
extern int   arg_IsAlnum (char c);
extern int   arg_IsXDigit (char c);
extern char  arg_ToUpper (char c);
extern int   arg_IsEnd (char c);
extern char  /*@dependent@*/*arg_Upper (char *str);
extern int   arg_Read (char **p, int style);
extern void  arg_ReadChar (char **p, int style);
extern int   arg_ReadField (char **p, int style);
extern int   arg_ReadToken (char **p, int style);
extern int   arg_ReadSized (char **p, int size, int style);
extern void  arg_ReadClose (void);
extern void  arg_SetCreateList (int set);

extern void  arg_ClearError ();
extern int   arg_CommandError (void);
extern int   arg_LabelError (void);
extern int   arg_OperandError (void);
extern int   arg_ParenthesisError (void);
extern int   arg_BracketError (void);
extern int   arg_OffsetError (void);
extern int   arg_AddressingError (void);
extern int   arg_Error (void);
extern int   arg_DivisionError (void);
extern void  arg_SetCommandStyle (int style, u16 value);
extern void  arg_SetLabelStyle (int style);

extern void  arg_SetStyle (int style);

extern void  arg_RTrim (char *p);
extern char  /*@null@*/*arg_StrAlloc (char *str);

#endif

