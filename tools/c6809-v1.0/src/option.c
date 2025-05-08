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
    #include <stdarg.h>
    #include <ctype.h>
#endif

#include "defs.h"
#include "error.h"
#include "symbol.h"
#include "arg.h"
#include "output.h"
#include "assemble.h"
#include "eval.h"
#include "encode.h"
#include "directiv/opt.h"
#include "output/bin.h"
#include "output/asm.h"
#include "output/console.h"

static void option_help (void);
static void option_version (void);
static void set_value (void);
static void quiet_on (void);

struct OPTION_LIST {
    char letter[5];
    char option[20];
    char arg[15];
    void /*@null@*/(*prog)(void);
    char en[80];
    char fr[80];
};

struct OPTION_TITLE {
    char en[80];
    char fr[80];
    struct OPTION_LIST /*@null@*/*list;
};

static struct OPTION_LIST option_c6809[] = {
    { "-h", "--help", "", option_help,
        "This help", "Aide du programme" },
    { "-v", "--version", "", option_version,
        "Display version", "Affiche la version" },
    { "-q", "--quiet", "", quiet_on,
        "No display in console", "Pas de sortie dans la console" },
    { "", "--verbose-min", "", error_SetVerboseMin,
        "Display each type of error once (default)",
        "Affiche chaque type d'erreur une fois (défaut)"},
    { "", "--verbose", "", error_SetVerboseMid,
        "Display all errors",
        "Affiche toutes les erreurs" },
    { "", "--verbose-max", "", error_SetVerboseMax,
        "Display all errors and lines",
        "Affiche toutes les erreurs et les lignes" },
    { "", "", "", NULL, "", "" }
};

static struct OPTION_LIST option_optimize[] = {
    { "-op", "--optimize", "", opt_SetOptimize, 
        "Notify optimizations", "Notifie les soptimisations" },
    { "", "", "", NULL, "", "" }
};

static struct OPTION_LIST option_operators[] = {
    { "", "--operator-soft", "", eval_SetSoftOperator,
        "Use soft operators and signs (default)",
        "Utilise les opérateurs et signes du soft (par défaut)" },
    { "", "--operator-c6809", "", eval_SetC6809Operator,
        "Use c6809 operators and signs",
        "Utilise les opérateurs et signes de c6809" },
    { "", "", "", NULL, "", "" }
};

static struct OPTION_LIST option_symbols[] = {
    { "", "--symbols-none", "", symbol_SetNone,
        "Do not display the symbol list (default)",
        "Pas d'affichage de la liste des symboles (par défaut)" },
    { "-s", "--symbols-by-alpha", "", symbol_SetAlphaOrder,
        "Display the symbol list in alphabetical order",
        "Affiche les symboles dans l'ordre alphabétique" },
    { "-se", "--symbols-by-error", "", symbol_SetErrorOrder,
        "Display the symbol list in error order",
        "Affiche les symboles dans l'ordre des erreurs" },
    { "-st", "--symbols-by-type", "", symbol_SetTypeOrder,
        "Display the symbol list in type order",
        "Affiche les symboles dans l'ordre des types" },
    { "-sn", "--symbols-by-times", "", symbol_SetTimesOrder,
        "Display the symbol list in times order",
        "Affiche les symboles dans l'ordre des fréquences" },
    { "", "--lone-symbols", "", symbol_SetLone,
        "Display a warning if the symbol is lone",
        "Affiche une mise en garde si le symbole est unique" },
    { "", "", "", NULL, "", "" }
};

static struct OPTION_LIST option_output[] = {
    { "-bd", "--binary-data", "", bin_SetDataFile,
        "Create a simple data binary (default)",
        "Crée un simple fichier de données (default)" },
    { "-b", "--binary-not-linear", "", bin_SetNonLinearFile,
        "Create a non-linear Thomson binary/(blocks of 128 bytes maximum)",
        "Crée un binaire Thomson non linéaire/(blocs de 128 octets maximum)" },
    { "-bl", "--binary-linear", "", bin_SetLinearFile,
        "Create a linear Thomson binary (one block)",
        "Crée un binaire Thomson linéaire (un bloc)" },
    { "-bh", "--binary-hybrid", "", bin_SetHybridFile,
        "Create an hybrid Thomson binary/(blocks of variable size)",
        "Crée un binaire Thomson hybride/(blocs de taille variable)" },
    { "", "", "", NULL, "", "" }
};

static struct OPTION_LIST option_argument[] = {
    { "-d", "", " label=value", set_value,
        "Set the value of a label",
        "Définit la valeur d'une étiquette" },
    { "", "", "", NULL, "", "" }
};

static struct OPTION_LIST option_creation[] = {
    { "-c", "--create-asm-files", "", asm_SetCreate,
        "Create the ASM Thomson files",
        "Crée les fichiers ASM Thomson" },
    { "", "", "", NULL, "", "" }
};

static struct OPTION_LIST option_compiler[] = {
    { "-a", "--assembler-c6809", "", assemble_SetC6809,
        "Compile like c6809",
        "Compile comme c6809" },
    { "-am", "--assembler-macro", "", assemble_SetMacroAssembler,
        "Compile like MACROASSEMBLER  (default)",
        "Compile comme MACROASSEMBLER (par défaut)" },
    { "", "--assembler-mo", "", assemble_SetAssemblerMo,
        "Compile like ASSEMBLER on TO (Thomson specific)",
        "Compile comme ASSEMBLER sur MO (spécifique à Thomson)" },
    { "", "--assembler-to", "", assemble_SetAssemblerTo,
        "Compile like TO ASSEMBLER on MO (Thomson specific)",
        "Compile comme ASSEMBLER sur TO (spécifique à Thomson)" },
    { "", "", "", NULL, "", "" }
};

static struct OPTION_TITLE option_title[] = {
    { "C6809"                 , "C6809"                 , option_c6809     },
    { "Optimizing"            , "Optimisation"          , option_optimize  },
    { "Operators"             , "Opérateurs"            , option_operators },
    { "Symbols"               , "Symboles"              , option_symbols   },
    { "Binary output"         , "Sortie binaire"        , option_output    },
    { "Argument passing"      , "Passage d'arguments"   , option_argument  },
    { "ASM creation (Thomson)", "Création ASM (Thomson)", option_creation  },
    { "Compiler type"         , "Type de compilateur"   , option_compiler  },
    { ""                      , ""                      , NULL             }
};

static int arg_c;
static int arg_i;
static char **arg_v;
static int option_exit = 0;
static char file_name[2][MAX_STRING+1];



/* base_name:
 *  Renvoie le nom de fichier seul.
 */
static char /*@dependent@*/*base_name (char *name)
{
    char *p;

    if ((p = strrchr (name, (int)'\\')) == NULL)
        p = strrchr (name, (int)'/');

    return (p == NULL) ? name : p+1;
}



/* print_con:
 *  Affiche un élément de l'erreur.
 */
static void print_con (const char *format, ...)
{
    va_list args;
    char string[MAX_STRING+1];

    va_start (args, format);
    (void)vsnprintf (string, MAX_STRING, format, args);
    con_Print (ENCODING_WINDOWS, string);
    va_end (args);
}



/* get_argv:
 *  Lit l'argument courant.
 */
static char /*@dependent@*/*get_argv (int i)
{
    static char str[MAX_STRING+1];

    str[0] = '\0';
    strncat (str, arg_v[i], MAX_STRING);
    arg_RTrim (str);

    return str;
}



/* option_version:
 *  Affiche la version.
 */
static void option_version (void)
{
    print_con ("%s v%s.%s.%s\n",
         base_name (get_argv (0)),
         PROG_VERSION_MAJOR,
         PROG_VERSION_MINOR,
         PROG_VERSION_MICRO);
    print_con ("Copyright (c) %s %s François Mouret\n",
         PROG_CREATION_MONTH,
         PROG_CREATION_YEAR);
    option_exit = 1;
}
    
    
    
/* option_help:
 *  Affiche l'aide.
 */
static void option_help (void)
{
    int i;
    int t;
    char str[MAX_STRING+1];
    char *p;
    struct OPTION_LIST *list;

    print_con ("Usage: %s [OPTION]... %s\n",
        base_name (get_argv (0)),
        (is_fr)?"FICHIER":"FILE");
    print_con ((is_fr)?"Compilateur Macro/Assembler 6809\n"
                      :"Macro/Assembler 6809 compiler\n");

    for (t=0; option_title[t].list != NULL; t++)
    {
        print_con ("\n %s:\n", (is_fr) ? option_title[t].fr
                                       : option_title[t].en);
        list = option_title[t].list;
        for (i=0; list[i].prog != NULL; i++)
        {
            (void)snprintf (str, MAX_STRING, "%s%s",
                            list[i].option, list[i].arg);
            print_con ("  %-3s %-20s", list[i].letter, str);
            (void)snprintf (str, MAX_STRING, "%s",
                            (is_fr)?list[i].fr:list[i].en);
            p = strtok (str, "/");
            while (p != NULL)
            {
                if (p != str)
                {
                    print_con ("%26s", "");
                }
                print_con ("%s\n", p);
                p = strtok (NULL, "/");
            }
        }
    }
    print_con ("\n");
    option_exit = 1;
}



/* option_advice:
 *  Affiche l'erreur et le conseil.
 */
static void option_advice (char *argstr, char *str)
{
    print_con ("***");
    if (argstr[0] != '\0')
    {
        print_con (" %s:", argstr);
    }
    print_con (" %s\n", str);
    print_con ((is_fr)?"Tapez '%s --help' pour afficher l'aide\n"
                      :"Type '%s --help' to display help\n",
                       base_name (get_argv (0)));
    option_exit = -1;
}



/* quiet_on:
 *  Oblitère la sortie Thomson.
 */
static void quiet_on (void)
{
    con_SetQuiet (CONSOLE_QUIET_ON);
}



/* set_value:
 *  Définit la valeur d'un symbole.
 */
static void set_value (void)
{
    int err = -1;
    char *str = NULL;
    u16 value = 0;

    if (++arg_i < arg_c)
    {
        str = get_argv (arg_i);
        assemble.ptr = str;
        if (arg_Read (&assemble.ptr, ARG_STYLE_NONE) == ARG_ALPHA)
        {
            assemble.label[0] = '\0';
            strncat (assemble.label, arg.str, MAX_STRING);
            if ((arg_Read (&assemble.ptr, ARG_STYLE_NONE) == ARG_SIGN)
             && (*arg.str == '=')
             && (eval_Do (&assemble.ptr, &value) == 0))
            {
                (void)symbol_Do (assemble.label, value, SYMBOL_TYPE_ARG);
                err = 0;
            }
        }
    }

    if (err != 0)
    {
        option_advice ((str == NULL) ? "-d" : str,
                (is_fr)?"erreur d'argument":"argument error");
    }
}


/* ------------------------------------------------------------------------- */


/* option_FileAss:
 *  Renvoie le nom de fichier d'entrée.
 */
char /*@dependent@*/*option_FileAss (void)
{
    return file_name[0];
}



/* option_FileBin:
 *  Renvoie le nom de fichier de sortie.
 */
char /*@dependent@*/*option_FileBin (void)
{
    return file_name[1];
}



/* option_Do:
 *  Traite la ligne de commande
 */
int option_Do (int argc, char *argv[])
{
    int i;
    int t;
    char *str;
    int count = 0;
    struct OPTION_LIST *list;

    assemble_Init ();
    bin_SetDataFile ();
    opt_Init ();

    file_name[0][0] = '\0';
    file_name[1][0] = '\0';
    option_exit = 0;

    arg_c = argc;
    arg_v = argv;

    /* Lit les options de la ligne de commande */
    if (arg_c == 1)
    {
        option_help ();
    }
    else
    for (arg_i = 1; (arg_i < arg_c) && (option_exit == 0); arg_i++)
    {
        option_exit = -2;
        str = get_argv (arg_i);
        if (str[0] == '-')
        {
            for (t=0; (option_title[t].list!=NULL) && (option_exit == -2); t++)
            {
                list = option_title[t].list;
                for (i=0; (list[i].prog != NULL) && (option_exit == -2); i++)
                {
                    if ((strcmp (str, list[i].option) == 0)
                     || (strcmp (str, list[i].letter) == 0))
                    {
                        option_exit = 0;
                        (*list[i].prog)();
                    }
                }
            }
            if (option_exit == -2)
            {
                option_advice (str, (is_fr)?"Option inconnue"
                                           :"Unknown option");
            }
        }
        else
        if (count < 2)
        {
            file_name[count][0] = '\0';
            strncat (file_name[count], str, MAX_STRING);
            count += 1;
            option_exit = 0;
        }
    }

    if ((count == 0)
     && (option_exit == 0))
    {
        option_advice ("", (is_fr)?"Nom de fichier absent":"Missing file name");
    }

    return option_exit;
}

