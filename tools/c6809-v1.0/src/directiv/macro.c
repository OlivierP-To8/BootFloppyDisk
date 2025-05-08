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

#ifndef SCAN_DEPEND
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
#endif

#include "defs.h"
#include "error.h"
#include "output.h"
#include "arg.h"
#include "symbol.h"
#include "assemble.h"
#include "directiv/if.h"
#include "directiv/macro.h"
#include "output/lst.h"

#define MACROCALL_MAX   10

struct MACRO_LIST {
     u16   id;
	 int   quiet; /* sam */
	 int   expanded_label;
     int   start;
     int   line;
     struct SOURCE_TEXT /*@dependent@*//*@null@*/*text;
     struct SOURCE_LIST /*@dependent@*//*@null@*/*source;
     struct MACRO_LIST /*@only@*//*@null@*/*next;
};

struct MACROCALL_ARG {
    int number;
    char  /*@only@*//*@null@*/*str;
    struct MACROCALL_ARG /*@only@*//*@null@*/*next;
};

struct MACROCALL_LIST {
    char /*@only@*//*@null@*/*name;
	int  quiet; /* sam */
    struct MACROCALL_ARG /*@only@*//*@null@*/*arg;
    int  line;
    struct SOURCE_TEXT /*@dependent@*//*@null@*/*text;
    struct SOURCE_LIST /*@dependent@*//*@null@*/*source;
    struct MACROCALL_LIST /*@only@*//*@null@*/*next;
};

static struct MACRO_LIST /*@only@*//*@null@*/*macro_list = NULL;
static struct MACROCALL_LIST /*@only@*//*@null@*/*macrocall_list = NULL;
static int macrocall_level = 0;
static u16 macro_id = 0;
static int macro_line = 0;
static int macro_quiet = 0;



/* get_pointer:
 *  Renvoie le pointeur sur la MACRO.
 */
struct MACRO_LIST /*@dependent@*//*@null@*/
    *get_pointer (struct MACRO_LIST /*@null@*/*list, u16 value)
{
    struct MACRO_LIST /*@dependent@*//*@null@*/*found = NULL;

    for (; (list != NULL) && (found == NULL); list = list->next)
    {
        found = (list->id == value) ? list : found;
    }

    return found;
}



/* add_macro:
 *  Ajoute une macro.
 */
static void add_macro (int quiet)
{
    struct MACRO_LIST *macro_list_old = macro_list;

    macro_list = malloc (sizeof (struct MACRO_LIST));
    if (macro_list != NULL)
    {
        macro_list->next = macro_list_old;
        macro_list->id = macro_id;
        macro_list->expanded_label = 0;
        macro_list->source = assemble.source;
        macro_list->start = assemble.line;
        macro_list->line = assemble.next_line;
        macro_list->text = assemble.next_text;
        macro_list->quiet = quiet;   /* sam */
    }
    else
    {
        macro_list = macro_list_old;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }
}



/* free_macro_call:
 *  Libère les ressources d'un appel de macro.
 */
static void free_macro_call (void)
{
    struct MACROCALL_LIST *macrocall_next;
    struct MACROCALL_ARG *next;

    if (macrocall_list != NULL)
    {
        macrocall_next = macrocall_list->next;

        if (macrocall_list->name != NULL)
        {
            free (macrocall_list->name);
        }

        while (macrocall_list->arg != NULL)
        {
            next = macrocall_list->arg->next;
            if (macrocall_list->arg->str != NULL)
            {
                free (macrocall_list->arg->str);
            }
            free (macrocall_list->arg);

            macrocall_list->arg = next;
        }

        free (macrocall_list);
        macrocall_list = macrocall_next;
        macrocall_level--;
    }
}



/* add_macro_call_arg:
 *  Ajoute un argument pour un appel de macro à la liste.
 */
static void add_macro_call_arg (
      struct MACROCALL_LIST /*@null@*/*list, int number, char *str)
{
    struct MACROCALL_ARG /*@null@*/*list_old;

    if (list != NULL)
    {
        list_old = list->arg;
        list->arg = malloc (sizeof (struct MACROCALL_ARG));
        if (list->arg != NULL)
        {
            list->arg->next = list_old;
            list->arg->number = number;
            list->arg->str = arg_StrAlloc (str);
        }
        else
        {
            list->arg = list_old;
            (void)error_Memory (__FILE__, __LINE__, __func__);
        }
    }
}



/* add_macro_call_arg_list:
 *  Ajoute la liste des arguments pour un appel de macro.
 */
static void add_macro_call_arg_list (struct MACROCALL_LIST *macrocall)
{
    int i;
    int err = 0;

    for (i=0; (err == 0)
           && (arg_ReadToken (&assemble.ptr, ARG_STYLE_NONE) != ARG_END); i++)
    {
        if (i < MACROCALL_MAX)
        {
            add_macro_call_arg (macrocall, i, arg.str);
        }
        else
        {
            err = error_Error ((is_fr) ?"{C}{c}trop d'arguments (max %d)"
                                       :"{C}{c}too many arguments (max %d)",
                                        MACROCALL_MAX);
        }

        if (arg_IsEnd (*assemble.ptr) == 0)
        {
            arg_ReadChar (&assemble.ptr, ARG_STYLE_SEPARATOR);
        }
    }
}



/* add_macro_call:
 *  Ajoute un appel de macro.
 */
static void add_macro_call (char *name)
{
    struct MACROCALL_LIST *macrocall_list_old = macrocall_list;

    macrocall_list = malloc (sizeof(struct MACROCALL_LIST));
    if (macrocall_list != NULL)
    {
        macrocall_list->next = macrocall_list_old;
        macrocall_list->source = assemble.source;
        macrocall_list->line = assemble.next_line;
        macrocall_list->text = assemble.next_text;
        macrocall_list->quiet = macro_quiet;
        macrocall_list->arg = NULL;
        macrocall_list->name = arg_StrAlloc (name);
        if (macrocall_list->name != NULL)
        {
            add_macro_call_arg_list (macrocall_list);
            macrocall_level++;
        }
    }
    else
    {
        macrocall_list = macrocall_list_old;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }
}



/* get_macro_arg_string_pointer:
 *  Renvoie le pointeur sur le texte d'argument de macro.
 */
static struct MACROCALL_ARG /*@dependent@*//*@null@*/
*get_macro_arg_string_pointer (
            struct MACROCALL_ARG /*@null@*/*macrocall_arg_list, int number)
{
    struct MACROCALL_ARG /*@dependent@*//*@null@*/*found = NULL;
    struct MACROCALL_ARG /*@null@*/*list = macrocall_arg_list;

    for (;(list != NULL) && (found == NULL); list = list->next)
    {
        found = (list->number == number) ? list : found;
    }

    return found;
}



/* get_macro_arg_string:
 *  Renvoie le pointeur sur la chaîne de l'argument ou NULL.
 */
static char /*@dependent@*//*@null@*/*get_macro_arg_string (int i)
{
    int   err = 0;
    int   istart = i;
    struct MACROCALL_LIST /*@null@*/*list = macrocall_list;
    struct MACROCALL_ARG /*@null@*/*found;
    char /*@null@*/*macro_arg_string = NULL;

    for (; (err == 0) && (list != NULL); list = list->next)
    {
        found = get_macro_arg_string_pointer (list->arg, i);
        if ((found != NULL)
         && (found->str != NULL))
        {
            macro_arg_string = found->str;
            err = (macro_arg_string[0] != '\\') ? 1 : 
                  ((i = (int)(macro_arg_string[1]-'0')&0xff) > 9) ?
                     error_Error ((is_fr)
                        ? "{+}{@%s}chiffre incorrect"
                        : "{+}{@%s}wrong decimal digit",
                         arg_FilteredChar (macro_arg_string[1])) : err;
        }
        else
        {
            err = error_Error ((is_fr)
                    ? "argument de macro #%d non trouvé"
                    : "macro argument #%d not found", istart);
        }
    }

    return macro_arg_string;
}



/* add_macro_call_entry:
 *  Ajoute une entrée pour les appels de macro.
 */
static void add_macro_call_entry (struct SYMBOL_LIST *symbol)
{
    struct MACRO_LIST /*@null@*/*macro = macro_list;

    if ((macrocall_level == 8)
     && (assemble.soft == ASSEMBLER_MACRO))
    {
        error_Assembler ((is_fr)?"{C}{c}8 appels récursifs maximum"
                                :"{C}{c}8 recursive calls maximum");
    }

    if (macrocall_level == 500)
    {
        (void)error_Fatal ((is_fr)?"{C}{c}trop d'appels récursifs (max 500)"
                                  :"{C}{c}too many recursive calls (max 500)");
    }
    else
    {
        add_macro_call (assemble.command);

        if ((error_IsFatal () == 0)
         && (symbol->prm != NULL))
        {
            macro = get_pointer (macro, symbol->prm->value);
            if (macro != NULL)
            {
                assemble.next_line = macro->line;
                assemble.next_text = macro->text;
                assemble.source = macro->source;
                macro_quiet = macro->quiet; /* sam */
	
                if (macro_quiet != 0)   /* sam */
                {
                    output_SetCode (OUTPUT_CODE_NONE);
                }
            }
            else
            {
                (void)error_Error ((is_fr)?"{C}{c}macro non trouvée"
                                          :"{C}{c}macro not found");
            }
        }
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


/* macro_AssembleMACRO:
 *  Assemble la directive MACRO.
 */
void macro_AssembleMACRO (void)
{
    int quiet = 0;

    allow ();
    macro_id++;
    macro_line = assemble.line;

    if (assemble.label[0] == '\0')
    {
        (void)error_Error ((is_fr)?"{C}{c}étiquette obligatoire"
                                  :"{C}{c}label required");
    }
    else
    {
        /* sam: QUIET argument */
        if (arg_Read (&assemble.ptr, ARG_STYLE_NONE) != ARG_END)
        {
            if (strcmp (arg_Upper (arg.str), "QUIET") == 0)
            {
                quiet = 1;
            }
            else
            if (*arg.str != '*')
            {
                (void)error_Error ((is_fr)?"{C}{c}option incorrecte"
                                          :"{C}{c}wrong option");
            }
        }

        if (assemble.pass == ASSEMBLE_PASS1)
        {
            add_macro (quiet);
        }

        if (error_IsFatal () == 0)
        {
            if ((if_GetLevel() > 1)
             && (assemble.soft == ASSEMBLER_MACRO))
            {
                error_Assembler ((is_fr)?"{C}{c}non supporté dans un IF"
                                        :"{C}{c}not supported inside IF");
            }

            if ((macrocall_level > 0)
             || ((assemble.lock & ASSEMBLE_LOCK_MACRO) != 0))
            {
                (void)error_Error ((is_fr)?"{C}{c}imbrication non supportée"
                                          :"{C}{c}embedding not supported");
            }
            else
            {
                assemble_Label (SYMBOL_TYPE_MACRO, macro_id);
            }
        }
    }

    assemble.lock |= ASSEMBLE_LOCK_MACRO;
}



/* macro_AssembleENDM:
 *  Assemble la directive ENDM.
 */
void macro_AssembleENDM (void)
{
    int err = 0;

    allow ();

    /* Dans exécution de MACRO */
    if (macrocall_level > 0)
    {
        err = -1;
        arg_SetCommandStyle (ARG_STYLE_MACRO_CALL, -1);
        if (macro_quiet != 0)
        {
            output_SetCode (OUTPUT_CODE_NONE);
        }
        if (macrocall_list != NULL)
        {
            assemble.next_line = macrocall_list->line;
            assemble.next_text = macrocall_list->text;
            assemble.source = macrocall_list->source;
            macro_quiet = macrocall_list->quiet; /* sam */
            free_macro_call ();
            err = 0;
        }
    }
    else
    {
        /* Dans scan de MACRO */
        err = ((assemble.lock & ASSEMBLE_LOCK_MACRO) == 0) ? -1 : 0;
    }

    if (err != 0)
    {
        (void)error_Error ((is_fr)?"{C}{c}MACRO manquant" 
                                  :"{C}{c}missing MACRO");
    }

    error_LabelNotSupported ();

    assemble.lock &= ~ASSEMBLE_LOCK_MACRO;
}



/* macro_Expansion:
 *  Expanse une macro.
 */
void macro_Expansion (void)
{
    int i;
    char str[MAX_STRING+1];
    char /*@null@*/*argstr;

    if (macrocall_level > 0)
    {
        str[0] = '\0';
        while (*assemble.ptr != '\0')
        {
            strncat (str, assemble.ptr, 1);
            if (*(assemble.ptr++) == '\\')
            {
                if ((i = (int)(*assemble.ptr - '0') & 0xff) <= 9)
                {
                    argstr = get_macro_arg_string (i);
                    if (argstr != NULL)
                    {
                        assemble.ptr += 1;
                        str[strlen (str) - 1] = '\0';
                        strcat (str, argstr);
                    }
                }
                else
                if (*assemble.ptr == '\0')
                {
                    (void)error_Error ((is_fr)?"{-}chiffre manquant"
                                              :"{-}missing digit");
                }
                else
                {
                    (void)error_Error ((is_fr)?"{+}chiffre incorrect"
                                              :"{+}wrong digit",
                                             arg_FilteredChar (*assemble.ptr));
                }
            }
        }
        assemble.buf[0] = '\0';
        strncat (assemble.buf, str, MAX_STRING);
        assemble.ptr = assemble.buf;
    }
}



/* macro_Call:
 *  Appel d'une macro.
 */
void macro_Call (void)
{
    struct SYMBOL_LIST *symbol;

    if (assemble.soft < ASSEMBLER_MACRO)
    {
        error_Assembler ((is_fr)?"{C}{c}appel de macro non supporté"
                                :"{C}{c}macro call not supported");
    }

    error_LabelNotSupported ();

    symbol = symbol_Do (assemble.command, 0, SYMBOL_TYPE_NONE);
    if ((symbol != NULL)
     && (symbol->prm != NULL))
    {
        if (symbol_Undefined () == 0)
        {
            if (symbol->prm->error == SYMBOL_ERROR_MULTIPLY_DEFINED)
            {
                (void)error_Error ((is_fr)?"{C}{c}symbole déjà utilisé"
                                          :"{C}{c}symbol already used");
            }
            else
            {
                arg_SetCommandStyle (ARG_STYLE_MACRO_HREF, symbol->prm->value);
                add_macro_call_entry (symbol);
            }
        }
        else
        {
            (void)error_Error ((is_fr)?"{C}{c}macro non définie"
                                      :"{C}{c}undefined macro");
        }
    }

    if (macro_quiet != 0)
    {
        output_SetCode ( OUTPUT_CODE_NONE);
    }
}



/* macro_GetQuiet:
 *  Retourne le flag de QUIET.
 */
int macro_GetQuiet (void)
{
    return macro_quiet;
}



/* macro_Init:
 *  Initialise les macros.
 */
void macro_Init (void)
{
    macro_quiet = 0;
    macrocall_level = 0;
    macro_id = 0;
}



/* macro_EndError:
 *  Génère une erreur à la fin d'un include.
 */
void macro_EndError (void)
{
    if ((assemble.lock & ASSEMBLE_LOCK_MACRO) != 0)
    {
        (void)error_Error ((is_fr)?"{@%d}ENDM manquant"
                                  :"{@%d}missing ENDM", macro_line);
    }
}



/* macro_CloseCall:
 *  Libère les ressources des appel de macros.
 */
void macro_CloseCall (void)
{
    while (macrocall_list != NULL)
    {
        free_macro_call ();
    }
}



/* macro_Close:
 *  Libère les ressources des macros.
 */
void macro_Close (void)
{
    struct MACRO_LIST *next_macro;

    while (macro_list != NULL)
    {
        next_macro = macro_list->next;
        macro_list->text = NULL;
        free (macro_list);
        macro_list = next_macro;
    }
}

