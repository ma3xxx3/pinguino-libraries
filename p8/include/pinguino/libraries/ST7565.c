/*
$Id:$

Ported to Pinguino by Samuel Ureta

ST7565 LCD library for Arduino

Copyright (C) 2010 Limor Fried, Adafruit Industries

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Some of this code was written by <cstone@pobox.com> originally; it is in the public domain.
*/

/**
 *  CHANGELOG :
 *      RB - Mar. 2014 - Renamed all function names to ST7565_xxxx()
 *  
 *  TODO :
 *      Add print, printf, printNumber, printFloat, setFont, ...
 *      see : SSD1306, PCD8544, lcdlib, ...
 **/

#include <ST7565.h>
#include <stdlib.h>
//#include <stdint.h>
#include <string.h>         // memset
#include <typedef.h>        // u8
#include <macro.h>          // swap
#include <digitalw.c>       // pinmode, digitalwrite
#include <delayms.c>        // Delayms

#define PROGMEM /* empty */
#define pgm_read_byte(x) (*((char *)x))
#define ST7565_STARTBYTES 1
#define _BV(bit) (1 << (bit))
#define  LSBFIRST   0
#define  MSBFIRST   1
#define  abs(x) (x>=0)?x:-x

u8 is_reversed = 0;
u8 sid, sclk, dc, rst, cs;

// a handy reference to where the pages are on the screen
const u8 pagemap[] = { 3, 2, 1, 0, 7, 6, 5, 4 };

// a 5x7 font table.
const u8 font[1275] = { 
  0x0, 0x0, 0x0, 0x0, 0x0,       // Ascii 0
  0x7C, 0xDA, 0xF2, 0xDA, 0x7C,  //ASC(01)
  0x7C, 0xD6, 0xF2, 0xD6, 0x7C,  //ASC(02)
  0x38, 0x7C, 0x3E, 0x7C, 0x38, 
  0x18, 0x3C, 0x7E, 0x3C, 0x18, 
  0x38, 0xEA, 0xBE, 0xEA, 0x38, 
  0x38, 0x7A, 0xFE, 0x7A, 0x38, 
  0x0, 0x18, 0x3C, 0x18, 0x0, 
  0xFF, 0xE7, 0xC3, 0xE7, 0xFF, 
  0x0, 0x18, 0x24, 0x18, 0x0, 
  0xFF, 0xE7, 0xDB, 0xE7, 0xFF, 
  0xC, 0x12, 0x5C, 0x60, 0x70, 
  0x64, 0x94, 0x9E, 0x94, 0x64, 
  0x2, 0xFE, 0xA0, 0xA0, 0xE0, 
  0x2, 0xFE, 0xA0, 0xA4, 0xFC, 
  0x5A, 0x3C, 0xE7, 0x3C, 0x5A, 
  0xFE, 0x7C, 0x38, 0x38, 0x10, 
  0x10, 0x38, 0x38, 0x7C, 0xFE, 
  0x28, 0x44, 0xFE, 0x44, 0x28, 
  0xFA, 0xFA, 0x0, 0xFA, 0xFA, 
  0x60, 0x90, 0xFE, 0x80, 0xFE, 
  0x0, 0x66, 0x91, 0xA9, 0x56, 
  0x6, 0x6, 0x6, 0x6, 0x6,
  0x29, 0x45, 0xFF, 0x45, 0x29, 
  0x10, 0x20, 0x7E, 0x20, 0x10, 
  0x8, 0x4, 0x7E, 0x4, 0x8, 
  0x10, 0x10, 0x54, 0x38, 0x10, 
  0x10, 0x38, 0x54, 0x10, 0x10, 
  0x78, 0x8, 0x8, 0x8, 0x8, 
  0x30, 0x78, 0x30, 0x78, 0x30, 
  0xC, 0x1C, 0x7C, 0x1C, 0xC, 
  0x60, 0x70, 0x7C, 0x70, 0x60, 
  0x0, 0x0, 0x0, 0x0, 0x0, 
  0x0, 0x0, 0xFA, 0x0, 0x0, 
  0x0, 0xE0, 0x0, 0xE0, 0x0, 
  0x28, 0xFE, 0x28, 0xFE, 0x28, 
  0x24, 0x54, 0xFE, 0x54, 0x48, 
  0xC4, 0xC8, 0x10, 0x26, 0x46, 
  0x6C, 0x92, 0x6A, 0x4, 0xA, 
  0x0, 0x10, 0xE0, 0xC0, 0x0, 
  0x0, 0x38, 0x44, 0x82, 0x0, 
  0x0, 0x82, 0x44, 0x38, 0x0, 
  0x54, 0x38, 0xFE, 0x38, 0x54, 
  0x10, 0x10, 0x7C, 0x10, 0x10, 
  0x0, 0x1, 0xE, 0xC, 0x0, 
  0x10, 0x10, 0x10, 0x10, 0x10, 
  0x0, 0x0, 0x6, 0x6, 0x0, 
  0x4, 0x8, 0x10, 0x20, 0x40, 
  0x7C, 0x8A, 0x92, 0xA2, 0x7C, 
  0x0, 0x42, 0xFE, 0x2, 0x0, 
  0x4E, 0x92, 0x92, 0x92, 0x62, 
  0x84, 0x82, 0x92, 0xB2, 0xCC, 
  0x18, 0x28, 0x48, 0xFE, 0x8, 
  0xE4, 0xA2, 0xA2, 0xA2, 0x9C, 
  0x3C, 0x52, 0x92, 0x92, 0x8C, 
  0x82, 0x84, 0x88, 0x90, 0xE0, 
  0x6C, 0x92, 0x92, 0x92, 0x6C, 
  0x62, 0x92, 0x92, 0x94, 0x78, 
  0x0, 0x0, 0x28, 0x0, 0x0, 
  0x0, 0x2, 0x2C, 0x0, 0x0, 
  0x0, 0x10, 0x28, 0x44, 0x82, 
  0x28, 0x28, 0x28, 0x28, 0x28, 
  0x0, 0x82, 0x44, 0x28, 0x10, 
  0x40, 0x80, 0x9A, 0x90, 0x60, 
  0x7C, 0x82, 0xBA, 0x9A, 0x72, 
  0x3E, 0x48, 0x88, 0x48, 0x3E, 
  0xFE, 0x92, 0x92, 0x92, 0x6C, 
  0x7C, 0x82, 0x82, 0x82, 0x44, 
  0xFE, 0x82, 0x82, 0x82, 0x7C, 
  0xFE, 0x92, 0x92, 0x92, 0x82, 
  0xFE, 0x90, 0x90, 0x90, 0x80, 
  0x7C, 0x82, 0x82, 0x8A, 0xCE, 
  0xFE, 0x10, 0x10, 0x10, 0xFE, 
  0x0, 0x82, 0xFE, 0x82, 0x0, 
  0x4, 0x2, 0x82, 0xFC, 0x80, 
  0xFE, 0x10, 0x28, 0x44, 0x82, 
  0xFE, 0x2, 0x2, 0x2, 0x2, 
  0xFE, 0x40, 0x38, 0x40, 0xFE, 
  0xFE, 0x20, 0x10, 0x8, 0xFE, 
  0x7C, 0x82, 0x82, 0x82, 0x7C, 
  0xFE, 0x90, 0x90, 0x90, 0x60, 
  0x7C, 0x82, 0x8A, 0x84, 0x7A, 
  0xFE, 0x90, 0x98, 0x94, 0x62, 
  0x64, 0x92, 0x92, 0x92, 0x4C, 
  0xC0, 0x80, 0xFE, 0x80, 0xC0, 
  0xFC, 0x2, 0x2, 0x2, 0xFC, 
  0xF8, 0x4, 0x2, 0x4, 0xF8, 
  0xFC, 0x2, 0x1C, 0x2, 0xFC, 
  0xC6, 0x28, 0x10, 0x28, 0xC6, 
  0xC0, 0x20, 0x1E, 0x20, 0xC0, 
  0x86, 0x9A, 0x92, 0xB2, 0xC2, 
  0x0, 0xFE, 0x82, 0x82, 0x82, 
  0x40, 0x20, 0x10, 0x8, 0x4, 
  0x0, 0x82, 0x82, 0x82, 0xFE, 
  0x20, 0x40, 0x80, 0x40, 0x20, 
  0x2, 0x2, 0x2, 0x2, 0x2, 
  0x0, 0xC0, 0xE0, 0x10, 0x0, 
  0x4, 0x2A, 0x2A, 0x1E, 0x2, 
  0xFE, 0x14, 0x22, 0x22, 0x1C, 
  0x1C, 0x22, 0x22, 0x22, 0x14, 
  0x1C, 0x22, 0x22, 0x14, 0xFE, 
  0x1C, 0x2A, 0x2A, 0x2A, 0x18, 
  0x0, 0x10, 0x7E, 0x90, 0x40, 
  0x18, 0x25, 0x25, 0x39, 0x1E, 
  0xFE, 0x10, 0x20, 0x20, 0x1E, 
  0x0, 0x22, 0xBE, 0x2, 0x0, 
  0x4, 0x2, 0x2, 0xBC, 0x0, 
  0xFE, 0x8, 0x14, 0x22, 0x0, 
  0x0, 0x82, 0xFE, 0x2, 0x0, 
  0x3E, 0x20, 0x1E, 0x20, 0x1E, 
  0x3E, 0x10, 0x20, 0x20, 0x1E, 
  0x1C, 0x22, 0x22, 0x22, 0x1C, 
  0x3F, 0x18, 0x24, 0x24, 0x18, 
  0x18, 0x24, 0x24, 0x18, 0x3F, 
  0x3E, 0x10, 0x20, 0x20, 0x10, 
  0x12, 0x2A, 0x2A, 0x2A, 0x24, 
  0x20, 0x20, 0xFC, 0x22, 0x24, 
  0x3C, 0x2, 0x2, 0x4, 0x3E, 
  0x38, 0x4, 0x2, 0x4, 0x38, 
  0x3C, 0x2, 0xC, 0x2, 0x3C, 
  0x22, 0x14, 0x8, 0x14, 0x22, 
  0x32, 0x9, 0x9, 0x9, 0x3E, 
  0x22, 0x26, 0x2A, 0x32, 0x22, 
  0x0, 0x10, 0x6C, 0x82, 0x0, 
  0x0, 0x0, 0xEE, 0x0, 0x0, 
  0x0, 0x82, 0x6C, 0x10, 0x0, 
  0x40, 0x80, 0x40, 0x20, 0x40, 
  0x3C, 0x64, 0xC4, 0x64, 0x3C, 
  0x78, 0x85, 0x85, 0x86, 0x48, 
  0x5C, 0x2, 0x2, 0x4, 0x5E, 
  0x1C, 0x2A, 0x2A, 0xAA, 0x9A, 
  0x84, 0xAA, 0xAA, 0x9E, 0x82, 
  0x84, 0x2A, 0x2A, 0x1E, 0x82, 
  0x84, 0xAA, 0x2A, 0x1E, 0x2, 
  0x4, 0x2A, 0xAA, 0x9E, 0x2, 
  0x30, 0x78, 0x4A, 0x4E, 0x48, 
  0x9C, 0xAA, 0xAA, 0xAA, 0x9A, 
  0x9C, 0x2A, 0x2A, 0x2A, 0x9A, 
  0x9C, 0xAA, 0x2A, 0x2A, 0x1A, 
  0x0, 0x0, 0xA2, 0x3E, 0x82, 
  0x0, 0x40, 0xA2, 0xBE, 0x42, 
  0x0, 0x80, 0xA2, 0x3E, 0x2, 
  0xF, 0x94, 0x24, 0x94, 0xF, 
  0xF, 0x14, 0xA4, 0x14, 0xF, 
  0x3E, 0x2A, 0xAA, 0xA2, 0x0, 
  0x4, 0x2A, 0x2A, 0x3E, 0x2A,
  0x3E, 0x50, 0x90, 0xFE, 0x92, 
  0x4C, 0x92, 0x92, 0x92, 0x4C, 
  0x4C, 0x12, 0x12, 0x12, 0x4C, 
  0x4C, 0x52, 0x12, 0x12, 0xC, 
  0x5C, 0x82, 0x82, 0x84, 0x5E, 
  0x5C, 0x42, 0x2, 0x4, 0x1E, 
  0x0, 0xB9, 0x5, 0x5, 0xBE, 
  0x9C, 0x22, 0x22, 0x22, 0x9C, 
  0xBC, 0x2, 0x2, 0x2, 0xBC, 
  0x3C, 0x24, 0xFF, 0x24, 0x24, 
  0x12, 0x7E, 0x92, 0xC2, 0x66, 
  0xD4, 0xF4, 0x3F, 0xF4, 0xD4, 
  0xFF, 0x90, 0x94, 0x6F, 0x4, 
  0x3, 0x11, 0x7E, 0x90, 0xC0, 
  0x4, 0x2A, 0x2A, 0x9E, 0x82, 
  0x0, 0x0, 0x22, 0xBE, 0x82, 
  0xC, 0x12, 0x12, 0x52, 0x4C, 
  0x1C, 0x2, 0x2, 0x44, 0x5E, 
  0x0, 0x5E, 0x50, 0x50, 0x4E, 
  0xBE, 0xB0, 0x98, 0x8C, 0xBE, 
  0x64, 0x94, 0x94, 0xF4, 0x14, 
  0x64, 0x94, 0x94, 0x94, 0x64, 
  0xC, 0x12, 0xB2, 0x2, 0x4, 
  0x1C, 0x10, 0x10, 0x10, 0x10, 
  0x10, 0x10, 0x10, 0x10, 0x1C, 
  0xF4, 0x8, 0x13, 0x35, 0x5D, 
  0xF4, 0x8, 0x14, 0x2C, 0x5F, 
  0x0, 0x0, 0xDE, 0x0, 0x0, 
  0x10, 0x28, 0x54, 0x28, 0x44, 
  0x44, 0x28, 0x54, 0x28, 0x10, 
  0x55, 0x0, 0xAA, 0x0, 0x55, 
  0x55, 0xAA, 0x55, 0xAA, 0x55, 
  0xAA, 0x55, 0xAA, 0x55, 0xAA,
  0x0, 0x0, 0x0, 0xFF, 0x0, 
  0x8, 0x8, 0x8, 0xFF, 0x0, 
  0x28, 0x28, 0x28, 0xFF, 0x0, 
  0x8, 0x8, 0xFF, 0x0, 0xFF, 
  0x8, 0x8, 0xF, 0x8, 0xF, 
  0x28, 0x28, 0x28, 0x3F, 0x0, 
  0x28, 0x28, 0xEF, 0x0, 0xFF, 
  0x0, 0x0, 0xFF, 0x0, 0xFF, 
  0x28, 0x28, 0x2F, 0x20, 0x3F, 
  0x28, 0x28, 0xE8, 0x8, 0xF8, 
  0x8, 0x8, 0xF8, 0x8, 0xF8, 
  0x28, 0x28, 0x28, 0xF8, 0x0, 
  0x8, 0x8, 0x8, 0xF, 0x0, 
  0x0, 0x0, 0x0, 0xF8, 0x8,
  0x8, 0x8, 0x8, 0xF8, 0x8,
  0x8, 0x8, 0x8, 0xF, 0x8,
  0x0, 0x0, 0x0, 0xFF, 0x8,
  0x8, 0x8, 0x8, 0x8, 0x8,
  0x8, 0x8, 0x8, 0xFF, 0x8,
  0x0, 0x0, 0x0, 0xFF, 0x28,
  0x0, 0x0, 0xFF, 0x0, 0xFF,
  0x0, 0x0, 0xF8, 0x8, 0xE8,
  0x0, 0x0, 0x3F, 0x20, 0x2F,
  0x28, 0x28, 0xE8, 0x8, 0xE8,
  0x28, 0x28, 0x2F, 0x20, 0x2F,
  0x0, 0x0, 0xFF, 0x0, 0xEF,
  0x28, 0x28, 0x28, 0x28, 0x28,
  0x28, 0x28, 0xEF, 0x0, 0xEF,
  0x28, 0x28, 0x28, 0xE8, 0x28,
  0x8, 0x8, 0xF8, 0x8, 0xF8,
  0x28, 0x28, 0x28, 0x2F, 0x28,
  0x8, 0x8, 0xF, 0x8, 0xF,
  0x0, 0x0, 0xF8, 0x8, 0xF8,
  0x0, 0x0, 0x0, 0xF8, 0x28,
  0x0, 0x0, 0x0, 0x3F, 0x28,
  0x0, 0x0, 0xF, 0x8, 0xF,
  0x8, 0x8, 0xFF, 0x8, 0xFF,
  0x28, 0x28, 0x28, 0xFF, 0x28,
  0x8, 0x8, 0x8, 0xF8, 0x0,
  0x0, 0x0, 0x0, 0xF, 0x8,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xF, 0xF, 0xF, 0xF, 0xF,
  0xFF, 0xFF, 0xFF, 0x0, 0x0, 
  0x0, 0x0, 0x0, 0xFF, 0xFF,
  0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
  0x1C, 0x22, 0x22, 0x1C, 0x22, 
  0x3E, 0x54, 0x54, 0x7C, 0x28, 
  0x7E, 0x40, 0x40, 0x60, 0x60, 
  0x40, 0x7E, 0x40, 0x7E, 0x40, 
  0xC6, 0xAA, 0x92, 0x82, 0xC6, 
  0x1C, 0x22, 0x22, 0x3C, 0x20, 
  0x2, 0x7E, 0x4, 0x78, 0x4, 
  0x60, 0x40, 0x7E, 0x40, 0x40, 
  0x99, 0xA5, 0xE7, 0xA5, 0x99, 
  0x38, 0x54, 0x92, 0x54, 0x38, 
  0x32, 0x4E, 0x80, 0x4E, 0x32, 
  0xC, 0x52, 0xB2, 0xB2, 0xC, 
  0xC, 0x12, 0x1E, 0x12, 0xC, 
  0x3D, 0x46, 0x5A, 0x62, 0xBC, 
  0x7C, 0x92, 0x92, 0x92, 0x0, 
  0x7E, 0x80, 0x80, 0x80, 0x7E, 
  0x54, 0x54, 0x54, 0x54, 0x54, 
  0x22, 0x22, 0xFA, 0x22, 0x22, 
  0x2, 0x8A, 0x52, 0x22, 0x2, 
  0x2, 0x22, 0x52, 0x8A, 0x2, 
  0x0, 0x0, 0xFF, 0x80, 0xC0, 
  0x7, 0x1, 0xFF, 0x0, 0x0, 
  0x10, 0x10, 0xD6, 0xD6, 0x10,
  0x6C, 0x48, 0x6C, 0x24, 0x6C, 
  0x60, 0xF0, 0x90, 0xF0, 0x60, 
  0x0, 0x0, 0x18, 0x18, 0x0, 
  0x0, 0x0, 0x8, 0x8, 0x0, 
  0xC, 0x2, 0xFF, 0x80, 0x80, 
  0x0, 0xF8, 0x80, 0x80, 0x78, 
  0x0, 0x98, 0xB8, 0xE8, 0x48, 
  0x0, 0x3C, 0x3C, 0x3C, 0x3C,};

// Now the memory buffer for the LCD.
// For PICs with databank divided in blocks of 256 bytes, you have to modify the lkr (linker) script 
// in a way like this: (example with 18F2550)
//
// Before: 
// ACCESSBANK	NAME=accessram  START=0x0            END=0x5F          PROTECTED
// DATABANK		NAME=gpr0       START=0x60           END=0xFF          PROTECTED
// DATABANK		NAME=gpr1       START=0x100          END=0x3FF
// DATABANK		NAME=gpr2       START=0x200          END=0x2FF
// DATABANK		NAME=gpr3       START=0x300          END=0x3FF
// DATABANK		NAME=usb4       START=0x400          END=0x4FF          PROTECTED
// DATABANK		NAME=usb5       START=0x500          END=0x5FF          PROTECTED
// DATABANK		NAME=usb6       START=0x600          END=0x6FF          PROTECTED
// DATABANK		NAME=usb7       START=0x700          END=0x7FF          PROTECTED
// ACCESSBANK	NAME=accesssfr  START=0xF60          END=0xFFF          PROTECTED
//
// After:
// ACCESSBANK	NAME=accessram  START=0x0            END=0x5F 			PROTECTED
// DATABANK		NAME=gpr0       START=0x60           END=0xFF 			PROTECTED
// DATABANK		NAME=gpr1       START=0x100          END=0x5FF 
// DATABANK		NAME=gpr2       START=0x600          END=0x6FF 
// DATABANK		NAME=gpr3       START=0x700          END=0x7FF 
// DATABANK		NAME=usb4       START=0x800          END=0x81F          PROTECTED 
// DATABANK		NAME=usb5       START=0x820          END=0x82F          PROTECTED 
// DATABANK		NAME=usb6       START=0x830          END=0x83F          PROTECTED 
// DATABANK		NAME=usb7       START=0x840          END=0x84F          PROTECTED 
// ACCESSBANK	NAME=accesssfr  START=0xF60          END=0xFFF          PROTECTED 
//
// Be careful with this modification, it's better to make it temporary only for this library.

u8 ST7565_buffer[1024] = { 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 

0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x3, 0x7, 0xF, 0x1F, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F, 0x7, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x7F, 0x3F, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x1F, 0x3F, 0x70, 0x70, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6, 0x6, 0x0, 0x0, 0x0, 0x3, 0x3, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 

0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0xF, 0x7, 0x7, 
0x7, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xF, 0x3F, 
0x70, 0x60, 0x60, 0x60, 0x60, 0x30, 0x7F, 0x3F, 0x0, 0x0, 0x1F, 0x3F, 0x70, 0x60, 0x60, 0x60, 
0x60, 0x39, 0xFF, 0xFF, 0x0, 0x6, 0x1F, 0x39, 0x60, 0x60, 0x60, 0x60, 0x30, 0x3F, 0x7F, 0x0, 
0x0, 0x60, 0xFF, 0xFF, 0x60, 0x60, 0x0, 0x7F, 0x7F, 0x70, 0x60, 0x60, 0x40, 0x0, 0x7F, 0x7F, 
0x0, 0x0, 0x0, 0x0, 0x7F, 0x7F, 0x0, 0x0, 0x0, 0x7F, 0x7F, 0x0, 0x0, 0x60, 0xFF, 0xFF, 
0x60, 0x60, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 

0x80, 0xF8, 0xFC, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0xE7, 0xE7, 0xE3, 
0xF3, 0xF9, 0xFF, 0xFF, 0xFF, 0xF7, 0x7, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 
0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x3F, 0x3F, 0x1F, 0xF, 0x7, 0x3, 0x0, 0x0, 0x0, 0xC0, 
0xE0, 0x60, 0x20, 0x20, 0x60, 0xE0, 0xE0, 0xE0, 0x0, 0x0, 0x80, 0xC0, 0xE0, 0x60, 0x20, 0x60, 
0x60, 0xE0, 0xE0, 0xE0, 0x0, 0x0, 0x80, 0xC0, 0x60, 0x60, 0x20, 0x60, 0x60, 0xE0, 0xE0, 0x0, 
0x0, 0x0, 0xE0, 0xE0, 0x0, 0x0, 0x0, 0xE0, 0xE0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0xE0, 
0x60, 0x60, 0x60, 0x60, 0xE0, 0x80, 0x0, 0x0, 0x0, 0xE0, 0xE0, 0x0, 0x0, 0x0, 0xE0, 0xE0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 

0x0, 0x0, 0x0, 0x3, 0x7, 0x1F, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xF1, 0xE3, 
0xE3, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFC, 0x7F, 0x3F, 0x3F, 0x3F, 0x3F, 0x7F, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFC, 0xF0, 0xE0, 0x80, 0x0, 0x0, 0x0, 0xC, 
0x1C, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7F, 0x7F, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x7, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1C, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 

0x0, 0x7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFE, 0xFC, 0xF8, 
0xF8, 0xF0, 0xFE, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF, 
0xFF, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xE0, 0xC0, 0xC0, 0xC0, 0xFF, 0x7F, 0x0, 0x0, 0x1E, 0x7F, 
0xE1, 0xC0, 0xC0, 0xC0, 0xC0, 0x61, 0xFF, 0xFF, 0x0, 0x0, 0xFE, 0xFF, 0x1, 0x0, 0x0, 0x0, 
0xFF, 0xFF, 0x0, 0x0, 0x21, 0xF9, 0xF8, 0xDC, 0xCC, 0xCF, 0x7, 0x0, 0xC0, 0xFF, 0xFF, 0xC0, 
0x80, 0x0, 0xFF, 0xFF, 0xC0, 0xC0, 0x80, 0x0, 0x0, 0xFF, 0xFF, 0x0, 0x0, 0x1F, 0x7F, 0xF9, 
0xC8, 0xC8, 0xC8, 0xC8, 0x79, 0x39, 0x0, 0x0, 0x71, 0xF9, 0xD8, 0xCC, 0xCE, 0x47, 0x3, 0x0, 

0x0, 0x0, 0x0, 0x0, 0x80, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xF8, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF0, 0xC0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xC0, 
0xC0, 0x0, 0x0, 0x0, 0xC0, 0xC0, 0x0, 0x0, 0x0, 0x0, 0xC0, 0xC0, 0x0, 0x0, 0x0, 0x80, 
0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x80, 0xC0, 0xC0, 0x0, 0x0, 0x0, 0x80, 0xC0, 0xC0, 0xC0, 0xC0, 
0xC0, 0x80, 0x0, 0x0, 0x80, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x0, 0x0, 0x0, 0xC0, 0xC0, 0x0, 
0x0, 0x0, 0xC0, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0xC0, 0xC0, 0x0, 0x0, 0x0, 0x80, 0xC0, 
0xC0, 0xC0, 0xC0, 0xC0, 0x80, 0x80, 0x0, 0x0, 0x80, 0xC0, 0xC0, 0xC0, 0xC0, 0x80, 0x0, 0x0, 

0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,};


// reduces how much is refreshed, which speeds it up!
// originally derived from Steve Evans/JCW's mod but cleaned up and
// optimized
#define enablePartialUpdate

#ifdef enablePartialUpdate
static u8 xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;
#endif

static void updateBoundingBox(u8 xmin, u8 ymin, u8 xmax, u8 ymax) {
#ifdef enablePartialUpdate
  if (xmin < xUpdateMin) xUpdateMin = xmin;
  if (xmax > xUpdateMax) xUpdateMax = xmax;
  if (ymin < yUpdateMin) yUpdateMin = ymin;
  if (ymax > yUpdateMax) yUpdateMax = ymax;
#endif
}

void ST7565_drawBitmap(u8 x, u8 y, const u8 *bitmap, u8 w, u8 h,u8 color) 
{
	u8 j, i;
  for (j=0; j<h; j++) 
  {
    for (i=0; i<w; i++ ) 
	{
      if (pgm_read_byte(bitmap + i + (j/8)*w) & _BV(j%8)) 
	  {
		ST7565_my_setPixel(x+i, y+j, color);
      }
    }
  }

  updateBoundingBox(x, y, x+w, y+h);
}

void ST7565_drawString(u8 x, u8 line, u8 *c) 
{
  while (c[0] != 0) 
  {
	if(c[0] == 10)
	{
		x = 0;    // ran out of this line
      	line++;
		c++;
	}
	else if((c[0]>127)&&(c[1]>127))
	{
    	ST7565_drawChar(x, line, c[1]+c[0]);
    	x += 6; // 6 pixels wide
    	if (x + 6 >= LCDWIDTH)
		{
     	x = 0;    // ran out of this line
    	line++;
    	}
		c+=2;
	}
	else
	{
		ST7565_drawChar(x, line, c[0]);
    	x += 6; // 6 pixels wide
    	if (x + 6 >= LCDWIDTH)
		{
      	x = 0;    // ran out of this line
      	line++;
    	}
		c++;
	}
    if (line >= (LCDHEIGHT/8))
      return;        // ran out of space :(
  }
}


void ST7565_drawString_P(u8 x, u8 line, const u8 *str) 
{
	u8 c;
  while (1) 
  {
	c = (u8)pgm_read_byte(str++);
    if (! c)
      return;
    ST7565_drawChar(x, line, c);
    x += 6; // 6 pixels wide
    if (x + 6 >= LCDWIDTH) {
      x = 0;    // ran out of this line
      line++;
    }
    if (line >= (LCDHEIGHT/8))
      return;        // ran out of space :(
  }
}

void ST7565_drawChar(u8 x, u8 line, u8 c) 
{
	u8 i;
  for (i =0; i<5; i++ ) 
  {
    ST7565_buffer[x + (line*128) ] = font[(c*5)+i];
    x++;
  }

  updateBoundingBox(x, line*8, x+5, line*8 + 8);
}


// bresenham's algorithm - thx wikpedia
void ST7565_drawLine(u8 x0, u8 y0, u8 x1, u8 y1, u8 color) 
{
  u8 dx, dy, steep = abs(y1 - y0) > abs(x1 - x0);
  u8 err, ystep;
  if (steep) 
  {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) 
  {
    swap(x0, x1);
    swap(y0, y1);
  }

  // much faster to put the test here, since we've already sorted the points
  updateBoundingBox(x0, y0, x1, y1);

  dx = x1 - x0;
  dy = abs(y1 - y0);

  err = dx / 2;

  if (y0 < y1) 
  {
    ystep = 1;
  } else {
    ystep = -1;}

  for (; x0<=x1; x0++) 
  {
    if (steep) 
	{
      ST7565_my_setPixel(y0, x0, color);
    } else {
      ST7565_my_setPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) 
	{
      y0 += ystep;
      err += dx;
    }
  }
}

// filled rectangle
void ST7565_fillRect(u8 x, u8 y, u8 w, u8 h, u8 color) 
{
	u8 i, j;
  // stupidest version - just pixels - but fast with internal buffer!
  for (i=x; i<x+w; i++) 
  {
    for (j=y; j<y+h; j++) 
	{
      ST7565_my_setPixel(i, j, color);
    }
  }

  updateBoundingBox(x, y, x+w, y+h);
}

// draw a rectangle
void ST7565_drawRect(u8 x, u8 y, u8 w, u8 h, u8 color) 
{
	u8 i;
  // stupidest version - just pixels - but fast with internal buffer!
  for (i=x; i<x+w; i++) 
  {
    ST7565_my_setPixel(i, y, color);
    ST7565_my_setPixel(i, y+h-1, color);
  }
  for (i=y; i<y+h; i++) 
  {
    ST7565_my_setPixel(x, i, color);
    ST7565_my_setPixel(x+w-1, i, color);
  } 

  updateBoundingBox(x, y, x+w, y+h);
}

// draw a circle outline
void ST7565_drawCircle(u8 x0, u8 y0, u8 r, u8 color) 
{
  u8 f, ddF_x, ddF_y, x, y;
  
  updateBoundingBox(x0-r, y0-r, x0+r, y0+r);

  f = 1 - r;
  ddF_x = 1;
  ddF_y = -2 * r;
  x = 0;
  y = r;

  ST7565_my_setPixel(x0, y0+r, color);
  ST7565_my_setPixel(x0, y0-r, color);
  ST7565_my_setPixel(x0+r, y0, color);
  ST7565_my_setPixel(x0-r, y0, color);

  while (x<y) 
  {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    ST7565_my_setPixel(x0 + x, y0 + y, color);
    ST7565_my_setPixel(x0 - x, y0 + y, color);
    ST7565_my_setPixel(x0 + x, y0 - y, color);
    ST7565_my_setPixel(x0 - x, y0 - y, color);
    
    ST7565_my_setPixel(x0 + y, y0 + x, color);
    ST7565_my_setPixel(x0 - y, y0 + x, color);
    ST7565_my_setPixel(x0 + y, y0 - x, color);
    ST7565_my_setPixel(x0 - y, y0 - x, color);
    
  }



}

void ST7565_fillCircle(u8 x0, u8 y0, u8 r, u8 color) 
{
  u8 f, ddF_x, ddF_y, x, y;
  u8 i;
  updateBoundingBox(x0-r, y0-r, x0+r, y0+r);

  f = 1 - r;
  ddF_x = 1;
  ddF_y = -2 * r;
  x = 0;
  y = r;

  for (i=y0-r; i<=y0+r; i++) 
  {
    ST7565_my_setPixel(x0, i, color);
  }

  while (x<y) 
  {
    if (f >= 0) 
	{
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    for (i=y0-y; i<=y0+y; i++) 
	{
      ST7565_my_setPixel(x0+x, i, color);
      ST7565_my_setPixel(x0-x, i, color);
    } 
    for (i=y0-x; i<=y0+x; i++) 
	{
      ST7565_my_setPixel(x0+y, i, color);
      ST7565_my_setPixel(x0-y, i, color);
    }    
  }
}

void ST7565_my_setPixel(u8 x, u8 y, u8 color) 
{
  if ((x >= LCDWIDTH) || (y >= LCDHEIGHT))
    return;

  // x is which column
  if (color) 
    ST7565_buffer[x+ (y/8)*128] |= _BV(7-(y%8));  
  else
    ST7565_buffer[x+ (y/8)*128] &= ~_BV(7-(y%8)); 
}

// the most basic function, set a single pixel
void ST7565_setPixel(u8 x, u8 y, u8 color) 
{
  if ((x >= LCDWIDTH) || (y >= LCDHEIGHT))
    return;

  // x is which column
  if (color) 
    ST7565_buffer[x+ (y/8)*128] |= _BV(7-(y%8));  
  else
    ST7565_buffer[x+ (y/8)*128] &= ~_BV(7-(y%8)); 

  updateBoundingBox(x,y,x,y);
}


// the most basic function, get a single pixel
u8 ST7565_getPixel(u8 x, u8 y) 
{
  if ((x >= LCDWIDTH) || (y >= LCDHEIGHT))
    return 0;

  return (ST7565_buffer[x+ (y/8)*128] >> (7-(y%8))) & 0x1;  
}

void ST7565_init(u8 SID, u8 SCLK, u8 DC, u8 RST, u8 CS) 
{
    sid = SID;
    sclk = SCLK;
    dc = DC;
    rst = RST;
    cs = CS;

  // set pin directions
  pinmode(sid, OUTPUT);
  pinmode(sclk, OUTPUT);
  pinmode(dc, OUTPUT);
  pinmode(rst, OUTPUT);
  pinmode(cs, OUTPUT);

  // toggle RST low to reset; CS low so it'll listen to us
  if (cs > 0)
    digitalwrite(cs, LOW);

  digitalwrite(rst, LOW);
  Delayms(500);
  digitalwrite(rst, HIGH);

  // LCD bias select
  ST7565_command(CMD_SET_BIAS_7);
  // ADC select
  ST7565_command(CMD_SET_ADC_NORMAL);
  // SHL select
  ST7565_command(CMD_SET_COM_NORMAL);
  // Initial display line
  ST7565_command(CMD_SET_DISP_START_LINE);

  // turn on voltage converter (VC=1, VR=0, VF=0)
  ST7565_command(CMD_SET_POWER_CONTROL | 0x4);
  // wait for 50% rising
  Delayms(50);

  // turn on voltage regulator (VC=1, VR=1, VF=0)
  ST7565_command(CMD_SET_POWER_CONTROL | 0x6);
  // wait >=50ms
  Delayms(50);

  // turn on voltage follower (VC=1, VR=1, VF=1)
  ST7565_command(CMD_SET_POWER_CONTROL | 0x7);
  // wait
  Delayms(10);

  // set lcd operating voltage (regulator resistor, ref voltage resistor)
  ST7565_command(CMD_SET_RESISTOR_RATIO | 0x6);

  // initial display line
  // set page address
  // set column address
  // write display data

  // set up a bounding box for screen updates

  updateBoundingBox(0, 0, LCDWIDTH-1, LCDHEIGHT-1);

  ST7565_command(CMD_DISPLAY_ON);
  ST7565_command(CMD_SET_ALLPTS_NORMAL);
  ST7565_setBrightness(0x18);
}

void ST7565_shiftOut(u8 dataPin, u8 clockPin, u8 bitOrder, u8 val)
{
        u8 i;
        for (i = 0; i < 8; i++)  {
                if (bitOrder == LSBFIRST)
                        digitalwrite(dataPin, !!(val & (1 << i)));
                else  
                        digitalwrite(dataPin, !!(val & (1 << (7 - i))));
                        
                digitalwrite(clockPin, HIGH);
                digitalwrite(clockPin, LOW);
        }
}

void ST7565_command(u8 c) 
{
  digitalwrite(dc, LOW);
  ST7565_shiftOut(sid, sclk, MSBFIRST, c);
}

void ST7565_data(u8 c) 
{
  digitalwrite(dc, HIGH);
  ST7565_shiftOut(sid, sclk, MSBFIRST, c);
}

void ST7565_setBrightness(u8 val) 
{
    ST7565_command(CMD_SET_VOLUME_FIRST);
    ST7565_command(CMD_SET_VOLUME_SECOND | (val & 0x3f));
}


void ST7565_refresh(void) 
{
  u8 col, maxcol, p;

  /*
  Serial.print("Refresh ("); Serial.print(xUpdateMin, DEC); 
  Serial.print(", "); Serial.print(xUpdateMax, DEC);
  Serial.print(","); Serial.print(yUpdateMin, DEC); 
  Serial.print(", "); Serial.print(yUpdateMax, DEC); Serial.println(")");
  */

  for(p = 0; p < 8; p++) {
    /*
      putstring("new page! ");
      uart_putw_dec(p);
      putstring_nl("");
    */
#ifdef enablePartialUpdate
    // check if this page is part of update
    if ( yUpdateMin >= ((p+1)*8) ) {
      continue;   // nope, skip it!
    }
    if (yUpdateMax < p*8) {
      break;
    }
#endif

    ST7565_command(CMD_SET_PAGE | pagemap[p]);


#ifdef enablePartialUpdate
    col = xUpdateMin;
    maxcol = xUpdateMax;
#else
    // start at the beginning of the row
    col = 0;
    maxcol = LCDWIDTH-1;
#endif

    ST7565_command(CMD_SET_COLUMN_LOWER | ((col+ST7565_STARTBYTES) & 0xf));
    ST7565_command(CMD_SET_COLUMN_UPPER | (((col+ST7565_STARTBYTES) >> 4) & 0x0F));
    ST7565_command(CMD_RMW);
    
    for(; col <= maxcol; col++) {
      //uart_putw_dec(col);
      //uart_putchar(' ');
      ST7565_data(ST7565_buffer[(128*p)+col]);
    }
  }

#ifdef enablePartialUpdate
  xUpdateMin = LCDWIDTH - 1;
  xUpdateMax = 0;
  yUpdateMin = LCDHEIGHT-1;
  yUpdateMax = 0;
#endif
}

// clear everything
void ST7565_clear(void) 
{
  memset(ST7565_buffer, 0, 1024);
  updateBoundingBox(0, 0, LCDWIDTH-1, LCDHEIGHT-1);
}


// this doesnt touch the buffer, just clears the display RAM - might be handy
void ST7565_clearDisplay(void) 
{
  u8 p, c;
  
  for(p = 0; p < 8; p++) {
    /*
      putstring("new page! ");
      uart_putw_dec(p);
      putstring_nl("");
    */

    ST7565_command(CMD_SET_PAGE | p);
    for(c = 0; c < 129; c++) {
      //uart_putw_dec(c);
      //uart_putchar(' ');
      ST7565_command(CMD_SET_COLUMN_LOWER | (c & 0xf));
      ST7565_command(CMD_SET_COLUMN_UPPER | ((c >> 4) & 0xf));
      ST7565_data(0x0);
    }     
  }
}
