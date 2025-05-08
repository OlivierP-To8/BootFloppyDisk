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

 
#ifndef C6809_ASSEMBLE_H
#define C6809_ASSEMBLE_H 1

#define ASSEMBLE_LOCK_NONE      0
#define ASSEMBLE_LOCK_IF        (1<<1)
#define ASSEMBLE_LOCK_MACRO     (1<<2)
#define ASSEMBLE_LOCK_COMMENT   (1<<4)

enum {
    ASSEMBLE_OPER_NO,
    ASSEMBLE_OPER_YES
};

enum {
    ASSEMBLE_SCAN = 1,
    ASSEMBLE_PASS1,
    ASSEMBLE_PASS2
};

enum {
    ASSEMBLER_MO = 0,
    ASSEMBLER_TO,
    ASSEMBLER_MACRO,
    ASSEMBLER_C6809
};

struct ASSEMBLE_STRUCT {
    int    line;     /* Numéro de ligne courante du code */
    int    count;    /* Compte des lignes */
    int    next_line; /* Numéro de ligne suivante */
    char   buf[MAX_STRING+1]; /* Buffer de la ligne courante */
    char   label[MAX_STRING+1]; /* Buffer pour l'étiquette */
    char   command[MAX_STRING+1]; /* Buffer pour la commande */
    char   /*@dependent@*/*ptr;     /* Pointeur dans la ligne courante */
    struct SOURCE_TEXT /*@dependent@*//*@null@*/*text; /* Ptr sur texte */
    struct SOURCE_TEXT /*@dependent@*//*@null@*/*next_text; /* Texte suivant */
    struct SOURCE_LIST /*@dependent@*//*@null@*/*source; /* Ptr sur source */
    int    soft;     /* Assembleur Thomson utilisateur */
    int    pass;     /* Numéro de pass */
    int    lock;     /* Verrou d'assemblage */
};

extern struct ASSEMBLE_STRUCT assemble;

extern void assemble_Init (void);
extern void assemble_Do (char *file_ass, char *file_bin);
extern void assemble_Label (int type, u16 value);
extern void assemble_Flush (int style);
extern int  assemble_CheckEnd (void);
extern int  assemble_HasOperand (char *name);

extern void assemble_SetC6809 (void);
extern void assemble_SetMacroAssembler (void);
extern void assemble_SetAssemblerTo (void);
extern void assemble_SetAssemblerMo (void);

#endif

