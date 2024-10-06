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
    #include <stdarg.h>
#endif

#include "defs.h"
#include "error.h"
#include "arg.h"
#include "source.h"
#include "assemble.h"
#include "encode.h"
#include "output.h"
#include "symbol.h"
#include "output/html.h"
#include "directiv/org.h"
#include "directiv/setequ.h"
#include "directiv/macro.h"

#define CSS_COLOR_GREEN    "#409000"
#define CSS_COLOR_MAROON   "#915b05"
#define CSS_COLOR_ROSE     "#d040a0"
#define CSS_COLOR_BLUE     "#2050b0"
#define CSS_COLOR_GREY     "#808080"
#define CSS_COLOR_RED      "#9a2020"
#define CSS_COLOR_ORANGE   "#e08020"
#define CSS_COLOR_BLACK    "#202020"
#define CSS_COLOR_WHITE    "#ffffff"

enum {
    HTM_STYLE_NONE = 0,
    HTM_STYLE_BOLD,
    HTM_STYLE_RED,
    HTM_STYLE_BLUE,
    HTM_STYLE_GREY,
    HTM_STYLE_GREEN,
    HTM_STYLE_MAROON,
    HTM_STYLE_BOLD_BLUE,
    HTM_STYLE_BOLD_GREEN,
    HTM_STYLE_BOLD_RED,
    HTM_STYLE_BOLD_MAROON,
    HTM_STYLE_BOLD_ORANGE,
    HTM_STYLE_BOLD_GREY,
    HTM_STYLE_VALUE,
    HTM_STYLE_COMMENT,
    HTM_STYLE_LABEL_NAME,
    HTM_STYLE_EQU_NAME,
    HTM_STYLE_SET_NAME,
    HTM_STYLE_MACRO_NAME,
    HTM_STYLE_LABEL_HREF,
    HTM_STYLE_EQU_HREF,
    HTM_STYLE_SET_HREF,
    HTM_STYLE_MACRO_HREF,
    HTM_STYLE_LABEL_LINK,
    HTM_STYLE_EQU_LINK,
    HTM_STYLE_SET_LINK,
    HTM_STYLE_MACRO_LINK,
    HTM_STYLE_ERROR
};

struct SPAN_LIST {
    int  style;
    char name[20];
};

static int  htm_encoding = ENCODING_UTF8;  /* Encodage pour le fichier HTML */
static FILE /*@null@*/*htm_file = NULL;
static int  htm_style = HTM_STYLE_NONE;



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



/* print_htm:
 *  Ecrit dans le fichier.
 */
static int print_htm (const char *format, ... )
{
    va_list args;
    char str[MAX_STRING+1];
    int pos = 0;

    va_start (args, format);
    pos = vsnprintf (str, MAX_STRING, format, args);
    if (htm_file != NULL)
    {
        (void)fprintf (htm_file, "%s", str);
    }
    va_end (args);

    return pos;
}



/* print_line:
 *  Ecrit une ligne avec CR.
 */
static void print_line (char *str)
{
    (void)print_htm ("%s\xd\xa", str);
}



/* print_header:
 *  Ecrit le début du fichier.
 */
static void print_header (char *name)
{
    print_line ("<html>");
    print_line ("<head>");
    (void)print_htm ("<title>");
    htm_Print (ENCODING_WINDOWS, name);
    print_line ("</title>");
    print_line ("<meta http-equiv=\"Content-Type\" content=\"text/html; " \
                "charset=utf-8\">");
    print_line ("<meta name=\"description\" content=\"6809 code\">");
    print_line ("<meta name=\"description\" " 
                    "content=\"6809 compiled program\">");

    print_line ("<style type=text/css>");

    print_line ("body { " \
                "background-color: "CSS_COLOR_WHITE"; " \
                "padding-right: 15px; " \
                "padding-left: 15px; " \
                "padding-top: 15px; " \
                "padding-bottom: 15px; " \
                "}");

    print_line ("td { " \
                "font-family: Verdana,Geneva,Arial,Helvetica,sans-serif; " \
                "font-size: 100%; " \
                "font-weight: normal; " \
                "font-style: normal; " \
                "text-align: left; " \
                "text-decoration: none; " \
                "color: "CSS_COLOR_BLACK"; " \
                "background-color: "CSS_COLOR_WHITE" " \
                "}");

    print_line ("a { " \
                "text-decoration: none; " \
                "}");

    /* instruction */
    print_line ("span.inst { " \
                "color: "CSS_COLOR_GREEN"; " \
                "font-weight: bold; " \
                "}");

    print_line ("span.green { " \
                "color: "CSS_COLOR_GREEN"; " \
                "}");

    /* directive */
    print_line ("span.dirc { " \
                "color: "CSS_COLOR_MAROON"; " \
                "font-weight: bold; " \
                "}");

    print_line ("span.maroon { " \
                "color: "CSS_COLOR_MAROON"; " \
                "}");

    /* valeur */
    print_line ("span.val { " \
                "color: "CSS_COLOR_ROSE"; " \
                "}");

    print_line ("span.comment { " \
                "color: "CSS_COLOR_BLUE"; " \
                "}");

    print_line ("span.bgrey { " \
                "color: "CSS_COLOR_GREY"; " \
                "font-weight: bold; " \
                "}");

    print_line ("span.grey { " \
                "color: "CSS_COLOR_GREY"; " \
                "}");

    print_line ("span.bold { " \
                "font-weight: bold; " \
                "}");

    print_line ("span.bred { " \
                "color: "CSS_COLOR_RED"; " \
                "font-weight: bold; " \
                "}");

    print_line ("span.red { " \
                "color: "CSS_COLOR_RED"; " \
                "}");

    print_line ("span.bblue { " \
                "color: "CSS_COLOR_BLUE"; " \
                "font-weight: bold; " \
                "}");

    print_line ("span.blue { " \
                "color: "CSS_COLOR_BLUE"; " \
                "}");

    print_line ("span.borange { " \
                "color: "CSS_COLOR_ORANGE"; " \
                "font-weight: bold; " \
                "}");

    print_line ("span.error { " \
                "background-color: "CSS_COLOR_RED"; " \
                "color: "CSS_COLOR_WHITE"; " \
                "font-weight: bold; " \
                "}");

    print_line ("a.href { " \
                "color: "CSS_COLOR_BLACK"; " \
                "font-weight: normal; " \
                "}");

    print_line ("a.href:hover { " \
                "text-decoration: underline; " \
                "}");

    print_line ("a.macro_href { " \
                "color: "CSS_COLOR_ORANGE"; " \
                "font-weight: bold; " \
                "}");

    print_line ("a.macro_href:hover { " \
                "text-decoration: underline; " \
                "}");

    print_line ("a.symbol_label { " \
                "color: "CSS_COLOR_BLACK"; " \
                "font-weight: bold; " \
                "}");

    print_line ("a.symbol_label:hover { " \
                "text-decoration: underline; " \
                "}");

    print_line ("a.symbol_equ { " \
                "color: "CSS_COLOR_BLUE"; " \
                "font-weight: bold; " \
                "}");

    print_line ("a.symbol_equ:hover { " \
                "text-decoration: underline; " \
                "}");

    print_line ("a.symbol_set { " \
                "color: "CSS_COLOR_GREEN"; " \
                "font-weight: bold; " \
                "}");

    print_line ("a.symbol_set:hover { " \
                "text-decoration: underline; " \
                "}");

    print_line ("a.symbol_macro { " \
                "color: "CSS_COLOR_MAROON"; " \
                "font-weight: bold; " \
                "}");

    print_line ("a.symbol_macro:hover { " \
                "text-decoration: underline; " \
                "}");

    print_line ("</style>");

    print_line ("</head>");
    print_line ("");
    print_line ("<body>");
    print_line ("<table width=\"100%\" height=\"0\" border=\"0\" " \
                        "cellpadding=\"0\" cellspacing=\"0\">");
    print_line ("<tr>");
    print_line ("<td width=\"100%\" height=\"0\">");
    print_line ("<pre>");
}



/* print_footer:
 *  Ecrit la fin du fichier.
 */
static void print_footer (void)
{
    print_line ("</pre>");
    print_line ("</td>");
    print_line ("</tr>");
    print_line ("</table>");
    print_line ("</body>");
    print_line ("</html>");
}



/* close_tag:
 *  Referme la balise.
 */
static void close_tag (void)
{
    if (htm_style != HTM_STYLE_NONE)
    {
        switch (htm_style)
        {
            case HTM_STYLE_LABEL_NAME:
            case HTM_STYLE_LABEL_LINK:
            case HTM_STYLE_LABEL_HREF:
            case HTM_STYLE_EQU_NAME:
            case HTM_STYLE_EQU_LINK:
            case HTM_STYLE_EQU_HREF:
            case HTM_STYLE_SET_NAME:
            case HTM_STYLE_SET_LINK:
            case HTM_STYLE_SET_HREF:
            case HTM_STYLE_MACRO_NAME:
            case HTM_STYLE_MACRO_LINK:
            case HTM_STYLE_MACRO_HREF:  (void)print_htm ("</a>");    break;
            default:                    (void)print_htm ("</span>"); break;
        }
        htm_style = HTM_STYLE_NONE;
    }
}



/* print_span:
 *  Ecrit une balise <span>.
 */
static void print_span (char *name, char *style)
{
    if (*name != '\0')
    {
        (void)print_htm ("<span class=\"%s\">", style);
    }
    else
    {
        htm_style = HTM_STYLE_NONE;
    }
}



/* print_name:
 *  Ecrit une balise <a name>.
 */
static void print_name (int type, char *name)
{
    int line;

    line = symbol_GetLine (type, name);
    if (line >= 0)
    {
        (void)print_htm ("<a name=\"%d\">", assemble.count);
    }
    else
    {
        htm_style = HTM_STYLE_NONE;
    }
}



/* print_label_name:
 *  Ecrit une balise <a name> pour une étiquette.
 */
static void print_label_name (char *name)
{
    print_name (SYMBOL_TYPE_LABEL, name);
}



/* print_equ_name:
 *  Ecrit une balise <a name> pour un EQU.
 */
static void print_equ_name (char *name)
{
    print_name (SYMBOL_TYPE_EQU, name);
}



/* print_set_name:
 *  Ecrit une balise <a name> pour un SET.
 */
static void print_set_name (char *name)
{
    print_name (SYMBOL_TYPE_SET, name);
}



/* print_macro_name:
 *  Ecrit une balise <a name> pour une MACRO.
 */
static void print_macro_name (char *name)
{
    print_name (SYMBOL_TYPE_MACRO, name);
}



/* print_href:
 *  Ecrit une balise <a href>.
 */
static void print_href (int type, char *name, char *href)
{
    int line;

    line = symbol_GetLine (type, name);
    if (line >= 0)
    {
        (void)print_htm ("<a href=\"#%d\" class=\"%s\">", line, href);
    }
    else
    {
        htm_style = HTM_STYLE_NONE;
    }
}



/* print_label_href:
 *  Ecrit une balise <a href> pour une étiquette.
 */
static void print_label_href (char *name)
{
    print_href (SYMBOL_TYPE_LABEL, name, "href");
}



/* print_equ_href:
 *  Ecrit une balise <a href> pour un EQU.
 */
static void print_equ_href (char *name)
{
    print_href (SYMBOL_TYPE_EQU, name, "href");
}



/* print_set_href:
 *  Ecrit une balise <a href> pour un SET.
 */
static void print_set_href (char *name)
{
    print_href (SYMBOL_TYPE_SET, name, "href");
}



/* print_macro_href:
 *  Ecrit une balise <a href> pour une MACRO.
 */
static void print_macro_href (char *name)
{
    print_href (SYMBOL_TYPE_MACRO, name, "macro_href");
}



/* print_link:
 *  Ecrit une balise <a href> pour la liste des symboles.
 */
static void print_link (int type, char *name, char *href)
{
    int line;

    line = symbol_GetLine (type, name);
    if (line >= 0)
    {
        (void)print_htm ("<a href=\"#%d\" class=\"%s\">", line, href);
    }
    else
    {
        htm_style = HTM_STYLE_NONE;
    }
}



/* print_label_link:
 *  Ecrit une balise <a href> pour une étiquette (liste des symboles).
 */
static void print_label_link (char *name)
{
    print_link (SYMBOL_TYPE_LABEL, name, "symbol_label");
}



/* print_equ_link:
 *  Ecrit une balise <a href> pour un EQU (liste des symboles).
 */
static void print_equ_link (char *name)
{
    print_link (SYMBOL_TYPE_EQU, name, "symbol_equ");
}



/* print_set_link:
 *  Ecrit une balise <a href> pour un SET (liste des symboles).
 */
static void print_set_link (char *name)
{
    print_link (SYMBOL_TYPE_SET, name, "symbol_set");
}



/* print_macro_link:
 *  Ecrit une balise <a href> pour une MACRO (liste des symboles).
 */
static void print_macro_link (char *name)
{
    print_link (SYMBOL_TYPE_MACRO, name, "symbol_macro");
}



/* open_tag:
 *  Ouvre une balise.
 */
static void open_tag (int encoding, char *str, int style)
{
    int i;
    char string[MAX_STRING];

    string[0] = '\0';
    for (i = 0; str[i] != '\0'; i++)
    {
        switch (str[i])
        {
            case '<': strcat (string, "&lt;"); break;
            case '>': strcat (string, "&gt;"); break;
            case '&': strcat (string, "&amp;"); break;
            case '"': strcat (string, "&quot;"); break;
            default : strncat (string, str+i, 1); break;
        }
    }

    if (style != htm_style)
    {
        close_tag ();
        htm_style = style;
        switch (style)
        {
            case HTM_STYLE_BOLD_GREEN:  print_span (str, "inst"); break;
            case HTM_STYLE_COMMENT:     print_span (str, "comment"); break;
            case HTM_STYLE_BOLD_GREY:   print_span (str, "bgrey"); break;
            case HTM_STYLE_VALUE:       print_span (str, "val"); break;
            case HTM_STYLE_BOLD_MAROON: print_span (str, "dirc"); break;
            case HTM_STYLE_BOLD:        print_span (str, "bold"); break;
            case HTM_STYLE_ERROR:       print_span (str, "error"); break;
            case HTM_STYLE_BOLD_RED:    print_span (str, "bred"); break;
            case HTM_STYLE_BOLD_BLUE:   print_span (str, "bblue"); break;
            case HTM_STYLE_BOLD_ORANGE: print_span (str, "borange"); break;
            case HTM_STYLE_RED:         print_span (str, "red"); break;
            case HTM_STYLE_BLUE:        print_span (str, "blue"); break;
            case HTM_STYLE_GREY:        print_span (str, "grey"); break;
            case HTM_STYLE_GREEN:       print_span (str, "green"); break;
            case HTM_STYLE_MAROON:      print_span (str, "maroon"); break;
            case HTM_STYLE_LABEL_NAME:  print_label_name (str); break;
            case HTM_STYLE_EQU_NAME:    print_equ_name (str);   break;
            case HTM_STYLE_SET_NAME:    print_set_name (str);   break;
            case HTM_STYLE_MACRO_NAME:  print_macro_name (str); break;
            case HTM_STYLE_LABEL_HREF:  print_label_href (str); break;
            case HTM_STYLE_EQU_HREF:    print_equ_href (str); break;
            case HTM_STYLE_SET_HREF:    print_set_href (str); break;
            case HTM_STYLE_MACRO_HREF:  print_macro_href (str); break;
            case HTM_STYLE_LABEL_LINK:  print_label_link (str); break;
            case HTM_STYLE_EQU_LINK:    print_equ_link (str); break;
            case HTM_STYLE_SET_LINK:    print_set_link (str); break;
            case HTM_STYLE_MACRO_LINK:  print_macro_link (str); break;
            default: style = HTM_STYLE_NONE; break;
        }
    }
    (void)print_htm ("%s", encode_String (encoding, htm_encoding, string));
}



/* htm_styled_print:
 *  Ecrit dans le fichier.
 */
static void htm_styled_print (int encoding, char *str, int output_style)
{
    int style;

    switch (output_style)
    {
        case ARG_STYLE_WARNING:
        case ARG_STYLE_OPTIMIZE:
        case ARG_STYLE_INSTRUCTION: style = HTM_STYLE_BOLD_GREEN; break;
        case ARG_STYLE_THOMSON:
        case ARG_STYLE_ASSEMBLER:   style = HTM_STYLE_BOLD_BLUE; break;
        case ARG_STYLE_FATAL:
        case ARG_STYLE_ERROR:       style = HTM_STYLE_BOLD_RED; break;
        case ARG_STYLE_RED:         style = HTM_STYLE_RED; break;
        case ARG_STYLE_BLUE:        style = HTM_STYLE_BLUE; break;
        case ARG_STYLE_GREEN:       style = HTM_STYLE_GREEN; break;
        case ARG_STYLE_GREY:        style = HTM_STYLE_GREY;  break;
        case ARG_STYLE_MAROON:      style = HTM_STYLE_MAROON; break;
        case ARG_STYLE_DIRECTIVE:   style = HTM_STYLE_BOLD_MAROON; break;
        case ARG_STYLE_BOLD:        style = HTM_STYLE_BOLD; break;
        case ARG_STYLE_CODE:        style = HTM_STYLE_BOLD_GREY; break;
        case ARG_STYLE_MACRO_CALL:  style = HTM_STYLE_BOLD_ORANGE; break;
        case ARG_STYLE_COMMENT:     style = HTM_STYLE_COMMENT; break;
        case ARG_STYLE_VALUE:       style = HTM_STYLE_VALUE; break;
        case ARG_STYLE_LABEL_NAME:  style = HTM_STYLE_LABEL_NAME; break;
        case ARG_STYLE_EQU_NAME:    style = HTM_STYLE_EQU_NAME; break;
        case ARG_STYLE_SET_NAME:    style = HTM_STYLE_SET_NAME; break;
        case ARG_STYLE_MACRO_NAME:  style = HTM_STYLE_MACRO_NAME; break;
        case ARG_STYLE_LABEL_HREF:  style = HTM_STYLE_LABEL_HREF; break;
        case ARG_STYLE_EQU_HREF:    style = HTM_STYLE_EQU_HREF; break;
        case ARG_STYLE_SET_HREF:    style = HTM_STYLE_SET_HREF; break;
        case ARG_STYLE_MACRO_HREF:  style = HTM_STYLE_MACRO_HREF; break;
        case ARG_STYLE_LABEL_LINK:  style = HTM_STYLE_LABEL_LINK; break;
        case ARG_STYLE_EQU_LINK:    style = HTM_STYLE_EQU_LINK; break;
        case ARG_STYLE_SET_LINK:    style = HTM_STYLE_SET_LINK; break;
        case ARG_STYLE_MACRO_LINK:  style = HTM_STYLE_MACRO_LINK; break;
        case ARG_STYLE_WRONG:       style = HTM_STYLE_ERROR; break;
        default: style = HTM_STYLE_NONE; break;
    }
    open_tag (encoding, str, style);
}


/* ------------------------------------------------------------------------- */


/* htm_Open:
 *  Ouvre le fichier.
 */
void htm_Open (char *name, char *source_name)
{
    static int first = 1;

    if (htm_file == NULL)
    {
        htm_file = fopen (name, "wb");
        if ((first != 0)
         && (htm_file == NULL))
        {
            first = 0;
            (void)error_ErrnoFOpen (name);
            error_PrintForced ();
        }

        if (htm_file != NULL)
        {
            print_header (base_name (source_name));
        }
    }
}



/* htm_Print:
 *  Ecrit dans le fichier.
 */
void htm_Print (int encoding, char *str)
{
    htm_styled_print (encoding, str, output_GetStyle ());
}



/* htm_PrintSource:
 *  Ecrit avec l'encodage du source.
 */
void htm_PrintSource (char *str)
{
    if (assemble.source != NULL)
    {
        htm_Print (assemble.source->encoding, str);
    }
}



/* htm_PrintSourceLine:
 *  Ecrit la ligne assembleur parsée avec l'encodage du source.
 */
void htm_PrintSourceLine (void)
{
    struct ARG_LIST *list;

    if ((assemble.source != NULL)
     && (arg.list != NULL))
    {
        for (list = arg.list; list != NULL; list = list->next)
        {
            if (list->str != NULL)
            {
                htm_styled_print (
                    assemble.source->encoding,
                    list->str,
                    (list->error == 0) ? list->style : ARG_STYLE_WRONG);
            }
        }
    }
}



/* htm_PrintCr:
 *  Ecrit un retour de ligne.
 */
void htm_PrintCr (void)
{
    close_tag ();
    output_SetStyle (ARG_STYLE_NONE);
    (void)print_htm ("\xd\xa");
}



/* htm_Close:
 *  Ferme le fichier.
 */
void htm_Close (void)
{
    close_tag ();
    print_footer ();

    if (htm_file != NULL)
    {
        (void)fclose (htm_file);
        htm_file = NULL;
    }
}

