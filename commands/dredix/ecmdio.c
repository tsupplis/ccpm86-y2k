#include "stdio.h"



#include "ctype.h"



#include "common.h"



#include "ebuf.h"



#include "ewin.h"



#include "ekey.h"



#include "ehelp.h"



#include "ecmdblk.h"



#include "ecmdio.h"




BOOLEAN
filecmd()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *s;

	if (!ask("File", MAXFNAME, "", &s))
		return(FALSE);
	toupr(s, buffer[curbuf].bfile);
	return(TRUE);
}




VOID
bufcmd()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *s;
	TBUFNUM b, bi;

	while (TRUE) {
		if (!ask("Buffer number or file", MAXFNAME, "", &s))
			return;
		if (strlen(s) > 1 || !ctoi(s, &b) || b < B1 || b > B4) {
			b = BNONE;
			for (bi = B1; bi <= B4; bi++)
				if (STREQ(s, buffer[bi].bfile)) {
					b = bi;
					break;
				}
			if (b == BNONE) {
				Wmsg("No buffer has that file");
				continue;
			}
		}
		Wchgbuf(b);
		return;
	}
}




VOID
readfile(name, fresh)
BYTE name[];
BOOLEAN fresh;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BOOLEAN first, readok, havemsg;
	BYTE s[MAXPOS + 1];
	FILE *fil;
	TLINENUM seekplace;

	readok = FALSE;
	havemsg = FALSE;
	if (fresh)
		seekplace = 1;
	else
		seekplace = buffer[curbuf].blnum + 1;
	changed = TRUE;
	Wsave();
	Wselect(WNONE);
	Whome();
	if (STREQ(name, "") || (fil = fopen(name, "r")) == NULL) {
		havemsg = TRUE;
		Wmsg("Can't open ");
		Wmsgx(name);
		if (fresh)
			Wmsgx("; new file assumed");
	}
	else {
		Wmsg("Reading ");
		Wmsgx(name);
		first = fresh;
		Whome();
		while (fgets(s, sizeof(s), fil) != NULL) {
			s[strlen(s) - 1] = '\0';
			Blapp(s);
			if (Bfull)
				break;
			if (first) {
				first = FALSE;
				Bseek(1, LEFT);
				Bldel(FALSE);
			}
		}
		readok = ferror(fil) == 0;
		fclose(fil);
	}
	Bseek(seekplace, LEFT);
	Wrestore();
	Wshow(1, MAXINT);
	if (!Bfull && !havemsg)
		if (readok)
			Wmsg("Read OK");
		else
			Wmsg("Read error");
}




VOID
edit(name)
BYTE name[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	if (!modcheck(curbuf))
		return;

	if (STREQ(name, "")) {
		if (!filecmd())
			return;
		Breset(Bfname());
	}
	else
		Breset(name);

	readfile(Bfname(), TRUE);

	clearmarks();

	buffer[curbuf].bmod = FALSE;

}




VOID
readin()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE savefile[MAXFNAME + 1], thisfile[MAXFNAME + 1];
	BYTE *text;
	TBUFFER *b = &buffer[curbuf];

	if (b->blast == 1 && STREQ(b->bfile, "") && b->bmod == FALSE) {
		text = Blgetcur();
		if (strlen(text) == 0) {
			edit("");
			return;
		}
	}
	strcpy(savefile, b->bfile);
	if (!filecmd())
		return;
	strcpy(thisfile, b->bfile);
	strcpy(b->bfile, savefile);
	readfile(thisfile, FALSE);
}




VOID
writeout()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TLINENUM i;
	BYTE *s;
	FILE *fil;
	BYTE backname[MAXFNAME + 1], filename[MAXFNAME + 1];
	WORD namestart, dot, len;
	BOOLEAN err;

/*if DEMO
	Wmsg("Demo only--can""t write")
else*/
	strcpy(filename, Bfname());
	while (STREQ(filename, "")) {
		if (!filecmd())
			return;
		strcpy(filename, Bfname());
	}
	strcpy(backname, filename);
	len = strlen(backname);
	for (dot = len - 1; dot >= 0; dot--)
		if (backname[dot] == '.')
			break;
	if (dot >= 0)
		SETLEN(backname, dot);
	strcat(backname, ".BAK");
	Wmsg("Opening ");
	Wmsgx(filename);
	Whome();
	unlink(backname);
	rename(filename, backname);
	if (STREQ(filename, "") || (fil = fopen(filename, "w")) == NULL) {
		Wmsg("Can't create ");
		Wmsgx(filename);
		return;
	}
	Wmsg("Writing ");
	Wmsgx(filename);
	err = FALSE;
	for (i = 1; ;i++) {
		s = Blget(i);
		if (s == NULL)
			break;
		else {
			Whome();
			if (fputs(s, fil) == EOF ||
			  fputc('\n', fil) == EOF) {
				err = TRUE;
				break;
			}
		}
	}
	Whome();
	if (fputc('\32', fil) == EOF)
		err = TRUE;
	if (!err) {
		if (fclose(fil) == -1)
			err = TRUE;
	}
	else
		fclose(fil);
	if (!err) {
		buffer[curbuf].bmod = FALSE;
		Wmsg("Written OK");
	}
	else {
		Wmsg("Write error");
		beep();
	}
/*  Stuff for device full--not detectable by Lattice C
		Serase(filename);
		if (backname != "")
			Srename(backname, filename);
		helpset("dvcfull");
		helpcmd;
		Wmsg("Device full")
*/
}




BOOLEAN
ask(prompt, limit, deflt, reply)
BYTE prompt[], deflt[];
TPOS limit;
BYTE **reply;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	TKEY k;
	BYTE c;
	BYTE nprompt[81];
	BOOLEAN done, result, first;
	WORD len;

	if (prompt[1] == '>')
		helpset(">");
	else
		helpset(prompt);
	Wsave();
	Wselect(WC);
	Breset("");
	strcpy(nprompt, deflt);
	Blapp(nprompt);
	Bseek(1, LEFT);
	Bldel(FALSE);
	Blapp("");
	strcpy(nprompt, prompt);
	c = nprompt[strlen(nprompt) - 1];
	if (c != '>' && c != ' ')
		strcat(nprompt, ": ");
	Wlabel(nprompt);
	done = FALSE;
	first = TRUE;
	while (!done) {
		Wcursor(FALSE);
		Wshow(1, MAXINT);
 		Woncursor(TRUE);
		if (first) {
			first = FALSE;
			VDsync();
			update_phys();
		}
		intensify(TRUE);
		k = keyhit(&c);
		Woncursor(FALSE);
		intensify(FALSE);
		switch (k) {
		case KUP:
			Btravel(UP);
			break;
		case KDOWN:
			Btravel(DOWN);
			break;
		case KLEFT:
			Btravel(LEFT);
			break;
		case KRIGHT:
			Btravel(RIGHT);
			break;
		case KHOME:
			Bseek(1, LEFT);
			break;
		case KDOWNHOME:
			Bseek(MAXINT, RIGHT);
			break;
		case KBGN:
			Bseek(Bline(), LEFT);
			break;
		case KEND:
			Bseek(Bline(), RIGHT);
			break;
		case KDC:
			Bcdel();
			break;
		case KDL:
			Bldel(FALSE);
			break;
		case KREPLMARK:
			Bcins(REPLMARK);
			break;
		case KENTER:
			*reply = Blgetcur();
			if (strlen(*reply) <= limit) {
				result = TRUE;
				done = TRUE;
				break;
			}
			Wmsg("Response too long");
			break;
		case KLIT:
			if (c == '\b') {
				if (Btravel(LEFT))
					Bcdel();
			}
			else
				Bcins(c);
			break;
		case KHELP:
			helpcmd();
			Wlabel(nprompt);
			changed = TRUE;
			break;
		case KQUIT:
			result = FALSE;
			done = TRUE;
			break;
		default:
			Wmsg("Invalid command");
		}
	}
	Wlabel("");
	Wrestore();
	helpset("default");
	return(result);
}




BOOLEAN
askint(prompt, mn, mx, n)
BYTE prompt[];
WORD mn, mx, *n;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *s;

	while (TRUE) {
		if (!ask(prompt, 6, "", &s))
			return(FALSE);
		if (!ctoi(s, n))
			Wmsg("Number required");
		else if (*n < mn || *n > mx)
			Wmsg("Out of range");
		else {
			Wmsg("");
			return(TRUE);
		}
	}
}




BOOLEAN
confirm(aprompt)
BYTE aprompt[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *reply;
	BYTE prompt[81];

	strcpy(prompt, aprompt);
	strcat(prompt, " (y/n)? ");
	while (TRUE) {
		if (!ask(prompt, 1, "", &reply))
			return(FALSE);
		if (strlen(reply) != 1 || (toupper(reply[0]) != 'Y') &&
		  toupper(reply[0]) != 'N') {
			Wmsg("Invalid response");
			continue;
		}
		Wmsg("");
	  	return(toupper(reply[0]) == 'Y');
	}
}




BOOLEAN
modcheck(b)
TBUFNUM b;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	BYTE *reply;

	if (buffer[b].bmod)
		return(confirm("OK to lose changes"));
	return(TRUE);
}




BOOLEAN
bufopen(mustcheck)
BOOLEAN mustcheck;
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	Bseek(1, LEFT);
	Wcursor(FALSE);
	if (mustcheck && !modcheck(curbuf))
		return(FALSE);
	Breset("");
	Wsave();
	Wselect(WNONE);
	return(TRUE);
}




VOID
bufwrite(s)
BYTE s[];
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	Blapp(s);
}




VOID
bufclose()
/****************************************************************/
/*								*/
/*								*/
/****************************************************************/
{
	Bseek(1, LEFT);
	if (buffer[curbuf].blast > 1)
		Bldel(FALSE);
	buffer[curbuf].bmod = FALSE;
	Wrestore();
	changed = TRUE;
	Wshow(1, MAXINT);
}
                                                                                                                                                                                                                                                                                                                                                                      