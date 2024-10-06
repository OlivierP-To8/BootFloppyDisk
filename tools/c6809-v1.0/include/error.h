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

#ifndef C6809_ERROR_H
#define C6809_ERROR_H 1

extern void  error_Clear (void);
extern void  error_Close (void);
extern void  error_Print (void);
extern void  error_PrintForced (void);
extern void  error_Optimize (const char *format, ...);
extern void  error_Assembler (const char *format, ...);
extern void  error_Warning (const char *format, ...);
extern int   error_Error (const char *format, ...);
extern int   error_Fatal (const char *format, ...);
extern int   error_IsFatal (void);
extern void  error_DirectiveNotSupported (int flag);
extern void  error_LabelNotSupported (void);
extern int   error_MissingRegister (void);
extern int   error_BadRegister (void);
extern int   error_BadSeparator (void);
extern int   error_BadAddressingMode (void);
extern int   error_BadCharacter (void);
extern int   error_BadDp (void);
extern int   error_MissingInformation (void);
extern int   error_OperandOutOfRange (void);
extern int   error_MissingOperand (void);
extern int   error_Memory (char *file, int line, const char *func);
extern int   error_Internal (char *file, int line, const char *func);
extern int   error_ErrnoFOpen (char *name);

extern void  error_SetVerboseMin (void);
extern void  error_SetVerboseMid (void);
extern void  error_SetVerboseMax (void);

#endif

