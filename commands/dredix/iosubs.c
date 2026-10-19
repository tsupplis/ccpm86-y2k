/*	IOSUBS.C    screen I/O subroutines		      */

/*		Copyright (c) 1984  RTEK Inc.		      */
/*				    2265 E. Becker Lane       */
/*				    Phoenix, Arizona 85028    */
/*				    (602) 867-9336	      */
/*	last mod:  4 MAR 85...JMB...serial console support    */
/*	new source lines marked by /*;			      */

/*** INCLUDE FILES ***********************************************************/

#include <stdio.h>
#define	 putchar_1(c)	putc_1(c,stdout)
#include <ctype.h>
#include "fkeys.h"
#include "iosubs.h"
#include "attr.h"

/*** DEFINES ***************************************************************/

#define BACKSPACE 0x08
#define BTAB 0x0c
#define TAB 0x09
#define RETCHAR 0x0d
#define ESCAPE 0x1b
#define CTL_C 0x03
#define BELL 0x07
#define CTL_Z 0x1a
#define CTL_E 0x05
#define CTL_F 0x06		/*; cursor left */
#define CTL_X 0x18
#define CTL_D 0x04
#define CTL_H 0x08
#define CTL_G 0x07
#define CTL_O 0x0f

/*** EXTERNAL FUNCTION DECLARATIONS *******************************************/

EXTERN	update_phys();
EXTERN	WM_MXQ();
EXTERN	FDRAW();
EXTERN	SDRAW();
EXTERN	FREPEAT();

/*** EXTERNAL VARIABLE DECLARATIONS *******************************************/


/*** GLOBAL VARIABLES *********************************************************/

MLOCAL	WORD	dotpos; 		/* index of decimal point in string */
MLOCAL	WORD	maxdind;		/* max string index for numbers */
MLOCAL	WORD	maxdcol;		/* max column no. for numbers */
MLOCAL	WORD	line_len;		/* length of 1 line on screen	*/
MLOCAL	WORD	lastchar;		/* last char posn of string	*/

/*** SUB FUNCTIONS ************************************************************/

/*** Get a FIELD from the screen **********************************************/

WORD	g_field(fp, sp)

struct	field	*fp;		    /* pointer to the field structure */
	BYTE	*sp;		    /* the string to return the field */

{
	WORD	ret;		    /* for return from c_field */

    fp->col = 0;		    /* reset the starting cursor posn */
    fp->line = 0;
    return (ret = c_field(fp, sp)); /* get the field and return */
}

/*** get Continued FIELD from the screen ************************************/
/*	Same as g_field except cursor posn is not reset     */

WORD	c_field(fp, sp)

struct	field	*fp;		    /* pointer to the field structure */
	BYTE	*sp;		    /* the string to return the field */

{
EXTERN	BYTE	*cur_attr;	    /* current attribute defintion table */
	WORD	ret;		    /* for return from getcbox */
	WORD	attr_save;	    /* temp save area for attribute */

    attr_save = fp->attr;	    /* save current attribute */
    fp->attr = INP_A;		    /* set attr to input attribute */
    ret = getcbox(fp, sp);	    /* get the field and return */
    if (fp->attr != attr_save)
    {
	fp->attr = attr_save;	    /* restore old attribute */
	scr_beg();
	putcbox(fp, sp);	    /* print the input with normal attr */
	scr_end();
    }
    return(ret);		    /* return to user with answer */
}
/*** get ANY KEY from the keyboard *******************************************/

any_key()
{
	WORD	c;		    /* a character */

/*;    putchar(ESCAPE);		    /* turn cursor off */
/*;    putchar('f');				       */
    cursor_off();		  /*;*/

    c = getraw();		    /* get a character from the user */

/*;    putchar(ESCAPE);		    /* turn cursor back on */
/*;    putchar('e');					   */
    cursor_on();

    return(c);
}

/*** GET RAW character from keyboard ******************************************/

getraw() 	/* Raw keyboard get character */
{
	return (__BDOS(0x06,0xfd));
}

/*** CLeaR BUFfer of any type ahead ******************************************/

clrbuf()
{

	while (  __BDOS( 0x06, 0xfe ) & 0xff != 0 )
	    __BDOS(0x06, 0xff);
}
/*** Print a STRing on the screen ********************************************/

VOID	pstr( string, col, line, length, attr )

	BYTE	string[];
	WORD	col;
	WORD	line;
	WORD	length;
	WORD	attr;
{
EXTERN	WORD	vc_segment;		    /* address of virtual image */
EXTERN	BYTE	*cur_attr;		    /* current attribute table */

    FDRAW( string, (line * 80 + col) * 2, length, cur_attr[attr], vc_segment);
}

/*** CLeaR a STRing on the screen ********************************************/

VOID	clrstr( col, line, length, attr )

	WORD	col;
	WORD	line;
	WORD	length;
	WORD	attr;
{
EXTERN	WORD	vc_segment;
EXTERN	BYTE	*cur_attr;		/* current attribute table */

    FREPEAT(' ', (line * 80 + col) * 2, length, cur_attr[attr], vc_segment);
}

/*** Get a STRing from the user **********************************************/

WORD	gstr(string, col, line, length, attr, fkeys)

	BYTE	string[];
	WORD	col;
	WORD	line;
	WORD	length;
	WORD	attr;
	WORD	fkeys;
{
MLOCAL	struct	field	p;

    p.left = col;
    p.right = col + length - 1;
    p.upper = line;
    p.lower = line;
    p.fkeys = fkeys;
    p.type  = 0;
    p.attr  = attr;
    p.col   = col;
    p.line  = line;

    return( getcbox( &p, string ) );
}

/*** PUT a Character BOX on the screen (fast version for CCP/M 3.1) ***********/

VOID putcbox(p,string)
struct	field *p;
	BYTE	string[];
{
	WORD	r;				/* current row */
	WORD	i;				/* temp */
	WORD	line_len;
	WORD	length;
EXTERN	WORD	vc_segment;			/* seg addr of virt screen */
EXTERN	BYTE	*cur_attr;			/* current attribute table */

	line_len = p->right - p->left + 1;	/* length of 1 line */
	length = strlen(string);		/* length of string   */

	i = 0;					/* string offset */
	r = p->upper;
						/* print non blank part */
	while (i <= length && r <= p->lower )
	{
	    FDRAW( &string[i], (r++ * 80 + p->left) * 2,
		   line_len, cur_attr[p->attr], vc_segment );
	    i += line_len;
	}
						/* clear the rest */
	while (r <= p->lower)
	    FREPEAT(' ', (r++ * 80 + p->left) * 2,
		    line_len, cur_attr[p->attr], vc_segment );
	return;
}

/*** PUT a Character BOX on the screen (slow version for non CCP/M 3.1) *******/

VOID putsbox(p,string)
struct	field *p;
	BYTE	string[];
{
	WORD	r;				/* current row */
	WORD	i;				/* temp */
	WORD	line_len;
EXTERN	BYTE	*cur_attr;			/* current attribute table */

	line_len = p->right - p->left + 1;	/* length of 1 line */

	i = 0;					/* string offset */
	r = p->upper;
	set_attr(cur_attr[p->attr]);		/* set proper attribute */

	while ( r <= p->lower )
	{
	    pcursor(p->left, r++);		/* position cursor */
	    SDRAW( &string[i], 0, line_len);	/* print part of string */
	    i += line_len;
	}
	return;
}

/*** Position CURSOR ********************************************************/

/*;VOID pcursor(col, line) 		new name */
VOID sq_pcursor(col, line)

	WORD	col,line;
{
	BYTE	c;
	putchar(ESCAPE);
	putchar('Y');
	putchar((c = line) + ' ');
	putchar((c = col) + ' ');
}

/*** Get Character BOX on screen   ******************************************/

/* routine to input/edit a character string   
   the character string is editted within a box defined
   by giving the routine the upper left and lower right
   coordintates in a struct field.
	left = left most column
	upper = upper most row
	right = right most column
	lower = lower most row
	type = 0 for an alpha field
	       1 for an upper case only alpha field
	       2 for a numeric field
	       3 for lower case alpha with character loss at end of lines
	fkeys = a bit mask telling what keys are valid input terminators
		bits 1 - 10 = F1 - F10
		bit 11 = Return
		bit 12 = Escape
		bit 13 = tab
		bit 14 = Control c
		bit 15 = Control z
		bit 16 = at end of box (when cursor 1st goes out of box at end)
	attr = attributes for the field being entered
		high nibble is background as defined in CCP/M manual
		low nibble is foreground as defined in CCP/M manual
	sign = whether a '-' allowed 0 => no; 1 => yes
	dplace = number of places to right of decimal point 0 => integer
	Returns the bit number of the terminator (1-16 or -13 for backtab)
									*/
getcbox(p,string)

struct	field	*p;
	UBYTE	string[];
{

MLOCAL	UBYTE	c;			/* current character		*/
MLOCAL	WORD	i,j;			/* temps			*/
MLOCAL	UBYTE	*ps;			/* temp pointer to string	*/
MLOCAL	UBYTE	*ps1;			/* temp pointer to string	*/
MLOCAL	UBYTE	*ps2;			/* temp pointer to string	*/

    /* Adjust the length of the string presented to be the size of the box */

	line_len = p->right - p->left + 1;		 /* length of line */
	lastchar = line_len * (p->lower - p->upper + 1); /* last char posn */
	if (p->type !=3)
	    string[lastchar--] = '\0';		/* mark string full length */
	i = strlen(string);			/* blank fill string to max */
	while (i <= lastchar)
	    string[i++] = ' ';

/*  Initialize for numeric field processing  */

	if (p->type == 2)
	{					/* numeric init */
	    dotpos = lastchar + 1;
	    for (i = 0; i <= lastchar; i++)
	    {
		if (string[i] == '.')
		{
		    dotpos = i;
		    break;
		}
	    }
	    maxdind = MIN(lastchar, dotpos + p->dplace);
	    maxdcol = MIN(p->right, maxdind + p->left);
	    for (i = maxdind + 1; i <= lastchar; i++)
		string[i] = ' ';		/* clear right of dec */
	}					/* end numeric init */
	set_attr(p->attr);			/* set colors */
	scr_beg();
	putcbox(p,string);			/* print string */
	scr_end();
	if (p->col < p->left || p->col > p->right)
	    p->col = p->left;
	if (p->line < p->upper || p->line > p->lower)
	    p->line = p->upper;
	pcursor( p->col, p->line );		       /* position cursor */

	while (TRUE) 
	{
	    c = getraw();
	    if (p->type == 2)			 /* numeric field? */
	    {
		if (c >= '0' && c <= '9')	/* a digit? */
		{
		    i = strind(p);		/* index into string */
		    if (i == dotpos)		/* wipe out dec point as reqd */
		    {
			dotpos = lastchar + 1;	/* set posn of dot past rite */
			maxdind = lastchar;	/* set max storage loc */
			maxdcol = p->right;	/* and max column */
		    }
		    string[i] = c;		/* store character */
		    putchar_1(c); 		/* show user the char  */
		    if (p->col < maxdcol)	   /* increment col if ok */
		       p->col += 1;
		    else			/* otherwise, reposn cursor */
		    {
			pcursor(p->col,p->line);
			if (p->fkeys & ATENDBIT) /* and return if requested */
			    return(ATENDRET);
		    }
		    continue;			/* get another character */
		}				/* end of digit processing */

/*  Process '-' sign as required */

		if (c == '-')			/* minus sign processing  */
		{
		    if (p->col == p->left && (p->sign))
		    {
			putchar_1(c);
			string[0] = '-';
			p->col += 1;
		    }
		    else
		    {
			putchar(BELL);
		    }
		    continue;			/* get next character */
		}  /* end of minus sign processing */

/* Process '.' character as required */

		if (c == '.')			/* decimal point processing */
		{
		    if (p->dplace)		/* decimal allowed? */
		    {
			deldot(p,string);	/* delete current decimal */
			dotpos = strind(p);	/* current position in str */
			string[dotpos] = c;	/* store the decimal in str */
			maxdind = MIN(lastchar,dotpos + p->dplace); /* end ind*/
			maxdcol = MIN(p->right, maxdind + p->left); /* end col*/
			for (i = maxdind + 1; i <= lastchar; i++)
			    string[i] = ' ';	/* clear extra digits as reqd */
			scr_beg();
			putcbox(p,string);	/* print new string */
			scr_end();
			if (p->col < maxdcol)	   /* incr cursor posn */
			    p->col += 1;
			pcursor(p->col,p->line);      /* re-posn the cursor */
		    }	/* end dec place allowed */
		    else			/* decimal point not allowed */
			putchar(BELL);
		    continue;
		}   /* end of decimal point processing */
	    } /* end of numeric field processing */

	    else
	    {					/* alpha field processing */
		if (!(c & 0x80) && c >= ' ') 
		{				/* regular character */
		    if (p->type == 1)
			c = toupper(c); 	/* xlate to upper case */
		    string[strind(p)] = c;	/* store character   */
		    putchar_1(c); 		/* print the char */
		    p->col += 1;		/* increment cursor posn */
		    if (p->col > p->right)
		    {				/* see if out of box */
			if (p->line == p->lower)
			{				/* is out of box */
			    p->col -= 1;		/* backup cursor */
			    if (p->fkeys & ATENDBIT)   /* chk return */
				return(ATENDRET);
			    else
			    {
				pcursor(p->col,p->line);
			    }
			}
			else
			{		    /* not out, go to next row */
			    p->col = p->left;
			    p->line += 1;
			    pcursor(p->col,p->line);
			}
		    } /* end if out of box */
		    continue;
		} /* end regular character processing */
	    }	/* end alpha field processing */

/* Process special field termination characters */

	    if (c == RETCHAR && (p->fkeys & RETBIT))	 /* check return done */
		return(RETRET);

	    if (c == ESCAPE && (p->fkeys & ESCBIT))	/* check escape done */
		return(ESCRET);

	    if (c == TAB && (p->fkeys & TABBIT))	/* check tab done */
		return(TABRET);

	    if (c == BTAB && (p->fkeys & TABBIT))	/* check tab done */
		return(BTABRET);

	    if (c == CTL_C && (p->fkeys & CTLCBIT))	/* check ctl c done */
		return(CTLCRET);

	    if (c == CTL_Z && (p->fkeys & CTLZBIT))	/* check ctl z done */
		return (CTLZRET);

/*;	    if (c >= F1_KEY && c <= F10_KEY		/* check F1 - F10  */
/*;		    && ((1 << (c - F1_KEY)) & p->fkeys)) */
/*;		return(c - F1_KEY + 1);			 */

	if (c == F1_KEY && (F1BIT & p->fkeys)) return (1);
	if (c == F2_KEY && (F2BIT & p->fkeys)) return (2);
	if (c == F3_KEY && (F3BIT & p->fkeys)) return (3);
	if (c == F4_KEY && (F4BIT & p->fkeys)) return (4);
	if (c == F5_KEY && (F5BIT & p->fkeys)) return (5);
	if (c == F6_KEY && (F6BIT & p->fkeys)) return (6);
	if (c == F7_KEY && (F7BIT & p->fkeys)) return (7);
	if (c == F8_KEY && (F8BIT & p->fkeys)) return (8);
	if (c == F9_KEY && (F9BIT & p->fkeys)) return (9);
	if (c == F10_KEY && (F10BIT & p->fkeys)) return (10);

	if ((c==squid_keys.home_k) || (c==ser_keys.home_k))	/*;*/
		c = 0xC7;
	if ((c==squid_keys.end_k) || (c==ser_keys.end_k))	/*;*/
		c = 0xCF;
/* Process cursor positioning; delete char; insert char     */

	    switch(c)
	    {
		case RETCHAR:			    /* return */
		    p->col = p->left;		    /* move to left */
		    if (p->line < p->lower)	    /* & down 1 if allowed */
			p->line += 1;
		    pcursor(p->col,p->line);
		    break;

		case TAB:			    /* tab  */
		    i = ((p->col - p->left + 1) / 8 + 1 ) * 8 + p->left - 1;
		    if (i <= p->right)
		    {
			p->col = i;
			pcursor(p->col,p->line);
		    }
		    else
			putchar(BELL);
		    break;

		case BTAB:			    /* back tab  */
		    i = ((p->col - p->left + 1) / 8 - 1 ) * 8 + p->left - 1;
		    if (i >= p->left)
		    {
			p->col = i;
			pcursor(p->col,p->line);
		    }
		    else
			putchar(BELL);
		    break;

		case CTL_H:
		case CTL_D:			/*;*/
		case LEFTARROW_KEY:
		case BACKSPACE:
		    if (p->col == p->left)
		    {
			if (p->line == p->upper)
			{
			    putchar(BELL);
			    break;
			}
			else
			{
			    p->line -= 1;
			    p->col = p->right;
			}
		    }
		    else
			p->col -= 1;
		    pcursor(p->col,p->line);
		    if (c == BACKSPACE)
		    {
			putchar_1(' ');
			pcursor(p->col,p->line);
			string[strind(p)] = ' ';    /* store blank  */
		    }
		    break;

		case CTL_E:
		case UPARROW_KEY:
		    if (p->line == p->upper)
		    {
			putchar(BELL);
			break;
		    }
		    else
			p->line -= 1;
		    pcursor(p->col,p->line);
		    break;

		case CTL_X:
		case DOWNARROW_KEY:
		    if (p->line == p->lower)
		    {
			putchar(BELL);
			break;
		    }
		    p->line += 1;
		    pcursor(p->col,p->line);
		    break;

		case CTL_F:
		case RIGHTARROW_KEY:
		    if (p->col == ((p->type == 2) ? maxdcol : p->right))
		    {
			if (p->line == p->lower)
			{
			    putchar(BELL);
			    break;
			}
			else
			{
			    p->line += 1;
			    p->col = p->left;
			}
		    }
		    else
		    {
			if ((p->type == 2)	     /* can't move over space */
				      && string[strind(p)] == ' ')
			{
			    putchar(BELL);
			    break;
			}
			p->col += 1;
		    }
		    pcursor(p->col,p->line);
		    break;

		case 0xC7:				/*;*/
		    p->col = p->left;
		    p->line = p->upper;
		    pcursor(p->col,p->line);
		    break;

		case 0xCF:				/*;*/
		    if (p->type == 2)	    /* if numeric field */
		    {
			putchar(BELL);	    /* don't allow	 */
			break;
		    }
		    p->col = p->right;	       /* otherwise, it's lower right */
		    p->line = p->lower;
		    pcursor(p->col,p->line);	  /* and set cursor */
		    break;

		case CTL_O:
		case INSERT_KEY:
		    i = strind(p);		     /* index of cursor posn */
		    if (p->type == 3)		/* toss at end of line */
		    {
			for (j = (p->line - p->upper + 1) * line_len - 2;
			     j >= i; j--)
			    string[j + 1] = string[j];
		    }
		    else
			for (j = lastchar - 1; j >= i; j--)
			    string[j + 1] = string[j];
		    string[i] = ' ';
		    if (p->type == 2)
		    {					   /* numeric field */
			string[i] = '0';	    /* inserted numbers are 0 */
			if (i <= dotpos)	    /* adjust posn of dot     */
			    dotpos += 1;	    /* it moves */
			else
			    if ((i = maxdind + 1) <= lastchar)
				string[i] = ' ';
			maxdind = MIN(lastchar,dotpos + p->dplace);
			maxdcol = MIN(p->right,maxdind + p->left);
		    }				       /* end numeric field */
		    clrbuf();			    /* zap type ahead */
		    scr_beg();
		    putcbox(p,string);
		    scr_end();
		    pcursor(p->col,p->line);
		    break;

		case CTL_G:
		case DELETE_KEY:
		    i = strind(p);		    /* index of cursor pos */
		    ps = &string[i];		    /* first char to cover    */
		    ps1 = ps + 1;		    /* first char to move     */
		    if (p->type == 3)		    /* toss at end of line */
			ps2 = &string[(p->line - p->upper + 1) * line_len - 1];
		    else
			ps2 = &string[lastchar];    /* last char to move */
		    while (ps1 <= ps2)
			*ps++ = *(ps1++);	    /* move the characters */
		    if (p->type == 2)
		    {				    /* numeric field */
			if (i == dotpos)
			{
			    dotpos = lastchar + 1;
			    maxdcol = p->right;
			    maxdind = lastchar;
			}
			if (i < dotpos && dotpos <= lastchar)
			    dotpos -= 1;
			maxdind = MIN(lastchar, dotpos + p->dplace);
			maxdcol = MIN(p->right, maxdind + p->left);
		    }				/* end numeric field */
		    *ps2 = ' '; 		/* insert blank at end */
		    clrbuf();
		    scr_beg();
		    putcbox(p,string);
		    scr_end();
		    pcursor(p->col,p->line);
		    break;

		default:
		    putchar(BELL);
		    break;

	    } /* switch  */
	} /* while */
} /* end getcbox() function */

/*	Function to calculate the position in string from col,line  */

WORD	strind(p)
struct	field	*p;
{
    return(p->col - p->left + (p->line - p->upper) * line_len);
}   /* end function strind() */

/*	Function to delete the current decimal point from string  */

deldot(p,string)

struct	field	*p;
	UBYTE	string[];
{
	WORD	i,j;

	for (j = dotpos + 1; j <= lastchar; j++)
	    string[j - 1] = string[j];
	string[lastchar] = ' ';

}   /* end function deldot */
/*** SET current ATTRibute ***************************************************/

/*	Set character attributes:
		background color
		foreground color
		blink
		bright
		reverse video
		(underline = blue background on monochrome monitor)
					*/
set_attr(attr_ind)
	WORD	attr_ind;		/* index into attribute table */
{
	WORD	attr;			/* the actual attribute */
EXTERN	BYTE	*cur_attr;		/* the attribute table */

    attr = cur_attr[attr_ind];		/* set the actual attribute */
    set_attrp(attr);			/* and tell cmp about it */
    return;
}

/*** Set Physical Attribute *********************************************/

/*;set_attrp(attr)	new name */
sq_set_attrp(attr)
	WORD	attr;
{
    putchar(ESCAPE);			/* set foreground color esc seq */
    putchar('b');
    putchar((BYTE) attr & 0x000f );
    putchar(ESCAPE);			/* set background color esc seq */
    putchar('c');
    putchar((BYTE) ( attr & 0x00f0) >> 4 );
}

sq_wrap_on()
{
	putchar(ESCAPE);
	putchar('v');
}

sq_wrap_off()
{
	putchar(ESCAPE);
	putchar('w');
}

sq_expand_on()
{
	putchar(ESCAPE);
	putchar('7');
}

sq_expand_off()
{
	putchar(ESCAPE);
	putchar('6');
}

sq_cursor_on()
{
	putchar(ESCAPE);
	putchar('e');
}

sq_cursor_off()
{
	putchar(ESCAPE);
	putchar('f');
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      