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
    #include <ctype.h>
    #include <stdarg.h>
/* Fatal Error SPLINT 1.3.2 pour <unistd.h> */
    #ifndef S_SPLINT_S
    #include <unistd.h>
    #include <sys/stat.h>
    #endif
#endif

#include "defs.h"
#include "assemble.h"
#include "error.h"
#include "arg.h"
#include "source.h"
#include "encode.h"
#include "mark.h"
#include "output.h"
#include "output/console.h"

#define ASM_MAX_SIZE    29000

#define ASM_COMMENT_LOCK   (1 << 8)

static int asm_cr_count = 0;
static int asm_total_size = 0;
static int asm_comment = 0;
static int asm_encoding = ENCODING_UTF8;
static int asm_create = 0;
static int asm_lock = 0;
static int asm_lock_next = 0;
static char asm_command[MAX_STRING];
static int write_err = 0;



/* write_char:
 *  Ecrit un caractère.
 */
static void write_char (char c, FILE *file)
{
    if (write_err != EOF)
    {
        write_err = fputc ((int)c, file);
    }
}



/* write_line:
 *  Compresse et écrit la ligne ASM.
 */
static void write_line (char *p, FILE *file)
{
    int i;
    char *c;
    int space_count = 0;

    if (asm_lock == 0)
    {
        for (i = 0; ((*p != '\0') && (i < MAX_ARG)); i++)
        {
            c = encode_GetCharToAsm (asm_encoding, &p);
            switch (*c)
            {
                case ' ':
                    space_count += 1;
                    break;

                case '\t':
                    space_count =
                        (space_count < 7) ? 7 :
                        (space_count < 14) ? 14 :
                        (space_count < 22) ? 22 :
                        ((space_count - 22) & ~7) + 8 + 22;
                    break;

                default:
                    for (; asm_cr_count > 0; asm_cr_count--)
                    {
                        write_char ('\xd', file);
                        asm_total_size += 2;
                    }

                    while (space_count > 0)
                    {
                        write_char ((char)(0xf0 | MIN(space_count, 15)), file);
                        asm_total_size += 1;
                        space_count -= MIN(space_count, 15);
                    }

                    write_char (*c, file);
                    asm_total_size += 1;
                    break;
            }
        }
        asm_cr_count++;
    }
}



/* skip_field:
 *  Passe un champ d'expression.
 */
static int skip_field (char *p)
{
    int i;

    for (i = 0; p[i] > ' '; i++)
    {
        p[i] = (assemble.soft < ASSEMBLER_MACRO)
                     ? arg_ToUpper (p[i]) : p[i];
    }

    return i;
}



/* skip_space:
 *  Passe un champ d'espaces.
 */
static int skip_space (char *p)
{
    int i;

    for (i = 0; (p[i] != '\0') && (p[i] <= ' '); i++) { }

    return i;
}



/* write_code_line:
 *  Compresse et écrit la ligne de code ASM.
 */
static void write_code_line (char *p, FILE *file)
{
    int i = 0;
    int pos;

    *asm_command = '\0';
    asm_lock_next = asm_lock;

    if (p[i] != '*')
    {
        i += skip_field (p + i);
        i += skip_space (p + i);

        if (p[i] != '*')
        {
            pos = i;
            i += skip_field (p + i);
            /*@-mayaliasunique@*/
            strncat (asm_command, p+pos, (size_t)(i-pos));
            /*@+mayaliasunique@*/
            i += skip_space (p + i);

            if (assemble_HasOperand (asm_command) == 1)
            {
                (void)skip_field (p + i);
            }
        }
    }
    
    write_line (p, file);
    asm_lock = asm_lock_next;
}



/* write_asm_line:
 *  Ecrit la ligne.
 */
static void write_asm_line (FILE *file, char *p)
{
    /* Limite la ligne à 40 caractères */
    switch (*p)
    {
        case '\0':
        case '*':  write_line (p, file); break;

        case '(' : break;

        case '/':  asm_comment ^= ASM_COMMENT_LOCK;
                   write_line (p, file);
                   break;

        default:   if (asm_lock != 0)
                   {
                       write_line (p, file);
                   }
                   else
                   {
                       write_code_line (p, file);
                   }
                   break;
    }
}



/* message_create_start:
 *  Affiche l'en-tête pour le message de création.
 */
static void message_create_start (void)
{
    char msg_fr[] = "Création des fichiers ASM...";
    char msg_en[] = "Creating ASM files...";

    output_SetStyle (ARG_STYLE_BOLD);
    con_Print (ENCODING_WINDOWS, (is_fr) ? msg_fr : msg_en);
    con_PrintCr ();
}



/* message_create_file:
 *  Affiche les noms de fichiers pour le message de création.
 */
static void message_create_file (int encoding, char *pc, char *thomson)
{
    if ((thomson != NULL)
     && (*thomson != '\0'))
    {
        output_SetStyle (ARG_STYLE_BOLD);
        con_Print (encoding, thomson);
        output_SetStyle (ARG_STYLE_NONE);
        con_Print (ENCODING_WINDOWS, " dans ");
    }
    output_SetStyle (ARG_STYLE_BOLD);
    con_Print (encoding, pc);
    output_SetStyle (ARG_STYLE_NONE);
    con_Print (ENCODING_WINDOWS, ": ");
}



/* message_print_string:
 *  Affiche la chaîne pour le message de création.
 */
static void message_print_string (const char *format, ...)
{
    char str[MAX_STRING+1];
    va_list args;

    va_start (args, format);
    (void)vsnprintf (str, MAX_STRING, format, args);
    va_end (args);
    output_SetStyle (ARG_STYLE_NONE);
    con_Print (ENCODING_WINDOWS, str);
    con_PrintCr ();
}



/* create_asm_files:
 *  Crée les fichiers ASM.
 */
static void create_asm_files (char *file_name)
{
    int count = 0;
    char *p;
    struct SOURCE_LIST *source;
    struct SOURCE_TEXT *text;
    FILE *file;
    char save_name[MAX_STRING+8+1];
    char save_dir[MAX_STRING+1];

    save_dir[0] = '\0';
    strcpy (save_dir, file_name);
    if (((p = strrchr (save_dir, '/')) == NULL)
     && ((p = strrchr (save_dir, '\\')) == NULL))
    {
        p = save_dir;
    }
    p[0] = '\0';

    con_SetWriteDoor (CONSOLE_WRITE_DOOR_OPENED);
    message_create_start ();
    
    for (source = source_list; source != NULL; source = source->next)
    {
        if ((source->pc != NULL)
         && (source->pc[0] != '\0')
         && (source->thomson != NULL)
         && (source->thomson[0] != '\0')
         && (source->used > 0))
        {
            (void)snprintf (save_name, MAX_STRING+7, "%s/disk%c",
                            save_dir, *source->thomson);
#ifndef S_SPLINT_S
            if (access (save_name, F_OK) < 0)
            {
#ifdef OS_LINUX
                (void)mkdir (save_name, S_IRWXU);
#else
                (void)mkdir (save_name);
#endif
            }
#endif
            (void)snprintf (save_name, MAX_STRING+8, "%s/disk%c/%s",
                            save_dir, *source->thomson, source->thomson+2);
            file = fopen (save_name, "wb");
            if (file != NULL)
            {
                asm_cr_count = 0;
                asm_total_size = 0;
                asm_comment = 0;
                asm_encoding = source->encoding;
                for (text = source->text; text != NULL; text = text->next)
                {
                    /* Les lignes sont déjà en majuscule pour Assembler */
                    if (text->str != NULL)
                    {
                        write_asm_line (file, text->str);
                    }
                }
                write_char ('\xd', file);
                (void)fclose (file);

                /* Vérifie la taille du fichier */
                if (asm_total_size > (ASM_MAX_SIZE-1000))
                {
                    message_create_file (source->encoding,
                                         source->pc, source->thomson);
                    message_print_string ("%s%d%s",
                        (is_fr) ? "Le fichier ASM pourrait être trop long ("
                                : "The ASM file could be too long (",
                                 asm_total_size,
                        (is_fr) ? " octets) pour être chargé sur Thomson."
                                : " bytes) to be loaded on Thomson.");
                }
            }
            else
            {
                message_create_file (source->encoding,
                                     source->pc, source->thomson);
                message_print_string ("%s%c%s",
                    (is_fr) ? "La création de fichiers ASM nécessite un " \
                              " répertoire 'disk"
                            : "The creation of ASM files needs a 'disk",
                              *source->thomson,
                    (is_fr) ? "' dans le répertoire racine de votre programme."
                            : "' directory in the root directory of your " \
                              "program.");
                /* Déclenche une erreur fatale pour déterminer
                 * la nature de l'erreur sur ouverture */
                assemble.line = 0;
                assemble.source = NULL;
                (void)error_ErrnoFOpen (save_name);
            }
            count += 1;
        }
    }
    if (count == 0)
    {
        (void)error_Fatal ("Aucun fichier marqué trouvé");
    }
}


/* ------------------------------------------------------------------------- */


/* asm_SetCreate:
 *  Demande la création des fichiers ASM.
 */
void asm_SetCreate (void)
{
    asm_create = 1;
}



/* asm_Create:
 *  Crée les fichiers ASM.
 */
void asm_Create (char *file_name)
{
    if (asm_create != 0)
    {
        asm_lock = 0;
        write_err = 0;
        create_asm_files (file_name);
    }
}

