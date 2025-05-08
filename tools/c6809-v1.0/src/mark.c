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
    #include <string.h>
#endif

#include "defs.h"
#include "assemble.h"
#include "error.h"
#include "arg.h"
#include "output.h"
#include "eval.h"
#include "mark.h"

static int mark_cycle;  /* Nombre de cycles de base (0 si pas) */
static int mark_plus;   /* Nombre de cycles ajoutés (-1 si pas) */
static int mark_cycle_total;  /* Total du nombre de cycles */
static int mark_size_total;   /* Total du nombre d'octets */



/* clear_total:
 *  Initialise le marquage pour totaux.
 */
void clear_total (void)
{
    mark_cycle_total = 0;
    mark_size_total = 0;
}



/* info_mark:
 *  Traite le marquage info.
 */
static void info_mark (void)
{
    output_SetStyle (ARG_STYLE_BOLD);
    (void)output_File ("(info) %d,%d", mark_cycle_total, mark_size_total);
    (void)output_FileCr ();
    clear_total ();
    output_SetCode (OUTPUT_CODE_NONE);
}



/* check_mark_value:
 *  Traite le marquage check pour une valeur.
 */
static void check_mark_value (int check, int pos)
{
    u16 value = 0;
    char argstr[MAX_STRING+1];
    char *p;

    if ((arg_ReadToken (&assemble.ptr, ARG_STYLE_NONE) != ARG_END)
     && (*arg.str != '\0'))
    {
        (void)snprintf (argstr, MAX_STRING+1, "%s", arg.str);
        p = argstr;

        if ((eval_Do (&p, &value) == 0)
         && (value != 0)
         && (value != (u16)check))
        {
            output_SetStyle (ARG_STYLE_BOLD);
            (void)output_File ((pos == 0) ? "" : ",");
            pos += output_File (" %d!=%d", value, check);
        }
    }
}



/* check_mark:
 *  Traite le marquage check.
 */
static void check_mark (void)
{
    (void)output_File ("(check)");
    check_mark_value (mark_cycle_total, 0);
    arg_ReadChar (&assemble.ptr, ARG_STYLE_NONE);
    if (*arg.str == ',')
    {
        check_mark_value (mark_size_total, 1);
    }
    (void)output_FileCr ();
    clear_total ();
    output_SetCode (OUTPUT_CODE_NONE);
}


/* ------------------------------------------------------------------------- */


/* mark_IsMark:
 *  Vérifie un marquage.
 *  Retour:
 *    -1 = erreur
 *     0 = pas un marquage
 *    >0 = numéro du marquage
 */
int mark_IsMark (char **p)
{
    int ret = MARK_TYPE_NONE;

    arg_CaseOn ();
    if (**p == '(')
    {
        /* A partir de là, il s'agit d'un marquage */
        (void)arg_Read (p, ARG_STYLE_NONE);
        switch (arg_Read (p, ARG_STYLE_NONE))
        {
            case ARG_ALPHA:
                ret = (strcmp (arg.str,"info") == 0) ? MARK_TYPE_INFO
                    : (strcmp (arg.str,"check") == 0) ? MARK_TYPE_CHECK
                    : (strcmp (arg.str,"main") == 0) ? MARK_TYPE_MAIN
                    : (strcmp (arg.str,"include") == 0) ? MARK_TYPE_INCLUDE
                    : error_Error ((is_fr) ? "{+}marquage non reconnu"
                                           : "{+}unknown mark");
                (void)arg_Read (p, ARG_STYLE_NONE);
                if (*arg.str != ')')
                {
                    ret = error_Error (
                                (is_fr)?"{p}parenthèse droite manquante"
                                       :"{p}missing right bracket");
                }
                break;

            case ARG_END:
                ret = error_Error ((is_fr)?"{-}marquage manquant"
                                          :"{-}missing mark");
                break;
            
            default:
                ret = error_Error ((is_fr)?"{+}marquage incorrect"
                                          :"{+}wrong mark");
                break;
        }
    }
    arg_CaseOff ();

    return ret;
}



/* mark_Read:
 *  Lit un marquage.
 */
void mark_Read (void)
{
    if (assemble.pass == ASSEMBLE_PASS2)
    {
        switch (mark_IsMark (&assemble.ptr))
        {
            case MARK_TYPE_INFO : info_mark () ; break;
            case MARK_TYPE_CHECK: check_mark (); break;
        }
    }
}



/* mark_AddCycle:
 *  Ajoute un nombre de cycles.
 */
void mark_AddCycle (int cycles)
{
    mark_cycle_total += cycles;
    mark_cycle += cycles;
}



/* mark_AddPlus:
 *  Ajoute un nombre de cycles.
 */
void mark_AddPlus (int cycles)
{
    mark_cycle_total += cycles;
    mark_plus = ((mark_plus < 0) ? 0 : mark_plus) + cycles;
}



/* mark_AddSize:
 *  Ajoute un nombre de taille.
 */
void mark_AddSize (int size)
{
    mark_size_total += size;
}



/* mark_GetCycle:
 *  Retourne le nombre ponctuel de cycles.
 */
int mark_GetCycle (void)
{
    return mark_cycle;
}



/* mark_GetPlus:
 *  Retourne le nombre ponctuel de cycles ajoutés.
 */
int mark_GetPlus (void)
{
    return mark_plus;
}



/* mark_Reset:
 *  Initialise le marquage pour une ligne de code.
 */
void mark_Reset (void)
{
    mark_cycle = 0;
    mark_plus = -1;
}



/* mark_Init:
 *  Initialise le marquage pour tout le source.
 */
void mark_Init (void)
{
    mark_Reset ();
    clear_total ();
}




