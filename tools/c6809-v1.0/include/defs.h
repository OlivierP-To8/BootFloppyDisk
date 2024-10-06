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


#ifndef C6809_DEFS_H
#define C6809_DEFS_H 1

#ifndef NULL
#   define NULL 0
#endif

#ifndef MIN
#   define MIN(a,b)  (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#   define MAX(a,b)  (((a)>(b))?(a):(b))
#endif

#define PROG_VERSION_MAJOR "1"
#define PROG_VERSION_MINOR "0"
#define PROG_VERSION_MICRO "0"
#define PROG_CREATION_YEAR "2024"
#define PROG_CREATION_MONTH_EN "September"
#define PROG_CREATION_MONTH_FR "août"

typedef unsigned char uchar;
typedef unsigned int  uint;
#ifndef bool
#include <stdbool.h>
#endif


#define SIGNED(a)   (0-((a)&0x8000)+((a)&0x7fff))

#define MAX_STRING      300
#define MAX_ARG         40

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define debug_print(fmt, ...) \
        do { if (DEBUG_TEST) { \
          (void)fprintf (stdout, "%s:%d:%s(): "fmt, \
          __FILE__, __LINE__, __func__, __VA_ARGS__); \
          (void)fflush (stdout); } \
        } while (0)

extern bool is_fr;

#endif

