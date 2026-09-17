/***

     This file contains all the necessary routines to perform the
functioniality of CHSET.  The two entry points are the functions
get_settings and set_fields.  Both functions are called with a
byte pointer that points to a record.  Both functions return an
error code from BDOS if you have set f_errmode (O.S. call 0x2d)
to 0xfe or 0xff.

Format of record:

	---------------------------------------------------
	|  d  |  filename  |  typ  |  password  |  fields |
        ---------------------------------------------------
byte       0     1      8     9 11    12     19    20   22

		d	drive number
	 filename	filename  (can have attribute bits on)
	      typ	type      (can have attribute bits on)
	 password	password
	   fields	offset		name
			  20		8087
			  21		SHARED
			  22		SUSPEND

GET_SETTINGS:

    On exit, without an error, all fields will have a byte number in
them corresponding to the bit pattern that was match in com_tab.  For
example the field SUSPEND has to possible settings ON and OFF, since 
ON is before OFF in com_tab a zero will represent it while a one will
represent the setting of OFF.

SET_FIELDS:

    On exit, without an error, all fields that where not EMPTY(i.e. -1)
will have caused the corresponding bit(s) in the given filespec to
be set or reset.


 NOTES:

	All offsets and counts are from zero unless specified otherwise.

	BDOS opens write password protected files in R/O mode when the
      wrong password is given; and then wrongly reports an R/O file
      error when we try to write to the file.  Therefore set_fields
      prints out a proper error msg.  If you set f_errmode to 0xff 
      you will need to compile with the -dno_errors option so this
      special error message will not be printed.


***/
	
#include	<portab.h>
#include	"cpmfunc.h"

EXTERN	WORD	_EXTERR;	/* holds AX after O.S. call */

#define		NOT_FOUND	-1
#define		EXIT_PROGRAM	-1
#define		CONTINUE	1

#define		LOW_TO_UP	('a' - 'A')

#define		T1		9		/* T1 attribute		*/
#define		F7		7		/* F7 attribute		*/
#define		F6		6		/* F6 attribute		*/
#define		ON		0x80	/* mask for attribute bit	*/

#define		PASSWORD	0x07ff	/* BDOS error for wrong password */
#define		RO_ERROR	0x03ff	/* BDOS error for R/O file	*/

#define		DMA_LEN		128
#define		FCB_LEN		36

/*** Record Constants ***/

	/* offsets */

#define		REC_PASSWORD	12
#define		REC_FIELDS	20

	/* values */

#define		EMPTY		-1	/* Can not be same as option number */


/***

ADDING NEW FIELDS:

	Add the correct data to com_tab[], add one to MAX_FIELD for
each new field and modify first_opt[] to reflect where the new options
are in the com_tab[].

***/

	struct		comm_str
	{
		LONG	rec_num;	/* Record  0,1,....big	*/
		WORD	byte_num;	/* Byte    0,1,...,127	*/
		BYTE	bit_num;	/* Bit     7,6,...,1,0	*/
		BYTE	*pattern;	/* Pattern 0 or 1 or X	*/
	};

	struct		comm_str	com_tab[] =
	{
		0L,	127,	6,	"X1",	/* 80 = on */
		0L,	127,	6,	"00",	/* 80 = of */
		0L,	127,	6,	"10",	/* 80 = op */

		0L,	0,	3,	"1",	/* sh = on */
		0L,	0,	3,	"0",	/* sh = of */

		0L,	127,	3,	"1",	/* su = on */
		0L,	127,	3,	"0"	/* su = of */
	};


#define	MAX_FIELD	3


	BYTE		first_opt[] =
	{
		0,		/* 8087 */
		3,		/* shared */
		5,		/* suspend */
		7		/* tells routines that suspend is 5,6 only */
	};

	BYTE	*bdos_01_msg = "\n\rFile password protected in Write mode\n\r";

	BYTE	buffer[DMA_LEN];

	BYTE	fcb_01[FCB_LEN];


WORD	get_settings(record)


	BYTE				*record;

	{

	 BYTE		*fcb;		/* FCB pointer	*/
	 BYTE		*dma;		/* DMA pointer	*/
	 BYTE		*field;		/* pts to field's options	*/
	 LONG		cur_rec;	/* current record in DMA	*/
	 WORD		error;
	 WORD		i;
	 BYTE		start,end;

	    fcb = &fcb_01[0];		/* set up FCB	*/
	    field = record + REC_FIELDS;	/* pts to the first field's option */
	    copy(record,fcb,12);	/* put dfilenametyp in FCB	*/
	    *(fcb+12) = 0;		/* extent number	*/
	    f_dmaset(record+REC_PASSWORD);	/* DMA points to the password	*/
	    *(fcb+F6) = *(fcb+F6) | ON;		/* Set to Read/Only	*/
	    f_open(fcb);
	    if ( (error = _EXTERR) != 0)
	    {
		return(error);
	    }
	    dma = &buffer[0];
	    f_dmaset(dma);
	    cur_rec = -1;		/* i.e. no record in DMA	*/
	    for ( i=0; i < MAX_FIELD; i++)
	    {
		if ( cur_rec != com_tab[i].rec_num )
		{
		    if ( (error=get_rec(fcb,com_tab[i].rec_num,FALSE)) != 0)
		    {
			f_close(fcb);
			return(error);
		    }
		}
		start = first_opt[i];
		end = first_opt[i+1] - 1;
		end = cmp_patterns(start,end,dma);
		*field++ = end;
	    }
	    f_close(fcb);
	    if (_EXTERR !=0)
	    {
		return(_EXTERR);
	    }
	    return(0);
	}



VOID	set_ran_rec(fcb,rec_number)

	BYTE			*fcb;
	LONG			rec_number;

	{

	    *(fcb+33) = (0xffL & rec_number);
	    *(fcb+34) = ((0xff00L & rec_number) >> 8);
	    *(fcb+35) = ((0xff0000L & rec_number) >> 16);

	}

WORD	cmp_patterns(start,end,dma)

	BYTE			start;
	BYTE			end;
	BYTE			*dma;		/* ptr to DMA buffer */

	{
	 BYTE			index;
	 BYTE			*byte_ptr;

	    for (index=start; index<=end; index++)
	    {
		if ( ((bits_match(&com_tab[index],dma)) == TRUE) )
		{
		    return(index-start);
		}
	    }
	    return(-1);
	}


WORD	bits_match(com_rec,dma)

	struct	comm_str	*com_rec;
	BYTE			*dma;

	{

	 BYTE			*byte_ptr;	/* points to byte 	*/
	 BYTE			bit_number;	/* 8 0's and 1's	*/
	 BYTE			bit;		/* contains a 0 or 1	*/
	 BYTE			*bit_pattern;	/* pts to pattern	*/
	 BYTE			is_on;

	    byte_ptr = dma + com_rec->byte_num;
	    bit_pattern = com_rec->pattern;
	    bit_number = com_rec->bit_num;
	    while ( (*bit_pattern != NULL) )
	    {					/* still in pattern	*/
		is_on = ((*byte_ptr) >> (bit_number)) & 0x01;
	/** is_on is 1 if bit coresponding to pattern is on **/
		if ( ((is_on == 1) && (*bit_pattern == '0')) ||
		     ((is_on == 0) && (*bit_pattern == '1')) )
		{		/* not a match	*/
		    return(FALSE);
		}
		bit_pattern++;	/* pts to next char that represents a bit */
		bit_number--;	/* dec bit postion to check pattern against */
	    }
	    return(TRUE);
	}

VOID	copy(source,dest,cnt)

	BYTE			*source;
	BYTE			*dest;
	WORD			cnt;

	{

	 BYTE			i;

	    for (i=0; i<cnt; i++)
	    {
		*dest++ = *source++;
	    }
	}


WORD	get_rec(fcb,rec_num,write)

	BYTE			*fcb;
	LONG			rec_num;
	WORD			write;		/* flag to dump DMA or not */

	{

	    if ( write == TRUE )
	    {			/* Will write out DMA buffer */
		f_writerand(fcb);
		if (_EXTERR != 0)
		{
		    return(_EXTERR);
		}
	    }
	    set_ran_rec(fcb,rec_num);
	    f_readrand(fcb);
	    if (_EXTERR != 0)
	    {
		return(_EXTERR);
	    }
	    return(0);
	}

WORD	set_fields(record)


	BYTE			*record;

	{

	 BYTE		*fcb;		/* FCB pointer	*/
	 BYTE		*dma;		/* DMA pointer	*/
	 BYTE		*field;		/* pts to field's options	*/
	 LONG		cur_rec;	/* current record in DMA	*/
	 WORD		error;
	 WORD		i;
	 WORD		ro_wr_pass = FALSE;	/* RO because of wrong pass */
	 BYTE		start;
	 WORD		end;
	 WORD		write;		/* Flag to write DMA or not	*/

	    fcb = &fcb_01[0];		/* set up FCB	*/
	    field = record + REC_FIELDS;	/* pts to the first field's option */
	    copy(record,fcb,12);	/* put dfilenametyp in FCB	*/
	    *(fcb+12) = 0;		/* extent number	*/
	    f_dmaset(record+REC_PASSWORD);	/* DMA points to the password	*/
	    f_open(fcb);
	    if (_EXTERR != 0)
	    {
		return(_EXTERR);
	    }
	    if ( (((*(fcb+T1)) & 0x80) != ON) && (((*(fcb+F7)) & 0x80) == ON) )
	    {	/* File is RW and write protected but wrong password	*/
		ro_wr_pass = TRUE;
	    }
	    dma = &buffer[0];
	    f_dmaset(dma);
	    cur_rec = -1;		/* i.e. no record in DMA	*/
	    write = FALSE;
	    for ( i=0; i < MAX_FIELD; i++)
	    {
		if ( (*(field+i) != ((BYTE)EMPTY)) )
		{		/* given field needs to be set		*/
		    if ( (cur_rec != com_tab[i].rec_num) )
		    {		/* In different record so fetch it	*/
			if ((error=get_rec(fcb,com_tab[i].rec_num,write)) != 0)
			{
			    f_close(fcb);
			    return(error);
			}
		      else
			{
			    cur_rec = com_tab[i].rec_num;
			    write = FALSE;	/* default after a read	*/
			}
		    }
		    start = first_opt[i] + *(field+i);	/* opt loc in table */
		    end =  cmp_patterns(start,start,dma);
		    if ( (end == -1) )
		    {		/* must set field			*/
			if ( ro_wr_pass == TRUE )
			{	/* Need password so bug user for it	*/
/***
	Next line needed as BDOS will not print a error msg.
		if you compile with -dno_errors then you will
		not get this error msg.
***/
#ifndef no_errors
			    c_writestr(bdos_01_msg);
#endif
			    return(PASSWORD);
			}
			write = TRUE;	/* will need to write DMA	*/
			set_pattern(&com_tab[start],dma);
		    }
		}
	    }
	    if ( write == TRUE)
	    {
		f_writerand(fcb);
		error = _EXTERR;
		if ( (error != 0) )
		{
		    f_close(fcb);	/* will try to close		*/
		    return(error);	/* returning write error	*/
		}
	    }
	    f_close(fcb);
	    if (_EXTERR !=0)
	    {
		return(_EXTERR);
	    }
	    return(0);
	}


VOID	set_pattern(com_rec,dma)

	struct	comm_str	*com_rec;
	BYTE			*dma;		/* pts to buffer	*/

	{

	 BYTE			*byte_ptr;	/* points to byte 	*/
	 BYTE			bit_number;	/* the bit number	*/
	 BYTE			*bit_pattern;	/* pts to pattern	*/
	 BYTE			mask;

	    byte_ptr = dma + com_rec->byte_num;
	    bit_pattern = com_rec->pattern;
	    bit_number = com_rec->bit_num;
	    while ( (*bit_pattern != NULL) )
	    {				/* still have bits to do	*/
		if ( (*bit_pattern != 'X') )
		{			/* either a '1' or '0' so set	*/
		    mask = (0x01 << (bit_number));
		    if (*bit_pattern == '0')
		    {				/* AND bit to zero	*/
			*byte_ptr = (*byte_ptr & ~(mask));
		    }
		  else
		    {				/* OR bit to a one	*/
			*byte_ptr = (*byte_ptr | mask);
		    }
		}
		bit_pattern++;		/* point to next bit in pattern	*/
		bit_number--;		/* point to next bit in DMA	*/
	    }
	}

