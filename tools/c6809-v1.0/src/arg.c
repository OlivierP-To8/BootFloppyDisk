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
    #include <stdlib.h>
    #include <ctype.h>
#endif

#include "defs.h"
#include "arg.h"
#include "error.h"
#include "output.h"
#include "source.h"
#include "assemble.h"
#include "encode.h"

struct REGISTER_LIST {
    char name[4];  /* Nom du registre */
    int code;      /* Code de registre */
};

static const struct REGISTER_LIST register_list[12] = {
    {   "A", ARG_A   },
    {   "B", ARG_B   },
    {   "D", ARG_D   },
    {   "X", ARG_X   },
    {   "Y", ARG_Y   },
    {   "U", ARG_U   },
    {   "S", ARG_S   },
    {  "CC", ARG_CC  },
    {  "DP", ARG_DP  },
    {  "PC", ARG_PC  },
    { "PCR", ARG_PCR },
    {    "", 0       }
};

static int keep_case = 0;
static int create_list = 0;
struct ARG_STRUCT arg;



/* is_digit:
 *  Vérifie que le caractère est numérique.
 */
static int is_digit (char c)
{
    return ((c >= '0') && (c <= '9')) ? 1 : 0;
}



/* is_lower:
 *  Vérifie que le caractère est alphabétique minuscule.
 */
static int is_lower (char c)
{
    return ((c >= 'a') && (c <= 'z')) ? 1 : 0;
}



/* is_alpha:
 *  Vérifie que le caractère est alphabétique.
 */
static int is_alpha (char c)
{
    return (((c >= 'a') && (c <= 'z'))
         || ((c >= 'A') && (c <= 'Z'))
          || (c == '_')) ? 1 : 0;
}


/* reset_arg:
 *  Réinitialisation des registres.
 */
static void reset_arg (void)
{
    arg.size = 0;
    arg.value = -1;
    *arg.str = '\0';
    arg.error = 0;
}



/* create_arg:
 *  Crée l'argument dans la liste.
 */
static void create_arg (char *str, int style)
{
    struct ARG_LIST *arg_list_old = arg.list;

    arg.list = malloc (sizeof (struct ARG_LIST));
    if (arg.list != NULL)
    {
        arg.list->next = arg_list_old;
        arg.list->style = style;
        arg.list->error = 0;
        arg.list->column = arg.column;
        arg.list->value = arg.value;
        arg.list->str = arg_StrAlloc (str);
    }
    else
    {
        arg.list = arg_list_old;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }

    arg.column += arg.size;
}



/* add_arg:
 *  Ajoute l'argument.
 */
static void add_arg (char *str, int style)
{
    if (create_list != 0)
    {
        create_arg (str, style);
    }
}



/* read_char:
 *  Lit un caractère.
 */
static void read_char (char **p)
{
    if ((assemble.soft < ASSEMBLER_MACRO)
     && (keep_case == 0)
     && (is_alpha (**p) != 0))
    {
        **p = arg_ToUpper (**p);
    }

    if (strlen (arg.str) < (MAX_STRING - 4))
    {
        arg.size += 1;
        strcat (arg.str, encode_GetChar ((assemble.source == NULL)
                                              ? ENCODING_UTF8
                                              : assemble.source->encoding, p));
    }
    else
    {
        (*p)++;
    }
}



/* highlight_error:
 *  Active l'affichage de l'erreur.
 */
static int highlight_error (int style)
{
    int count = 0;
    int column = 0;
    struct ARG_LIST *list;

    for (list = arg.list; (list != NULL) && (count >= 0); list = list->next)
    {
        count += (list->style == ARG_STYLE_PARENTHESIS_ON) ? -1 :
                 (list->style == ARG_STYLE_PARENTHESIS_OFF) ? 1 : 0;

        if ((list->style == ARG_STYLE_SPACE)
         || ((count == 0) && (list->style == style)))
        {
            count = -1;
        }
        else
        {
            list->error = 1;
            column = list->column;
        }
    }

    return column;
}



/* swap_list:
 *  Restaure la liste dans l'ordre d'entrée.
 */
static struct ARG_LIST /*@null@*/*swap_list (
                struct ARG_LIST /*@returned@*//*@null@*/*old_list)
{
    struct ARG_LIST /*@null@*/*list = old_list;
    struct ARG_LIST /*@null@*/*next = NULL;
    struct ARG_LIST /*@null@*/*new_next = NULL;

    while (list != NULL)
    {
        next = list->next;
        list->next = new_next;
        new_next = list;
        list = next;
    }

    return new_next;
}


/* ------------------------------------------------------------------------- */


/* arg_IsAlnum:
 *  Vérifie que le caractère est alphabétique ou numérique.
 */
int arg_IsAlnum (char c)
{
    return ((is_alpha (c) != 0)
         || (is_digit (c) != 0)) ? 1 : 0;
}



/* arg_IsXDigit:
 *  Vérifie que le caractère est un chiffre hexadécimal.
 */
int arg_IsXDigit (char c)
{
    return (((c >= 'a') && (c <= 'f'))
         || ((c >= 'A') && (c <= 'F'))
         || (is_digit (c) != 0)) ? 1 : 0;
}



/* arg_ToUpper:
 *  Passe une minuscule en majuscule.
 */
char arg_ToUpper (char c)
{
    return (is_lower (c) != 0) ? c - '\x20' : c;
}



/* arg_RTrim:
 *  Elimine les caractères de contrôle de fin de chaîne.
 */
void arg_RTrim (char *p)
{
    int i = (int)strlen (p);

    while ((--i >= 0)
        && (p[i] >= '\0')
        && (p[i] <= ' '))
    {
        p[i] = '\0';
    }
}



/* arg_StrAlloc:
 *  Alloue une chaîne de caractères.
 */
char /*@null@*/*arg_StrAlloc (char *str)
{
    char *string;

    string = malloc (strlen (str) + 1);
    if (string != NULL)
    {
        string[0] = '\0';
        strcat (string, str);
    }
    else
    {
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }

    return string;
}



/* arg_ClearError:
 *  Efface les erreurs.
 */
void arg_ClearError (void)
{
    struct ARG_LIST /*@null@*/*list;

    for (list = arg.list; list != NULL; list = list->next)
    {
        list->error = 0;
    }
}



/* arg_DivisionError:
 *  Active le highlight pour une erreur de division.
 */
int arg_DivisionError (void)
{
    return highlight_error (ARG_STYLE_DIVISION);
}



/* arg_AddressingError:
 *  Active le highlight pour une erreur de mode d'adressage.
 */
int arg_AddressingError (void)
{
    int column = 0;
    struct ARG_LIST *list;

    for (list = arg.list;
         (list != NULL) && (list->style >= ARG_STYLE_ADDRESSING);
         list = list->next)
    {
        if (list->style == ARG_STYLE_ADDRESSING)
        {
            list->error = 1;
            column = list->column;
        }
    }
    return column;
}



/* arg_OffsetError:
 *  Active le highlight pour une erreur de l'offset.
 */
int arg_OffsetError (void)
{
    int column = 0;
    struct ARG_LIST *list;

    for (list = arg.list;
         (list != NULL) && (list->style > ARG_STYLE_ADDRESSING);
         list = list->next)
    {
        if ((list->style != ARG_STYLE_SEPARATOR)
         && (list->style != ARG_STYLE_REGISTER))
        {
            list->error = 1;
            column = list->column;
        }
    }

    return column;
}



/* arg_CommandError:
 *  Active le highlight pour une erreur de l'instruction/directve/macro.
 */
int arg_CommandError (void)
{
    int column = 0;
    struct ARG_LIST *list;
    
    for (list = arg.list; list != NULL; list = list->next)
    {
        if (list->style >= ARG_STYLE_INSTRUCTION)
        {
            list->error = 1;
            column = list->column;
        }
    }

    return column;
}



/* arg_LabelError:
 *  Active le highlight pour une erreur de l'étiquette.
 */
int arg_LabelError (void)
{
    int column = 0;
    struct ARG_LIST *list;

    for (list = arg.list; list != NULL; list = list->next)
    {
        if ((list->style >= ARG_STYLE_LABEL)
         && (list->style <= ARG_STYLE_MACRO_NAME))
        {
            list->error = 1;
            column = list->column;
        }
    }

    return column;
}



/* arg_ParenthesisError:
 *  Active le highlight pour une erreur de parenthèse (eval).
 */
int arg_ParenthesisError (void)
{
    return highlight_error (ARG_STYLE_SPACE);
}



/* arg_BracketError:
 *  Active le highlight pour une erreur de crochet (eval).
 */
int arg_BracketError (void)
{
    int column = 0;
    struct ARG_LIST *list;

    for (list = arg.list;
         (list != NULL) && (list->style >= ARG_STYLE_ADDRESSING);
         list = list->next)
    {
        if ((list->style == ARG_STYLE_BRACKET_ON)
         || (list->style == ARG_STYLE_BRACKET_OFF))
        {
            list->error = 1;
            column = list->column;
        }
    }
    return column;
}



/* arg_OperandError:
 *  Active le highlight pour une erreur de l'opérande.
 */
int arg_OperandError (void)
{
    int end = 0;
    int column = 0;
    struct ARG_LIST *list;
    
    for (list = arg.list; (list != NULL) && (end == 0); list = list->next)
    {
        if ((list->style != ARG_STYLE_SPACE)
         && (list->style != ARG_STYLE_SEPARATOR))
        {
            list->error = 1;
            column = list->column;
        }
        else
        {
            end = 1;
        }
    }

    return column;
}



/* arg_SetCommandStyle:
 *  Définit un style et une valeur pour une commande.
 */
void arg_SetCommandStyle (int style, u16 value)
{
    struct ARG_LIST *list;

    for (list = arg.list; list != NULL; list = list->next)
    {
        if (list->style >= ARG_STYLE_INSTRUCTION)
        {
            list->style = style;
            list->value = value;
        }
    }
}



/* arg_SetLabelStyle:
 *  Définit un style pour l'étiquette.
 */
void arg_SetLabelStyle (int style)
{
    struct ARG_LIST *list;

    for (list = arg.list; list != NULL; list = list->next)
    {
        list->style = (list->style == ARG_STYLE_LABEL) ? style : list->style;
    }
}



/* arg_Upper:
 *  Renvoie une chaîne en majuscule.
 */
char *arg_Upper (char *p)
{
    int i;
    static char str[MAX_STRING+1];

    for (i=0; (i < MAX_STRING) && (p[i] != '\0'); i++)
    {
        str[i] = arg_ToUpper (p[i]);
    }
    str[i] = '\0';

    return str;
}



/* arg_FilteredChar:
 *  Prévient l'affichage de caractères étranges.
 */
char *arg_FilteredChar (char c)
{
    static char filtered[FILTERED_ARG_LENGTH+1];

    filtered[0] = '\0';

    if ((c > ' ') && (c < '\x7f'))
    {
        (void)snprintf (filtered, FILTERED_ARG_LENGTH,
                        "'%c'", (uchar)c);
    }
    else
    {
        (void)snprintf (filtered, FILTERED_ARG_LENGTH,
                        "'$%02x'", (uint)c & 0xff);
    }
    return filtered;
}



/* arg_GetRegisterCode:
 *  Vérifie si l'argument est un registre.
 */
int arg_GetRegisterCode (char *name)
{
    int i;
    int code = -1;
    char *upper;

    if ((int)strlen (name) < 4)
    {
        upper = arg_Upper (name);

        for (i=0; (register_list[i].name[0] != '\0') && (code == -1); i++)
        {
            code = (strcmp (upper, register_list[i].name) == 0) ? i : -1;
        }
    }
    return code;
}



/* arg_IsEnd:
 *  Vérifie la fin d'un champ.
 */
int arg_IsEnd (char c)
{
    return ((c >= '\0') && (c <= ' ')) ? 1 : 0;
}



/* arg_SkipSpaces:
 *  Passe les espaces.
 */
void arg_SkipSpaces (char **p)
{
    reset_arg ();
    if ((**p > '\0') && (**p <= ' '))
    {
        while ((**p > '\0') && (**p <= ' '))
        {
            read_char (p);
        }
        add_arg (arg.str, ARG_STYLE_SPACE);
    }
}



/* arg_ReadField:
 *  Lit un champ d'expression.
 *  Terminé par un code de contrôle, un espace ou 0.
 */
int arg_ReadField (char **p, int style)
{
    int code = ARG_END;

    reset_arg ();
    if (arg_IsEnd (**p) == 0)
    {
        while (arg_IsEnd (**p) == 0)
        {
            read_char (p);
        }
        add_arg (arg.str, style);
        code = ARG_OK;
    }

    return code;
}



/* arg_ReadToken:
 *  Lit une chaîne.
 *  Terminé par une virgule, un code de contrôle, un espace ou 0.
 */
int arg_ReadToken (char **p, int style)
{
    int code = ARG_END;

    reset_arg ();
    if (arg_IsEnd (**p) == 0)
    {
        while ((arg_IsEnd (**p) == 0) && (**p != ','))
        {
            read_char (p);
        }
        add_arg (arg.str, style);
        code = ARG_OK;
    }

    return code;
}



/* arg_ReadSized:
 *  Lit un argument avec taille.
 */
int arg_ReadSized (char **p, int size, int style)
{
    int code = ARG_END;

    reset_arg ();
    if (arg_IsEnd (**p) == 0)
    {
        while ((arg_IsEnd (**p) == 0) && (arg.size < size))
        {
            read_char (p);
        }
        add_arg (arg.str, style);
        code = ARG_OK;
    }

    return code;
}



/* arg_ReadChar:
 *  Lit un caractère.
 */
void arg_ReadChar (char **p, int style)
{
    reset_arg ();
    read_char (p);
    add_arg (arg.str, style);
}



/* arg_Read:
 *  Lit un argument et renvoie son code.
 */
int arg_Read (char **p, int style)
{
    int i = 0;
    int code = ARG_END;

    reset_arg ();
    if (arg_IsEnd (**p) == 0)
    {
        read_char (p);
        while ((i < MAX_STRING)
              && (arg_IsAlnum (**p) != 0)
              && (arg_IsAlnum (*((*p)-1)) != 0))
        {
            read_char (p);
        }

        code = ARG_ALPHA;
        if (is_alpha (*arg.str) != 0)  /* Si alphabétique */
        {
            i = arg_GetRegisterCode (arg.str);
            if (i >= 0)
            {
                code = register_list[i].code;
            }
        }
        else
        /* Si numérique ou signe */
        code = (is_digit (*arg.str) != 0) ? ARG_NUMERIC : ARG_SIGN;

        add_arg (arg.str, style);
    }

    return code;
}



/* arg_ReadClose:
 *  Clôt le parsing.
 */
void arg_ReadClose (void)
{
    /* Ferme le parsing (le reste est forcément du commentaire) */
    add_arg (assemble.ptr, ARG_STYLE_COMMENT);

    /* Restaure le parsing dans l'ordre d'entrée */
    arg.list = swap_list (arg.list);
}



/* arg_Close:
 *  Libère les ressources du parsing.
 */
void arg_Close (void)
{
    struct ARG_LIST *list = arg.list;
    struct ARG_LIST *next;
    
    while (list != NULL)
    {
        next = list->next;
        if (list->str != NULL)
        {
            free (list->str);
        }
        free (list);
        list = next;
    }
    arg.list = NULL;
}



/* arg_SetStyle:
 *  Définit le style de l'argument courant.
 */
void arg_SetStyle (int style)
{
    if (arg.list != NULL)
    {
        arg.list->style = style;
    }
}



/* arg_Error:
 *  Active l'erreur pour l'argument courant.
 */
int arg_Error (void)
{
    int column = 1; 

    if (arg.list != NULL)
    {
        arg.list->error = 1;
        column = arg.list->column;
    }

    return column;
}



/* arg_CaseOn:
 *  Conserve la casse pour les chaînes de caractères.
 */
void arg_CaseOn (void)
{
    keep_case = (keep_case * 2) + 1;
}



/* arg_CaseOff:
 *  Ne conserve pas la casse pour les chaînes de caractères.
 */
void arg_CaseOff (void)
{
    keep_case /= 2;
}



/* arg_Init:
 *  Initialise le parsing.
 */
void arg_Reset (void)
{
    arg.column = 1;
    reset_arg ();
}



/* arg_Init:
 *  Initialise le module.
 */
void arg_Init (void)
{
    keep_case = 0;
}



/* arg_SetCreateList:
 *  Autorise/Invalide la création d'une liste.
 */
void arg_SetCreateList (int set)
{
    create_list = set;
}

