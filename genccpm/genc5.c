/**************************************************************************/
/**************************************************************************/
/*	genc5.c: M E N U  handling routines
/*
/*	Written by Bill Fitler
/**************************************************************************/
/**************************************************************************/
#ifndef MAINMODULE
#include <genccpm.h>
EXTERN BOOLEAN verbose;			/* flag: be wordy */
#endif

MLOCAL BOOLEAN eflag = FALSE;		/* user error */

/**************************************************************************/
/* bldmenu: builds a menu list from successive calls
/**************************************************************************/
    MENU *
bldmenu(muroot,typ,ptr,name,description)
    MENU *muroot;			/* root of menu list (or NULLPTR) */
    WORD typ;				/* type of menu item */
    BYTE *ptr;				/* pointer to something... */
    BYTE *name;				/* command name for this menu item */
    BYTE *description;			/* long label for this item */
{
	REG MENU *mu;			/* ptr to created item */
	REG MENU *tmu;			/* temp ptr for linked list scan */

	mu = (MENU *)malloc(sizeof(*mu));	/* allocate memory for item */
	mu->mutype = typ;
	mu->muiptr = ptr;
	mu->muname = name;
	mu->mudesc = description;
	mu->munext = NULLPTR;
	if( muroot == NULLPTR )		/* if NULL list */
		return mu;		/* return something to start with */
	for( tmu=muroot; tmu->munext != NULLPTR; tmu = tmu->munext )
		;			/* append to end of list */
	tmu->munext = mu;	
	return muroot;			/* keep list in order */
}

/**************************************************************************/
/* domenu: executes the specified commands (in cbuf) from the menu (mnu)
/**************************************************************************/
#define CMDLEN 80
#define USERR ++eflag;printf
#define ECHK(fn) _errchk(fn,cmd,cmdval)

    VOID
domenu(cbuf,mnu)
    BYTE *cbuf;				/* command buffer */
    MENU *mnu;				/* table of menu specs */
{
	REG MENU *mi;			/* ptr to menu item */
	LOCAL BYTE cmd[CMDLEN];		/* place for command & value */
	LOCAL BYTE *cmdval;		/* ptr within cmd */
	LOCAL BYTE *cp;			/* temp char ptr */
	BYTE *msetbool();		/* set a boolean value */
	BYTE *msetbyte();		/* set a numeric byte value */
	BYTE *msetword();		/* set an integer pointer */
	BYTE *msettxt();		/* set a text value */
	BYTE *(*proc)();		/* procedure pointer */
	BYTE *msetdrv();		/* set a drive value */
	BYTE *_nxtcmd();		/* command parser */
	MENU *_fndcmd();		/* command lookup */

	eflag = FALSE;			/* no errors yet */
	while((cbuf=_nxtcmd(cbuf,cmd))){ /* while there are commands in cbuf */
	    cmdval = NULLPTR;		/* init value ptr for command */
	    for( cp=cmd; *cp; cp++ ) {	/* scan command for assign op */
		if( isupper(*cp) )	/* convert all commands to LOWER CASE */
		    *cp = tolower(*cp);
		if( *cp == '=' ) {	/* found? */
		    *cp++ = NULL;	/* terminate command */
		    cmdval = cp;	/* point value to after assign op */
		    break;		/* and terminate loop */
		}
	    }
	    if( (mi=_fndcmd(cmd,mnu)) == NULLPTR ){ /* scan menu commands */
		USERR("'%s' is not a command for this menu\n",cmd); 
	    } else {			/******** cmd loop ********/
		switch(mi->mutype) {	/* handle cmd type */
		case MBOOL:		/** BOOLEAN command **/
		    ECHK(msetbool(mi->muiptr,cmdval));
		    break;
		case MBYTE:		/** BYTE command **/
		    ECHK(msetbyte(mi->muiptr,cmdval));
		    break;
		case MWORD:		/** INTEGER command **/
		    ECHK(msetword(mi->muiptr,cmdval));
		    break;
		case MTEXT:		/** TEXT command **/
		    ECHK(msettxt(mi->muiptr,cmdval));
		    break;
		case MPROC:		/** PROCEDURAL command **/
		    proc = mi->muiptr;	/* assign to function ptr */
		    ECHK(((*proc)(cmdval,mi->mudesc)));
		    break;
		case MDRIV:		/** DRIVE letter command **/
		    ECHK(msetdrv(mi->muiptr,cmdval));
		    break;
		}			/* end switch */
	    }				/********* end cmd loop *********/
	    if(eflag) {			/* user error? */
		press_return();
		*cbuf = NULL;
	    } else if(verbose)		/* are we being wordy? */
		prtmival(mi);		/* let the user know what happened */
	}				/**** end while loop ****/
}



/* _nxtcmd: parses cin into cout */
    BYTE *
_nxtcmd(cin,cout)			/* returns ptr to after next cmd */
    BYTE *cin;				/* command in buf */
    BYTE *cout;				/* command out buf */
{
	/***** handle quoted strings??? *****/
	while( isspace(*cin) )		/* skip leading spaces */
		++cin;
	if( *cin == NULL )		/* check for EOS */
		return NULLPTR;
	while( *cin && !isspace(*cin) )	/* scan to eos or whitespace */
		*cout++ = *cin++;	/* xfer to command out buf */
	*cout = NULL;			/* null terminate command out buf */
	return cin;			/* return ptr to after command */
}



/* _fndcmd: scans thru menu structure 'mu' looking for command name 'cn' */
/*		returns ptr to menu item or NULLPTR */
    MENU *
_fndcmd(cn,mus)
    BYTE *cn;				/* command name */
    MENU *mus;				/* ptr to menu list */
{
	REG WORD cnl;			/* cmd name len */
	MENU *musave;			/* extra ptr to menu item */

	cnl = strlen(cn);
	for( ; ; mus = mus->munext) {	/* scan menu list */
	    if( mus==NULLPTR )		/* if we hit the end then fail */
		return NULLPTR;
	    if( strncmp(cn,mus->muname,cnl) == 0 )	/* cn prefix of name?*/
		break;			/* yes, we have a candidate */
	}
	musave = mus;
	for( mus=mus->munext; ; mus=mus->munext ) { /* finish scanning list */
	    if( mus==NULLPTR )		/* if we hit the end then succeed */
		return musave;		/* we reached end of list: found it! */
	    if( strncmp(cn,mus->muname,cnl) == 0 )
		return NULLPTR;		/* another match: ambiguous command */
	}
}



/* _errchk: checks for error returns on functions */
    VOID
_errchk(emsg,cm,cmv)
    BYTE *emsg;				/* error message to check */
    BYTE *cm;				/* command generating the error */
    BYTE *cmv;				/* value generating the error */
{
	if( emsg ) {			/* non NULL error message return? */
		USERR("Error on command '%s",cm);
		if(cmv)
			USERR("=%s",cmv);
		USERR("': %s\n",emsg);
	}
}



/* msetbool: sets the BOOLEAN pointed to by 'bp' according to value in 'cvp' */
    BYTE *				/* return NULLPTR, or ptr to err msg */
msetbool(bp,cvp)
    BYTE *bp;				/* pointer to boolean to set */
    BYTE *cvp;				/* pointer to value to set to */
{
	REG WORD bv;
	REG BYTE *tp;

	if( cvp==NULLPTR )		/* no val: toggle boolean */
		bv = (*bp ? 4 : 1);	/* toggle by setting bv index */
	else {
	    for( tp=cvp; *tp; ++tp ) 
		if( isupper(*tp) ) *tp=tolower(*tp);
	    if( (bv=whichof(cvp,",on,yes,true,off,no,false"))==0 ) 
		return "value must be 'yes' or 'no'";
	}
	if( bv < 4 )
		*bp = -1;		/* zap to 0xFFFF */
	else 	*bp = 0;		/* zap to zero */
	return NULLPTR;
}


    BYTE *
msetbyte(bp,cvp)			/* set numeric BYTE value */
    BYTE *bp;				/* ptr to BYTE to set */
    BYTE *cvp;				/* value to set BYTE to */
{
	REG WORD val;			/* temp holding place */

	val=0;
	if(cvp != NULLPTR) val=atoih(cvp);	/* assume numbers are in HEX!! */
	if( cvp==NULLPTR || (val==0  &&  !isdigit(*cvp)) )
	    return "value must be a number";
	if( val > 255 )
	    return "value must be less than FF hex (255 decimal)";
	*bp = val;
	return NULLPTR;
}


    BYTE *
msetword(ip,cvp)
    WORD *ip;				/* ptr to WORD to set */
    BYTE *cvp;				/* value to set WORD to */
{
	REG WORD val;			/* temp holding place */

	val=0;
	if(cvp != NULLPTR) val=atoih(cvp);	/* assume numbers are in HEX!! */
	if( cvp==NULLPTR || (val==0  &&  !isdigit(*cvp)) )
	    return "value must be an unsigned hex number between 0 and FFFFh";
	*ip = val;
	return NULLPTR;
}


    WORD
atoih(cp)				/* conver ascii hex to word */
    BYTE *cp;				/* buffer number */
{
	REG WORD v;			/* resulting value */
	REG BYTE cv;			/* character value */
	BYTE *savcp;			/* save orig ptr */

	v = 0; savcp = cp;		/* init value */
	for( cv = *cp; ; cv = *++cp ){	/* for each char in cp */
	    if( isdigit(cv) )
		v = (v<<4) + (cv-'0');	/* convert digit and add */
	    else {			/* maybe it's a letter */
		if( isupper(cv) )	/* upper case letter? */
		    cv = tolower(cv);	/* convert it */
		if( cv<'a' || cv>'f')	/* range check */
		    break;		/* terminate loop if failed */
		v = (v<<4)+(10+cv-'a');	/* convert letter and add */
	    }
	}
	if( cp-savcp <= 4 )
	     return v;			/* return the value */
	else { 
	     *savcp |= 0x80; 
	     return 0; 		/* turn on hi bit, make isdigit test fail */
	}
}

    BYTE *
msettxt(tp,cvp)
    BYTE **tp;				/* place to store ptr to value */
    BYTE *cvp;				/* value to set BYTEs to */
{
	REG BYTE *tmp;			/* temp ptr for allocated storage */

	if( cvp==NULLPTR )	
	    cvp="";			/* point to null string, instead */
	tmp = malloc(1+strlen(cvp));	/* get space for string */
	strcpy(tmp,cvp);		/* copy to allocated space */
	*tp = tmp;			/* assign ptr */
	return NULLPTR;
}

    BYTE *
msetdrv(dp,cvp)
    BYTE *dp;				/* place to store ptr to value */
    BYTE *cvp;				/* value to set DRIVE to */
{
	REG WORD v;			/* work place */

	if( cvp!=NULLPTR ) {		/* check for drive letter */
	    if( isupper(*cvp) )
		*cvp = tolower(*cvp);
	    v = *cvp - 'a';		/* convert drive spec to nibble */
	}
	if( cvp==NULLPTR  ||  v<0  ||  v>15  || cvp[1] != ':' )
	     return "you must specify a drive 'A:' thru 'P:'";
	*dp = v;			/* looks okay, set it */
	return NULLPTR;			/* no errors */
}


/**************************************************************************/
/* prtmenu: displays the menu labels, values and descriptors
/**************************************************************************/
    VOID 
prtmenu(mtitle,mnu)
    BYTE *mtitle;			/* title of menu */
    MENU *mnu;				/* table specifying menu */
{
	printf("\n\n%s\n",mtitle);	/* give the menu a title */
	for( ; mnu!=NULLPTR; mnu = mnu->munext ) /* travel down list */
	    prtmival(mnu);	
}


prtmival(mnu)				/* return ptr to value of item */
    MENU *mnu;				/* item to get value of */
{
	REG BYTE *ival;
	BYTE ivbuf[10];			/* local buf for display */
	BYTE *vbp;			/* value byte ptr */
	WORD *vwp;			/* value word ptr */
	WORD v;				/* place to put value */

	printf("%12.12s ",mnu->muname); /* first the name */
	ival = ivbuf; *ival = NULL;	/* init value */
	switch(mnu->mutype) {	/* now the value */
	    case MBOOL:
		if( *(mnu->muiptr) )
		     ival="[Y]";
		else ival="[N]";
		break;
	    case MBYTE:
		vbp = mnu->muiptr;
		v = (*vbp & 0xFF);
		sprintf(ivbuf,"[%2.2x]",v);	/* display in hex */
		break;
	    case MWORD:
		vwp = mnu->muiptr;
		v = *vwp;
		sprintf(ivbuf,"[%4.4x]",v);	/* display in hex */
		break;
	    case MTEXT:
		vbp = *((BYTE **)mnu->muiptr);	/* go get the char ptr */
		if( strlen(vbp) > 4 )
		     sprintf(ivbuf,"[%-4.4s>",vbp);
		else sprintf(ivbuf,"[%s]",vbp);
		break;
	    case MPROC:
		/* leave blank */
		break;
	    case MDRIV:
		sprintf(ivbuf,"[%c:]",'A'+(*mnu->muiptr));
		break;
	}
	printf("%-6s ",ival);
	printf("%s\n",mnu->mudesc);	/* finally the description */
}


/************************************************************************
/* whichof: scans the string 'set' (delimited by 1st char in 'set')
/*	for 1st occurrence of string 'sample'.
/*	Returns 0 if 'sample' not a prefix of any item in set or if
/*		'sample' is a prefix of more than one item in set;
/*		delimiter# o.w.
/* E.G. if set = ",on,yes,true,off,no,false"  then
/*	whichof("yes",set) == 2;
/*	whichof("y",set) == 2;
/*	whichof("o",set) == 0;
/*	whichof("x",set) == 0;
/* NOTE: results not guaranteed if item delimiter (*set) is in string 'sample',
/*	or if null item is in set (adjacent delimiters).
/*************************************************************************/

/* first, a subroutine: */
/* _wget: returns NULLPTR if sa not prefix of item in st; */
/*	  returns end of item (ptr in st) otherwise */
BYTE *_wget(sa,st,de)
BYTE *sa;
BYTE *st;
BYTE de;
{
	BYTE *s2;

	for( ;; ) {					/* do forever */
		for( s2=sa; *s2 && *s2 == *st; ){
			++s2; ++st;			/* try match */
		}
		if( *s2	== NULL ){	/* success! we've matched all of sa */
			while( *st && *st != de ) ++st;	/* adv to nxt delim */
			return( --st );			/* backup one char */
		}
		while( *st  &&  *st != de ) /* no match, look for next item */
			++st;				/* adv to nxt delim */
		if( *st == NULL )
			return( NULLPTR );		/* end of list */
		++st;
	}
}

WORD whichof(sample,set)
BYTE *sample;
BYTE *set;
{
	WORD sx;
	BYTE *sp, *np;

	if( (sp=_wget(sample,set+1,*set)) == NULLPTR )
		return 0;				/* not found */
	if( _wget(sample,sp+1,*set) != NULLPTR )
		return 0;				/* ambiguous */
	for( sx=0, np=set; np<sp; np++ )
		if( *np == *set ) sx++;		/* count delimiters */
	return sx;
}




