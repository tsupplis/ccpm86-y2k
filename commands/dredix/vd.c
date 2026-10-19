#include "stdio.h"



#include "caparm.h"



#include "common.h"



#include "vd.h"



extern VOID GENCOPY();				/* in WMCA.A86		*/
extern VOID GENALT();

extern VOID WM_PK();				/* in WWCALL.A86	*/
extern VOID WM_CALL();
extern VOID IO_CALL();
extern VOID WM_MXQ();

WORD TEXTLINE, STATLINE, TOTLINES, NARROW, h;
BOOLEAN VDtrapon, VDsaveon, VDhalfscroll, VDterm, VDhold;

WORD	vc_number;
WORD	vc_segment;
WORD	ds;

#define P_TERMCPM 0x00
#define C_WRITE 0x02
#define C_RAWIO 0x06
#define C_WRITEBLK 0x6f
#define C_WRITESTR 0x09
#define C_GET 0x99
#define C_DELIMIT 0x6e
#define S_OSVER 0xa3
#define F_ERRMODE 0x2d

#define VS_TOP_LEFT 2
#define VS_BOT_RIGHT 4
#define VS_WIN_SIZE 12
#define VS_VC_SEG 22
#define VS_TRUE_VIEW 42

#define IO_SWITCH 0x07
#define IO_NEW_WINDOW 0x14

#define Vnorm 0x07
#define Vinverse 0x70
#define Vintense 0x08
#define Vunder 0x01
#define Vblink 0x80

struct chcb {
	BYTE *c_offset;
	WORD c_segment;
	WORD c_length;
}; 

#define DSPOFF(r,c) (160 * (r - 1) + 2 * (c - 1))

WORD attrib;	




VOID
VDsync()
{
	wm_sync(vc_number);
}




static VOID
bdos_conout(s)
BYTE *s;
{
	__BDOS(C_WRITESTR, s);
}




WORD
version_chk()
/****************************************************************/
/*								*/
/*		VERSION CHecK routine				*/
/*								*/
/****************************************************************/
{
	BYTE v;

	/* do something	that verifies this is Concurrent CP/M 3.2
	   with an XIOS that supports windows */

	v = __BDOS(S_OSVER, 0x0);

	return(v >= 0x31);
}




VOID
wm_nrnc(vc, pnr, pnc)
WORD vc;
WORD *pnr;
WORD *pnc;
/****************************************************************/
/*								*/
/*	Window Manager Number of Rows and Number of Columns	*/
/*								*/
/****************************************************************/
{
	WORD value, tl, br;

	WM_PK(0, vc, VS_TOP_LEFT, &tl);
	WM_PK(0, vc, VS_BOT_RIGHT, &br);
	WM_PK(0, vc, VS_WIN_SIZE, &value);

	*pnc = value & 0x00ff;

 	*pnr = (value & 0xff00) >> 8;

	if ((tl == 0x0000) && (br == 0x174f)) {
		*pnc = 80;
		*pnr = 24; 
	}

	if (TOTLINES == 0) {
		TOTLINES = *pnr;
		if (TOTLINES < 6)
			TOTLINES = 6;
  		NARROW = (*pnc != 80 || (tl & 0xff) != 0);  
	}
	*pnc=80; *pnr=24; tl=0; br=0x174f; NARROW=0; TOTLINES=24;
}




VOID
wm_vrvc(vc, pvr, pvc)
WORD vc;
WORD *pvr;
WORD *pvc;
/****************************************************************/
/*								*/
/*	Window Manager View Row and View Column			*/
/*								*/
/****************************************************************/
{
	WORD value;


	WM_PK(0, vc, VS_TRUE_VIEW, &value);

	*pvc = (value & 0x00ff) + 1;

 	*pvr = ((value & 0xff00) >> 8) + 1;

	*pvc = 1;
	*pvr = 1;

}




VOID
wm_sync(number)
UWORD number;
/****************************************************************/
/*								*/
/*	Window Manager SYNChronize screen image and vc image	*/
/*								*/
/****************************************************************/
{
	UWORD ax, bx, cx, dx;
	UWORD imhere, nvc, top;
	UWORD vr, vc, nr, nc;

	/* if vc is full screen and on top make sure image and
	   screen are in sync */

	WM_CALL(&imhere, &nvc, &top);


	if (top == number) {
		wm_nrnc(number, &nr, &nc);
		wm_vrvc(number, &vr, &vc);
		if ((nr == 24) && (nc == 80) && (vr == 1) && (vc == 1)) {

		/* IO SWITCH to the vc that is on top will cause the vc buffer
		   to be updated with what is on the physical */

			ax = IO_SWITCH;
			bx = cx = 0;
			dx = number;
			IO_CALL(&ax, &bx, &cx, &dx);
		}
	}
}




draw(row, col, length, pstring, att)
WORD row;
WORD col;
WORD length;
BYTE *pstring;
BYTE att;
{
	struct ca_parm src,dest;
	WORD temp;

	if (length == 0)
		return;

	/* make sure we don't wrap */

	if ((col + length) > 80)
		length = 80 - col;

	/* set up copy src to point at string */

	temp = ((row * 80) + col) * 2;
	src.ca_choffset = pstring;
	src.ca_chsegment = ds;
	src.ca_atoffset = &att;
	src.ca_atsegment = ds;
	src.ca_chcoladj = 1;
	src.ca_atcoladj = 0;
	src.ca_chrowadj = 0;
	src.ca_atrowadj = 0;

	/* set up copy dest. to point to local buffer */
	temp = ((row * 80) + col) * 2;
	dest.ca_choffset = temp;
	dest.ca_chsegment = vc_segment;
	dest.ca_atoffset = temp + 1;
	dest.ca_atsegment = vc_segment;
	dest.ca_chcoladj = 2;
	dest.ca_atcoladj = 2;
	dest.ca_chrowadj = 0;
	dest.ca_atrowadj = 0;

	/* get mxq */

	WM_MXQ(0, vc_number);


	/* do the copy */

	GENCOPY(&src, &dest, 1, length);


	/* free up mxq */

	WM_MXQ(1, vc_number);

}




erase(row, col, length)
WORD row;
WORD col;
WORD length;
{
	struct ca_parm dest;
	WORD temp;

	if (length == 0)
		return;

	/* make sure we don't wrap */
	if ((col + length) > 80)
		length = 80 - col;


	/* set up alter dest. to point to cell in vc buffer */

	temp = ((row * 80) + col) * 2;
	dest.ca_choffset = temp;
	dest.ca_chsegment = vc_segment;
	dest.ca_atoffset = temp + 1;
	dest.ca_atsegment = vc_segment;
	dest.ca_chcoladj = 2;
	dest.ca_atcoladj = 2;
	dest.ca_chrowadj = 0;
	dest.ca_atrowadj = 0;


						/*	get mxq		*/
	WM_MXQ(0, vc_number);

						/*	do the copy	*/
	temp = ' ';
	GENALT(&dest, 1, length, 0x0000, (temp << 8) + 0x07);

						/*	free up mxq	*/
	WM_MXQ(1, vc_number);
}




setattr(row, col, attr)
WORD row;
WORD col;
BYTE attr;
{
	struct ca_parm dest;
	WORD temp;

					/* set up alter dest. to point	*/
					/* to cell in vc buffer */
	temp = ((row * 80) + col) * 2;
	dest.ca_choffset = temp;
	dest.ca_chsegment = vc_segment;
	dest.ca_atoffset = temp + 1;
	dest.ca_atsegment = vc_segment;
	dest.ca_chcoladj = 2;
	dest.ca_atcoladj = 2;
	dest.ca_chrowadj = 0;
	dest.ca_atrowadj = 0;

						/*	get mxq		*/
	WM_MXQ(0, vc_number);

						/*	do the copy	*/
	GENALT(&dest, 1, 1, 0xFF00, attr);

						/*	free up mxq	*/
	WM_MXQ(1, vc_number);
}




draw_ch(row, col, ch)
WORD row;
WORD col;
BYTE ch;
/****************************************************************/
/*								*/
/*		DRAW CHaracter routine				*/
/*								*/
/****************************************************************/
{
	struct ca_parm dest;
	WORD temp;

	if (col > 79)
		return;

					/* set up alter dest. to point	*/
					/* to cell in vc buffer		*/
	temp = ((row * 80) + col) * 2;
	dest.ca_choffset = temp;
	dest.ca_chsegment = vc_segment;
	dest.ca_atoffset = temp + 1;
	dest.ca_atsegment = vc_segment;
	dest.ca_chcoladj = 0;
	dest.ca_atcoladj = 0;
	dest.ca_chrowadj = 0;
	dest.ca_atrowadj = 0;

						/* 	get mxq		*/
	WM_MXQ(0, vc_number);

						/*	do the copy	*/
	temp = ch;
	GENALT(&dest, 1, 1, 0x0000, (temp << 8) + 0x07);

						/*	free up mxq	*/
	WM_MXQ(1, vc_number);
}




VOID
update_phys()
/****************************************************************/
/*								*/
/*		UPDATE PHYSical routine				*/
/*								*/
/****************************************************************/
{
	WORD ax, bx, cx, dx;


	WM_PK(0, vc_number, VS_TOP_LEFT, &cx);
	WM_PK(0, vc_number, VS_BOT_RIGHT, &bx);

	ax = IO_NEW_WINDOW;
	dx = vc_number;
	IO_CALL(&ax, &bx, &cx, &dx);	/* set up new window		*/
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
/****************************************************************/
/*								*/
/*		Visual Display CLEAR routine			*/
/*								*/
/****************************************************************/
{
	bdos_conout("E");

	for (h=0; h<23; h=h+1)
		erase(h,0,80);

	VDsync();

	update_phys();

	attrib = Vnorm;
}




VOID
VDcurpos(row, col)
WORD row, col;
/****************************************************************/
/*								*/
/*	Visual Dispaly CURsor POSition routine			*/
/*								*/
/****************************************************************/
{
	static BYTE *bf = "Yrc";

	bf[2] = (BYTE)(31 + row);
	bf[3] = (BYTE)(31 + col);
	bdos_conout(bf);
}




VOID
VDsavecurpos()
{
	bdos_conout("j");
}




VOID
VDrestcurpos()
{
	bdos_conout("k");
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
WORD r1, c1, r2, c2;
TDIRECTION dir;
WORD amt;
{
	WORD r;

	if (VDsaveon)
		fatal("VDscroll - VDsaveon");
	switch (dir) {
	case D_UP:
		/*
		if (c2 - c1 == 79)
			MOVEDATA(vc_segment, DSPOFF(r1, 1), vc_segment,
			  DSPOFF(r1 - amt, 1), 160 * (r2 - r1 + 1));
		else
		*/
			for (r = r1; r <= r2; r++)
				MOVEDATA(vc_segment, DSPOFF(r, c1), vc_segment,
				  DSPOFF(r - amt, c1), 2 * (c2 - c1 + 1));
		break;
	case D_DOWN:
		/*
		if (c2 - c1 == 79)
			MOVEDATAfromright(vc_segment, DSPOFF(r1, 1), vc_segment,
			  DSPOFF(r1 + amt, 1), 160 * (r2 - r1 + 1));
		else
		*/
			for (r = r2; r >= r1; r--)
				MOVEDATA(vc_segment, DSPOFF(r, c1), vc_segment,
				  DSPOFF(r + amt, c1), 2 * (c2 - c1 + 1));
		break;
	case D_LEFT:
		fatal("VDscroll: D_LEFT is disabled");
	case D_RIGHT:
		fatal("VDscroll: D_RIGHT is disabled");
	}
	update_phys();
}




VOID
VDintense(on, row, col, amt)
BOOLEAN on;
WORD row, col, amt;
{
	setattr(row - 1, col - 1, on ? Vinverse : Vnorm);
}




VOID
VDshorout(row, col, ch, amt)
WORD row, col;
BYTE ch;
WORD amt;
{
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




static VOID
cpcdos_init()
/****************************************************************/
/*								*/
/*	Concurrent PC DOS INITialization			*/
/*								*/
/****************************************************************/
{
	BYTE *ptemp;


	__BDOS(C_DELIMIT, '\0');

	__BDOS(F_ERRMODE, 0xff);

					/* Check for valid version	*/
	if (!version_chk()) {
		__BDOS(C_WRITESTR, "Need version 3.1 or 3.2\r\n");
		__BDOS(P_TERMCPM, 0);
	}


	/* figure out your current data segment */

	ptemp = 9;
	ds = *ptemp++ + (*ptemp << 8);

	/* get virtual console number */

	vc_number = __BDOS(C_GET, 0x0);
	if (vc_number > 3) {
		__BDOS (C_WRITESTR, "DR EDIX cannot be run from external terminal.");
		__BDOS (P_TERMCPM, 0);
	}


	/* get segment to your virtual console screen buffer.
	   the offset is always 0x0 */

	WM_PK(0, vc_number, VS_VC_SEG, &vc_segment);

	__BDOS(C_WRITESTR, "6");


	/* if vc is full screen and on top make sure image and screen are
	   in sync */

	VDsync();
}




VOID
VDinit()
/************************************************************************/
/*									*/
/*		Visual Display INITialization				*/
/*									*/
/************************************************************************/
{

	TOTLINES = 0;

	cpcdos_init();

	attrib = Vnorm;

	VDterm = FALSE;

	TEXTLINE = TOTLINES - 2;

	STATLINE = TOTLINES - 1;

	VDtrapon = FALSE;

	VDsaveon = FALSE;

	VDclear();
}
                                                                                                                                                                                                                                                                                           