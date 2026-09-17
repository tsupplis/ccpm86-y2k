/*******************************************************************/
/*******************************************************************/
/* genc5.c: 							   */
/* some miscellaneous functions which belong in the system library */
/*******************************************************************/
/*******************************************************************/
#ifndef MAINMODULE
#include <genccpm.h>
#endif

#define FCB struct fcbstruct
FCB {
	BYTE	fcbdr;			/* drive */
	BYTE	fcbname[8];		/* name */
	BYTE	fcbext[3];		/* extension */
	BYTE	fcbrest[24];		/* all the rest */
};


#define FSEARCH(faddr) (__OSIF(17,faddr)&0xFF)
#define NSEARCH(faddr) (__OSIF(18)&0xFF)	/* doesn't use FCB */
#define SETDMA(dma) __OSIF(26,dma)	/* where to put dir info */
#define SECSIZE 128
#define AMASK 0x7F

/* dirsearch: returns a DLIST of names matching 'fs' */
    DLIST *				/* returns list of names found in */
dirsearch(fs)				/* directory search for */
    BYTE *fs;				/* file spec */
{
	FCB f, *fp;			/* use this FCB */
	REG WORD ret;			/* ptr */
	DLIST *_newdli(), *ls;		/* list */
	BYTE dmabuf[SECSIZE];		/* place for directory info */

	fp = &f;			/* everything else uses address */
	parsefn(fs,fp);			/* fill fcb with filespec */
	SETDMA(dmabuf);			/* tell em where to put it */
	if( (ret=FSEARCH(fp)) > 3 )	/* look for first occurrence */
	    return NULLPTR;		/* no luck */
	ls=_newdli(dmabuf+(ret<<5),NULLPTR);	/* start off a list */
	while( (ret=NSEARCH(fp)) <= 3 ) { /* do all the search_nexts */
	    ls = _newdli(dmabuf+(ret<<5),ls); /* link them together */
	}
	return ls;
}


    DLIST *
_newdli(f,lnk)
    FCB *f;
    DLIST *lnk;
{
	REG WORD i;
	BYTE *np, nb[15];		/* name buffer */
	DLIST *newl;			/* new list item */

					/* make a pretty filename */
	np = nb;
	for( i=0; i<11; ++i )		/* zap out any high bits in filename */
	    f->fcbname[i] &= AMASK;	/* mask off attribute bits */
	for( i=0; i<8; ++i ) {		/* handle name spec */
	    if( f->fcbname[i] == ' ' )	/* end of name? */
		break;			/* okay */
	    *np++ = f->fcbname[i];	/* o.w. stuff into name buffer */
	}
	*np++ = '.';			/* tack this on */
	for( i=0; i<3; ++i ){		/* handle ext spec */
	    if( f->fcbext[i] == ' ' )	/* end of ext? */
		break;
	    *np++ = f->fcbext[i];
	}
	*np++ = NULL;			/* null terminate string */
	/*************************************************************/

	np = malloc(np-nb);		/* save it away */
	strcpy(np,nb);			/* do the copy */
	newl = (DLIST *)malloc(sizeof(*newl));	/* get ptr area set up */
	newl->dlname = np;
	newl->dlnext = 	lnk;
	return newl;
}



parsefn(fs,f)
    BYTE *fs;				/* file spec */
    FCB *f;				/* fcb */
{
	REG WORD i;
	REG BYTE *cp;

	for( cp=fs; *cp; cp++ )		/* make everything upper case */
	    if( islower(*cp) )
		*cp = toupper(*cp);
	for( i=0; i<24; ++i )		/* init fcb */
	    f->fcbrest[i] = 0;
	if( *(fs+1) == ':' ) {		/* drive spec? */
	    f->fcbdr = 1 + *fs - 'A';	/* 0=default, 1 = A:,... */
	    fs += 2;			/* skip dr spec */
	} else f->fcbdr = 0;		/* default drive */
	for( i=0; *fs && *fs != '.' && i<8; fs++, ++i ) {	/* NAME. */
	    if( *fs == '*' ) {		/* wildcard? */
		for( ; i<8; ++i )	/* fill up rest of name */
		    f->fcbname[i]='?';	/* with search spec */
		for( ; *fs && *fs!='.'; fs++ )	/* ignore to end or '.' */
		    ;
		break;			/* terminate loop */
	    }
	    f->fcbname[i] = *fs;
	}
	for( ; i<8; ++i )		/* fill up rest of name */
	    f->fcbname[i] = ' ';	/* with blanks */
	for( ; *fs; fs++ )		/* skip rest of name */
	    if( *fs == '.' ) {		/* end of name? */
	        fs++; break;		/* yes, so exit this loop */
	    }
  	for( i=0; *fs && i<3; fs++,++i ) {			/* .EXT */
	    if( *fs == '*' ){		/* wildcard */
		for( ; i<3; ++i )
		    f->fcbext[i] = '?';
		break;
	    }
	    f->fcbext[i] = *fs;
	}
	for( ; i<3; ++i )		/* fill up rest of ext */
	    f->fcbext[i] = ' ';		/* with blanks */
}



/* access: tests for existence: return NULL if exist, -1 o.w. */
    WORD
access(fn)
    BYTE *fn;
{
	FCB f;				/* use this FCB */
	BYTE dmabuf[SECSIZE];		/* place for directory info */

	parsefn(fn,&f);			/* fill fcb with filespec */
	SETDMA(dmabuf);			/* tell em where to put it */
	if( FSEARCH(&f) > 3 )		/* look for first occurrence */
	    return -1;			/* no luck */
	return 0;
}

    BYTE *
gets(s)
    BYTE *s;
{
	BYTE *sav;

	if( fgets(s,32767,stdin) == NULLPTR )
		return NULLPTR;
	if( !isatty(0) )		/* reading from CON:? */
		fputs(s,stdout);	/* echo if not */
	for( sav=s; *s && *s != '\n'; ++s )
		;
	*s = '\0';
	return sav;
}

/************************************************************************/
    BYTE
getdrive()
{
	return __OSIF(25,0);
}


/************************************************************************/

/* press_return: let user look at screen */
press_return()
{
	BYTE bitbucket[40];		/* leave room */

	if( !isatty(0) ) {		/* stdin from CON:? */
	    printf("Error in command file: terminating program\n");
	    exit(1);
	}
	printf("\07Press RETURN to continue ");
	if( gets(bitbucket)!=bitbucket )
		exit(1);		/* eof, quit now */
}

/* patch: provides a patch area */
patch()					/* should never get called */
{
	int i;
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	i=i+5; i=i+5; i=i+5; i=i+5; i=i+5; 
	return;
}


/*******************************************************************/
/*******************************************************************/
/*			T H E   E N D
/*******************************************************************/
/*******************************************************************/
