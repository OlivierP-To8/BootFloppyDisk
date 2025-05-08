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
    #include <ctype.h>
#endif

#include "defs.h"
#include "arg.h"
#include "symbol.h"
#include "error.h"
#include "output.h"
#include "assemble.h"
#include "directiv/opt.h"
#include "directiv/org.h"
#include "directiv/setequ.h"
#include "directiv/macro.h"

enum {
    SYMBOL_ORDER_ALPHA = 0,
    SYMBOL_ORDER_ERROR,
    SYMBOL_ORDER_TYPE,
    SYMBOL_ORDER_TIMES
};

struct ERROR_TABLE {
    char  str_en[9];
    char  str_fr[9];
    int   type;
};

struct TYPE_TABLE {
    char  string[6];
    int   type;
    int   style;
    int   link;
};

static const struct ERROR_TABLE error_table[] = {
    { "Unknown" , "Inconnu" , SYMBOL_ERROR_NOT_DEFINED      },
    { "Multiply", "Multiple", SYMBOL_ERROR_MULTIPLY_DEFINED },
    { ""        , ""        , SYMBOL_ERROR_NONE             },
    { ""        , ""        , -1                            }
};

static const struct TYPE_TABLE type_table[] = {
    { "???"  , SYMBOL_TYPE_NONE , ARG_STYLE_NONE  , ARG_STYLE_NONE       },
    { "Arg"  , SYMBOL_TYPE_ARG  , ARG_STYLE_RED   , ARG_STYLE_ARG_LINK   },
    { "Set"  , SYMBOL_TYPE_SET  , ARG_STYLE_GREEN , ARG_STYLE_SET_LINK   },
    { "Equ"  , SYMBOL_TYPE_EQU  , ARG_STYLE_BLUE  , ARG_STYLE_EQU_LINK   },
    { "Label", SYMBOL_TYPE_LABEL, ARG_STYLE_NONE  , ARG_STYLE_LABEL_LINK },
    { "Macro", SYMBOL_TYPE_MACRO, ARG_STYLE_MAROON, ARG_STYLE_MACRO_LINK },
    { ""     , -1               , ARG_STYLE_NONE  , ARG_STYLE_NONE       }
};

static int symbol_order = SYMBOL_ORDER_ALPHA;
static struct SYMBOL_LIST /*@only@*//*@null@*/*symbol_list = NULL;
static int symbol_lone = 0;
static int symbol_activated = 0;


/* create_symbol_prm:
 *  Crée les paramètres d'un symbole.
 */
static struct SYMBOL_PRM /*@null@*/*create_symbol_prm (char *name)
{
    struct SYMBOL_PRM *prm;

    prm = malloc (sizeof(struct SYMBOL_PRM));
    if (prm != NULL)
    {
        prm->error = SYMBOL_ERROR_NOT_DEFINED;
        prm->line = -1;
        prm->changed = 0;
        prm->quiet = 0;
        prm->value = 0;
        prm->pass = ASSEMBLE_SCAN;
        prm->type = SYMBOL_TYPE_NONE;
        prm->times = 0;
        prm->name = arg_StrAlloc (name);
    }
    else
    {
        prm = NULL;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }

    return prm;
}



/* add_symbol:
 *  Crée un symbole.
 */
static void add_symbol (char *name)
{
    struct SYMBOL_LIST /*@null@*/*symbol = symbol_list;

    symbol_list = malloc (sizeof(struct SYMBOL_LIST));
    if (symbol_list != NULL)
    {
        symbol_list->next = symbol;
        symbol_list->prm = create_symbol_prm (name);
    }
    else
    {
        symbol_list = symbol;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }
}



/* find_symbol:
 *  Repère le symbole dans la liste.
 */
static struct SYMBOL_LIST /*@null@*/*find_symbol (
            struct SYMBOL_LIST /*@returned@*//*@null@*/*list, char *name)
{
    struct SYMBOL_LIST /*@null@*/*symbol = NULL;

    for (; (list != NULL) && (symbol == NULL); list = list->next)
    {
        symbol = ((list->prm != NULL)
               && (list->prm->name != NULL)
               && (strcmp (name, list->prm->name) == 0)) ? list : symbol;
    }

    return symbol;
}



/* get_symbol:
 *  Récupère le pointeur de symbole.
 */
static struct SYMBOL_LIST /*@dependent@*//*@null@*/*get_symbol (char *name)
{
    struct SYMBOL_LIST *symbol;

    symbol = symbol_list;
    symbol = find_symbol (symbol, name);
    if (symbol == NULL)
    {
        add_symbol (name);
        symbol = symbol_list;
        symbol = find_symbol (symbol, name);
    }

    return symbol;
}



/* swap_list:
 *  Restaure la liste dans l'ordre d'entrée.
 */
static struct SYMBOL_LIST /*@null@*/*swap_list (
                struct SYMBOL_LIST /*@returned@*//*@null@*/*old_list)
{
    struct SYMBOL_LIST /*@null@*/*list = old_list;
    struct SYMBOL_LIST /*@null@*/*next = NULL;
    struct SYMBOL_LIST /*@null@*/*new_next = NULL;

    while (list != NULL)
    {
        next = list->next;
        list->next = new_next;
        new_next = list;
        list = next;
    }

    return new_next;
}



static void exchange_name (struct SYMBOL_LIST *symbol1,
                           struct SYMBOL_LIST *symbol2)
{
    struct SYMBOL_PRM *prm;

    prm = symbol1->prm;
    symbol1->prm = symbol2->prm;
    symbol2->prm = prm;
}



/* alphabetical_order:
 *  Met la liste des symboles dans l'ordre alphabétique (casse insensible)
 */
static void alphabetical_order (struct SYMBOL_LIST *symbol)
{
    struct SYMBOL_LIST *symbol1;
    struct SYMBOL_LIST *symbol2;
    char upper1[MAX_STRING];
    char upper2[MAX_STRING];

    symbol1 = symbol;
    while (symbol1 != NULL)
    {
        symbol2 = symbol1;
        while (symbol2 != NULL)
        {
            if ((symbol2 != symbol1)
             && (symbol1->prm != NULL)
             && (symbol2->prm != NULL)
             && (symbol1->prm->name != NULL)
             && (symbol2->prm->name != NULL))
            {
                strcpy (upper1, arg_Upper (symbol1->prm->name));
                strcpy (upper2, arg_Upper (symbol2->prm->name));
                if (strcmp (upper1, upper2) > 0)
                {
                    exchange_name (symbol1, symbol2);                
                }
            }
            symbol2 = symbol2->next;
        }
        symbol1 = symbol1->next;
    }
}



/* print_count_message:
 *  Affiche un message de compte.
 */
static int print_count_message (int pos, int count, char *message)
{
    int len;
    char str[MAX_STRING];
    char never_message[2][7]   = { "never"  , "jamais"  };
    char times_message[2][6]   = { "times"  , "fois"    };

    len = pos + 8 + snprintf (str, MAX_STRING, "%s %s ",
                             message , times_message[(is_fr)?1:0]);
    if (count <= 0)
    {
        (void)snprintf (str, MAX_STRING, "%s %s ", never_message[(is_fr)?1:0],
                                                   message);
        str[0] = arg_ToUpper (str[0]);
        output_SetStyle (ARG_STYLE_GREY);
        pos += output_File ("%s ", str);
    }
    else
    {
        (void)snprintf (str, MAX_STRING, "%s", message);
        str[0] = arg_ToUpper (str[0]);
        output_SetStyle (ARG_STYLE_NONE);
        pos += output_File ("%s ", str);
        output_SetStyle (ARG_STYLE_VALUE);
        pos += output_File ("%d ", count);
        output_SetStyle (ARG_STYLE_NONE);
        pos += output_File ("%s ", times_message[(is_fr)?1:0]);
    }

    while (pos < len)
    {
        pos += output_File (" ");
    }

    return pos;
}



/* print_symbol:
 *  Affiche un symbole.
 */
static void print_symbol (struct SYMBOL_LIST *symbol)
{
    int error = 0;
    int type = 0;
    int pos = 0;
    char used_message[2][8]    = { "used"   , "utilisé" };
    char changed_message[2][8] = { "changed", "changé"  };

    if (symbol->prm != NULL)
    {
        while (symbol->prm->error != error_table[error].type)
        {
            error++;
        }

        while (symbol->prm->type != type_table[type].type)
        {
            type++;
        }

        output_SetStyle ((error_table[error].str_en[0] != '\0')
                          ? ARG_STYLE_ERROR : ARG_STYLE_NONE);;
        pos += output_File ("%8s ", (is_fr)?error_table[error].str_fr
                                           :error_table[error].str_en);
        pos = print_count_message (pos, symbol->prm->times,
                                     used_message[(is_fr)?1:0]);
        (void)print_count_message (pos, symbol->prm->changed,
                               changed_message[(is_fr)?1:0]);
        output_SetStyle (ARG_STYLE_NONE);
        (void)output_File ("%04X ", (int)symbol->prm->value&0xffff);
        output_SetStyle (type_table[type].style);
        (void)output_File ("%6s ", type_table[type].string);
        output_SetStyle (((symbol->prm->changed <= 1)
                       && (symbol->prm->quiet == 0))
                          ? type_table[type].link : ARG_STYLE_GREY);
        if (symbol->prm->name != NULL)
        {
            output_FileSource (symbol->prm->name);
        }
        output_FileCr ();
    }
}



/* symbols_by_alpha:
 *  Affiche la liste des symboles par ordre alphabétique.
 */
static void symbols_by_alpha (void)
{
    struct SYMBOL_LIST *symbol;

    for (symbol = symbol_list; symbol != NULL; symbol = symbol->next)
    {
        print_symbol (symbol);
    }
}



/* symbols_by_error:
 *  Affiche la liste des symboles par erreur.
 */
static void symbols_by_error (void)
{
    int i = 0;
    struct SYMBOL_LIST *symbol;
    
    for (i = 0; error_table[i].type >= 0; i++)
    {
        for (symbol = symbol_list; symbol != NULL; symbol = symbol->next)
        {
            if ((symbol->prm != NULL)
             && (symbol->prm->error == error_table[i].type))
            {
                print_symbol (symbol);
            }
        }
    }
}



/* symbols_by_type:
 *  Affiche la liste des symboles par type.
 */
static void symbols_by_type (void)
{
    int i = 0;
    struct SYMBOL_LIST *symbol;
    
    for (i = 0; type_table[i].type >= 0; i++)
    {
        for (symbol = symbol_list; symbol != NULL; symbol = symbol->next)
        {
            if ((symbol->prm != NULL)
             && (symbol->prm->type == type_table[i].type))
            {
                print_symbol (symbol);
            }
        }
    }
}



/* symbols_by_times:
 *  Affiche la liste des symboles par fréquence.
 */
static void symbols_by_times (void)
{
    struct SYMBOL_LIST *symbol;
    int times = 0;
    int nexttimes = 0;
    int prevtimes = 0;
    
    do
    {
        prevtimes = nexttimes;
        nexttimes = (int)((uint)-1>>1);

        for (symbol = symbol_list; symbol != NULL; symbol = symbol->next)
        {
            if (symbol->prm != NULL)
            {
                if ((symbol->prm->times > times)
                 && (symbol->prm->times < nexttimes))
                {
                    nexttimes = symbol->prm->times;
                }

                if (symbol->prm->times == times)
                {
                    print_symbol (symbol);
                }
            }
        }
        times = nexttimes;
    } while (nexttimes != prevtimes);
}


/* ------------------------------------------------------------------------- */


/* symbol_Close:
 *  Libère les ressources des symboles.
 */
void symbol_Close (void)
{
    struct SYMBOL_LIST *next;

    while (symbol_list != NULL)
    {
        next = symbol_list->next;
        if (symbol_list->prm != NULL)
        {
            if (symbol_list->prm->name != NULL)
            {
                free (symbol_list->prm->name);
            }
            free (symbol_list->prm);
        }
        free (symbol_list);
        symbol_list = next;
    }
}



/* symbol_Do:
 *  Gère l'enregistrement et la lecture des symboles
 *   1. Ajoute éventuellement le symbole à la liste si inexistant
 *   2. Met éventuellement le symbole à jour
 *   3. Renvoie le pointeur de symbole
 */
struct SYMBOL_LIST *symbol_Do (char *name, u16 value, int type)
{
    struct SYMBOL_LIST *symbol;
    int symbol_changed = 0;

    symbol = get_symbol (name);
    if ((symbol != NULL)
     && (symbol->prm != NULL))
    {
        if (type == SYMBOL_TYPE_NONE)  /* lecture */
        {
            if (assemble.pass == ASSEMBLE_PASS1)
            {
                symbol->prm->times++;
            }
            
            if (symbol->prm->type == SYMBOL_TYPE_ARG)
            {
                symbol->prm->pass = assemble.pass;
            }
                
            symbol_activated |= (symbol->prm->pass == assemble.pass) ? 0 : 1;
        }
        else  /* écriture */
        {
            symbol->prm->pass = assemble.pass;

            if (symbol->prm->type == SYMBOL_TYPE_NONE)
            {
                symbol->prm->line = assemble.count;
                symbol->prm->type = type;
                symbol->prm->quiet = macro_GetQuiet();
                symbol->prm->value = value;
                symbol->prm->error = SYMBOL_ERROR_NONE;
            }
            else
            if ((type == SYMBOL_TYPE_SET)
             && (symbol->prm->type == SYMBOL_TYPE_SET))
            {
                symbol->prm->line = assemble.count;
                symbol_changed = (symbol->prm->value != value) ? 1 : 0;
                symbol->prm->value = value;
                symbol->prm->quiet = macro_GetQuiet();
            }
            else
            if (symbol->prm->value != value)
            {
                symbol_changed = 1;
                symbol->prm->error = SYMBOL_ERROR_MULTIPLY_DEFINED;
            }

            if (assemble.pass == ASSEMBLE_PASS2)
            {
                symbol->prm->changed += symbol_changed;

                if ((symbol->prm->error == SYMBOL_ERROR_NONE)
                 && (symbol_lone != 0)
                 && (symbol->prm->times == 0))
                {
                    symbol->prm->error = SYMBOL_ERROR_LONE;
                }
            }
        }
    }

    return symbol;
}



/* symbol_Undefined:
 *  Retourne le flag de définition du symbole.
 */
int symbol_Undefined (void)
{
    return symbol_activated;
}



/* symbol_Clear:
 *  Initialise les symboles pour une ligne.
 */
void symbol_Clear (void)
{
    symbol_activated = 0;
}



/* symbol_GetLine:
 *  Renvoie lnuméro de ligne selon son type et son nom.
 */
int symbol_GetLine (int type, char *name)
{
    int line = -1;
    struct SYMBOL_LIST *list;

    for (list = symbol_list; (list != NULL) && (line < 0); list = list->next)
    {
        if ((list->prm != NULL)
         && (list->prm->type == type)
         && (list->prm->changed <= 1)
         && (list->prm->quiet == 0)
         && (list->prm->name != NULL)
         && (strcmp (list->prm->name, name) == 0))
        {
            line = list->prm->line;
        }
    }

    return line;
}



/* symbol_DisplayList:
 *  Affiche la liste des symboles.
 */
void symbol_DisplayList (void)
{
    int i = 0;
    struct SYMBOL_LIST *symbol;

    /* Affiche le compte total des symboles */
    for (symbol = symbol_list; symbol != NULL; symbol = symbol->next)
    {
        i++;
    }
    output_FileCr ();
    output_SetStyle (ARG_STYLE_BOLD);
    (void)output_File ("%d %s%s%s", i, (is_fr) ? "symbole" : "total symbol",
                                       (i > 1) ? "s" : "",
                                       (is_fr) ? " au total" : "");
    output_FileCr ();

    symbol_list = swap_list (symbol_list); /* Liste dans l'ordre d'entrée */
    if (symbol_list != NULL)
    {
        alphabetical_order (symbol_list); /* Dans l'ordre alphabétique */
    }

    switch (symbol_order)
    {
        case SYMBOL_ORDER_ALPHA: symbols_by_alpha (); break;
        case SYMBOL_ORDER_ERROR: symbols_by_error (); break;
        case SYMBOL_ORDER_TYPE : symbols_by_type () ; break;
        case SYMBOL_ORDER_TIMES: symbols_by_times (); break;
    }
    output_FileCr ();
}



/* symbol_SetNone:
 *  Oblitère l'affichage des symboles.
 */
void symbol_SetNone (void)
{
    opt_SetIni (OPT_WS, 0);
}



/* symbol_SetAlphaOrder:
 *  Demande l'affichage des symboles dans l'ordre alphabétique.
 */
void symbol_SetAlphaOrder (void)
{
    symbol_order = SYMBOL_ORDER_ALPHA;
    opt_SetIni (OPT_WS, 1);
}



/* symbol_SetErrorOrder:
 *  Demande l'affichage des symboles dans l'ordre des erreurs.
 */
void symbol_SetErrorOrder (void)
{
    symbol_order = SYMBOL_ORDER_ERROR;
    opt_SetIni (OPT_WS, 1);
}



/* symbol_SetTypeOrder:
 *  Demande l'affichage des symboles dans l'ordre des types.
 */
void symbol_SetTypeOrder (void)
{
    symbol_order = SYMBOL_ORDER_TYPE;
    opt_SetIni (OPT_WS, 1);
}



/* symbol_SetTimesOrder:
 *  Demande l'affichage des symboles dans l'ordre des fréquences.
 */
void symbol_SetTimesOrder (void)
{
    symbol_order = SYMBOL_ORDER_TIMES;
    opt_SetIni (OPT_WS, 1);
}



/* symbol_SetLone:
 *  Demande le repérage des symboles isolés.
 */
void symbol_SetLone (void)
{
    symbol_lone = 1;
}

