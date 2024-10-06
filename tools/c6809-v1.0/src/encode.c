 /*
 *  c6809 version 1.0.0
 *  copyright (c) 2024 Fran�ois Mouret
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
#include "encode.h"
#include "assemble.h"   /* ************************ */
#include "output/html.h"

#define ENCODING_UNKNOWN  -1
#define ENCODING_ASM      4
#define ENCODING_ASC      5

static char list[][6][4] = {
    { "\xe2\x96\x88", "\x7f", "\xdb", "\x7f", "\x7f", "\x7f"     }, /* block */
    { "\xe2\x86\x90", ""    , ""    , ""    , ""    , ""         }, /* larr */
    { "\xe2\x86\x91", ""    , ""    , ""    , ""    , ""         }, /* uarr */
    { "\xe2\x86\x92", ""    , ""    , ""    , ""    , ""         }, /* rarr */
    { "\xe2\x86\x93", ""    , ""    , ""    , ""    , ""         }, /* darr */
    { "\xc5\x92"    , ""    , ""    , "\xce", ""    , ""         }, /* OE */
    { "\xc5\x93"    , ""    , ""    , "\xcf", ""    , ""         }, /* oe */
    { "\xc2\xa0"    , "\xa0", "\xff", "\xca", "\x7f", "\x7f"     }, /* nbsp */
    { "\xc2\xa1"    , "\xa1", "\xad", "\xc1", ""    , ""         }, /* � */
    { "\xc2\xa2"    , "\xa2", "\xbd", "\xa2", ""    , ""         }, /* � */
    { "\xc2\xa3"    , "\xa3", "\x9c", "\xa3", ""    , ""         }, /* � */
    { "\xc2\xa4"    , "\xa4", "\xcf", "\xdb", ""    , ""         }, /* diams */
    { "\xc2\xa5"    , "\xa5", "\xbe", "\xb4", ""    , ""         }, /* � */
    { "\xc2\xa6"    , "\xa6", "\xdd", ""    , ""    , ""         }, /* vbar */
    { "\xc2\xa7"    , "\xa7", "\xf5", "\xa4", ""    , ""         }, /* � */
    { "\xc2\xa8"    , "\xa8", "\xf9", "\xac", ""    , ""         }, /* uml */
    { "\xc2\xa9"    , "\xa9", "\xb8", "\xa9", ""    , ""         }, /* � */
    { "\xc2\xaa"    , "\xaa", "\xa6", "\xbb", ""    , ""         }, /* � */
    { "\xc2\xab"    , "\xab", "\xae", "\xc7", ""    , ""         }, /* � */
    { "\xc2\xac"    , "\xac", "\xaa", "\xc2", ""    , ""         }, /* � */
    { "\xc2\xad"    , "\xad", "\xc4", "\xd1", "\x60", "\x60"     }, /* - */
    { "\xc2\xae"    , "\xae", "\xa9", "\xa8", ""    , ""         }, /* � */
    { "\xc2\xaf"    , "\xaf", "\xee", ""    , "\x7e", "\x7e"     }, /* � */
    { "\xc2\xb0"    , "\xb0", "\xf8", "\xa1", ""    , ""         }, /* � */
    { "\xc2\xb1"    , "\xb1", "\xf1", "\xb1", ""    , ""         }, /* � */
    { "\xc2\xb2"    , "\xb2", "\xfd", ""    , ""    , ""         }, /* � */
    { "\xc2\xb3"    , "\xb3", "\xfc", ""    , ""    , ""         }, /* � */
    { "\xc2\xb4"    , "\xb4", "\xef", "\xd5", ""    , ""         }, /* aigu */
    { "\xc2\xb5"    , "\xb5", "\xe6", "\xb5", ""    , ""         }, /* � */
    { "\xc2\xb6"    , "\xb6", "\xf4", "\xa6", ""    , ""         }, /* � */
    { "\xc2\xb7"    , "\xb7", "\xfa", "\xe1", ""    , ""         }, /* � */
    { "\xc2\xb8"    , "\xb8", "\xf7", "\xfc", ""    , ""         }, /* cedil */
    { "\xc2\xb9"    , "\xb9", "\xfb", ""    , ""    , ""         }, /* � */
    { "\xc2\xba"    , "\xba", "\xa7", "\xbc", ""    , ""         }, /* � */
    { "\xc2\xbb"    , "\xbb", "\xaf", "\xc8", ""    , ""         }, /* � */
    { "\xc2\xbc"    , "\xbc", "\xac", ""    , ""    , ""         }, /* 1/4 */
    { "\xc2\xbd"    , "\xbd", "\xab", ""    , ""    , ""         }, /* 1/2 */
    { "\xc2\xbe"    , "\xbe", "\xf3", ""    , ""    , ""         }, /* 3/4 */
    { "\xc2\xbf"    , "\xbf", "\xa8", "\xc0", ""    , ""         }, /* � */
    { "\xc3\x80"    , "\xc0", "\xb7", "\xcb", ""    , ""         }, /* � */
    { "\xc3\x81"    , "\xc1", "\xb5", "\xe7", ""    , ""         }, /* � */
    { "\xc3\x82"    , "\xc2", "\xb6", "\xe5", ""    , ""         }, /* � */
    { "\xc3\x83"    , "\xc3", "\xc7", "\xcc", ""    , ""         }, /* � */
    { "\xc3\x84"    , "\xc4", "\x8e", "\x80", ""    , ""         }, /* � */
    { "\xc3\x85"    , "\xc5", "\x8f", "\x81", ""    , ""         }, /* � */
    { "\xc3\x86"    , "\xc6", "\x92", "\xae", ""    , ""         }, /* � */
    { "\xc3\x87"    , "\xc7", "\x80", "\x82", ""    , ""         }, /* � */
    { "\xc3\x88"    , "\xc8", "\xd4", "\xe9", ""    , ""         }, /* � */
    { "\xc3\x89"    , "\xc9", "\x90", "\x83", ""    , ""         }, /* � */
    { "\xc3\x8a"    , "\xca", "\xd2", "\xe6", ""    , ""         }, /* � */
    { "\xc3\x8b"    , "\xcb", "\xd3", "\xe8", ""    , ""         }, /* � */
    { "\xc3\x8c"    , "\xcc", "\xde", "\xed", ""    , ""         }, /* � */
    { "\xc3\x8d"    , "\xcd", "\xd6", "\xea", ""    , ""         }, /* � */
    { "\xc3\x8e"    , "\xce", "\xd7", "\xeb", ""    , ""         }, /* � */
    { "\xc3\x8f"    , "\xcf", "\xd8", "\xec", ""    , ""         }, /* � */
    { "\xc3\x90"    , "\xd0", "\xd1", ""    , ""    , ""         }, /* � */
    { "\xc3\x91"    , "\xd1", "\xa5", "\x84", ""    , ""         }, /* � */
    { "\xc3\x92"    , "\xd2", "\xe3", "\xf1", ""    , ""         }, /* � */
    { "\xc3\x93"    , "\xd3", "\xe0", "\xee", ""    , ""         }, /* � */
    { "\xc3\x94"    , "\xd4", "\xe2", "\xef", ""    , ""         }, /* � */
    { "\xc3\x95"    , "\xd5", "\xe5", "\xcd", ""    , ""         }, /* � */
    { "\xc3\x96"    , "\xd6", "\x99", "\x85", ""    , ""         }, /* � */
    { "\xc3\x97"    , "\xd7", "\x9e", ""    , ""    , ""         }, /* � */
    { "\xc3\x98"    , "\xd8", "\x9d", "\xaf", ""    , ""         }, /* � */
    { "\xc3\x99"    , "\xd9", "\xeb", "\xf4", ""    , ""         }, /* � */
    { "\xc3\x9a"    , "\xda", "\xe9", "\xf2", ""    , ""         }, /* � */
    { "\xc3\x9b"    , "\xdb", "\xea", "\xf3", ""    , ""         }, /* � */
    { "\xc3\x9c"    , "\xdc", "\x9a", "\x86", ""    , ""         }, /* � */
    { "\xc3\x9d"    , "\xdd", "\xed", ""    , ""    , ""         }, /* � */
    { "\xc3\x9e"    , "\xde", "\xe7", ""    , ""    , ""         }, /* � */
    { "\xc3\x9f"    , "\xdf", "\xe1", "\xa7", ""    , ""         }, /* � */
    { "\xc3\xa0"    , "\xe0", "\x85", "\x88", "\x84", "\x16""Aa" }, /* � */
    { "\xc3\xa1"    , "\xe1", "\xa0", "\x87", "\x81", "\x16""Ba" }, /* � */
    { "\xc3\xa2"    , "\xe2", "\x83", "\x89", "\x82", "\x16""Ca" }, /* � */
    { "\xc3\xa3"    , "\xe3", "\xc6", "\x8b", ""    , ""         }, /* � */
    { "\xc3\xa4"    , "\xe4", "\x84", "\x8a", "\x83", "\x16""Ha" }, /* � */
    { "\xc3\xa5"    , "\xe5", "\x86", "\x8c", ""    , ""         }, /* � */
    { "\xc3\xa6"    , "\xe6", "\x91", "\xbe", ""    , ""         }, /* � */
    { "\xc3\xa7"    , "\xe7", "\x87", "\x8d", "\x80", "\x16""Kc" }, /* � */
    { "\xc3\xa8"    , "\xe8", "\x8a", "\x8f", "\x88", "\x16""Ae" }, /* � */
    { "\xc3\xa9"    , "\xe9", "\x82", "\x8e", "\x85", "\x16""Be" }, /* � */
    { "\xc3\xaa"    , "\xea", "\x88", "\x90", "\x86", "\x16""Ce" }, /* � */
    { "\xc3\xab"    , "\xeb", "\x89", "\x91", "\x87", "\x16""He" }, /* � */
    { "\xc3\xac"    , "\xec", "\x8d", "\x93", ""    , ""         }, /* � */
    { "\xc3\xad"    , "\xed", "\xa1", "\x92", ""    , ""         }, /* � */
    { "\xc3\xae"    , "\xee", "\x8c", "\x94", "\x89", "\x16""Ci" }, /* � */
    { "\xc3\xaf"    , "\xef", "\x8b", "\x95", "\x8a", "\x16""Hi" }, /* � */
    { "\xc3\xb0"    , "\xf0", "\xd0", ""    , ""    , ""         }, /* � */
    { "\xc3\xb1"    , "\xf1", "\xa4", "\x96", ""    , ""         }, /* � */
    { "\xc3\xb2"    , "\xf2", "\x95", "\x98", ""    , ""         }, /* � */
    { "\xc3\xb3"    , "\xf3", "\xa2", "\x97", ""    , ""         }, /* � */
    { "\xc3\xb4"    , "\xf4", "\x93", "\x99", "\x8b", "\x16""Co" }, /* � */
    { "\xc3\xb5"    , "\xf5", "\xe4", "\x9b", ""    , ""         }, /* � */
    { "\xc3\xb6"    , "\xf6", "\x94", "\x9a", "\x8c", "\x16""Ho" }, /* � */
    { "\xc3\xb7"    , "\xf7", "\xf6", "\xd6", ""    , ""         }, /* � */
    { "\xc3\xb8"    , "\xf8", "\x9b", "\xbf", ""    , ""         }, /* � */
    { "\xc3\xb9"    , "\xf9", "\x97", "\x9d", "\x8f", "\x16""Au" }, /* � */
    { "\xc3\xba"    , "\xfa", "\xa3", "\x9c", ""    , ""         }, /* � */
    { "\xc3\xbb"    , "\xfb", "\x96", "\x9e", "\x8d", "\x16""Cu" }, /* � */
    { "\xc3\xbc"    , "\xfc", "\x81", "\x9f", "\x8e", "\x16""Hu" }, /* � */
    { "\xc3\xbd"    , "\xfd", "\xec", ""    , ""    , ""         }, /* � */
    { "\xc3\xbe"    , "\xfe", "\xe8", ""    , ""    , ""         }, /* � */
    { "\xc3\xbf"    , "\xff", "\x98", "\xd8", ""    , ""         }, /* y+uml */
    { ""            , ""    , ""    , ""    , ""    , ""         }  /* -Fin- */
};

static int os_encode = ENCODING_WINDOWS;



/* get_extended_char_size:
 *  Rep�re la taille d'un caract�re �tendu (UTF-8 ou Unicode).
 */
static int get_extended_char_size (char *p)
{
    int add = 1;
    int mask;

    if ((p[0] >= '\xc2')
     && (p[1] < '\x00'))
    {
         /* UTF-8 2 octets */
        if ((p[0] < '\xe0')
         && (p[1] < '\xd8'))
        {
            add = 2;
        }
        else

        /* UTF-8 3 octets */
        if ((p[0] < '\xf0')
         && (p[1] < '\x00')
         && (p[2] < '\x00'))
        {
            switch ((int)p[0] & 0xf)
            {
                case 0x0 : mask = 0x20; break;
                case 0xd : mask = 0x10; break;
                default  : mask = 0x30; break;
            }

            if (((mask & (1 << (((uint)p[1] & 0xff) >> 5))) != 0)
             && (p[2] < '\xd8'))
            {
                add = 3;
            }
        }
        else

        /* UTF-8 4 octets */
        if ((p[0] <= '\xf4')
         && (p[1] < '\x00')
         && (p[2] < '\x00')
         && (p[3] < '\x00'))
        {
            switch (((uint)p[0]&0xff) >> 4)
            {
                case 0x9 :
                case 0xa :
                case 0xb : mask = 0x0f; break;
                case 0x8 : mask = 0x1e; break;
                default  : mask = 0x00; break;
            }

            if (((mask & (1<<(((uint)p[0]&0xff) & 7))) != 0)
             && (p[2] < '\xd8')
             && (p[3] < '\xd8'))
            {
                add = 4;
            }
        }
    }

    return add;
}



/* encode_char:
 *  Convertit le caract�re selon l'encodage.
 */
static char /*@dependent@*/*encode_char (int in, int out, char **p)
{
    int i;
    static char str[MAX_STRING+1];

    for (i = 0;
        (list[i][ENCODING_UTF8][0] != '\0')
        && ((list[i][in][0] == '\0')
         || (strncmp (*p, list[i][in], strlen (list[i][in])) != 0));
        i++)
    { }

    str[0] = '\0';
    if ((list[i][in][0] != '\0')
     && (list[i][out][0] != '\0'))
    {
        strcat (str, list[i][out]);
        *p += strlen (list[i][in]);
    }
    else
    {
        strncat (str, *p, 1);
        *p += 1;
    }

    return str;
}



/* encode_string:
 *  Convertit une cha�ne de caract�res selon l'encodage.
 */
static char /*@dependent@*/*encode_string (int in, int out, char *p)
{
    static char str[MAX_STRING+1];

    str[0] = '\0';
    while (*p != '\0')
    {
        strcat (str, encode_char (in, out, &p));
    }

    return str;
}



/* get_encoding_from:
 *  Fixe le type d'encodage d'un texte.
 */
static int get_encoding_from (int in, char *p)
{
    int add;

    while (*p != '\0')
    {
        add = 1;
        if (*p < '\0')
        {
            add = get_extended_char_size (p);
            if (add == 1)
            {
                in = os_encode;
            }
        }
        p += add;
    }
    return in;
}


/* ------------------------------------------------------------------------- */


/* encode_Get:
 *  Renvoie le type d'encodage d'une cha�ne de caract�res.
 */
int encode_Get (int in, char *str)
{
    return get_encoding_from (in, str);
}



/* encode_String:
 *  Renvoie une chaine de caract�res encod�e.
 */
char *encode_String (int in, int out, char *str)
{
    return encode_string (in, out, str);
}



/* encode_GetChar:
 *  Lit le caract�re courant.
 */
char *encode_GetChar (int in, char **p)
{
    return encode_char (in, in, p);
}



/* encode_GetCharToAsm:
 *  Lit et convertit un caract�re pour l'ASM.
 */
char *encode_GetCharToAsm (int in, char **p)
{
    return encode_char (in, ENCODING_ASM, p);
}



/* encode_GetCharToAsc:
 *  Lit et convertit un caract�re pour l'ASC.
 */
char *encode_GetCharToAsc (int in, char **p)
{
    return encode_char (in, ENCODING_ASC, p);
}



/* encode_Os:
 *  Initialise l'encodage de l'OS.
 */
void encode_Os (int encoding)
{
    os_encode = encoding;
}

