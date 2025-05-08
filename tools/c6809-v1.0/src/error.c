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
    #include <stdarg.h>
    #include <errno.h>
#endif

#include "defs.h"
#include "error.h"
#include "arg.h"
#include "encode.h"
#include "assemble.h"
#include "source.h"
#include "output.h"
#include "directiv/opt.h"
#include "directiv/setdp.h"
#include "output/console.h"

enum {
    ERROR_TYPE_NONE = 0,
    ERROR_TYPE_OPTIMIZE,
    ERROR_TYPE_ASSEMBLER,
    ERROR_TYPE_WARNING,
    ERROR_TYPE_ERROR,
    ERROR_TYPE_FATAL
};

enum {
    ERROR_VERBOSE_MIN = 0,
    ERROR_VERBOSE_MID,
    ERROR_VERBOSE_MAX
};

struct ERROR_LIST {
    int  type;
    char /*@only@*//*@null@*/*string;
    struct ERROR_LIST /*@only@*//*@null@*/*next;
};

static int  error_type = ERROR_TYPE_NONE;
static char *error_name;
static char error_start [MAX_STRING+1];
static char error_string[MAX_STRING+1];
static char error_arg[MAX_STRING+1];
static char error_command[MAX_STRING+1];
static char error_label[MAX_STRING+1];
static char error_file[MAX_STRING+1];
static char error_line[MAX_STRING+1];
static char error_column[MAX_STRING+1];

static int verbose = ERROR_VERBOSE_MIN;

static struct ERROR_LIST /*@only@*//*@null@*/*error_list = NULL;



/* add_error:
 *  Ajoute une erreur dans la liste
 */
static void add_error (void)
{
    struct ERROR_LIST *error_old;

    error_old = error_list;
    error_list = malloc (sizeof (struct ERROR_LIST));
    if (error_list != NULL)
    {
        error_list->type = error_type;
        error_list->string = arg_StrAlloc (error_string);
        error_list->next = error_old;
    }
    else
    {
        error_list = error_old;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }
}



/* display_colon:
 *  Affiche un caractère ':'.
 */
static void display_colon (void)
{
    output_SetStyle (ARG_STYLE_NONE);
    output_Print (":");
}



/* display_error_start:
 *  Affiche le nom de fichier pour l'erreur.
 */
static void display_error_start (void)
{
    char error_c6809[] = "c6809";

    /*@-nullpass@*/
    (void)snprintf (error_start, MAX_STRING+1, "%s",
                  ((assemble.source != NULL) && (assemble.source->pc != NULL))
                      ? assemble.source->pc :
                  (error_file[0] != '\0') ? error_file : error_c6809);
    /*@+nullpass@*/
    output_Print (error_start);
    display_colon ();
}



/* display_error_pos:
 *  Affiche le numéro de ligne et de colonne pour l'erreur.
 */
static void display_error_pos (void)
{
    /* Numéro de ligne */
    if ((*error_line != '\0')
     && (strcmp (error_line, "0") != 0))
    {
        output_SetStyle (ARG_STYLE_BOLD);
        output_Print (error_line);
        display_colon ();

        if ((*error_column != '\0')
         && (strcmp (error_column, "0") != 0))
        {
            output_SetStyle (ARG_STYLE_BOLD);
            output_Print (error_column);
            display_colon ();
        }
    }
}



/* display_error_step:
 *  Affiche une chaîne avec un style pour l'erreur.
 */
static void display_error_step (int style, char *str)
{
    if ((str != NULL)
     && (*str != '\0'))
    {
        output_SetStyle (style);
        output_Print (" ");
        output_Print (str);
        display_colon ();
    }
}



/* display_error_file:
 *  Affiche un chemin de fichier pour l'erreur.
 */
static void display_error_file (void)
{
    if ((error_file[0] != '\0')
     && (strcmp (error_file, error_start) != 0))
    {
        display_error_step (ARG_STYLE_BOLD, error_file);
    }
}



/* display_error_string:
 *  Affiche le message pour l'erreur.
 */
static void display_error_string (void)
{
    output_SetStyle (ARG_STYLE_NONE);
    output_Print (" ");
    output_Print (error_string);
    output_PrintCr ();
}



/* display_error_verbose_max:
 *  Affichage du buffer de ligne pour la sortie console.
 */
static void display_error_verbose_max (void)
{
    if ((verbose == ERROR_VERBOSE_MAX)
     && (arg.list != NULL)
     && (error_IsFatal () == 0))
    {
        con_PrintSourceLine ();
        con_PrintCr ();
    }
}



/* display_error:
 *  Affiche l'erreur.
 */
static void display_error (void)
{
    int style = (error_type == ERROR_TYPE_OPTIMIZE) ? ARG_STYLE_OPTIMIZE :
                (error_type == ERROR_TYPE_ASSEMBLER) ? ARG_STYLE_ASSEMBLER :
                (error_type == ERROR_TYPE_WARNING) ? ARG_STYLE_WARNING :
                (error_type == ERROR_TYPE_ERROR) ? ARG_STYLE_ERROR :
                (error_type == ERROR_TYPE_FATAL) ? ARG_STYLE_FATAL :
                ARG_STYLE_NONE;

    if (style != ARG_STYLE_NONE)
    {
        display_error_start ();
        display_error_pos ();
        display_error_step (style, error_name);
        display_error_step (ARG_STYLE_BOLD, error_command);
        display_error_step (ARG_STYLE_BOLD, error_label);
        display_error_step (ARG_STYLE_BOLD, error_arg);
        display_error_file ();
        display_error_string ();
        display_error_verbose_max ();
    }
}



/* set_known:
 *  Définit un texte connu.
 */
static void set_known (char *in, char *out)
{
    char str[MAX_STRING-6];

    (void)snprintf (str, MAX_STRING-6, "%s", in);
    str[20] = '\0';

    if (strlen (in) > strlen (str))
    {
        (void)snprintf (out, MAX_STRING, "%s[...]", str);
    }
    else
    {
        (void)snprintf (out, MAX_STRING, "%s", in);
    }
}



/* set_text:
 *  Définit un texte spécial.
 */

static char *set_text (char /*@returned@*/*in, char *out)
{
    int i;

    for (i = 0; (*in != '}') && (*in != '\0'); i++)
    {
         out[i] = *(in++);
    }
    out[i] = '\0';

    return in;
}



/* set_error:
 *  Prépare l'erreur.
 */
static void set_error (int type, char *name, const char *format, va_list *args)
{
    char *p;
    int column = 0;
    static char str[MAX_STRING+1];

    if (type > error_type)
    {
        error_type = type;
        error_name = name;

        error_label[0] = '\0';
        error_command[0] = '\0';
        error_start[0] = '\0';
        error_file[0] = '\0';
        error_arg[0] = '\0';
        arg_ClearError ();

        (void)vsnprintf (str, MAX_STRING, format, *args);
        p = str;
        while (*p == '{')
        {
            switch (*(++p))
            {
                case '@': p = set_text (p+1, error_arg); break;
                case '>': p = set_text (p+1, error_file); break;
                case 'C': set_known (assemble.command,error_command); break;
                case 'L': set_known (assemble.label, error_label); break;
                case '+': column = arg_Error (); break;
                case 'c': column = arg_CommandError (); break;
                case 'l': column = arg_LabelError (); break;
                case 'o': column = arg_OperandError (); break;
                case '[': column = arg_BracketError (); break;
                case 'p': column = arg_ParenthesisError (); break;
                case '/': column = arg_DivisionError (); break;
                case 'f': column = arg_OffsetError (); break;
                case 'a': column = arg_AddressingError (); break;
                default : column = arg.column; break;
            }
            p += (*p != '\0') ? 1 : 0;
            p += (*p == '}') ? 1 : 0;
        }

        (void)snprintf (error_line, MAX_STRING, "%d", assemble.line);
        (void)snprintf (error_column, MAX_STRING, "%d", column);
        (void)snprintf (error_string, MAX_STRING+1, "%s", p);
    }
}


/* ------------------------------------------------------------------------- */


/* error_Close:
 *  Libère les ressources des erreurs.
 */
void error_Close (void)
{
    struct ERROR_LIST *next;

    while (error_list != NULL)
    {
        next = error_list->next;
        if (error_list->string != NULL)
        {
            free (error_list->string);
        }
        free (error_list);
        error_list = next;
    }
}
        


/* error_PrintForced:
 *  Affiche une erreur forcée.
 */
void error_PrintForced (void)
{
    int write_door = CONSOLE_WRITE_DOOR_OPENED;
    struct ERROR_LIST *list;

    if (verbose == ERROR_VERBOSE_MIN)
    {
        for (list = error_list;
             (list != NULL) && (write_door == CONSOLE_WRITE_DOOR_OPENED);
             list = list->next)
        {
            if ((list->type == error_type)
             && (list->string != NULL)
             && (strcmp (list->string, error_string) == 0))
            {
                write_door = CONSOLE_WRITE_DOOR_CLOSED;
            }
        }
    }

    con_SetWriteDoor (write_door);
    display_error ();
    add_error ();
}    



/* error_Print:
 *  Affiche une erreur.
 */
void error_Print (void)
{
    if (assemble.pass == ASSEMBLE_PASS2)
    {
        error_PrintForced ();
    }
}



/* error_Optimize:
 *  Prépare un warning pour une optimisation.
 */
void error_Optimize (const char *format, ...)
{
    static char fr[] = "optimisation";
    static char en[] = "optimizing";
    va_list args;

    va_start (args, format);
    if (opt_GetUsr (OPT_OP) != 0)
    {
        set_error (ERROR_TYPE_OPTIMIZE, (is_fr) ? fr : en, format, &args);
    }
    va_end (args);
}




/* error_Assembler:
 *  Prépare un warning pour l'assembleur.
 */
void error_Assembler (const char *format, ...)
{
    static char to[] = "ASSEMBLER-TO";
    static char mo[] = "ASSEMBLER-MO";
    static char macro[] = "MACRO-ASSEMBLER";
    static char c6809[] = "C6809-ASSEMBLER";
    va_list args;

    va_start (args, format);
    set_error (ERROR_TYPE_ASSEMBLER,
                (assemble.soft == ASSEMBLER_TO) ? to :
                (assemble.soft == ASSEMBLER_MO) ? mo :
                (assemble.soft == ASSEMBLER_MACRO) ? macro : c6809,
                format, &args);
    va_end (args);
}



/* error_Warning:
 *  Prépare un warning.
 */
void error_Warning (const char *format, ...)
{
    static char fr[] = "attention";
    static char en[] = "warning";
    va_list args;

    va_start (args, format);
    set_error (ERROR_TYPE_WARNING, (is_fr) ? fr : en, format, &args);
    va_end (args);
}



/* error_Error:
 *  Prépare une erreur.
 */
int error_Error (const char *format, ...)
{
    static char fr[] = "erreur";
    static char en[] = "error";
    va_list args;

    va_start (args, format);
    set_error (ERROR_TYPE_ERROR, (is_fr) ? fr : en, format, &args);
    va_end (args);
    return -1;
}



/* error_Fatal:
 *  Affiche une erreur fatale.
 */
int error_Fatal (const char *format, ...)
{
    static char fr[] = "erreur fatale";
    static char en[] = "fatal error";
    va_list args;

    va_start (args, format);
    con_SetWriteDoor (CONSOLE_WRITE_DOOR_OPENED);
    set_error (ERROR_TYPE_FATAL, (is_fr) ? fr : en, format, &args);
    display_error ();
    va_end (args);
    return -1;
}



/* error_Memory:
 *  Erreur fatale pour mémoire insuffisante.
 */
int error_Memory (char *file, int line, const char *func)
{
    return error_Fatal ((is_fr) ? "%s:%d:%s(): mémoire insuffisante"
                                : "%s:%d:%s(): not enough memory",
                                file, line, func);
}



/* error_Internal:
 *  Erreur interne fatale.
 */
int error_Internal (char *file, int line, const char *func)
{
    return error_Fatal ((is_fr) ? "%s:%d:%s(): erreur interne"
                                : "%s:%d:%s(): internal error",
                                file, line, func);
}



/* error_IsFatal:
 *  Teste si le code d'erreur est fatal.
 */
int error_IsFatal (void)
{
    return (error_type == ERROR_TYPE_FATAL) ? 1 : 0;
}



/* error_Clear:
 *  Initialise le code d'erreur.
 */
void error_Clear (void)
{
    error_type = ERROR_TYPE_NONE;
}



/* error_ErrnoFOpen:
 *  Affiche le message selon errno pour une erreur d'ouverture de fichier.
 */
int error_ErrnoFOpen (char *name)
{
#ifndef S_SPLINT_S
    switch (errno)
    {
        case ENOTDIR: (void)error_Fatal ((is_fr)
                     ? "{>%s}N'est pas un dossier"
                     : "{>%s}Not a directory", name); break;
        case ENFILE: (void)error_Fatal ((is_fr)
                     ? "{>%s}Trop de fichiers ouverts dans le système"
                     : "{>%s}Too many open files in system", name); break;
        case ENAMETOOLONG: (void)error_Fatal ((is_fr)
                     ? "{>%s}Nom de fichier trop long"
                     : "{>%s}Filename too long", name); break;
        case ENOENT: (void)error_Fatal ((is_fr)
                     ? "{>%s}fichier ou répertoire introuvable"
                     : "{>%s}no such file or directory", name); break;
        case EACCES: (void)error_Fatal ((is_fr)
                     ? "{>%s}permission refusée"
                     : "{>%s}permission denied", name); break;
        case EISDIR: (void)error_Fatal ((is_fr)
                     ? "{>%s}le fichier est un répertoire"
                     : "{>%s}file is a directory", name); break;
        case EMFILE: (void)error_Fatal ((is_fr)
                     ? "{>%s}trop de fichiers ouverts"
                     : "{>%s}too many open files", name); break;
        case EROFS:  (void)error_Fatal ((is_fr)
                     ? "{>%s}système de fichier en lecture seule"
                     : "{>%s}read-only file system", name); break;
        case EIO:    (void)error_Fatal ((is_fr)
                     ? "{>%s}erreur d'entrée-sortie"
                     : "{>%s}I/O error", name); break;
        default:     (void)error_Fatal ((is_fr)
                     ? "{>%s}erreur d'ouverture du fichier"
                     : "{>%s}file opening error", name); break;
    }
#else
    name = name;
#endif
    return -1;
}



/* error_DirectiveNotSupported:
 *  Helper pour erreur.
 */
void error_DirectiveNotSupported (int flag)
{
    if (flag == 1)
    {
        error_Assembler ((is_fr) ? "{C}{c}directive non supportée"
                                 : "{C}{c}directive not supported");
    }
}



/* error_LabelNotSupported:
 *  Helper pour erreur.
 */
void error_LabelNotSupported (void)
{
    if (assemble.label[0] != '\0')
    {
        error_Warning ((is_fr) ? "{L}{l}étiquette non supportée"
                               : "{L}{l}label not supported");
    }
}



/* error_BadAddressingMode:
 *  Helper pour erreur.
 */
int error_BadAddressingMode (void)
{
    return error_Error ((is_fr) ? "{+}mode d'adressage incorrect"
                                : "{+}bad addressing mode");
}



/* error_OperandOutOfRange:
 *  Helper pour erreur.
 */
int error_OperandOutOfRange (void)
{
    return error_Error ((is_fr) ? "{C}{o}opérande hors champ"
                                : "{C}{o}operand out of range");
}



/* error_BadSeparator:
 *  Helper pour erreur.
 */
int error_BadSeparator (void)
{
    return error_Error ((is_fr) ? "{+}mauvais séparateur"
                                : "{+}bad separator");
}



/* error_BadCharacter:
 *  Helper pour erreur.
 */
int error_BadCharacter (void)
{
    return error_Error ((is_fr) ? "{+}caractère incorrect"
                                : "{+}wrong character");
}



/* error_BadRegister:
 *  Helper pour erreur.
 */
int error_BadRegister (void)
{
    return error_Error ((is_fr) ? "{+}registre incorrect" : "{+}bad register");
}



/* error_MissingInformation:
 *  Helper pour erreur.
 */
int error_MissingInformation (void)
{
    return error_Error ((is_fr) ? "{-}information manquante"
                                : "{-}missing information");
}



/* error_MissingRegister:
 *  Helper pour erreur.
 */
int error_MissingRegister (void)
{
    return error_Error ((is_fr) ? "{-}registre manquant"
                                : "{-}missing register");
}



/* error_MissingOperand:
 *  Helper pour erreur.
 */
int error_MissingOperand (void)
{
    return error_Error ((is_fr) ? "{-}opérande manquante"
                                : "{-}missing operand");
}



/* error_BadDp:
 *  Helper pour erreur.
 */
int error_BadDp (void)
{
    return error_Error ((is_fr) ? "{o}DP incorrect ($%02x)"
                                : "{o}bad DP ($%02x)",
                                setdp_Get());
}



/* error_SetVerboseMin:
 *  Demande un bavardage minimum pour les erreurs.
 */
void error_SetVerboseMin (void)
{
    verbose = ERROR_VERBOSE_MIN;
}



/* error_SetVerboseMid:
 *  Demande un bavardage normal pour les erreurs.
 */
void error_SetVerboseMid (void)
{
    verbose = ERROR_VERBOSE_MID;
}



/* error_SetVerboseMax:
 *  Demande un bavardage maximum pour les erreurs.
 */
void error_SetVerboseMax (void)
{
    verbose = ERROR_VERBOSE_MAX;
}

