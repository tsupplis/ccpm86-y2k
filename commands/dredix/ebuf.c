#include "stdio.h"



#include "ctype.h"



#include "common.h"



#include "ecommon.h"



#include "ebuf.h"



#include "ewin.h"



#include "ecmdblk.h"



#include "emem.h"




TBUFNUM curbuf, prevbuf;

struct ST_TBUFFER buffer[N_TBUFNUM];

BOOLEAN Bfull; 					/* Is the heap full?	*/

#define NOCURCOL 0			/* no current column indicator	*/


TCOL
tabcol(s, apos)
BYTE s[];
TPOS apos;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TCOL col;
	TPOS pos;
	WORD slen;

	col = 1;

	slen = strlen(s);

	for (pos = 1; pos <= slen; pos++) {
		if (pos == apos)
			break;
		else if (s[pos - 1] == '\t')
			col += tabvalue(col);
		else
			col++;
	}

	return(col);

}




TPOS
tabpos(s, acol)
BYTE s[];
TCOL *acol;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TCOL col;
	TPOS pos;
	WORD slen;

	slen = strlen(s);
	col = 1;
	pos = 1;
	while (pos <= slen) {
		if (col >= *acol)
			break;
		else if (s[pos - 1] == '\t')
			col += tabvalue(col);
		else
			col++;
		pos++;
	}
	*acol = col;
	return(pos);
}




TCOL
tabvalue(col)
TCOL col;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TCOL c;
	BYTE *map;
	WORD maplen, tabv;

	map = buffer[curbuf].btabmap;
	maplen = strlen(map);
	tabv = 1;
	for (c = col; c < maplen; c++) {
		if (map[c] != ' ')
			break;
		tabv++;
	}
	return(tabv);
}




static VOID
Binitone(bf, name)
TBUFNUM bf;
BYTE name[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TBUFFER *b = &buffer[bf];

	Mreset(bf);
	b->blnum = 1;
	b->bpos = 1;
	b->bcol = 1;
	b->bcurcol = 0;
	strcpy(b->bfile, name);
	b->blast = 1;
	b->bmod = FALSE;
}




VOID
Binit()
/****************************************************************/
/*								*/
/*		Buffer INITialization				*/
/*								*/
/****************************************************************/
{
	TBUFNUM b;
	BYTE s[51];
	WORD i;


	if ((buffer[BC].btabmap = malloc(MAXTAB + 1)) == NULL)
		fatal("Binit");

	SETLEN(buffer[BC].btabmap, MAXTAB);

	for (i = 1; i <= MAXTAB; i++) {
		if ((i - 1) % 8 == 0)
			buffer[BC].btabmap[i - 1] = 'X';
		else
			buffer[BC].btabmap[i - 1] = ' ';
	}

	Bfull = FALSE;

	for (b = LOWERB; b <= UPPERB; b++) {
		Binitone(b, "");
		buffer[b].btabmap = buffer[BC].btabmap;
	}

	curbuf = B1;
	prevbuf = B1;
}




BYTE
*Bfname()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	return(buffer[curbuf].bfile);
}




VOID
Breset(name)
BYTE name[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	Binitone(curbuf, name);
}




VOID
Bselect(n)
TBUFNUM n;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	Wselect(Wwhich(n)); /*might select WNONE*/
	curbuf = n;
}




VOID
Brestore()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	Bselect(prevbuf);
}




VOID
Bsave()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	prevbuf = curbuf;
}




VOID
Bseek(lnum, dir)
TLINENUM lnum;
TDIR dir;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *text;
	TBUFFER *b = &buffer[curbuf];

	if (lnum > b->blast)
		lnum = b->blast;
	b->blnum = lnum;
	text = Mget(curbuf, b->blnum);
	switch (dir) {
	case LEFT:
		b->bpos = 1;
		while (isspace(text[b->bpos - 1]))
			b->bpos = b->bpos + 1;
		break;
	case RIGHT:
		b->bpos = strlen(text) + 1;
		break;
	default:
		if (b->bcurcol == 0)
			b->bcurcol = b->bcol;
		else    
			b->bcol = b->bcurcol;
		b->bpos = tabpos(text, &(b->bcol));
	}
	b->bcol = tabcol(text, b->bpos);
}




VOID
Bseekpos(lnum, apos)
TLINENUM lnum;
TPOS apos;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *text;
	WORD textlen;
	TBUFFER *b = &buffer[curbuf];

	if (lnum > b->blast)
		lnum = b->blast;
	b->blnum = lnum;
	text = Mget(curbuf, b->blnum);
	textlen = strlen(text) + 1;
	if (apos > textlen)
		apos = textlen;
	b->bpos = apos;
	b->bcol = tabcol(text, b->bpos);
}




TLINENUM
Bline()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	return(buffer[curbuf].blnum);
}




TLINENUM
Blastline()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	return(buffer[curbuf].blast);
}




BOOLEAN
Btravel(dir)
TDIR dir;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *text;
	TBUFFER *b = &buffer[curbuf];

	switch (dir) {
	case RIGHT:
		text = Mget(curbuf, b->blnum);
		if (b->bpos == strlen(text) + 1)
			if (b->blnum == b->blast)
				return(FALSE);
			else {
				b->blnum++;
				b->bpos = b->bcol = 1;
				break;
			}
		b->bpos = b->bpos + 1;
		if (text[b->bpos - 2] == '\t')
			b->bcol = tabcol(text, b->bpos);
		else
			b->bcol = b->bcol + 1;
		break;
	case LEFT:
		text = Mget(curbuf, b->blnum);
		if (b->bpos == 1)
			if (b->blnum == 1)
				return(FALSE);
			else {
				Bseek(b->blnum - 1, RIGHT);
				break;
			}
		b->bpos = b->bpos - 1;
		if (text[b->bpos - 1] == '\t')
			b->bcol = tabcol(text, b->bpos);
		else
			b->bcol = b->bcol - 1;
		break;
	case DOWN:
		if (b->blnum == b->blast)
			return(FALSE);
		b->blnum = b->blnum + 1;
		text = Mget(curbuf, b->blnum);
		if (b->bcurcol == NOCURCOL)
			b->bcurcol = b->bcol;
		else
			b->bcol = b->bcurcol;
		b->bpos = tabpos(text, &(b->bcol));
		break;
	case UP:
		if (b->blnum == 1)
			return(FALSE);
		b->blnum = b->blnum - 1;
		text = Mget(curbuf, b->blnum);
		if (b->bcurcol == NOCURCOL)
			b->bcurcol = b->bcol;
		else
			b->bcol = b->bcurcol;
		b->bpos = tabpos(text, &(b->bcol));
	}
	return(TRUE);
}




#define FINDSPACE 0
#define FINDNONSPACE 1


VOID
Bwdtravel(dir)
TDIR dir;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *text;
	WORD state;
	BOOLEAN isspc, done;
	TBUFFER *b = &buffer[curbuf];

	text = Mget(curbuf, b->blnum);
	switch (dir) {
	case RIGHT:
		state = FINDSPACE;
		done = FALSE;
		while (TRUE) {
			isspc = !isalnum(text[b->bpos - 1]);
			switch (state) {
			case FINDSPACE:
				if (isspc)
					state = FINDNONSPACE;
				break;
			case FINDNONSPACE:
				if (!isspc)
					done = TRUE;
			}
			if (done)
				break;
			if (b->bpos == strlen(text) + 1) {
				if (b->blnum == b->blast)
					break;
				b->blnum++;
				text = Mget(curbuf, b->blnum);
				b->bpos = 1;
			}
			else
				b->bpos++;
		}
		b->bcol = tabcol(text, b->bpos);
		break;
	case LEFT:
		state = FINDNONSPACE;
		done = FALSE;
		while (!done) {
			if (b->bpos == 1) {
				if (b->blnum == 1)
					break;
				b->blnum--;
				text = Mget(curbuf, b->blnum);
				b->bpos = strlen(text) + 1;
			}
			else
				b->bpos--;
			isspc = !isalnum(text[b->bpos - 1]);
			switch (state) {
			case FINDSPACE:
				if (isspc) {
					if (b->bpos == strlen(text) + 1) {
						b->blnum++;
						text = Mget(curbuf, b->blnum);
						b->bpos = 1;
					}
					else
						b->bpos++;
					done = TRUE;
					break;
				}
				else if (b->bpos == 1) {
					done = TRUE;
					break;
				}
			case FINDNONSPACE:
				if (!isspc)
					state = FINDSPACE;
			}
		}
		b->bcol = tabcol(text, b->bpos);
	}
}




BYTE
*Blget(lnum)
TLINENUM lnum;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TBUFFER *b = &buffer[curbuf];

	if (lnum > b->blast)
		return(NULL);
	else
		return(Mget(curbuf, lnum));
}




BYTE
*Blgetcur()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	return(Blget(buffer[curbuf].blnum));
}




static VOID
delpush(s)
BYTE s[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TWNAME w;

	Wsave();
	Bsave();
	Bselect(BD);
	if (Blastline() > MAXUNDO) {
		Bseek(1, LEFT);
		Bldel(FALSE);
		Bseek(MAXINT, LEFT);
	}
	Blapp(s);
	Brestore();
	Wrestore();
}




VOID
Bdelpop()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *s;

	if (buffer[BD].blast == 1) {
		Wmsg("Empty delete stack");
		return;
	}
	Bsave();
	Bselect(BD);
	s = Blgetcur();
	Brestore();
	Blins(s);
	Bselect(BD);
	Bldel(FALSE);
	Brestore();
}




VOID
Bldel(stackok)
BOOLEAN stackok;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TLINENUM n;
	BYTE *text;
	TBUFFER *b = &buffer[curbuf];

	text = Mget(curbuf, b->blnum);
	if (b->blast == 1 && strlen(text) == 0) {
		Wmsg("Empty buffer");
		return;
	}
	if (stackok)
		delpush(text);
	n = b->blnum;
	b->bpos = 1;
	b->bcol = 1;
	if (b->blast == 1) {
		SETLEN(text, 0);
		changed = TRUE;
		Wshow(b->blast, b->blast);
	}
	else {
		Mdel(curbuf, b->blnum);
		b->blast = b->blast - 1;
		changed = TRUE;
		Wldel();
		if (b->blnum > b->blast)
			b->blnum = b->blast;
	}
	delfix(n);
	b->bmod = TRUE;
}




VOID
Blapp(s)
BYTE s[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TBUFFER *b = &buffer[curbuf];

	if (!Mapp(curbuf, b->blnum, s))
		return;
	appfix(b->blnum);
	b->blast++; /*moved*/
	Wlopen();
	b->blnum++;
	b->bpos = 1;
	b->bcol = 1;
	changed = TRUE;
	Wshow(b->blnum, b->blnum);
	b->bmod = TRUE;
}




VOID
Blins(s)
BYTE s[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *newstr;
	TBUFFER *b = &buffer[curbuf];

	if (!Mapp(curbuf, b->blnum - 1, s)) /*blnum - 1 might be zero*/
		return;
	appfix(b->blnum - 1);
	Wlopen(); /*opens at wrong place*/
	b->blast++;
	b->bpos = 1;
	b->bcol = 1;
	changed = TRUE;
	Wshow(b->blnum, b->blnum + 1);
	b->bmod = TRUE;
}




VOID
Bcins(c)
BYTE c;
/****************************************************************/
/*								*/
/*	BCINS - Buffer Character INSert				*/
/*								*/
/****************************************************************/
{
	TPOS copypos, pos;
	TCOL m;
	BYTE newtext[256];
	BYTE *text;
	WORD i;
	TBUFFER *b = &buffer[curbuf];
	BOOLEAN allblank;

	m = margin;

	text = Mget(curbuf, b->blnum);

	if (c == '\r') {
		pos = b->bpos;
		copypos = pos;
		if (copypos > 1)
			while (isspace(text[copypos - 1]))
				copypos = copypos + 1;

		strcpy(newtext, &text[copypos - 1]);
		SETLEN(text, pos - 1);
		changed = TRUE;
		Wshowstart = tabcol(text, b->bpos);
		Wshow(b->blnum, b->blnum);
		Blapp(newtext);
	}
	else {
		if (!overtype || text[b->bpos - 1] == '\0') {
			if (!Mneed(curbuf, b->blnum, strlen(text) + 1))
				return;

			text = Mget(curbuf, b->blnum); /*maybe reallocated*/

			for (pos = strlen(text) + 2; pos >= b->bpos; pos--)
				text[pos - 1] = text[pos - 2];
		}

		Wshowstart = b->bcol;	
		text[b->bpos - 1] = c;		/* insert the character	*/
		b->bpos = b->bpos + 1;		/* increment pos counter*/
		b->bcol = tabcol(text,b->bpos);
		changed = TRUE;			/* mark text as changed	*/
		Wshow(b->blnum,b->blnum);
	}

	b->bmod = TRUE;				/* buffer is modified	*/

}




VOID
Bcdel()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *next, *text;
	TLINENUM lnum;
	TPOS pos, copypos;
	TCOL col;
	WORD textlen;
	TBUFFER *b = &buffer[curbuf];

	text = Mget(curbuf, b->blnum);
	textlen = strlen(text);
	if (textlen == 0) {
		Bldel(TRUE);
		return;
	}
	if (b->bpos == textlen + 1) {
		if (b->blnum == b->blast) {
			Wmsg("Can't delete");
			return;
		}
		next = Mget(curbuf, b->blnum + 1); /*next & text must be in core!*/
		if (!Mneed(curbuf, b->blnum, textlen + strlen(next)))
			return;
		text = Mget(curbuf, b->blnum); /*Mneed might have reallocated*/
		strcat(text, " ");
		copypos = 1;
		while (isspace(next[copypos - 1]))
			copypos++;
		delete(next, 1, copypos - 1);
		strcat(text, next);
		lnum = b->blnum;
		pos = b->bpos;
		col = b->bcol;
		b->blnum++;
		Bldel(TRUE);
		b->blnum = lnum;
		b->bpos = pos;
		b->bcol = col;
	}
	else
		delete(text, b->bpos, 1);
	changed = TRUE;
	Wshowstart = b->bcol;
	Wshow(b->blnum, b->blnum);
	b->bmod = TRUE;
}




VOID
Bfancydel()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *text;
	TPOS endpos;
	WORD textlen;
	TBUFFER *b = &buffer[curbuf];

	text = Mget(curbuf, b->blnum);
	textlen = strlen(text) + 1;
	endpos = textlen - 1;
	if (endpos >= b->bpos)
		delete(text, b->bpos, endpos - b->bpos + 1);
	changed = TRUE;
	Wshowstart = b->bcol;
	Wshow(b->blnum, b->blnum);
	b->bmod = TRUE;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                             