/*********************************************************************
*            (c) 1998 - 2019 Segger Microcontroller GmbH             *
*        Solutions for real time microcontroller applications        *
*                           www.segger.com                           *
**********************************************************************
*                                                                    *
* C-file generated by                                                *
*                                                                    *
*        Bitmap Converter for emWin (Demo version) V6.12.            *
*        Compiled Apr  8 2020, 10:19:28                              *
*                                                                    *
*        (c) 1998 - 2019 Segger Microcontroller GmbH                 *
*                                                                    *
*        May not be used in a product                                *
*                                                                    *
**********************************************************************
*                                                                    *
* Source file: example1_umbrella_text                                *
* Dimensions:  184 * 50                                              *
* NumColors:   8                                                     *
*                                                                    *
**********************************************************************
*/

#include <stdlib.h>

#include "GUI.h"

#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif

extern GUI_CONST_STORAGE GUI_BITMAP bmexample1_umbrella_text;

/*********************************************************************
*
*       Palette
*
*  Description
*    The following are the entries of the palette table.
*    The entries are stored as a 32-bit values of which 24 bits are
*    actually used according to the following bit mask: 0xBBGGRR
*
*    The lower   8 bits represent the Red   component.
*    The middle  8 bits represent the Green component.
*    The highest 8 bits represent the Blue  component.
*/
static GUI_CONST_STORAGE GUI_COLOR _Colorsexample1_umbrella_text[] = {
#if (GUI_USE_ARGB == 0)
  0x000000, 0x0000FF, 0x00FF00, 0x00FFFF,
  0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
#else
  0xFF000000, 0xFFFF0000, 0xFF00FF00, 0xFFFFFF00,
  0xFF0000FF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF
#endif

};

static GUI_CONST_STORAGE GUI_LOGPALETTE _Palexample1_umbrella_text = {
  8,  // Number of entries
  0,  // No transparency
  (const LCD_COLOR *)&_Colorsexample1_umbrella_text[0]
};

static GUI_CONST_STORAGE unsigned char _acexample1_umbrella_text[] = {
  /* RLE: 1668 Pixels @ 000,000 */ 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 144, 0x00, 
  /* RLE: 001 Pixels @ 012,009 */ 1, 0x07, 
  /* RLE: 011 Pixels @ 013,009 */ 11, 0x00, 
  /* ABS: 004 Pixels @ 024,009 */ 0, 4, 0x70, 0x07, 
  /* RLE: 009 Pixels @ 028,009 */ 9, 0x00, 
  /* RLE: 001 Pixels @ 037,009 */ 1, 0x07, 
  /* RLE: 009 Pixels @ 038,009 */ 9, 0x00, 
  /* RLE: 001 Pixels @ 047,009 */ 1, 0x07, 
  /* RLE: 006 Pixels @ 048,009 */ 6, 0x00, 
  /* RLE: 001 Pixels @ 054,009 */ 1, 0x07, 
  /* RLE: 023 Pixels @ 055,009 */ 23, 0x00, 
  /* RLE: 001 Pixels @ 078,009 */ 1, 0x07, 
  /* RLE: 033 Pixels @ 079,009 */ 33, 0x00, 
  /* RLE: 001 Pixels @ 112,009 */ 1, 0x07, 
  /* RLE: 019 Pixels @ 113,009 */ 19, 0x00, 
  /* ABS: 004 Pixels @ 132,009 */ 0, 4, 0x70, 0x07, 
  /* RLE: 019 Pixels @ 136,009 */ 19, 0x00, 
  /* RLE: 001 Pixels @ 155,009 */ 1, 0x07, 
  /* RLE: 037 Pixels @ 156,009 */ 37, 0x00, 
  /* RLE: 004 Pixels @ 009,010 */ 4, 0x07, 
  /* RLE: 011 Pixels @ 013,010 */ 11, 0x00, 
  /* RLE: 004 Pixels @ 024,010 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 028,010 */ 0, 2, 0x00, 
  /* RLE: 009 Pixels @ 030,010 */ 9, 0x07, 
  /* RLE: 008 Pixels @ 039,010 */ 8, 0x00, 
  /* RLE: 008 Pixels @ 047,010 */ 8, 0x07, 
  /* ABS: 002 Pixels @ 055,010 */ 0, 2, 0x00, 
  /* RLE: 017 Pixels @ 057,010 */ 17, 0x07, 
  /* RLE: 003 Pixels @ 074,010 */ 3, 0x00, 
  /* RLE: 018 Pixels @ 077,010 */ 18, 0x07, 
  /* ABS: 002 Pixels @ 095,010 */ 0, 2, 0x00, 
  /* RLE: 017 Pixels @ 097,010 */ 17, 0x07, 
  /* RLE: 001 Pixels @ 114,010 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 115,010 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,010 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,010 */ 5, 0x07, 
  /* RLE: 018 Pixels @ 136,010 */ 18, 0x00, 
  /* RLE: 008 Pixels @ 154,010 */ 8, 0x07, 
  /* RLE: 031 Pixels @ 162,010 */ 31, 0x00, 
  /* RLE: 004 Pixels @ 009,011 */ 4, 0x07, 
  /* RLE: 011 Pixels @ 013,011 */ 11, 0x00, 
  /* RLE: 004 Pixels @ 024,011 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 028,011 */ 0, 2, 0x00, 
  /* RLE: 009 Pixels @ 030,011 */ 9, 0x07, 
  /* RLE: 007 Pixels @ 039,011 */ 7, 0x00, 
  /* RLE: 009 Pixels @ 046,011 */ 9, 0x07, 
  /* ABS: 002 Pixels @ 055,011 */ 0, 2, 0x00, 
  /* RLE: 018 Pixels @ 057,011 */ 18, 0x07, 
  /* ABS: 002 Pixels @ 075,011 */ 0, 2, 0x00, 
  /* RLE: 018 Pixels @ 077,011 */ 18, 0x07, 
  /* ABS: 002 Pixels @ 095,011 */ 0, 2, 0x00, 
  /* RLE: 017 Pixels @ 097,011 */ 17, 0x07, 
  /* RLE: 001 Pixels @ 114,011 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 115,011 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,011 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,011 */ 5, 0x07, 
  /* RLE: 018 Pixels @ 136,011 */ 18, 0x00, 
  /* RLE: 009 Pixels @ 154,011 */ 9, 0x07, 
  /* RLE: 030 Pixels @ 163,011 */ 30, 0x00, 
  /* RLE: 004 Pixels @ 009,012 */ 4, 0x07, 
  /* RLE: 011 Pixels @ 013,012 */ 11, 0x00, 
  /* RLE: 004 Pixels @ 024,012 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 028,012 */ 0, 2, 0x00, 
  /* RLE: 010 Pixels @ 030,012 */ 10, 0x07, 
  /* RLE: 006 Pixels @ 040,012 */ 6, 0x00, 
  /* RLE: 009 Pixels @ 046,012 */ 9, 0x07, 
  /* ABS: 002 Pixels @ 055,012 */ 0, 2, 0x00, 
  /* RLE: 018 Pixels @ 057,012 */ 18, 0x07, 
  /* ABS: 002 Pixels @ 075,012 */ 0, 2, 0x00, 
  /* RLE: 019 Pixels @ 077,012 */ 19, 0x07, 
  /* RLE: 001 Pixels @ 096,012 */ 1, 0x00, 
  /* RLE: 017 Pixels @ 097,012 */ 17, 0x07, 
  /* RLE: 001 Pixels @ 114,012 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 115,012 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,012 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,012 */ 5, 0x07, 
  /* RLE: 017 Pixels @ 136,012 */ 17, 0x00, 
  /* RLE: 010 Pixels @ 153,012 */ 10, 0x07, 
  /* RLE: 030 Pixels @ 163,012 */ 30, 0x00, 
  /* RLE: 004 Pixels @ 009,013 */ 4, 0x07, 
  /* RLE: 011 Pixels @ 013,013 */ 11, 0x00, 
  /* RLE: 004 Pixels @ 024,013 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 028,013 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 030,013 */ 4, 0x07, 
  /* RLE: 001 Pixels @ 034,013 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 035,013 */ 5, 0x07, 
  /* RLE: 005 Pixels @ 040,013 */ 5, 0x00, 
  /* RLE: 005 Pixels @ 045,013 */ 5, 0x07, 
  /* RLE: 001 Pixels @ 050,013 */ 1, 0x00, 
  /* RLE: 004 Pixels @ 051,013 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,013 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 057,013 */ 5, 0x07, 
  /* RLE: 009 Pixels @ 062,013 */ 9, 0x00, 
  /* RLE: 004 Pixels @ 071,013 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 075,013 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 077,013 */ 5, 0x07, 
  /* RLE: 009 Pixels @ 082,013 */ 9, 0x00, 
  /* RLE: 005 Pixels @ 091,013 */ 5, 0x07, 
  /* RLE: 001 Pixels @ 096,013 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 097,013 */ 5, 0x07, 
  /* RLE: 013 Pixels @ 102,013 */ 13, 0x00, 
  /* RLE: 005 Pixels @ 115,013 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,013 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,013 */ 5, 0x07, 
  /* RLE: 017 Pixels @ 136,013 */ 17, 0x00, 
  /* RLE: 005 Pixels @ 153,013 */ 5, 0x07, 
  /* RLE: 001 Pixels @ 158,013 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 159,013 */ 5, 0x07, 
  /* RLE: 029 Pixels @ 164,013 */ 29, 0x00, 
  /* RLE: 004 Pixels @ 009,014 */ 4, 0x07, 
  /* RLE: 011 Pixels @ 013,014 */ 11, 0x00, 
  /* RLE: 004 Pixels @ 024,014 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 028,014 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 030,014 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 034,014 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 036,014 */ 5, 0x07, 
  /* RLE: 004 Pixels @ 041,014 */ 4, 0x00, 
  /* RLE: 004 Pixels @ 045,014 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 049,014 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 051,014 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,014 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 057,014 */ 4, 0x07, 
  /* RLE: 010 Pixels @ 061,014 */ 10, 0x00, 
  /* RLE: 004 Pixels @ 071,014 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 075,014 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 077,014 */ 5, 0x07, 
  /* RLE: 009 Pixels @ 082,014 */ 9, 0x00, 
  /* RLE: 005 Pixels @ 091,014 */ 5, 0x07, 
  /* RLE: 001 Pixels @ 096,014 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 097,014 */ 5, 0x07, 
  /* RLE: 013 Pixels @ 102,014 */ 13, 0x00, 
  /* RLE: 005 Pixels @ 115,014 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,014 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,014 */ 5, 0x07, 
  /* RLE: 016 Pixels @ 136,014 */ 16, 0x00, 
  /* RLE: 005 Pixels @ 152,014 */ 5, 0x07, 
  /* RLE: 003 Pixels @ 157,014 */ 3, 0x00, 
  /* RLE: 005 Pixels @ 160,014 */ 5, 0x07, 
  /* RLE: 028 Pixels @ 165,014 */ 28, 0x00, 
  /* RLE: 004 Pixels @ 009,015 */ 4, 0x07, 
  /* RLE: 011 Pixels @ 013,015 */ 11, 0x00, 
  /* RLE: 004 Pixels @ 024,015 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 028,015 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 030,015 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 034,015 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 036,015 */ 5, 0x07, 
  /* RLE: 003 Pixels @ 041,015 */ 3, 0x00, 
  /* RLE: 005 Pixels @ 044,015 */ 5, 0x07, 
  /* ABS: 002 Pixels @ 049,015 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 051,015 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,015 */ 0, 2, 0x00, 
  /* RLE: 018 Pixels @ 057,015 */ 18, 0x07, 
  /* ABS: 002 Pixels @ 075,015 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 077,015 */ 5, 0x07, 
  /* RLE: 009 Pixels @ 082,015 */ 9, 0x00, 
  /* RLE: 005 Pixels @ 091,015 */ 5, 0x07, 
  /* RLE: 001 Pixels @ 096,015 */ 1, 0x00, 
  /* RLE: 016 Pixels @ 097,015 */ 16, 0x07, 
  /* ABS: 002 Pixels @ 113,015 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 115,015 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,015 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,015 */ 5, 0x07, 
  /* RLE: 016 Pixels @ 136,015 */ 16, 0x00, 
  /* RLE: 005 Pixels @ 152,015 */ 5, 0x07, 
  /* RLE: 003 Pixels @ 157,015 */ 3, 0x00, 
  /* RLE: 005 Pixels @ 160,015 */ 5, 0x07, 
  /* RLE: 028 Pixels @ 165,015 */ 28, 0x00, 
  /* RLE: 004 Pixels @ 009,016 */ 4, 0x07, 
  /* RLE: 011 Pixels @ 013,016 */ 11, 0x00, 
  /* RLE: 004 Pixels @ 024,016 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 028,016 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 030,016 */ 4, 0x07, 
  /* RLE: 003 Pixels @ 034,016 */ 3, 0x00, 
  /* RLE: 005 Pixels @ 037,016 */ 5, 0x07, 
  /* ABS: 002 Pixels @ 042,016 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 044,016 */ 4, 0x07, 
  /* RLE: 003 Pixels @ 048,016 */ 3, 0x00, 
  /* RLE: 004 Pixels @ 051,016 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,016 */ 0, 2, 0x00, 
  /* RLE: 017 Pixels @ 057,016 */ 17, 0x07, 
  /* RLE: 003 Pixels @ 074,016 */ 3, 0x00, 
  /* RLE: 005 Pixels @ 077,016 */ 5, 0x07, 
  /* RLE: 008 Pixels @ 082,016 */ 8, 0x00, 
  /* RLE: 005 Pixels @ 090,016 */ 5, 0x07, 
  /* ABS: 002 Pixels @ 095,016 */ 0, 2, 0x00, 
  /* RLE: 016 Pixels @ 097,016 */ 16, 0x07, 
  /* ABS: 002 Pixels @ 113,016 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 115,016 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,016 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,016 */ 5, 0x07, 
  /* RLE: 015 Pixels @ 136,016 */ 15, 0x00, 
  /* RLE: 005 Pixels @ 151,016 */ 5, 0x07, 
  /* RLE: 005 Pixels @ 156,016 */ 5, 0x00, 
  /* RLE: 005 Pixels @ 161,016 */ 5, 0x07, 
  /* RLE: 027 Pixels @ 166,016 */ 27, 0x00, 
  /* RLE: 005 Pixels @ 009,017 */ 5, 0x07, 
  /* RLE: 010 Pixels @ 014,017 */ 10, 0x00, 
  /* RLE: 004 Pixels @ 024,017 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 028,017 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 030,017 */ 5, 0x07, 
  /* RLE: 003 Pixels @ 035,017 */ 3, 0x00, 
  /* RLE: 004 Pixels @ 038,017 */ 4, 0x07, 
  /* RLE: 001 Pixels @ 042,017 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 043,017 */ 5, 0x07, 
  /* RLE: 003 Pixels @ 048,017 */ 3, 0x00, 
  /* RLE: 004 Pixels @ 051,017 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,017 */ 0, 2, 0x00, 
  /* RLE: 018 Pixels @ 057,017 */ 18, 0x07, 
  /* ABS: 002 Pixels @ 075,017 */ 0, 2, 0x00, 
  /* RLE: 018 Pixels @ 077,017 */ 18, 0x07, 
  /* ABS: 002 Pixels @ 095,017 */ 0, 2, 0x00, 
  /* RLE: 016 Pixels @ 097,017 */ 16, 0x07, 
  /* ABS: 002 Pixels @ 113,017 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 115,017 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,017 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,017 */ 5, 0x07, 
  /* RLE: 015 Pixels @ 136,017 */ 15, 0x00, 
  /* RLE: 004 Pixels @ 151,017 */ 4, 0x07, 
  /* RLE: 006 Pixels @ 155,017 */ 6, 0x00, 
  /* RLE: 005 Pixels @ 161,017 */ 5, 0x07, 
  /* RLE: 027 Pixels @ 166,017 */ 27, 0x00, 
  /* RLE: 005 Pixels @ 009,018 */ 5, 0x07, 
  /* RLE: 010 Pixels @ 014,018 */ 10, 0x00, 
  /* RLE: 004 Pixels @ 024,018 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 028,018 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 030,018 */ 5, 0x07, 
  /* RLE: 003 Pixels @ 035,018 */ 3, 0x00, 
  /* RLE: 009 Pixels @ 038,018 */ 9, 0x07, 
  /* RLE: 004 Pixels @ 047,018 */ 4, 0x00, 
  /* RLE: 004 Pixels @ 051,018 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,018 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 057,018 */ 5, 0x07, 
  /* RLE: 009 Pixels @ 062,018 */ 9, 0x00, 
  /* RLE: 005 Pixels @ 071,018 */ 5, 0x07, 
  /* RLE: 001 Pixels @ 076,018 */ 1, 0x00, 
  /* RLE: 017 Pixels @ 077,018 */ 17, 0x07, 
  /* RLE: 003 Pixels @ 094,018 */ 3, 0x00, 
  /* RLE: 005 Pixels @ 097,018 */ 5, 0x07, 
  /* RLE: 013 Pixels @ 102,018 */ 13, 0x00, 
  /* RLE: 005 Pixels @ 115,018 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,018 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,018 */ 5, 0x07, 
  /* RLE: 014 Pixels @ 136,018 */ 14, 0x00, 
  /* RLE: 017 Pixels @ 150,018 */ 17, 0x07, 
  /* RLE: 026 Pixels @ 167,018 */ 26, 0x00, 
  /* RLE: 005 Pixels @ 009,019 */ 5, 0x07, 
  /* RLE: 009 Pixels @ 014,019 */ 9, 0x00, 
  /* RLE: 005 Pixels @ 023,019 */ 5, 0x07, 
  /* ABS: 002 Pixels @ 028,019 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 030,019 */ 5, 0x07, 
  /* RLE: 004 Pixels @ 035,019 */ 4, 0x00, 
  /* RLE: 008 Pixels @ 039,019 */ 8, 0x07, 
  /* RLE: 004 Pixels @ 047,019 */ 4, 0x00, 
  /* RLE: 004 Pixels @ 051,019 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,019 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 057,019 */ 4, 0x07, 
  /* RLE: 010 Pixels @ 061,019 */ 10, 0x00, 
  /* RLE: 005 Pixels @ 071,019 */ 5, 0x07, 
  /* RLE: 001 Pixels @ 076,019 */ 1, 0x00, 
  /* RLE: 018 Pixels @ 077,019 */ 18, 0x07, 
  /* ABS: 002 Pixels @ 095,019 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 097,019 */ 5, 0x07, 
  /* RLE: 013 Pixels @ 102,019 */ 13, 0x00, 
  /* RLE: 005 Pixels @ 115,019 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 120,019 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 131,019 */ 5, 0x07, 
  /* RLE: 013 Pixels @ 136,019 */ 13, 0x00, 
  /* RLE: 018 Pixels @ 149,019 */ 18, 0x07, 
  /* RLE: 026 Pixels @ 167,019 */ 26, 0x00, 
  /* RLE: 019 Pixels @ 009,020 */ 19, 0x07, 
  /* ABS: 002 Pixels @ 028,020 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 030,020 */ 5, 0x07, 
  /* RLE: 004 Pixels @ 035,020 */ 4, 0x00, 
  /* RLE: 007 Pixels @ 039,020 */ 7, 0x07, 
  /* RLE: 005 Pixels @ 046,020 */ 5, 0x00, 
  /* RLE: 004 Pixels @ 051,020 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,020 */ 0, 2, 0x00, 
  /* RLE: 019 Pixels @ 057,020 */ 19, 0x07, 
  /* RLE: 001 Pixels @ 076,020 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 077,020 */ 5, 0x07, 
  /* RLE: 008 Pixels @ 082,020 */ 8, 0x00, 
  /* RLE: 005 Pixels @ 090,020 */ 5, 0x07, 
  /* ABS: 002 Pixels @ 095,020 */ 0, 2, 0x00, 
  /* RLE: 016 Pixels @ 097,020 */ 16, 0x07, 
  /* ABS: 002 Pixels @ 113,020 */ 0, 2, 0x00, 
  /* RLE: 015 Pixels @ 115,020 */ 15, 0x07, 
  /* RLE: 001 Pixels @ 130,020 */ 1, 0x00, 
  /* RLE: 015 Pixels @ 131,020 */ 15, 0x07, 
  /* RLE: 003 Pixels @ 146,020 */ 3, 0x00, 
  /* RLE: 019 Pixels @ 149,020 */ 19, 0x07, 
  /* RLE: 025 Pixels @ 168,020 */ 25, 0x00, 
  /* RLE: 019 Pixels @ 009,021 */ 19, 0x07, 
  /* ABS: 002 Pixels @ 028,021 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 030,021 */ 5, 0x07, 
  /* RLE: 005 Pixels @ 035,021 */ 5, 0x00, 
  /* RLE: 006 Pixels @ 040,021 */ 6, 0x07, 
  /* RLE: 005 Pixels @ 046,021 */ 5, 0x00, 
  /* RLE: 004 Pixels @ 051,021 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,021 */ 0, 2, 0x00, 
  /* RLE: 019 Pixels @ 057,021 */ 19, 0x07, 
  /* RLE: 001 Pixels @ 076,021 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 077,021 */ 5, 0x07, 
  /* RLE: 009 Pixels @ 082,021 */ 9, 0x00, 
  /* RLE: 004 Pixels @ 091,021 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 095,021 */ 0, 2, 0x00, 
  /* RLE: 017 Pixels @ 097,021 */ 17, 0x07, 
  /* RLE: 001 Pixels @ 114,021 */ 1, 0x00, 
  /* RLE: 015 Pixels @ 115,021 */ 15, 0x07, 
  /* RLE: 001 Pixels @ 130,021 */ 1, 0x00, 
  /* RLE: 016 Pixels @ 131,021 */ 16, 0x07, 
  /* RLE: 001 Pixels @ 147,021 */ 1, 0x00, 
  /* RLE: 006 Pixels @ 148,021 */ 6, 0x07, 
  /* RLE: 009 Pixels @ 154,021 */ 9, 0x00, 
  /* RLE: 005 Pixels @ 163,021 */ 5, 0x07, 
  /* RLE: 026 Pixels @ 168,021 */ 26, 0x00, 
  /* RLE: 017 Pixels @ 010,022 */ 17, 0x07, 
  /* RLE: 003 Pixels @ 027,022 */ 3, 0x00, 
  /* RLE: 005 Pixels @ 030,022 */ 5, 0x07, 
  /* RLE: 005 Pixels @ 035,022 */ 5, 0x00, 
  /* RLE: 005 Pixels @ 040,022 */ 5, 0x07, 
  /* RLE: 006 Pixels @ 045,022 */ 6, 0x00, 
  /* RLE: 004 Pixels @ 051,022 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,022 */ 0, 2, 0x00, 
  /* RLE: 018 Pixels @ 057,022 */ 18, 0x07, 
  /* ABS: 002 Pixels @ 075,022 */ 0, 2, 0x00, 
  /* RLE: 005 Pixels @ 077,022 */ 5, 0x07, 
  /* RLE: 009 Pixels @ 082,022 */ 9, 0x00, 
  /* RLE: 005 Pixels @ 091,022 */ 5, 0x07, 
  /* RLE: 001 Pixels @ 096,022 */ 1, 0x00, 
  /* RLE: 017 Pixels @ 097,022 */ 17, 0x07, 
  /* RLE: 001 Pixels @ 114,022 */ 1, 0x00, 
  /* RLE: 015 Pixels @ 115,022 */ 15, 0x07, 
  /* RLE: 001 Pixels @ 130,022 */ 1, 0x00, 
  /* RLE: 016 Pixels @ 131,022 */ 16, 0x07, 
  /* RLE: 001 Pixels @ 147,022 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 148,022 */ 5, 0x07, 
  /* RLE: 011 Pixels @ 153,022 */ 11, 0x00, 
  /* RLE: 005 Pixels @ 164,022 */ 5, 0x07, 
  /* RLE: 026 Pixels @ 169,022 */ 26, 0x00, 
  /* RLE: 015 Pixels @ 011,023 */ 15, 0x07, 
  /* RLE: 004 Pixels @ 026,023 */ 4, 0x00, 
  /* RLE: 005 Pixels @ 030,023 */ 5, 0x07, 
  /* RLE: 006 Pixels @ 035,023 */ 6, 0x00, 
  /* RLE: 004 Pixels @ 041,023 */ 4, 0x07, 
  /* RLE: 006 Pixels @ 045,023 */ 6, 0x00, 
  /* RLE: 004 Pixels @ 051,023 */ 4, 0x07, 
  /* ABS: 002 Pixels @ 055,023 */ 0, 2, 0x00, 
  /* RLE: 017 Pixels @ 057,023 */ 17, 0x07, 
  /* RLE: 003 Pixels @ 074,023 */ 3, 0x00, 
  /* RLE: 005 Pixels @ 077,023 */ 5, 0x07, 
  /* RLE: 009 Pixels @ 082,023 */ 9, 0x00, 
  /* RLE: 005 Pixels @ 091,023 */ 5, 0x07, 
  /* RLE: 001 Pixels @ 096,023 */ 1, 0x00, 
  /* RLE: 017 Pixels @ 097,023 */ 17, 0x07, 
  /* RLE: 001 Pixels @ 114,023 */ 1, 0x00, 
  /* RLE: 015 Pixels @ 115,023 */ 15, 0x07, 
  /* RLE: 001 Pixels @ 130,023 */ 1, 0x00, 
  /* RLE: 015 Pixels @ 131,023 */ 15, 0x07, 
  /* RLE: 001 Pixels @ 146,023 */ 1, 0x00, 
  /* RLE: 005 Pixels @ 147,023 */ 5, 0x07, 
  /* RLE: 012 Pixels @ 152,023 */ 12, 0x00, 
  /* RLE: 005 Pixels @ 164,023 */ 5, 0x07, 
  /* RLE: 425 Pixels @ 169,023 */ 254, 0x00, 171, 0x00, 
  /* RLE: 010 Pixels @ 042,026 */ 10, 0x07, 
  /* RLE: 003 Pixels @ 052,026 */ 3, 0x00, 
  /* RLE: 010 Pixels @ 055,026 */ 10, 0x07, 
  /* RLE: 003 Pixels @ 065,026 */ 3, 0x00, 
  /* RLE: 010 Pixels @ 068,026 */ 10, 0x07, 
  /* RLE: 003 Pixels @ 078,026 */ 3, 0x00, 
  /* RLE: 009 Pixels @ 081,026 */ 9, 0x07, 
  /* RLE: 004 Pixels @ 090,026 */ 4, 0x00, 
  /* RLE: 010 Pixels @ 094,026 */ 10, 0x07, 
  /* RLE: 003 Pixels @ 104,026 */ 3, 0x00, 
  /* RLE: 011 Pixels @ 107,026 */ 11, 0x07, 
  /* RLE: 006 Pixels @ 118,026 */ 6, 0x00, 
  /* RLE: 004 Pixels @ 124,026 */ 4, 0x07, 
  /* RLE: 004 Pixels @ 128,026 */ 4, 0x00, 
  /* RLE: 011 Pixels @ 132,026 */ 11, 0x07, 
  /* ABS: 007 Pixels @ 143,026 */ 0, 7, 0x07, 0x77, 0x00, 0x00, 
  /* RLE: 010 Pixels @ 150,026 */ 10, 0x07, 
  /* ABS: 002 Pixels @ 160,026 */ 0, 2, 0x00, 
  /* RLE: 006 Pixels @ 162,026 */ 6, 0x07, 
  /* RLE: 005 Pixels @ 168,026 */ 5, 0x00, 
  /* ABS: 002 Pixels @ 173,026 */ 0, 2, 0x77, 
  /* RLE: 050 Pixels @ 175,026 */ 50, 0x00, 
  /* RLE: 011 Pixels @ 041,027 */ 11, 0x07, 
  /* ABS: 002 Pixels @ 052,027 */ 0, 2, 0x00, 
  /* RLE: 012 Pixels @ 054,027 */ 12, 0x07, 
  /* ABS: 002 Pixels @ 066,027 */ 0, 2, 0x00, 
  /* RLE: 011 Pixels @ 068,027 */ 11, 0x07, 
  /* ABS: 002 Pixels @ 079,027 */ 0, 2, 0x00, 
  /* RLE: 011 Pixels @ 081,027 */ 11, 0x07, 
  /* RLE: 001 Pixels @ 092,027 */ 1, 0x00, 
  /* RLE: 012 Pixels @ 093,027 */ 12, 0x07, 
  /* ABS: 002 Pixels @ 105,027 */ 0, 2, 0x00, 
  /* RLE: 012 Pixels @ 107,027 */ 12, 0x07, 
  /* RLE: 004 Pixels @ 119,027 */ 4, 0x00, 
  /* RLE: 006 Pixels @ 123,027 */ 6, 0x07, 
  /* RLE: 003 Pixels @ 129,027 */ 3, 0x00, 
  /* RLE: 011 Pixels @ 132,027 */ 11, 0x07, 
  /* ABS: 006 Pixels @ 143,027 */ 0, 6, 0x07, 0x77, 0x00, 
  /* RLE: 012 Pixels @ 149,027 */ 12, 0x07, 
  /* RLE: 001 Pixels @ 161,027 */ 1, 0x00, 
  /* RLE: 006 Pixels @ 162,027 */ 6, 0x07, 
  /* RLE: 005 Pixels @ 168,027 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 173,027 */ 3, 0x07, 
  /* RLE: 049 Pixels @ 176,027 */ 49, 0x00, 
  /* RLE: 012 Pixels @ 041,028 */ 12, 0x07, 
  /* RLE: 001 Pixels @ 053,028 */ 1, 0x00, 
  /* RLE: 013 Pixels @ 054,028 */ 13, 0x07, 
  /* RLE: 001 Pixels @ 067,028 */ 1, 0x00, 
  /* RLE: 012 Pixels @ 068,028 */ 12, 0x07, 
  /* RLE: 001 Pixels @ 080,028 */ 1, 0x00, 
  /* RLE: 011 Pixels @ 081,028 */ 11, 0x07, 
  /* RLE: 001 Pixels @ 092,028 */ 1, 0x00, 
  /* RLE: 013 Pixels @ 093,028 */ 13, 0x07, 
  /* RLE: 001 Pixels @ 106,028 */ 1, 0x00, 
  /* RLE: 012 Pixels @ 107,028 */ 12, 0x07, 
  /* RLE: 004 Pixels @ 119,028 */ 4, 0x00, 
  /* RLE: 007 Pixels @ 123,028 */ 7, 0x07, 
  /* ABS: 002 Pixels @ 130,028 */ 0, 2, 0x00, 
  /* RLE: 011 Pixels @ 132,028 */ 11, 0x07, 
  /* ABS: 005 Pixels @ 143,028 */ 0, 5, 0x07, 0x77, 0x00, 
  /* RLE: 013 Pixels @ 148,028 */ 13, 0x07, 
  /* RLE: 001 Pixels @ 161,028 */ 1, 0x00, 
  /* RLE: 007 Pixels @ 162,028 */ 7, 0x07, 
  /* RLE: 004 Pixels @ 169,028 */ 4, 0x00, 
  /* RLE: 003 Pixels @ 173,028 */ 3, 0x07, 
  /* RLE: 049 Pixels @ 176,028 */ 49, 0x00, 
  /* RLE: 003 Pixels @ 041,029 */ 3, 0x07, 
  /* RLE: 006 Pixels @ 044,029 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 050,029 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 053,029 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 057,029 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 064,029 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 067,029 */ 0, 4, 0x07, 0x77, 
  /* RLE: 006 Pixels @ 071,029 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 077,029 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 080,029 */ 0, 4, 0x07, 0x77, 
  /* RLE: 005 Pixels @ 084,029 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 089,029 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 092,029 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 096,029 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 103,029 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 106,029 */ 0, 4, 0x07, 0x77, 
  /* RLE: 006 Pixels @ 110,029 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 116,029 */ 3, 0x07, 
  /* RLE: 003 Pixels @ 119,029 */ 3, 0x00, 
  /* RLE: 003 Pixels @ 122,029 */ 3, 0x07, 
  /* ABS: 005 Pixels @ 125,029 */ 0, 5, 0x00, 0x77, 0x70, 
  /* RLE: 006 Pixels @ 130,029 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 136,029 */ 3, 0x07, 
  /* RLE: 005 Pixels @ 139,029 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 144,029 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 147,029 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 151,029 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 158,029 */ 3, 0x07, 
  /* ABS: 005 Pixels @ 161,029 */ 0, 5, 0x07, 0x77, 0x00, 
  /* RLE: 004 Pixels @ 166,029 */ 4, 0x07, 
  /* RLE: 003 Pixels @ 170,029 */ 3, 0x00, 
  /* RLE: 003 Pixels @ 173,029 */ 3, 0x07, 
  /* RLE: 049 Pixels @ 176,029 */ 49, 0x00, 
  /* RLE: 003 Pixels @ 041,030 */ 3, 0x07, 
  /* RLE: 010 Pixels @ 044,030 */ 10, 0x00, 
  /* RLE: 003 Pixels @ 054,030 */ 3, 0x07, 
  /* RLE: 007 Pixels @ 057,030 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 064,030 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 067,030 */ 0, 4, 0x07, 0x77, 
  /* RLE: 006 Pixels @ 071,030 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 077,030 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 080,030 */ 0, 4, 0x07, 0x77, 
  /* RLE: 005 Pixels @ 084,030 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 089,030 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 092,030 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 096,030 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 103,030 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 106,030 */ 0, 4, 0x07, 0x77, 
  /* RLE: 006 Pixels @ 110,030 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 116,030 */ 3, 0x07, 
  /* RLE: 003 Pixels @ 119,030 */ 3, 0x00, 
  /* RLE: 003 Pixels @ 122,030 */ 3, 0x07, 
  /* ABS: 002 Pixels @ 125,030 */ 0, 2, 0x00, 
  /* RLE: 004 Pixels @ 127,030 */ 4, 0x07, 
  /* RLE: 005 Pixels @ 131,030 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 136,030 */ 3, 0x07, 
  /* RLE: 005 Pixels @ 139,030 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 144,030 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 147,030 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 151,030 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 158,030 */ 3, 0x07, 
  /* ABS: 015 Pixels @ 161,030 */ 0, 15, 0x07, 0x77, 0x00, 0x77, 0x70, 0x00, 0x77, 0x70, 
  /* RLE: 049 Pixels @ 176,030 */ 49, 0x00, 
  /* RLE: 003 Pixels @ 041,031 */ 3, 0x07, 
  /* RLE: 010 Pixels @ 044,031 */ 10, 0x00, 
  /* RLE: 003 Pixels @ 054,031 */ 3, 0x07, 
  /* RLE: 007 Pixels @ 057,031 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 064,031 */ 3, 0x07, 
  /* RLE: 001 Pixels @ 067,031 */ 1, 0x00, 
  /* RLE: 011 Pixels @ 068,031 */ 11, 0x07, 
  /* ABS: 002 Pixels @ 079,031 */ 0, 2, 0x00, 
  /* RLE: 011 Pixels @ 081,031 */ 11, 0x07, 
  /* ABS: 004 Pixels @ 092,031 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 096,031 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 103,031 */ 3, 0x07, 
  /* RLE: 001 Pixels @ 106,031 */ 1, 0x00, 
  /* RLE: 011 Pixels @ 107,031 */ 11, 0x07, 
  /* RLE: 003 Pixels @ 118,031 */ 3, 0x00, 
  /* RLE: 003 Pixels @ 121,031 */ 3, 0x07, 
  /* RLE: 004 Pixels @ 124,031 */ 4, 0x00, 
  /* RLE: 003 Pixels @ 128,031 */ 3, 0x07, 
  /* RLE: 005 Pixels @ 131,031 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 136,031 */ 3, 0x07, 
  /* RLE: 005 Pixels @ 139,031 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 144,031 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 147,031 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 151,031 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 158,031 */ 3, 0x07, 
  /* ABS: 006 Pixels @ 161,031 */ 0, 6, 0x07, 0x77, 0x00, 
  /* RLE: 004 Pixels @ 167,031 */ 4, 0x07, 
  /* ABS: 005 Pixels @ 171,031 */ 0, 5, 0x00, 0x77, 0x70, 
  /* RLE: 049 Pixels @ 176,031 */ 49, 0x00, 
  /* RLE: 003 Pixels @ 041,032 */ 3, 0x07, 
  /* RLE: 006 Pixels @ 044,032 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 050,032 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 053,032 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 057,032 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 064,032 */ 3, 0x07, 
  /* RLE: 001 Pixels @ 067,032 */ 1, 0x00, 
  /* RLE: 011 Pixels @ 068,032 */ 11, 0x07, 
  /* ABS: 002 Pixels @ 079,032 */ 0, 2, 0x00, 
  /* RLE: 011 Pixels @ 081,032 */ 11, 0x07, 
  /* ABS: 004 Pixels @ 092,032 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 096,032 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 103,032 */ 3, 0x07, 
  /* RLE: 001 Pixels @ 106,032 */ 1, 0x00, 
  /* RLE: 011 Pixels @ 107,032 */ 11, 0x07, 
  /* RLE: 003 Pixels @ 118,032 */ 3, 0x00, 
  /* RLE: 011 Pixels @ 121,032 */ 11, 0x07, 
  /* RLE: 004 Pixels @ 132,032 */ 4, 0x00, 
  /* RLE: 003 Pixels @ 136,032 */ 3, 0x07, 
  /* RLE: 005 Pixels @ 139,032 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 144,032 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 147,032 */ 0, 4, 0x07, 0x77, 
  /* RLE: 007 Pixels @ 151,032 */ 7, 0x00, 
  /* RLE: 003 Pixels @ 158,032 */ 3, 0x07, 
  /* ABS: 007 Pixels @ 161,032 */ 0, 7, 0x07, 0x77, 0x00, 0x00, 
  /* RLE: 004 Pixels @ 168,032 */ 4, 0x07, 
  /* ABS: 004 Pixels @ 172,032 */ 0, 4, 0x07, 0x77, 
  /* RLE: 049 Pixels @ 176,032 */ 49, 0x00, 
  /* RLE: 003 Pixels @ 041,033 */ 3, 0x07, 
  /* RLE: 006 Pixels @ 044,033 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 050,033 */ 3, 0x07, 
  /* RLE: 001 Pixels @ 053,033 */ 1, 0x00, 
  /* RLE: 004 Pixels @ 054,033 */ 4, 0x07, 
  /* RLE: 005 Pixels @ 058,033 */ 5, 0x00, 
  /* RLE: 004 Pixels @ 063,033 */ 4, 0x07, 
  /* RLE: 001 Pixels @ 067,033 */ 1, 0x00, 
  /* RLE: 012 Pixels @ 068,033 */ 12, 0x07, 
  /* RLE: 001 Pixels @ 080,033 */ 1, 0x00, 
  /* RLE: 009 Pixels @ 081,033 */ 9, 0x07, 
  /* RLE: 003 Pixels @ 090,033 */ 3, 0x00, 
  /* RLE: 004 Pixels @ 093,033 */ 4, 0x07, 
  /* RLE: 005 Pixels @ 097,033 */ 5, 0x00, 
  /* RLE: 004 Pixels @ 102,033 */ 4, 0x07, 
  /* RLE: 001 Pixels @ 106,033 */ 1, 0x00, 
  /* RLE: 012 Pixels @ 107,033 */ 12, 0x07, 
  /* RLE: 001 Pixels @ 119,033 */ 1, 0x00, 
  /* RLE: 012 Pixels @ 120,033 */ 12, 0x07, 
  /* RLE: 004 Pixels @ 132,033 */ 4, 0x00, 
  /* RLE: 003 Pixels @ 136,033 */ 3, 0x07, 
  /* RLE: 005 Pixels @ 139,033 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 144,033 */ 3, 0x07, 
  /* RLE: 001 Pixels @ 147,033 */ 1, 0x00, 
  /* RLE: 004 Pixels @ 148,033 */ 4, 0x07, 
  /* RLE: 005 Pixels @ 152,033 */ 5, 0x00, 
  /* RLE: 004 Pixels @ 157,033 */ 4, 0x07, 
  /* ABS: 004 Pixels @ 161,033 */ 0, 4, 0x07, 0x77, 
  /* RLE: 004 Pixels @ 165,033 */ 4, 0x00, 
  /* RLE: 007 Pixels @ 169,033 */ 7, 0x07, 
  /* RLE: 049 Pixels @ 176,033 */ 49, 0x00, 
  /* RLE: 012 Pixels @ 041,034 */ 12, 0x07, 
  /* RLE: 001 Pixels @ 053,034 */ 1, 0x00, 
  /* RLE: 012 Pixels @ 054,034 */ 12, 0x07, 
  /* ABS: 005 Pixels @ 066,034 */ 0, 5, 0x00, 0x77, 0x70, 
  /* RLE: 006 Pixels @ 071,034 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 077,034 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 080,034 */ 0, 4, 0x07, 0x77, 
  /* RLE: 009 Pixels @ 084,034 */ 9, 0x00, 
  /* RLE: 013 Pixels @ 093,034 */ 13, 0x07, 
  /* ABS: 004 Pixels @ 106,034 */ 0, 4, 0x07, 0x77, 
  /* RLE: 006 Pixels @ 110,034 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 116,034 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 119,034 */ 0, 4, 0x07, 0x77, 
  /* RLE: 006 Pixels @ 123,034 */ 6, 0x00, 
  /* RLE: 004 Pixels @ 129,034 */ 4, 0x07, 
  /* RLE: 003 Pixels @ 133,034 */ 3, 0x00, 
  /* RLE: 003 Pixels @ 136,034 */ 3, 0x07, 
  /* RLE: 005 Pixels @ 139,034 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 144,034 */ 3, 0x07, 
  /* RLE: 001 Pixels @ 147,034 */ 1, 0x00, 
  /* RLE: 013 Pixels @ 148,034 */ 13, 0x07, 
  /* ABS: 004 Pixels @ 161,034 */ 0, 4, 0x07, 0x77, 
  /* RLE: 004 Pixels @ 165,034 */ 4, 0x00, 
  /* RLE: 007 Pixels @ 169,034 */ 7, 0x07, 
  /* RLE: 050 Pixels @ 176,034 */ 50, 0x00, 
  /* RLE: 010 Pixels @ 042,035 */ 10, 0x07, 
  /* RLE: 003 Pixels @ 052,035 */ 3, 0x00, 
  /* RLE: 011 Pixels @ 055,035 */ 11, 0x07, 
  /* ABS: 005 Pixels @ 066,035 */ 0, 5, 0x00, 0x77, 0x70, 
  /* RLE: 006 Pixels @ 071,035 */ 6, 0x00, 
  /* RLE: 003 Pixels @ 077,035 */ 3, 0x07, 
  /* ABS: 004 Pixels @ 080,035 */ 0, 4, 0x07, 0x77, 
  /* RLE: 010 Pixels @ 084,035 */ 10, 0x00, 
  /* RLE: 011 Pixels @ 094,035 */ 11, 0x07, 
  /* ABS: 005 Pixels @ 105,035 */ 0, 5, 0x00, 0x77, 0x70, 
  /* RLE: 006 Pixels @ 110,035 */ 6, 0x00, 
  /* RLE: 006 Pixels @ 116,035 */ 6, 0x07, 
  /* RLE: 008 Pixels @ 122,035 */ 8, 0x00, 
  /* RLE: 003 Pixels @ 130,035 */ 3, 0x07, 
  /* RLE: 003 Pixels @ 133,035 */ 3, 0x00, 
  /* RLE: 003 Pixels @ 136,035 */ 3, 0x07, 
  /* RLE: 005 Pixels @ 139,035 */ 5, 0x00, 
  /* RLE: 003 Pixels @ 144,035 */ 3, 0x07, 
  /* ABS: 002 Pixels @ 147,035 */ 0, 2, 0x00, 
  /* RLE: 011 Pixels @ 149,035 */ 11, 0x07, 
  /* ABS: 005 Pixels @ 160,035 */ 0, 5, 0x00, 0x77, 0x70, 
  /* RLE: 005 Pixels @ 165,035 */ 5, 0x00, 
  /* RLE: 006 Pixels @ 170,035 */ 6, 0x07, 
  /* RLE: 2584 Pixels @ 176,035 */ 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 254, 0x00, 44, 0x00, 
  0
};  // 1448 bytes for 9200 pixels

GUI_CONST_STORAGE GUI_BITMAP bmexample1_umbrella_text = {
  184, // xSize
  50, // ySize
  92, // BytesPerLine
  GUI_COMPRESS_RLE4, // BitsPerPixel
  (const unsigned char *)_acexample1_umbrella_text,  // Pointer to picture data (indices)
  &_Palexample1_umbrella_text,  // Pointer to palette
  GUI_DRAW_RLE4
};

/*************************** End of file ****************************/
