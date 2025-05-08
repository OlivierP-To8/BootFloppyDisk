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
#include "error.h"
#include "mark.h"
#include "assemble.h"
#include "directiv/end.h"
#include "directiv/opt.h"
#include "directiv/org.h"
#include "directiv/rmx.h"
#include "output/bin.h"

enum {
    BIN_TYPE_BIN = 0,
    BIN_TYPE_LINEAR,
    BIN_TYPE_HYBRID,
    BIN_TYPE_DATA
};

/* Ecriture */
static long  wbin_fpos = 0;
static int   wbin_addr = -1;
static int   wbin_type = BIN_TYPE_DATA;
static int   wbin_size = 0;
static int   wbin_err = 0;
static FILE  /*@dependent@*//*@null@*/*wbin_file = NULL;
static char  wbin_name[MAX_STRING+1];
/* Lecture INCBIN/INCDAT */
static int   rbin_type = BIN_TYPE_DATA;
static int   rbin_size = 0;
static int   rbin_err = 0;
static FILE  /*@dependent@*//*@null@*/*rbin_file = NULL;
static char  rbin_name[MAX_STRING+1];
static int   bin_flush_size = 0;

struct BIN_STRUCT bin;



/* wbin_error:
 *  Erreur d'écriture.
 */
static void wbin_error (void)
{
    wbin_err = error_Error ((is_fr)?"{+}%s: erreur d'écriture "
                                   :"{+}%s: write error", wbin_name);
}



/* wbin_ftell:
 *  Renvoie la position d'écriture.
 */
static long wbin_ftell (void)
{
    long fpos = 0;

    if ((wbin_err == 0)
     && (wbin_file != NULL)
     && ((fpos = ftell (wbin_file)) < 0))
    {
        wbin_error ();
    }
    return fpos;
}



/* wbin_fseek:
 *  Change la position d'écriture.
 */
static void wbin_fseek (long fpos)
{
    if ((wbin_err == 0)
     && (wbin_file != NULL)
     && (fseek (wbin_file, fpos, SEEK_SET) < 0))
    {
        wbin_error ();
    }
}


/* wbin_fputc:
 *  Ecrit une donnée.
 */
static void wbin_fputc (int c)
{
    if ((wbin_err == 0)
     && (wbin_file != NULL)
     && (fputc (c, wbin_file) == EOF))
    {
        wbin_error ();
    }
}



/* write_close_hunk:
 *  Ferme un hunk binaire Thomson.
 */
static void write_close_hunk (void)
{
    long fpos;

    if (wbin_type != BIN_TYPE_DATA)
    {
        fpos = wbin_ftell ();
        wbin_fseek (wbin_fpos);
        wbin_fputc ((wbin_size & 0xff00) / 256);
        wbin_fputc (wbin_size & 0xff);
        wbin_fseek (fpos);
    }

    wbin_size = 0;
}
        
        

/* write_open_hunk:
 *  Ouvre un hunk binaire ou un fichier de donnée Thomson.
 */
static void write_open_hunk (int flag, int addr)
{
    if (wbin_type != BIN_TYPE_DATA)
    {
        wbin_fputc (flag);
        wbin_fpos = wbin_ftell ();
        wbin_fputc (0);
        wbin_fputc (0);
        wbin_fputc ((addr & 0xff00) / 256);
        wbin_fputc (addr&0xff);
    }

    wbin_addr = (int)org_Get();
}



/* get_hunk_max_size:
 *  Retourne la taille maximum du hunk.
 */
static int get_hunk_max_size (void)
{
    return (wbin_type == BIN_TYPE_BIN) ? 0x80 :
           (wbin_type == BIN_TYPE_DATA) ? (int)(((uint)-1) / 2) :
           0xffff;
}



/* write_char:
 *  Ecrit une donnée.
 */
static void write_char (int i, int size)
{
    if ((assemble.pass == ASSEMBLE_PASS2)
     && (opt_GetUsr(OPT_NO) == 0))
    {
        if ((wbin_file == NULL)
         && (wbin_err == 0))
        {
            wbin_file = fopen (wbin_name, "w+b");
            wbin_err = (wbin_file == NULL) ? error_ErrnoFOpen (wbin_name) : 0;
        }

        if ((wbin_size == get_hunk_max_size ())
         || (((org_Get()+size)&0xffff) != ((wbin_addr+wbin_size)&0xffff)))
        {
            write_close_hunk ();
            if ((wbin_err == 0)
             && (wbin_file != NULL)
             && (wbin_addr != -1)
             && ((wbin_type == BIN_TYPE_DATA)
              || (wbin_type == BIN_TYPE_LINEAR)))
            {
                error_Warning ((is_fr) ? "{c}binaire non linéaire"
                                       : "{c}binary not linear");
            }

            write_open_hunk (0x00, org_Get()+size);
        }

        wbin_fputc ((int)bin.buf[i]);
        wbin_size += 1;
    }
}



/* rbin_fgetc.
 *  Lit une donnée.
 */
static int rbin_fgetc (void)
{
    int c = EOF;

    if (rbin_file != NULL)
    {
        if ((c = fgetc (rbin_file)) == EOF)
        {
            if ((feof (rbin_file) == 0)
             || (rbin_type == BIN_TYPE_BIN))
            {
                (void)error_Error ((is_fr)?"{+}%s: erreur de lecture"
                                          :"{+}%s: read error", rbin_name);
            }
            rbin_err = -1;
        }
    }

    return c;
}



/* read_open_hunk.
 *  Ouvre un hunk binaire ou un fichier de données.
 */
static void read_open_hunk (void)
{
    rbin_err = (rbin_fgetc () == 0) ? 0 : -1;
    rbin_size  = (rbin_fgetc () & 0xff) * 256;
    rbin_size |= rbin_fgetc () & 0xff;
    (void)rbin_fgetc ();
    (void)rbin_fgetc ();
}



/* read_char:
 *  Lit une donnée.
 */
static int read_char (void)
{
    int c = 0;

    if (rbin_file != NULL)
    {
        if (rbin_err == 0)
        {
            if ((rbin_type != BIN_TYPE_DATA)
             && (rbin_size == 0))
            {
                read_open_hunk ();
            }

            if (rbin_err == 0)
            {
                c = rbin_fgetc () & 0xff;
                rbin_size -= 1;
            }
        }
    }
    else
    {
        rbin_err = -1;
    }

    return (rbin_err == 0) ? c : rbin_err;
}


/* ------------------------------------------------------------------------- */


/* bin_ROpen:
 *  Ouvre un fichier (lecture).
 */
int bin_ROpen (char *name, char *extension)
{
    int err = 0;

    (void)snprintf (rbin_name, MAX_STRING, "%s", name);
    rbin_err = 0;
    rbin_type = (strcmp (extension, "BIN") == 0) ?BIN_TYPE_BIN :BIN_TYPE_DATA;
    rbin_file = fopen (rbin_name, "rb");
    err = (rbin_file == NULL) ? error_ErrnoFOpen (rbin_name) : 0;
    rbin_size = 0;

    return err;
}



/* bin_GetC:
 *  Lit un caractère.
 */
int bin_GetC (void)
{
    return read_char ();
}



/* bin_RClose:
 *  Ferme le fichier (lecture).
 */
void bin_RClose (void)
{
    if (rbin_file != NULL)
    {
        (void)fclose (rbin_file);
        rbin_file = NULL;
    }
}



/* bin_Open:
 *  Définit le nom du fichier de sortie.
 */
void bin_Open (char *name)
{
    (void)snprintf (wbin_name, MAX_STRING, "%s", name);
}



/* bin_Flush:
 *  Vide le buffer d'assemblage (écriture).
 */
void bin_Flush (void)
{
    int i;

    bin_flush_size = 0;
    if (bin.buf[0] != '\0')
    {
        write_char (0, bin_flush_size);
        bin_flush_size += 1;
    }

    for (i = 1; i <= bin.size; i++)
    {
        write_char (i, bin_flush_size);
        bin_flush_size += 1;
    }
}



/* bin_MoveOrg:
 *  Déplace l'adresse d'assemblage.
 */
void bin_MoveOrg (void)
{
    org_Set (org_Get() + (u16)bin_flush_size + rmx_GetSize ());
    mark_AddSize (bin_flush_size);
}



/* bin_PutC:
 *  Ecrit un caractère.
 */
void bin_PutC (char c)
{
    if (bin.size == bin.max)
    {
        bin_Flush ();
        bin_Clear ();
    }

    bin.buf[++bin.size] = c;
}



/* bin_WClose:
 *  Ferme le fichier (écriture)
 */
void bin_WClose (void)
{
    write_close_hunk ();

    if (wbin_type != BIN_TYPE_DATA)
    {
        write_open_hunk (0xff, (int)end_GetExec());
    }

    if (wbin_file != NULL)
    {
        (void)fclose (wbin_file);
        wbin_file = NULL;
    }
}



/* bin_Clear:
 *  Remet à 0 le module.
 */
void bin_Clear (void)
{
    bin.size = 0;
    bin.buf[0] = '\0';
}



/* bin_Reset:
 *  Initialise le module.
 */
void bin_Reset (void)
{
    bin.max = BIN_SIZE_MIN;
    bin_Clear ();
}



/* bin_SetNonLinearFile:
 *  Demande une sortie binaire non linéaire (BIN standard).
 */
void bin_SetNonLinearFile (void)
{
    wbin_type = BIN_TYPE_BIN;
}



/* bin_SetHybridFile:
 *  Demande une sortie binaire hybride (taille variable de hunk).
 */
void bin_SetHybridFile (void)
{
    wbin_type = BIN_TYPE_HYBRID;
}



/* bin_SetLinearFile:
 *  Demande une sortie binaire linéaire (un seul bloc).
 */
void bin_SetLinearFile (void)
{
    wbin_type = BIN_TYPE_LINEAR;
}



/* bin_SetDataFile:
 *  Demande une sortie de données (un seul bloc).
 */
void bin_SetDataFile (void)
{
    wbin_type = BIN_TYPE_DATA;
}

