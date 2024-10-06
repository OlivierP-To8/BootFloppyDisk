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
    #include <stdlib.h>
#endif

#include "defs.h"
#include "assemble.h"
#include "error.h"
#include "eval.h"
#include "symbol.h"

#if 0
#define DO_PRINT  1      /* if output wanted */
#endif

#define IF_LEVEL_MAX  16

enum {
    IF_IFNE = 0,
    IF_IFEQ,
    IF_IFGT,
    IF_IFLT,
    IF_IFGE,
    IF_IFLE
};
 
enum {
    IF_TRUE = 0,
    IF_TRUE_ELSE,
    IF_FALSE,
    IF_FALSE_ELSE,
    IF_STOP
};

struct IF_LIST {
    int status;
    struct IF_LIST /*@only@*//*@null@*/*next;
};

static struct IF_LIST /*@only@*//*@null@*/*if_list = NULL;
static int if_line = 0;

int if_level = 0;



/* optimize_can_be_8_bits:
 *  Helper pour erreur.
 */
static void error_multiple_definition (void)
{
    (void)error_Error ((is_fr) ? "{Cc@%d}définition multiple"
                               : "{Cc@%d}multiple definition", if_line);
}



/* update_lock:
 *  Met à jour le verrou d'assemblage.
 */
static void update_lock (void)
{
    if (if_list != NULL)
    {
        assemble.lock = ((if_list->status == IF_FALSE_ELSE)
                      || (if_list->status == IF_TRUE))
                        ? assemble.lock & ~ASSEMBLE_LOCK_IF
                        : assemble.lock | ASSEMBLE_LOCK_IF;

#ifdef DO_PRINT
        (void)printf ("%6d: %-4s (%-2d) %-3s %s\n%s",
            assemble.line,
            assemble.command,
            if_level,
            ((assemble.lock & ASSEMBLE_LOCK_IF) == 0) ? "on" : "off",
            (if_list->status == IF_TRUE) ? "IF_TRUE" :
            (if_list->status == IF_TRUE_ELSE) ? "IF_TRUE_ELSE" :
            (if_list->status == IF_FALSE) ? "IF_FALSE" :
            (if_list->status == IF_FALSE_ELSE) ? "IF_FALSE_ELSE" :
            (if_list->status == IF_STOP) ? "IF_STOP" : "UNKNOWN",
            (if_level == 1) ? "\n" : "");
#endif
    }
}



/* add_if:
 *  Ajoute un if.
 */
static void add_if (int status)
{
    struct IF_LIST *if_list_old = if_list;

    if_line = assemble.line;

    if_list = malloc (sizeof(struct IF_LIST));
    if (if_list != NULL)
    {
        if ((if_level == (IF_LEVEL_MAX+1))
         && (assemble.soft == ASSEMBLER_MACRO))
        {
            error_Assembler ((is_fr)?"{Cc}%d imbrications maximum"
                                    :"{Cc}%d embedding maximum", IF_LEVEL_MAX);
        }

        if_list->next = if_list_old;
        if_list->status = status;
        if_level++;
        update_lock ();
    }
    else
    {
        if_list = if_list_old;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }
}


static void set_status (int status)
{
    if ((if_list != NULL)
     && (if_level > 1))
    {
        if_list->status = status;
        update_lock ();
    }
}



/* free_if:
 *  Libère les ressources d'un if.
 */
static void free_if (void)
{
    struct IF_LIST *next;

    if (if_list != NULL)
    {
        next = if_list->next;
        free (if_list);
        if_list = next;
        if_level -= 1;
        update_lock ();
    }
}



/* assemble_if:
 *  Assemble un if.
 */
static void assemble_if (int type)
{
    int err;
    int value = 0;
    int status = IF_STOP;

    if ((assemble.lock & ASSEMBLE_LOCK_IF) == 0)
    {
        error_LabelNotSupported ();
        err = eval_Do (&assemble.ptr, &value);
        if (err == 0)
        {
            if (symbol_Undefined () == 0)
            {
                status =
                    (type == IF_IFNE) ? (value != 0) ? IF_TRUE : IF_FALSE :
                    (type == IF_IFEQ) ? (value == 0) ? IF_TRUE : IF_FALSE :
                    (type == IF_IFGT) ? (value >  0) ? IF_TRUE : IF_FALSE :
                    (type == IF_IFLT) ? (value <  0) ? IF_TRUE : IF_FALSE :
                    (type == IF_IFGE) ? (value >= 0) ? IF_TRUE : IF_FALSE :
                    (type == IF_IFLE) ? (value <= 0) ? IF_TRUE : IF_FALSE :
                     IF_STOP;
            }
            else
            {
                (void)error_Error ((is_fr)?"{C}symbole d'opérande inconnu"
                                          :"{C}unknown operand symbol");
            }
        }
    }

    add_if (status);
}



/* assemble_endx:
 *  Assemble un ENDC/ENDIF.
 */
static void assemble_endx (void)
{
    error_LabelNotSupported ();

    if (if_level > 1)
    {
        free_if ();
    }
    else
    {
        (void)error_Error ((is_fr)?"{-}IF manquant":"{-}missing IF");
    }
}



/* assemble_else:
 *  Assemble la directive ELSE.
 */
static void assemble_else (void)
{
    error_LabelNotSupported ();

    if ((if_list != NULL)
     && (if_level > 1))
    {
        switch (if_list->status)
        {
            case IF_TRUE : set_status (IF_TRUE_ELSE); break;
            case IF_FALSE: set_status (IF_FALSE_ELSE); break;
            case IF_TRUE_ELSE  : error_multiple_definition (); break;
            case IF_FALSE_ELSE : error_multiple_definition (); break;
        }
    }
    else
    {
        (void)error_Error ((is_fr) ? "{-}IF manquant" : "{-}missing IF");
    }
}



/* allow:
 *  Limite l'autorisation de l'usage.
 */
static void allow (void)
{
    error_DirectiveNotSupported ((assemble.soft < ASSEMBLER_MACRO) ? 1 : 0);
}


/* ------------------------------------------------------------------------- */


/* if_AssembleIF:
 *  Assemble la directive IF.
 */
void if_AssembleIF (void)
{
    allow ();
    assemble_if (IF_IFNE);
}



/* if_AssembleIFNE:
 *  Assemble la directive IFNE.
 */
void if_AssembleIFNE (void)
{
    allow ();
    assemble_if (IF_IFNE);
}



/* if_AssembleIFEQ:
 *  Assemble la directive IFEQ.
 */
void if_AssembleIFEQ (void)
{
    allow ();
    assemble_if (IF_IFEQ);
}



/* if_AssembleIFGT:
 *  Assemble la directive IFGT.
 */
void if_AssembleIFGT (void)
{
    allow ();
    assemble_if (IF_IFGT);
}



/* if_AssembleIFLT:
 *  Assemble la directive IFLT.
 */
void if_AssembleIFLT (void)
{
    allow ();
    assemble_if (IF_IFLT);
}



/* if_AssembleIFGE:
 *  Assemble la directive IFGE.
 */
void if_AssembleIFGE (void)
{
    allow ();
    assemble_if (IF_IFGE);
}



/* if_AssembleIFLE:
 *  Assemble la directive IFLE.
 */
void if_AssembleIFLE (void)
{
    allow ();
    assemble_if (IF_IFLE);
}



/* if_AssembleELSE:
 *  Assemble la directive ELSE.
 */
void if_AssembleELSE (void)
{
    allow ();
    assemble_else ();
}



/* if_AssembleENDC:
 *  Assemble la directive ENDC.
 */
void if_AssembleENDC (void)
{
    allow ();
    assemble_endx ();
}



/* if_AssembleENDIF:
 *  Assemble la directive ENDIF.
 */
void if_AssembleENDIF (void)
{
    allow ();
    assemble_endx ();
}



/* if_EndError:
 *  Génère une erreur à la fin d'un include.
 */
void if_EndError (void)
{
    if (if_level > 1)
    {
        (void)error_Error ((is_fr)?"{@%d}ENDC manquant"
                                  :"{@%d}missing ENDC", if_line);
    }
}



/* if_Close:
 *  Libère les ressources des IF.
 */
void if_Close (void)
{
    while (if_list != NULL)
    {
        free_if ();
    }
}



/* if_GetLevel:
 *  Retourne le degré de IF.
 */
int if_GetLevel (void)
{    
    return if_level;
}



/* if_Init:
 *  Initialise les IF.
 */
void if_Init (void)
{    
    if_level = 0;
    add_if (IF_TRUE);
    update_lock ();
}

