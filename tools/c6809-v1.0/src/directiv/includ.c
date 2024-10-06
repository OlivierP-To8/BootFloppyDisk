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
    #include <string.h>
#endif

#include "defs.h"
#include "assemble.h"
#include "error.h"
#include "output.h"
#include "mark.h"
#include "source.h"
#include "directiv/includ.h"
#include "directiv/end.h"
#include "output/bin.h"
#include "output/lst.h"

#if 0
#include "encode.h"
#define DO_PRINT  1      /* if output wanted */
#endif

struct INCLUDE_LIST {
    int    line;     /* Numéro de ligne courante */
    int    locked;   /* Etat du verrou de commentaire */
    struct SOURCE_LIST /*@dependent@*//*@null@*/*source; /* Pointeur source */
    struct SOURCE_TEXT /*@dependent@*//*@null@*/*text;   /* Pointeur ligne */
    struct INCLUDE_LIST /*@only@*//*@null@*/*next;
};

static struct INCLUDE_LIST /*@only@*//*@null@*/*include_list = NULL;
static int include_level = 0;



#ifdef DO_PRINT
/* display_include:
 *  Affiche le premier include.
 */
static void display_include (void)
{
    int i;
    struct SOURCE_TEXT *text;
    struct SOURCE_LIST *source;

    if (include_list != NULL)
    {
        source = include_list->source;
        if (source != NULL)
        {
            (void)fprintf (stderr, "===================================\n");
            if (source->pc != NULL)
            {
                (void)fprintf (stderr, "%8s%-11s'%s'\n", "", "Pc", source->pc);
            }
            if (source->thomson != NULL)
            {
                (void)fprintf (stderr, "%8s%-11s'%s'\n", "", "Thomson",
                                source->thomson);
            }
            (void)fprintf (stderr, "%8s%-11s%s (%d)\n", "", "Type",
                (source->type == MARK_TYPE_NONE)    ? "MARK_TYPE_NONE"    :
                (source->type == MARK_TYPE_INFO)    ? "MARK_TYPE_NONE"    :
                (source->type == MARK_TYPE_CHECK)   ? "MARK_TYPE_CHECK"   :
                (source->type == MARK_TYPE_MAIN)    ? "MARK_TYPE_MAIN"    :
                (source->type == MARK_TYPE_INCLUDE) ? "MARK_TYPE_INCLUDE" :
                "Unknown", source->type);
            (void)fprintf (stderr, "%8s%-11s%d\n", "", "Level",
                include_level);
            (void)fprintf (stderr, "%8s%-11s%d\n", "", "Pass",
                assemble.pass - 1);
            (void)fprintf (stderr, "%8s%-11s%d\n", "", "Line",
                include_list->line);
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
    }
}
#endif



/* check_include:
 *  Vérifie la validité d'un include.
 */
static void check_include (void)
{
    if (assemble.soft < ASSEMBLER_MACRO)
    {
        if (include_level > 1)
        {
            error_Assembler ((is_fr)?"{Cc}appel dans un INCLUD non supporté"
                                    :"{Cc}call from an INCLUD not supported");
        }
    }
    else
    if (assemble.soft == ASSEMBLER_MACRO)
    {
        if ((assemble.lock & ASSEMBLE_LOCK_MACRO) != 0)
        {
            error_Assembler (
                    (is_fr)?"{Cc}appel dans une MACRO non supporté"
                           :"{Cc}call from a MACRO not supported");
        }
        else
        if (include_level > 8)
        {
            error_Assembler ((is_fr)?"{Cc}8 imbrications maximum"
                                    :"{Cc}8 embeddings maximum");
        }
    }

    if (include_level >= 150)
    {
        (void)error_Fatal ((is_fr)?"{Cc}trop d'imbrications (max 150)"
                                  :"{Cc}too many embeddings (max 150)");
    }
}



/* display_separator_message:
 *  Affiche le séparateur de message.
 */
static void display_separator_message (void)
{
    int pos = 0;

    pos = output_File ("%15s", "");
    while (pos < 79)
    {
        pos += output_File ("-");
    }
    output_FileCr ();
}



/* display_enter_message:
 *  Affiche le message pour une entrée d'include.
 */
static void display_enter_message (void)
{
    if (assemble.pass == ASSEMBLE_PASS2)
    {
        output_FileCr ();
        display_separator_message ();
    }
}



/* display_exit_message:
 *  Affiche le message pour une sortie d'include.
 */
static void display_exit_message (void)
{
    if (assemble.pass == ASSEMBLE_PASS2)
    {
        display_separator_message ();
        output_FileCr ();
    }
}



/* remove_include:
 *  Elimine le dernier include.
 */
static void remove_include (void)
{
    struct INCLUDE_LIST *next;

    if (include_list != NULL)
    {
        next = include_list->next;
        free (include_list);
        include_list = next;
    }
}



/* exit_include:
 *  Elimine le dernier include.
 */
static void exit_include (void)
{
    remove_include ();

    if (include_list != NULL)
    {
        assemble.source = include_list->source;
        assemble.text = include_list->text;
        assemble.line = include_list->line;
        display_exit_message ();
        include_level--;
    }
}



/* add_include:
 *  Ajoute un include.
 */
static void add_include (struct SOURCE_LIST /*@dependent@*/*source)
{
    struct INCLUDE_LIST *include_list_old = include_list;

    include_list = malloc (sizeof (struct INCLUDE_LIST));
    if (include_list != NULL)
    {
        include_list->next = include_list_old;
        include_list->line = 0;
        include_list->source = source;
        include_list->text = NULL;
        include_list->locked = 0;
        if (assemble.pass == ASSEMBLE_PASS1)
        {
            source->used += 1;
        }

        assemble.source = include_list->source;
        if (source != NULL)
        {
            assemble.next_text = source->text;
            assemble.next_line = source->line;
        }

        include_level++;
        if (include_level > 1)
        {
            display_enter_message ();
        }
#ifdef DO_PRINT
        display_include ();
#endif
    }
    else
    {
        include_list = include_list_old;
        (void)error_Memory (__FILE__, __LINE__, __func__);
    }
}



/* read_bin:
 *  Lit un fichier INCBIN/INCDAT.
 */
static void read_bin (char *extension)
{
    int c = 0;

    bin.max = BIN_SIZE_MAX;
    check_include ();
    if (source_OpenBin (extension) == 0)
    {
        assemble_Flush (OUTPUT_CODE_COMMENT);

        while ((c = bin_GetC ()) >= 0)
        {
            bin_PutC ((char)c);

            if (bin.size == bin.max)
            {
                assemble_Flush (OUTPUT_CODE_BYTES_ONLY);
            }
        }

        if (bin.size < bin.max)
        {
            assemble_Flush (OUTPUT_CODE_BYTES_ONLY);
        }

        bin_Flush ();
        bin_Clear ();
        bin_RClose ();
        output_SetCode (OUTPUT_CODE_NONE);
    }
}



/* allow:
 *  Limite l'autorisation de l'usage.
 */
static void allow (void)
{
    error_DirectiveNotSupported ((assemble.soft < ASSEMBLER_MACRO) ? 1 : 0);
    error_LabelNotSupported ();
}


/* ------------------------------------------------------------------------- */


/* includ_Close:
 *  Libère les ressources des includes.
 */
void includ_Close (void)
{
    while (include_list != NULL)
    {
        exit_include ();
    }
}



/* includ_AssembleINCLUD:
 *  Assemble la directive INCLUD.
 */
void includ_AssembleINCLUD (void)
{
    struct SOURCE_LIST /*@dependent@*//*@null@*/*source;

    error_LabelNotSupported ();

    check_include ();
    source = source_Includ ();
    if ((source != NULL)
     && (include_list != NULL))
    {
        include_list->text = assemble.next_text;
        include_list->line = assemble.next_line;
        add_include (source);
    }
}



/* includ_AssembleINCBIN:
 *  Assemble la directive INCBIN.
 */
void includ_AssembleINCBIN (void)
{
    allow ();
    read_bin ("BIN");
}



/* includ_AssembleINCDAT:
 *  Assemble la directive INCDAT.
 */
void includ_AssembleINCDAT (void)
{
    allow ();
    read_bin ("DAT");
}



/* includ_Next:
 *  Retourne éventuellement à l'INCLUD précédent.
 */
int includ_Next (void)
{
    while (((end_IsEnd() != 0)
         || (assemble.text == NULL)
         || (error_IsFatal () != 0))
        && (include_list != NULL))
    {
        exit_include ();
    }

    return (include_list == NULL) ? -1 : 0;
}



/* includ_Load:
 *  Charge le fichier maître.
 */
int includ_Load (char *name)
{
    struct SOURCE_LIST *source;

    include_level = 0;
    source = source_Load (name);
    if (source != NULL)
    {
        add_include (source);
        assemble.text = assemble.next_text;
        assemble.line = assemble.next_line;
    }
    return (include_list == NULL) ? -1 : 0;
}

