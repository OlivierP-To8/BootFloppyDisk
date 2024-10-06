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
    #include <string.h>
    #include <stdlib.h>
    #include <ctype.h>
#endif

#include "defs.h"
#include "assemble.h"
#include "error.h"
#include "arg.h"
#include "encode.h"
#include "mark.h"
#include "source.h"
#include "output/bin.h"
#include "output/lst.h"

#if 0
#define DO_PRINT  1      /* if output wanted */
#endif

#define ASM_MAX_SIZE 29000 /* Taille maximum pour un fichier ASM */

struct SOURCE_LIST *source_list = NULL;
static struct SOURCE_TEXT /*@null@*/**source_text_next = NULL;
static char path_thomson[MAX_STRING+1];
static char path_pc[MAX_STRING+1];
static char fgets_str[MAX_STRING];
static char *pdesc;




#ifdef DO_PRINT
/* display_source:
 *  Affiche toutes les sources résidentes.
 */
static void display_source (void)
{
    int i;
    struct SOURCE_LIST *source;
    struct SOURCE_TEXT *text;

    for (source = source_list; source != NULL; source = source->next)
    {
        (void)fprintf (stderr, "===================================\n");
        (void)fprintf (stderr, "%8s%-11s%s (%d)\n", "", "Type",
            (source->type == MARK_TYPE_NONE)    ? "MARK_TYPE_NONE"    :
            (source->type == MARK_TYPE_INFO)    ? "MARK_TYPE_NONE"    :
            (source->type == MARK_TYPE_CHECK)   ? "MARK_TYPE_CHECK"   :
            (source->type == MARK_TYPE_MAIN)    ? "MARK_TYPE_MAIN"    :
            (source->type == MARK_TYPE_INCLUDE) ? "MARK_TYPE_INCLUDE" :
            "Unknown", source->type);
        if (source->pc != NULL)
        {
            (void)fprintf (stderr, "%8s%-11s'%s'\n", "", "Pc", source->pc);
        }
        if (source->thomson != NULL)
        {
            (void)fprintf (stderr, "%8s%-11s'%s'\n", "", "Thomson",
                           source->thomson);
        }
        (void)fprintf (stderr, "%8s%-11s%s (%d)\n", "", "Encoding",
            (source->encoding == ENCODING_UNKNOWN) ? "ENCODING_UNKNOWN" :
            (source->encoding == ENCODING_UTF8)    ? "ENCODING_UTF8"    :
            (source->encoding == ENCODING_WINDOWS) ? "ENCODING_WINDOWS" :
            (source->encoding == ENCODING_MSDOS)   ? "ENCODING_MSDOS"   :
            (source->encoding == ENCODING_MAC)     ? "ENCODING_MAC"     :
            "Unknown", source->encoding);
        (void)fprintf (stderr, "%8s%-11s%d\n", "", "Line", source->line);
        (void)fprintf (stderr, "%8s%-11sx%d\n", "", "Used", source->used);

        for (i = 0, text = source->text;
            (text != NULL) && (text->str != NULL) && (i < 10);
            text = text->next, i++)
        {
            (void)fprintf (stderr, "    '%s'\n", text->str);
        }
    }
    (void)fprintf (stderr, "===================================\n");
}
#endif



/* error_wrong_thomson_file:
 *  Helper pour erreur.
 */
static int error_wrong_thomson_file (char *name)
{
    return error_Error ((is_fr) ? "{Cc}%s de fichier Thomson incorrect"
                                : "{Cc}wrong Thomson file %s", name);
}



/* error_missing_pc_quote:
 *  Helper pour erreur.
 */
static int error_missing_pc_quote (void)
{
    return error_Error (
            (is_fr) ? "{Cc}guillemet manquante pour nom de fichier PC"
                    : "{Cc}missing quote for PC file name");
}



/* error_missing_file_name:
 *  Helper pour erreur.
 */
static int error_missing_file_name (void)
{
    return error_Error ((is_fr) ? "{Cc}nom de fichier manquant"
                                : "{Cc}missing file name");
}



/* error_include_conflict:
 *  Helper pour erreur.
 */
static int error_include_conflict (char *name)
{
    return error_Fatal ((is_fr) ? "{Cc>%s}conflit d'include"
                                : "{Cc>%s}include conflict", name);
}



/* error_duplicate_main:
 *  Helper pour erreur.
 */
static int error_duplicate_main (char *name)
{
    return error_Fatal ((is_fr) ? "{Cc>%s}main en double"
                                : "{Cc>%s}duplicate main", name);
}



/* error_duplicate_include:
 *  Helper pour erreur.
 */
static int error_duplicate_include (char *name)
{
    return error_Fatal ((is_fr) ? "{Cc>%s}include en double"
                                : "{Cc>%s}duplicate include", name);
}



/* get_source_list:
 *  Renvoie le pointeur du source.
 */
static struct SOURCE_LIST /*@dependent@*//*@null@*/*
    get_source_list (struct SOURCE_LIST /*@null@*/*list)
{
    struct SOURCE_LIST /*@dependent@*//*@null@*/*src_list = list;

    return src_list;
}



/* filter_file_name:
 *  Filtre le nom de fichier ASM Thomson.
 */
static int filter_file_name (int size_max, char /*@null@*/*p, char *name)
{
    int i;
    int err = 0;
    char *thomson = path_thomson;

    if (p != NULL)
    {
        strncat (thomson, p, (size_t)size_max+100);
        for (i = 0; p[i] > '\0'; i++) {}
        err = ((strlen (p) > (size_t)size_max)
            || (strpbrk (p, "():.") != NULL)
            || (p[i] < '\0')) ? error_wrong_thomson_file (name) : 0;
    }
    else
    {
        err = error_Internal (__FILE__, __LINE__, __func__);
    }

    return err;
}



/* get_thomson_path:
 *  Récupère un chemin de fichier ASM Thomson.
 */
static int get_thomson_path (char *extension)
{
    int err = 0;
    char bname[MAX_STRING+3];
    char *p = NULL;
    char *file_extension = extension;
    
    path_thomson[0] = '\0';
    if (arg_ReadField (&pdesc, ARG_STYLE_VALUE) != ARG_END)
    {
        if (*arg.str != '"')
        {
            bname[0] = '\0';
            strncat (bname, arg.str, MAX_STRING);
            p = bname;

            if (((p[0] >= '0') && (p[0] <= '4')) && (p[1] == ':'))
            {
                strncat (path_thomson, p, 2);
                p += 2;
            }
            else
            {
                strcat (path_thomson, "0:");
            }

            p = strtok (p, ".");
            if (p != NULL)
            {
                err |= filter_file_name (8, p, (is_fr)?"nom":"name");
                p = strtok (NULL, " ");
                p = (p == NULL) ? file_extension : p;
                strcat (path_thomson, ".");
                err |= filter_file_name (3, p, "extension");
            }
            else
            {
                err = error_Error ((is_fr)
                                   ? "{+}descripteur de fichier incorrect"
                                   : "{+}wrong file descriptor");
                error_PrintForced ();
            }
        }
        else
        {
            err = error_Error ((is_fr) ? "{+}descripteur de fichier incorrect"
                                       : "{+}wrong file descriptor");
            error_PrintForced ();
        }
    }
    else
    {
        err = error_Error ((is_fr) ? "{+}descripteur de fichier incorrect"
                                   : "{+}wrong file descriptor");
        error_PrintForced ();
    }

    return err;
}



/* get_pc_path:
 *  Récupère un chemin de fichier PC.
 */
static int get_pc_path (void)
{
    int err = 0;
    char *quote;

    if (arg_ReadField (&pdesc, ARG_STYLE_VALUE) == ARG_END)
    {
        err = error_missing_file_name ();
    }
    else
    if (*arg.str == '"')
    {
        quote = strchr (arg.str+1, (int)'"');
        if (quote != NULL)
        {
            if ((quote - arg.str) > 2)
            {
                strncpy (path_pc, arg.str+1, quote-arg.str-1);
            }
            else
            {
                err = error_missing_file_name ();
            }
        }
        else
        {
            err = error_missing_pc_quote ();
        }
    }
    else
    {
        err = error_missing_pc_quote ();
    }

    return err;
}



/* get_include_path:
 *  Récupère un chemin de fichier.
 */
static int get_include_path (char *extension)
{
    int err;
    char *file_extension = extension;

    path_thomson[0] = '\0';
    path_pc[0] = '\0';

    if (*pdesc != '"')
    {
        err = get_thomson_path (file_extension);
        if (err == 0)
        {
            arg_SkipSpaces (&pdesc);
            (void)arg_ReadField (&pdesc, ARG_STYLE_NONE);
            if (strcmp (arg.str, "from") == 0)
            {
                arg_SkipSpaces (&pdesc);
                err = get_pc_path ();
            }
        }
    }
    else
    {
        err = get_pc_path ();
    }

    err |= ((path_thomson[0] == '\0') && (path_pc[0] == '\0'))
                ? error_missing_file_name () : 0;

    return err;
}



/* close_source:
 *  Referme tous les sources.
 */
static void close_source (void)
{
    struct SOURCE_LIST *list_next;
    struct SOURCE_TEXT *text_next;

    while (source_list != NULL)
    {
        list_next = source_list->next;

        if (source_list->pc != NULL)
        {
            free (source_list->pc);
        }

        if (source_list->thomson != NULL)
        {
            free (source_list->thomson);
        }

        while (source_list->text != NULL)
        {
            text_next = source_list->text->next;
            if (source_list->text->str != NULL)
            {
                free (source_list->text->str);
            }
            free (source_list->text);
            source_list->text = text_next;
        }
        free (source_list);
        source_list = list_next;
   }
}



/* check_if_duplicate:
 *  Erreur si MAIN ou INCLUDE en double.
 */
static int check_if_duplicate (void)
{
    int err = 0;
    struct SOURCE_LIST /*@null@*/*src = get_source_list (source_list);
    struct SOURCE_LIST /*@null@*/*dst = get_source_list (source_list);

    for (; (err == 0) && (dst != NULL); dst = dst->next)
    {
        if ((dst != src)
         && (src != NULL)
         && (dst->thomson != NULL)
         && (src->thomson != NULL))
        {
            if ((src->type == MARK_TYPE_MAIN)
             && (dst->type == MARK_TYPE_MAIN))
            {
                err = error_duplicate_main (src->thomson);
            }
            else
            if (strcmp (dst->thomson, src->thomson) == 0)
            {
                err = error_duplicate_include (src->thomson);
            }
        }
    }

    return err;
}



/* add_thomson_name:
 *  Ajoute le nom de fichier au format Thomson.
 */
static int add_thomson_name (char *thomson)
{
    int err = 0;

    if (source_list != NULL)
    {
        if (source_list->thomson == NULL)
        {
            source_list->thomson = arg_StrAlloc (thomson);
            err = check_if_duplicate ();
        }
    }

    return err;
}



/* create_source:
 *  Crée une entrée pour un source.
 */
static void create_source_entry (int type, char *pc, int line)
{
    struct SOURCE_LIST *source_list_old = source_list;

    source_list = malloc (sizeof (struct SOURCE_LIST));
    if (source_list != NULL)
    {
        source_list->next = source_list_old;
        source_list->type = type;
        source_list->line = line;
        source_list->used = 0;
        source_list->encoding = ENCODING_UTF8;
        source_list->pc = arg_StrAlloc (pc);
        source_list->thomson = NULL;
        source_list->text = NULL;
        /* Pour le chaînage progressif */
        source_text_next = &source_list->text;
    }
    else
    {
        source_list = source_list_old;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }
}
    


/* add_source:
 *  Ajoute un source dans la bibliothèque.
 */
static void add_source (int type, char *pc, char /*@null@*/*thomson, int line)
{
    create_source_entry (type, pc, line);
    
    if ((thomson != NULL)
     && (source_list != NULL))
    {
        (void)add_thomson_name (thomson);
    }

    assemble.source = get_source_list (source_list);
}



/* add_source_marked:
 *  Ajoute un source marqué (MAIN ou INCLUDE).
 */
static void add_source_marked (int type, char *name)
{
    if (get_thomson_path ("ASM") == 0)
    {
        if (path_thomson[0] != '\0')
        {
            add_source (type, name, path_thomson, assemble.line+1);
        }
        else
        {
            (void)error_missing_file_name ();
        }
    }
}



/* create_text_entry:
 *  Crée une ligne de texte.
 */
static void create_text_entry (char *str)
{
    struct SOURCE_TEXT *text = NULL;

    if (source_text_next != NULL)
    {
        text = malloc (sizeof (struct SOURCE_TEXT));
        if (text != NULL)
        {
            text->next = NULL;
            text->str = arg_StrAlloc (str);
            /* Assure le chaînage progressif */
            *source_text_next = text;
            source_text_next = &text->next;
        }
        else
        {
            (void)error_Memory (__FILE__, __LINE__, __func__);
        }
    }
    else
    {
        (void)error_Internal (__FILE__, __LINE__, __func__);
    }
}



/* add_text:
 *  Ajoute une ligne de texte au source.
 */
static void add_text (char *str)
{
    if (source_list != NULL)
    {
        create_text_entry (str);
        source_list->encoding = encode_Get (source_list->encoding, str);
    }
}



/* load_source:
 *  Charge un fichier source.
 */
static void load_source (char *name)
{
    FILE *file;
    int start = 0;

    file = fopen (name, "rb");
    if (file != NULL)
    {
        assemble.line = 0;
        while ((error_IsFatal () == 0)
            && (fgets (fgets_str, MAX_STRING, file) != NULL))
        {
            assemble.line += 1;
            arg_RTrim (fgets_str);
            if ((fgets_str[0] != '\0') || (start != 0))
            {
                pdesc = fgets_str;
                switch (mark_IsMark (&pdesc))
                {
                    case MARK_TYPE_MAIN:
                        add_source_marked (MARK_TYPE_MAIN, name);
                        break;

                    case MARK_TYPE_INCLUDE:
                        add_source_marked (MARK_TYPE_INCLUDE, name);
                        break;

                    default:
                        if (start == 0)
                        {
                            add_source (MARK_TYPE_NONE, name, NULL,
                                        assemble.line);
                        }
                        add_text (fgets_str);
                        break;
                }
                start = 1;
            }
        }

        if ((error_IsFatal () == 0)
         && (feof (file) == 0))
        {
            (void)error_ErrnoFOpen (name);
        }
    }
    else
    {
        (void)error_ErrnoFOpen (name);
    }
}



/* find_by_type:
 *  Recherche un fichier selon le type.
 */
static struct SOURCE_LIST /*@dependent@*//*@null@*/*find_by_type (int type,
                                                                  char *pc)
{
    struct SOURCE_LIST /*@null@*/*found = NULL;
    struct SOURCE_LIST /*@null@*/*source = get_source_list (source_list);

    for (; (found == NULL) && (source != NULL); source = source->next)
    {
        if ((source->pc != NULL)
         && (strcmp (source->pc, pc) == 0)
         && (source->type == type))
        {
            found = source;
        }
    }

    return found;
}



/* find_master:
 *  Recherche le fichier maître.
 */
static struct SOURCE_LIST /*@dependent@*//*@null@*/*find_master (char *pc)
{
    struct SOURCE_LIST /*@null@*/*source;

    source = find_by_type (MARK_TYPE_MAIN, pc);
    return (source == NULL) ? find_by_type (MARK_TYPE_NONE, pc) : source;
}



/* find_by_pc:
 *  Recherche le source selon le nom de fichier PC.
 */
static struct SOURCE_LIST /*@dependent@*//*@null@*/*find_by_pc (void)
{
    struct SOURCE_LIST /*@null@*/*found = NULL;
    struct SOURCE_LIST /*@null@*/*source = get_source_list (source_list);

    for (; (found == NULL) && (source != NULL); source = source->next)
    {
        if ((source->pc != NULL)
         && (strcmp (source->pc, path_pc) == 0))
        {
            found = source;
        }
    }

    return found;
}



/* get_include_by_pc:
 *  Recherche/Charge le source selon le nom de fichier PC.
 */
static struct SOURCE_LIST /*@dependent@*//*@null@*/*load_by_pc (void)
{
    struct SOURCE_LIST /*@null@*/*found;

    found = find_by_pc ();
    if (found == NULL)
    {
        load_source (path_pc);
        if (error_IsFatal () == 0)
        {
            found = find_by_pc ();
        }
    }

    return found;
}



/* load_by_thomson:
 *  Recherche le source selon le nom de fichier Thomson.
 */
static struct SOURCE_LIST /*@dependent@*//*@null@*/*load_by_thomson (void)
{
    struct SOURCE_LIST /*@null@*/*found = NULL;
    struct SOURCE_LIST /*@null@*/*source = get_source_list (source_list);

    for (; (found == NULL) && (source != NULL); source = source->next)
    {
        if ((source->thomson != NULL)
         && (strcmp (source->thomson, path_thomson) == 0))
        {
            found = source;
        }
    }

    return found;
}



/* load_by_from:
 *  Recherche/Charge le source selon le nom de fichier Thomson et PC.
 */
static struct SOURCE_LIST /*@dependent@*//*@null@*/*load_by_from (void)
{
    struct SOURCE_LIST /*@null@*/*found = NULL;
    
    found = load_by_thomson ();
    if (found != NULL)
    {
        if ((found->pc != NULL)
         && (strcmp (found->pc, path_pc) != 0))
        {
            (void)error_include_conflict (
                (found->thomson == NULL) ? found->pc : found->thomson);
            found = NULL;
        }
    }
    else
    {
        found = load_by_pc ();
        if (found != NULL)
        {
            if (found->thomson == NULL)
            {
                found = (add_thomson_name (path_thomson) != 0) ? found : NULL;
            }
            else
            {
                (void)error_include_conflict (found->thomson);
                found = NULL;
            }
        }
    }

    return found;
}


/* ------------------------------------------------------------------------- */


/* source_Free:
 *  Libère les ressources des sources.
 */
void source_Close (void)
{
#ifdef DO_PRINT
    display_source ();
#endif
    close_source ();
}



/* source_Load:
 *  Charge le fichier source maître.
 */
struct SOURCE_LIST *source_Load (char *name)
{
    struct SOURCE_LIST /*@null@*/*source;

    arg_CaseOn ();

    source = find_master (name);
    if (source == NULL)
    {
        load_source (name);
        if (error_IsFatal () == 0)
        {
            source = find_master (name);
        }
    }
    arg_Close ();

    arg_CaseOff ();

    return source;
}



/* source_Includ:
 *  Charge un INCLUD.
 */
struct SOURCE_LIST *source_Includ (void)
{
    struct SOURCE_LIST /*@null@*/*source = NULL;

    arg_CaseOn ();
    pdesc = assemble.ptr;

    if (get_include_path ("ASM") == 0)
    {
        if ((path_thomson[0] != '\0')
         && (path_pc[0] != '\0'))
        {
            source = load_by_from ();
        }
        else
        if (path_thomson[0] != '\0')
        {
            source = load_by_thomson ();
        }
        else
        if (path_pc[0] != '\0')
        {
            source = load_by_pc ();
        }
    }

    if (source == NULL)
    {
        (void)error_Error ((is_fr)
            ? "{Cc>%s}fichier ou répertoire introuvable"
            : "{Cc>%s}no such file or directory",
             (path_thomson[0] != '\0') ? path_thomson : path_pc);
    }

    assemble.ptr = pdesc;
    arg_CaseOff ();

    return source;
}



/* source_OpenBin:
 *  Ouvre un fichier DAT ou BIN.
 */
int source_OpenBin (char *extension)
{
    int err = 0;

    arg_CaseOn ();
    pdesc = assemble.ptr;

    err = get_include_path (extension);
    if (err == 0)
    {
        if (path_pc[0] != '\0')
        {
            err = bin_ROpen (path_pc, extension);
        }
        else
        {
            err = error_missing_file_name ();
        }
    }

    assemble.ptr = pdesc;
    arg_CaseOff ();

    return err;
}

