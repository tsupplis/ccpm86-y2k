/****************************************************************************/
/*									    */
/*				   C P M . H				    */
/*				   ---------				    */
/*	Copyright 1982 by Digital Research Inc. All rights reserved.	    */
/*									    */
/*	Edits:								    */
/*	6-Oct-83 whf	Redefine reserved area in fcb for parsefn() return  */
/*	25-June-83 sw	Add user number to file "fd" structure.		    */
/*									    */
/*	This file contains CP/M specific definitions for the CP/M 	    */
/*	C Run Time Library.						    */
/*	This file is intended only for inclusion with those functions	    */
/*	dealing directly with the BDOS, as well as any function which	    */
/*	has hardware dependent code (byte storage order, for instance).	    */
/*									    */
/*	<portab.h> must be included BEFORE this file.			    */
/*									    */
/****************************************************************************/

/*
 *	CP/M FCB definition
 */

struct	fcbtab					/****************************/
{						/*			    */
	BYTE	drive;				/* Disk drive field	    */
	BYTE	fname[8];			/* File name		    */
	BYTE	ftype[3];			/* File type		    */
	BYTE	extent;				/* Current extent number    */
	BYTE	s1,s2;				/* "system reserved"	    */
	BYTE	reccnt;				/* Record counter	    */
	BYTE	fpasswd[8];			/* Parsefn passwd area	    */
	BYTE	fuser;				/* Parsefn user# area	    */
	BYTE	resvd[7];			/* More "system reserved"   */
	LONG	record;				/* Note -- we overlap the   */
						/* current record field to  */
						/* make this useful.	    */
};						/****************************/
#define SECSIZ		128			/* size of CP/M sector	    */
#define _MAXSXFR	1			/* max # sectors xferrable  */
#define _MAXSHFT	12			/* shift right BDOS rtn val */
						/*   to obtain nsecs on err */
						/****************************/

/****************************************************************************/
/*									    */
/*	Channel Control Block (CCB)					    */
/*									    */
/*	One CCB is allocated (statically) for each of the 16 possible open  */
/*	files under C (including STDIN, STDOUT, STDERR).  Permanent data    */
/*	regarding the channel is kept here.				    */
/*									    */
/*									    */
/****************************************************************************/

struct	ccb				/************************************/
{					/*				    */
	WORD	flags;			/*sw	Flags byte		    */
	BYTE	user;			/*sw	User #			    */
	BYTE	chan;			/*	Channel number being used   */
	LONG	offset;			/*	File offset word (bytes)    */
	LONG	sector;			/* 	Sector currently in buffer  */
	LONG	hiwater;		/*	High water mark		    */
	struct fcbtab fcb;		/*	File FCB (may have TTY info)*/
	BYTE	buffer[SECSIZ];		/*	Read/write buffer	    */
};					/************************************/

#define	MAXCCBS	16			/*	Maximum # CC Blocks 	    */
extern	struct	ccb	_fds[]; /*  */	/*	Declare storage		    */
#define FD struct ccb			/*	FD Type definition	    */
					/************************************/
/*	Flags word bit definitions					    */
					/************************************/
#define	OPENED	0x01			/*	Channel is OPEN		    */
#define	ISTTY	0x02			/*	Channel open to TTT	    */
#define	ISLPT	0x04			/*	Channel open to LPT	    */
#define	ISREAD	0x08			/*	Channel open readonly	    */
#define	ISASCII	0x10			/*	ASCII file attached	    */
#define	ATEOF	0x20			/*	End of file encountered	    */
#define DIRTY	0x40			/*	Buffer needs writing	    */
#define ISSPTTY	0x80			/*	Special tty info	    */
#define	ISAUX	0x100			/*sw	Auxiliary device	    */
					/************************************/
#define READ	0			/* Read mode parameter for open	    */
#define WRITE	1			/* Write mode			    */

/*	CCB manipulation macros		*************************************/
#define _getccb(i) (&_fds[i])		/* 	Get CCB addr		    */

/*	Error handling 			*************************************/
EXTERN WORD errno;			/* error place for assigning	    */
EXTERN WORD __cpmrv;			/* the last BDOS return value	    */
EXTERN WORD _errcpm;			/* place to save __cpmrv	    */
#define RETERR(val,err) {errno=(err);_errcpm=__cpmrv;return(val);}
					/************************************/


/****************************************************************************/
/*									    */
/*		B D O S   F u n c t i o n   D e f i n i t i o n s	    */
/*		-------------------------------------------------	    */
/*									    */
/*	Following are BDOS function definitions used by the C runtime 	    */
/*	library.							    */
/*									    */
/****************************************************************************/

						/****************************/
#if CPM68K					/*			    */
#define __OSIF(fn,arg) __BDOS((fn),(LONG)(arg))	/* CPM68K does it this way  */
#else						/*			    */
#define __OSIF(fn,arg) __BDOS((fn),(arg))	/* DRC does it this way	    */
#endif						/*			    */
						/****************************/
#define	EXIT	 0				/* Exit to BDOS		    */
#define CONIN	 1				/* direct echoing con input */
#define	CONOUT	 2				/* Direct console output    */
#define	LSTOUT	 5				/* Direct list device output*/
#define	CONIO	 6				/* Direct console I/O	    */
#define C_WRITESTR	9			/* Console string output    */
#define	CONBUF	10				/* Read console buffer	    */
#define	OPEN	15				/* OPEN a disk file	    */
#define	CLOSE	16				/* Close a disk file	    */
#define SEARCHF	17				/* Search for first	    */
#define SEARCHN	18				/* Search for next	    */
#define	DELETE	19				/* Delete a disk file	    */
#define	CREATE	22				/* Create a disk file	    */
#define F_RENAME	23			/* Rename a disk file	    */
#define	SETDMA	26				/* Set DMA address	    */
#define	USER	32				/*sw Get / set user number  */
#define	B_READ	33				/* Read Random record	    */
#define	B_WRITE	34				/* Write Random record	    */
#define FILSIZ	35				/* Compute File Size	    */
#define SETMSC	44				/* Set Multi-Sector Count   */
#define P_CHAIN		47			/* Program Chain	    */
#define	SETVEC	61				/* Set exception vector	    */
#define C_DELIMIT	110			/* Get/Set Output Delimiter */
						/****************************/

/****************************************************************************/
/* Other CP/M definitions						    */
/****************************************************************************/
#define	TERM	"CON:"				/* Console file name	    */
#define	LIST	"LST:"				/* List device file name    */
#define	EOFCHAR	0x1a				/* End of file character-^Z */
						/****************************/

/****************************************************************************/
/*	Hardware dependencies						    */
/****************************************************************************/
#include "machine.h"				/* For which machine?	    */
						/****************************/
#ifdef MC68000 || Z8000				/* 68K or Z8000		    */
#define HILO 1					/* used when bytes stored   */
#else						/*			    */
#define HILO 0					/*			    */
#endif						/*			    */
						/*			    */
#if HILO					/* Hi/Lo storage used in    */
struct {					/*   68K		    */
	BYTE lbhihi;				/* Use this for accessing   */
	BYTE lbhilo;				/*   ordered bytes in 32 bit*/
	BYTE lblohi;				/*   LONG qtys.		    */
	BYTE lblolo;				/*			    */
};						/*			    */
struct {					/* Use this for accessing   */
	WORD lwhi;				/*   ordered words in 32 bit*/
	WORD lwlo;				/*   LONG qtys.		    */
};						/*			    */
#else						/****************************/
struct {					/* Lo/Hi storage use on     */
	BYTE lblolo;				/*   PDP-11, VAX, 8086,...  */
	BYTE lblohi;				/*			    */
	BYTE lbhilo;				/*			    */
	BYTE lbhihi;				/*			    */
};						/*			    */
struct {					/*			    */
	WORD lwlo;				/*			    */
	WORD lwhi;				/*			    */
};						/*			    */
#endif						/****************************/
/*************************** end of cpm.h ***********************************/
