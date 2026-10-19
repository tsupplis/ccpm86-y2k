#include "stdio.h"



#include "common.h"



#include "ebuf.h"



#include "ewin.h"



#include "ewindef.h"



#include "ecmdblk.h"



#include "ekey.h"



#include "ehelp.h"



#include "ecommon.h"



#include "vd.h"




BOOLEAN changed;
WORD Wshowstart;

#define MAXABSCOL 80
#define MAXMSGCOL 50
#define MAXCMDWIDTH 39
#define STACKSIZE 5

typedef TBUFNUM TWINBUF[N_TWNAME];

static TWINBUF winbuf = {BC, B1, B2, BNONE};
static TWNAME curwind;
static WORD wsi;
static TWNAME wstack[STACKSIZE + 1];
static WORD dline;




VOID
Wclear()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	dline = 0;
	VDclear();
}




VOID
Winit()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	Wshowstart = -1;
	Wclear();
	WDinit();
	curwind = W1;
	VDhalfscroll = FALSE;
}




TWNAME
Wwhich(b)
TBUFNUM b;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TWNAME w;
	for (w = LOWERW; w <= UPPERW; w++)
		if (winbuf[w] == b)
			return(w);
	return(WNONE);
}




VOID
Wsave()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	if (wsi == STACKSIZE)
		Wmsg("Window stack overflow");
	else
		wstack[++wsi] = curwind;
}




VOID
Wrestore()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	if (wsi > 0)
		Wselect(wstack[wsi--]);
}




static VOID
refreshone(w)
TWNAME w;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	if (window[w].wvisible) {
		Wselect(w);
		changed = TRUE;
		Wshow(1, MAXINT);
		Wcursor(TRUE);
		Wshow(1, MAXINT); /* in case scrolled left or right */
	}
}




VOID
Wchgbuf(b)
TBUFNUM b;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TWNAME w;

	Wsave();
	w = Wwhich(b);
	if (w != WNONE) {
		winbuf[w] = curbuf;
		refreshone(w);
	}
	Wrestore();
	winbuf[curwind] = b;
	curbuf = b;
	refreshone(curwind);
}




static VOID
refresh()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TWNAME w;

	Wsave();
	for (w = W1; w <= W2; w++)
		refreshone(w);
	Wrestore();
}




VOID
Wrefresh()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	Wclear();
	WDrepaint();
	refresh();
	Wstatus("..", 1, 1, 1, B1);
	Wstatus(buffer[curbuf].bfile, buffer[curbuf].bcol,
	  buffer[curbuf].blnum, buffer[curbuf].blast, curbuf);
}




VOID
Wrepaint()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE c;
	static WORD depth = 0;

	depth++;
	VDattrib(A_INVERSE);
	VDhorout(TOTLINES, 1, "Press space bar to resume editing ...", 1, 37);
	VDattrib(A_NORM);
	if (keyhit(&c) == KHELP)
		helpcmd(); /*will call Wrepaint*/
	if (depth == 1)
		Wrefresh();
	depth--;
}




VOID
Wselect(w)
TWNAME w;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	curwind = w;
	if (w != WNONE) {
		if (WDselect(w))
			refresh();
		curbuf = winbuf[w];
	}
}




VOID
Wkill()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TWNAME w;

	Wselect(W1);
	WDkill();
	refresh();
}




VOID
Wlabel(s)
BYTE s[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE l[81];
	TWINDOW *w = &window[curwind];
	WORD len;

	w->wabscol = 1;
	w->wbcol = 1;
	w->wwidth = MAXCMDWIDTH;
	strcpy(l, s);
	len = strlen(l) + 1;
	if (len > MAXCMDWIDTH - 4) {
		len = MAXCMDWIDTH - 4;
		SETLEN(l, len);
	}
	sput(w->wabsrow, l);
	w->wabscol = len;
	w->wwidth = MAXCMDWIDTH - (len - 1);
}




VOID
Wcursor(center)
BOOLEAN center;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	WORD target;
	TLINENUM amt;
	TWINDOW *w = &window[curwind];
	TBUFFER *b = &buffer[curbuf];

	target = b->blnum;
	if (b->blnum < w->wblnum) {
		if (center) {
			target = b->blnum - w->wheight / 2;
			if (target < 1)
				target = 1;
		}
		amt = w->wblnum - target;
		w->wblnum = target;
		if (amt < w->wheight)
			scroll(w->wabsrow, w->wabsrow + w->wheight - amt - 1,
			  w->wabscol, w->wabscol + w->wwidth - 1, DOWN, amt);
		changed = TRUE;
		Wshow(w->wblnum, w->wblnum + amt - 1);
	}
	else if (b->blnum >= w->wblnum + w->wheight) {
		if (center) {
			target = b->blnum + w->wheight / 2;
			if (target > b->blast)
				target = b->blast;
		}
		amt = target - (w->wblnum + w->wheight - 1);
		w->wblnum = target - w->wheight + 1;
		if (amt < w->wheight)
			scroll(w->wabsrow + amt, w->wabsrow + w->wheight - 1,
			  w->wabscol, w->wabscol + w->wwidth - 1, UP, amt);
		changed = TRUE;
		Wshow(w->wblnum + w->wheight - amt, w->wblnum + w->wheight - 1);
	}
	if (b->bcol < w->wbcol) {
		w->wbcol = b->bcol;
		changed = TRUE;
	}
	else if (b->bcol >= w->wbcol + w->wwidth) {
		w->wbcol = b->bcol - w->wwidth + 1;
		changed = TRUE;
	}
	VDcurpos(w->wabsrow + b->blnum - w->wblnum,
	  w->wabscol + b->bcol - w->wbcol);
}




static VOID
scroll(row1, row2, col1, col2, dir, amt)
TROW row1, row2;
TCOL col1, col2;
TDIR dir;
TROW amt;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TROW r;
	TCOL c;

	/* Not all implementations of VDscroll can handle < 80 columns. */
	if (col2 - col1 != 79 && !VDhalfscroll) {
		changed = TRUE;
		Wshow(1, MAXINT);
		return;
	}
	if (curwind == WNONE)
		return;
	if (dir == UP)
		VDscroll(row1, col1, row2, col2, D_UP, amt);
	else
		VDscroll(row1, col1, row2, col2, D_DOWN, amt);
}




VOID
Woncursor(on)
BOOLEAN on;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	if (on)
		if (overtype)
			VDcursize(C_HALF);
		else
			VDcursize(C_NORM);
	else
		VDcursize(C_OFF);
}




VOID
Wshow(line1, line2)
TLINENUM line1, line2;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *s;
	TLINENUM i;
	TWINDOW *w = &window[curwind];

	if (!changed || curwind == WNONE)
		return;
	if (line2 != line1)
		VDhold = TRUE;
	if (line1 < w->wblnum)
		line1 = w->wblnum;
	if (line2 > w->wblnum + w->wheight - 1)
		line2 = w->wblnum + w->wheight - 1;
	for (i = line1; i <= line2; i++) {
		s = Blget(i);
		if (s == NULL)
			sput(w->wabsrow + i - w->wblnum, " ");
		else
			sput(w->wabsrow + i - w->wblnum, s);
	}
	changed = FALSE;
}




VOID
Wpage(dir)
TDIR dir;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TLINENUM last;
	WORD lineoffset;
	TWINDOW *w = &window[curwind];

	lineoffset = Bline() - w->wblnum;
	last = Blastline();
	if (last <= w->wheight)
		return;
	if (dir == UP)
		if (w->wblnum <= w->wheight)
			w->wblnum = 1;
		else
			w->wblnum = w->wblnum - w->wheight;
	else
		if (w->wblnum + 2 * w->wheight > Blastline() + 1)
			w->wblnum = Blastline() - w->wheight + 1;
		else
			w->wblnum = w->wblnum + w->wheight;
	changed = TRUE;
	Wshow(1, MAXINT);
	Bseek(w->wblnum + lineoffset, UP);
}




VOID
Wtopbot(dir)
TDIR dir;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TWINDOW *w = &window[curwind];

	switch (dir) {
	case UP:
		Bseek(w->wblnum, LEFT);
		break;
	case DOWN:
		Bseek(w->wblnum + w->wheight - 1, LEFT);
	}
}




VOID
sput(row, s)
TROW row;
BYTE s[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TPOS pos;
	TCOL col;
	WORD expand, slen;
	BYTE ch, setup[91], *p;
	WORD start;
	TWINDOW *w = &window[curwind];

	p = setup;
	slen = strlen(s);
	pos = 1;
	expand = 0;
	for (col = 1; col <= w->wbcol + w->wwidth - 1; col++) {
		if (expand > 0) {
			expand--;
			ch = ' ';
		}
		else if (pos > slen)
			break;
		else if (s[pos - 1] == '\t') {
			expand = tabvalue(col) - 1;
			pos = pos + 1;
			ch = ' ';
		}
		else {
			ch = s[pos - 1];
			pos++;
		}
		if (col >= w->wbcol)
			*p++ = ch;
	}
	*p = '\0';
	if (Wshowstart == -1)
		Wshowstart = w->wbcol;
	start = Wshowstart - w->wbcol + 1;
	if (start <= w->wwidth && start > 0)
		VDhorout(row, w->wabscol + start - 1, setup, start,
		  w->wwidth - start + 1);
	Wshowstart = -1;
}




VOID
Wlopen()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TWINDOW *w = &window[curwind];
	TBUFFER *b = &buffer[curbuf];

	if (b->blnum >= w->wblnum && b->blnum < w->wblnum + w->wheight - 1)
		scroll(w->wabsrow + b->blnum + 1 - w->wblnum,
		  w->wabsrow + w->wheight - 2, w->wabscol,
		  w->wabscol + w->wwidth - 1, DOWN, 1);
}




VOID
Wldel()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TWINDOW *w = &window[curwind];
	TBUFFER *b = &buffer[curbuf];

	if (b->blnum >= w->wblnum && b->blnum < w->wblnum + w->wheight) {
		scroll(w->wabsrow + b->blnum + 1 - w->wblnum,
		  w->wabsrow + w->wheight - 1, w->wabscol,
		  w->wabscol + w->wwidth - 1, UP, 1);
		Wshow(w->wblnum + w->wheight - 1, w->wblnum + w->wheight - 1);
	}
}




static VOID
putmsg(s, extend)
BYTE s[];
BOOLEAN extend;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	static TCOL startcol;
	TCOL stopcol;
	WORD slen;

	slen = strlen(s);
	if (!extend)
		startcol = 1;
	stopcol = startcol + slen - 1;
	if (stopcol > MAXMSGCOL)
		stopcol = MAXMSGCOL;
	slen = stopcol - startcol + 1;
	VDattrib(A_INVERSE);
	VDhorout(TOTLINES, startcol, s, 1, slen);
	startcol = stopcol + 1;
	VDattrib(A_NORM);
	VDhorout(TOTLINES, startcol, "", 1, MAXMSGCOL - startcol + 1);
	Wstatus("..", 1, 1, 1, B1);
}




VOID
Wmsg(s)
BYTE s[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	putmsg(s, FALSE);
}




VOID
Wmsgx(s)
BYTE s[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	putmsg(s, TRUE);
}




VOID
Wintense(on, line1, line2)
BOOLEAN on;
TLINENUM line1, line2;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TROW r;
	TCOL c;
	BYTE ch;
	TLINENUM i;
	TWINDOW *w;
	TBUFFER *b;

	if (curwind == WNONE)
		return;
	w = &window[curwind];
	b = &buffer[curbuf];
	if (line1 < w->wblnum)
		line1 = w->wblnum;
	if (line2 >= w->wblnum + w->wheight)
		line2 = w->wblnum + w->wheight - 1;
	if ((line2 -line1 + 1) > 0) {
		for (i = line1; i <= line2; i++)
			VDintense(on, w->wabsrow + i - w->wblnum, w->wabscol,
			  w->wwidth);
		VDgo();
	}
}




VOID
Wmodeindicator()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	static BOOLEAN prevmode = FALSE;

	if (overtype) {
		VDattrib(A_INVERSE);
		VDhorout(TOTLINES - 2, 80, "O", 1, 1);
		VDattrib(A_NORM);
	}
	else if (overtype != prevmode)
		VDspecial(TOTLINES - 2, 80, S_H);
	prevmode = overtype;
}




VOID
Wstatus(name, col, line, last, b)
BYTE name[];
TCOL col;
TLINENUM line, last;
TBUFNUM b;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE s[66], f[66];
	WORD c, slen;
	static BYTE prev_name[66] = "..";
	static WORD prev_col = -1, prev_line = -1, prev_last = -1;
	static TBUFNUM prev_buf = BNONE;

	if (STREQ(name, "..")) {
		strcpy(prev_name, "..");
		prev_col = -1;
		prev_line = -1;
		prev_last = -1;
		prev_buf = BNONE;
		return;
	}
	Wmodeindicator();
	if (b == BC)
		return;
	if (!STREQ(name, prev_name)) {
		strcpy(prev_name, name);
		setmem(f, 40, ' ');
		SETLEN(f, 40);
		strcpy(s, name);
		slen = strlen(s);
		if (slen > 40) {
			SETLEN(s, 40);
			slen = 40;
		}
		if (slen <= 20)
			c = 21;
		else
			c = 40 - slen + 1;
		strcpy(&f[c - 1], s);
		VDhorout(TOTLINES - 1, 41, f, 1, 40);
	}
	if (b != prev_buf) {
		prev_buf = b;
		itoc(b, s);
		VDhorout(TOTLINES, 61, "B", 1, 1);
		VDhorout(TOTLINES, 62, s, 1, 2);
	}
	if (col != prev_col) {
		prev_col = col;
		sprintf(s, "%4d", col);
		VDhorout(TOTLINES, 65, s, 1, 4);
	}
	if (line != prev_line) {
		prev_line = line;
		sprintf(s, "%5d", line);
		VDhorout(TOTLINES, 70, s, 1, 5);
		VDhorout(TOTLINES, 75, "/", 1, 1);
	}
	if (last != prev_last) {
		prev_last = last;
		itoc(last, s);
		VDhorout(TOTLINES, 76, s, 1, 5);
	}
}




VOID
Wdisp(s)
BYTE s[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	if (++dline < TOTLINES)
		VDhorout(dline, 1, s, 1, strlen(s));
}




VOID
Wfinish()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
}




VOID
Whome()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	VDcurpos(1, 1);
}
  Dcurpos(1, 1); ************************************************/ ) );                                                                                                          ü'”ìø|ìø3Ø P             ü/´ìø˜ìøMd            œ        pÕþx×þ
  Dcurpos(1, 1); ************************************************/ ) );                                                          
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           