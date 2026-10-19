/* BDOS console backend for Concurrent CP/M-86 3.1.
 * All screen storage belongs to this process; no XIOS pointers or calls.
 */
#include "stdio.h"
#include "common.h"
#include "vd.h"

#define Vnorm 0x07
#define Vinverse 0x70
#define Vintense 0x08
#define Vblink 0x80

WORD TEXTLINE, STATLINE, TOTLINES, NARROW;
BOOLEAN VDtrapon, VDsaveon, VDhalfscroll, VDterm, VDhold;
static BYTE cells[1920], attrs[1920], dirty[24];
static WORD currow = 1, curcol = 1, saverow = 1, savecol = 1;
static WORD attrib, saved_mode, saved_delim;
static BOOLEAN active;

static VOID
bdos_conout(s)
BYTE *s;
{
    __BDOS(9, s);
}

static VOID
position(row, col)
WORD row, col;
{
    BYTE seq[5];
    seq[0] = 27; seq[1] = 'Y';
    seq[2] = row + 32; seq[3] = col + 32; seq[4] = 0;
    bdos_conout(seq);
}

static VOID
attribute(a)
WORD a;
{
    BYTE seq[7];
    /* Color arguments use the low nibble, so ASCII avoids NUL bytes. */
    seq[0] = 27; seq[1] = 'b'; seq[2] = '0' + (a & 15);
    seq[3] = 27; seq[4] = 'c'; seq[5] = '0' + ((a >> 4) & 15);
    seq[6] = 0;
    bdos_conout(seq);
}

VOID
update_phys()
{
    WORD r, c, end, a;
    BYTE line[81];
    bdos_conout("\033w");
    for (r = 0; r < 24; r++) {
        if (!dirty[r]) continue;
        c = 0;
        while (c < 80) {
            a = attrs[r * 80 + c] & 255;
            end = c;
            while (end < 80 && (attrs[r * 80 + end] & 255) == a) {
                line[end - c] = cells[r * 80 + end];
                end++;
            }
            line[end - c] = 0;
            position(r, c);
            attribute(a);
            bdos_conout(line);
            c = end;
        }
        dirty[r] = FALSE;
    }
    position(currow - 1, curcol - 1);
}

VOID
VDsync()
{
    update_phys();
}

draw(row, col, length, text, att)
WORD row, col, length;
BYTE *text, att;
{
    WORD i, p;
    if (row < 0 || row >= 24 || col < 0 || col >= 80) return;
    if (length > 80 - col) length = 80 - col;
    for (i = 0; i < length; i++) {
        p = row * 80 + col + i;
        cells[p] = (text[i] & 255) < 32 ? ' ' : text[i];
        attrs[p] = att;
    }
    dirty[row] = TRUE;
}

erase(row, col, length)
WORD row, col, length;
{
    BYTE blank = ' ';
    while (length-- > 0 && col < 80) draw(row, col++, 1, &blank, Vnorm);
}

setattr(row, col, att)
WORD row, col;
BYTE att;
{
    if (row < 0 || row >= 24 || col < 0 || col >= 80) return;
    attrs[row * 80 + col] = att;
    dirty[row] = TRUE;
}

VOID
setattrib(a)
WORD a;
{
	attrib = a;
}




VOID
VDbeep()
/****************************************************************/
/*								*/
/*		Visual Display BEEP routine			*/
/*		  i.e.	send a BELL to screen			*/
/*								*/
/****************************************************************/
{
	bdos_conout("\007");
}




VOID
VDcolor(color1, color2, color3)
unsigned color1, color2, color3;
{
}




VOID
VDclear()
{
    WORD i;
    bdos_conout("\033z\033E\033w");
    for (i = 0; i < 1920; i++) {
        cells[i] = ' ';
        attrs[i] = Vnorm;
    }
    for (i = 0; i < 24; i++) dirty[i] = FALSE;
    currow = curcol = 1;
    attrib = Vnorm;
}




VOID
VDcurpos(row, col)
WORD row, col;
{
    currow = row;
    curcol = col;
    position(row - 1, col - 1);
}




VOID
VDsavecurpos()
{
    saverow = currow;
    savecol = curcol;
}




VOID
VDrestcurpos()
{
    VDcurpos(saverow, savecol);
}




VOID
VDcursize(size)
TCURSIZE size;
{
	static TCURSIZE cur = C_NORM, prev = C_NORM;

	if (size == C_PREV)
		size = prev;
	prev = cur;
	cur = size;
	switch (size) {
	case C_BIG:
	case C_NORM:
	case C_LITTLE:
	case C_UNNORM:
	case C_HALF:
		bdos_conout("e");
		break;
	case C_OFF:
		bdos_conout("f");
	}
}




VOID
VDattrib(attr)
TATTRIBUTE attr;
{
	switch (attr) {
	case A_NORM:
		attrib = Vnorm;
		break;
	case A_INVERSE:
		attrib = (attrib & 0x88) | Vinverse;
		break;
	case A_INTENSE:
	case A_EMPHASIZE:
		attrib = attrib | Vintense;
		break;
	case A_UNDER:
		/*attrib = (attrib & 0x88) | Vunder*/;
		break;
	case A_BLINK:
		attrib = attrib | Vblink;
	}
}




VOID
VDmattrib(attr)
TATTRIBUTE attr;
{
	switch (attr) {
	case A_INVERSE:
		attrib = (attrib & 0x88) | Vnorm;
		break;
	case A_INTENSE:
		attrib = attrib & 0xF7;
		break;
	case A_UNDER:
		attrib = (attrib & 0x88) | Vnorm;
		break;
	case A_BLINK:
		attrib = attrib & 0x7F;
	}
}




VOID
VDhorout(row, col, s, start, amt)
WORD row, col;
BYTE *s;
WORD start, amt;
/****************************************************************/
/*								*/
/*	Visual Display HORizontal OUTput			*/
/*								*/
/****************************************************************/
{
	WORD padcnt, fin, len;

	VDhalfscroll = TRUE;
	len = strlen(s);
	if ((len - start + 1) < amt) {
		fin = len;
		padcnt = amt - (len - start + 1);
	}
	else {
		fin = start + amt - 1;
		padcnt = 0;
	}
	len = fin - start + 1;
	draw(row - 1, col - 1, len, &s[start - 1], attrib);
	if (padcnt > 0)
		erase(row - 1, col + len - 1, padcnt);
	update_phys();
}




VOID
VDverout()
{
	fatal("VDverout is disabled");
}




VOID
VDspecial(row, col, ch)
WORD row, col;
TSPECIAL ch;
{
	BYTE s[2];

	s[1] = '\0';
	switch (ch) {
	case S_H:
		s[0] = CHR(0xC4);
		break;
	case S_V:
		s[0] = CHR(0xB3);
		break;
	case S_HVUP:
		s[0] = CHR(0xC1);
		break;
	case S_HVDN:
		s[0] = CHR(0xC2);
		break;
	case S_VHRT:
		s[0] = CHR(0xC3);
		break;
	case S_VHLF:
		s[0] = CHR(0xB4);
		break;
	case S_HV:
		s[0] = CHR(0xC5);
	}
	VDhorout(row, col, s, 1, 1);
}




VOID
VDscroll(r1, c1, r2, c2, dir, amt)
WORD r1, c1, r2, c2, dir, amt;
{
    WORD r, c, src, dest, step, last;
    if (amt <= 0) return;
    /* The editor supplies source rows, with destination displaced by amt. */
    step = dir == D_UP ? 1 : -1;
    r = dir == D_UP ? r1 : r2;
    last = dir == D_UP ? r2 : r1;
    for (;;) {
        for (c = c1; c <= c2; c++) {
            src = (r - 1) * 80 + c - 1;
            dest = src + (dir == D_UP ? -amt : amt) * 80;
            if (src >= 0 && src < 1920 && dest >= 0 && dest < 1920) {
                cells[dest] = cells[src];
                attrs[dest] = attrs[src];
                dirty[dest / 80] = TRUE;
            }
        }
        if (r == last) break;
        r += step;
    }
    update_phys();
}




VOID
VDintense(on, row, col, amt)
BOOLEAN on;
WORD row, col, amt;
{
    while (amt-- > 0 && col <= 80)
        setattr(row - 1, col++ - 1, on ? Vinverse : Vnorm);
}




VOID
VDshorout(row, col, ch, amt)
WORD row, col;
BYTE ch;
WORD amt;
{
    while (amt-- > 0 && col <= 80) draw(row - 1, col++ - 1, 1, &ch, attrib);
    update_phys();
}




VOID
VDrestore()
{
	VDclear();
}




VOID
VDgo()
{
	update_phys();
}




VOID
VDinit()
{
    saved_mode = __BDOS(109, 0xffff);
    saved_delim = __BDOS(110, 0xffff);
    __BDOS(109, saved_mode | 4); /* raw output: preserve graphics and ESC */
    __BDOS(110, 0);
    active = TRUE;
    TOTLINES = 24;
    NARROW = FALSE;
    TEXTLINE = 22;
    STATLINE = 23;
    attrib = Vnorm;
    VDterm = VDtrapon = VDsaveon = VDhold = FALSE;
    VDhalfscroll = TRUE;
    bdos_conout("\033w"); /* no wrap, including the bottom right cell */
    VDclear();
}

VOID
VDfinish()
{
    if (active) {
        bdos_conout("\033z\033v\033e");
        __BDOS(110, saved_delim);
        __BDOS(109, saved_mode);
        active = FALSE;
    }
}
