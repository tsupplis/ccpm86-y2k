/***************************************************************/
/***************************************************************/
/* genc2.c: more genccpm.cc, broken up for shorter compiles    */
/***************************************************************/
/***************************************************************/
#ifndef MAINMODULE
#include <genccpm.h>
#endif


/**************************************************************************/
/* init_sds: points the system data pointers to the right place
/**************************************************************************/
    VOID
init_sds(sysdat)
    BYTE *sysdat;			/* ptr to system data buffer */
{
	sd_supmod     = sysdat + 0x00;	/* (4) */
	sd_rtmmod     = sysdat + 0x08;	/* (4) */
	sd_memmod     = sysdat + 0x10;	/* (4) */
	sd_ciomod     = sysdat + 0x18;	/* (4) */
	sd_bdosmod    = sysdat + 0x20;	/* (4) */
	sd_xiosmod    = sysdat + 0x28;	/* (4) */
	sd_netmod     = sysdat + 0x30;	/* (4) */
	sd_mpmseg     = sysdat + 0x40;
	sd_rspseg     = sysdat + 0x42;
	sd_endseg     = sysdat + 0x44;
	sd_module_map = sysdat + 0x46;
	sd_ncns       = sysdat + 0x47;
	sd_nlst       = sysdat + 0x48; 
	sd_nccb       = sysdat + 0x49;
	sd_nflags     = sysdat + 0x4A;
	sd_srchdisk   = sysdat + 0x4B;
	sd_mmp        = sysdat + 0x4C;
	sd_nslaves    = sysdat + 0x4E;
	sd_dayfile    = sysdat + 0x4F;
	sd_tempdisk   = sysdat + 0x50;
	sd_tickspsec  = sysdat + 0x51;
	sd_lul        = sysdat + 0x52;
	sd_ccb        = sysdat + 0x54;
	sd_flags      = sysdat + 0x56;
	sd_mdul       = sysdat + 0x58;
	sd_nxmds      = sysdat + 0x58;
	sd_mfl        = sysdat + 0x5A;
	sd_nmparts    = sysdat + 0x5A;
	sd_pul        = sysdat + 0x5C;
	sd_nxpd       = sysdat + 0x5C;
	sd_qul        = sysdat + 0x5E;
	sd_nqds       = sysdat + 0x5D;        
	sd_qmau       = sysdat + 0x60;	/* (4) */
	sd_qmastart   = sysdat + 0x62;
	sd_qmalen     = sysdat + 0x64;
	sd_verptr     = sysdat + 0x78;
	sd_vn         = sysdat + 0x7A;
	sd_mpmvn      = sysdat + 0x7C;
	sd_ncondev    = sysdat + 0x83;
	sd_nlstdev    = sysdat + 0x84;
	sd_nciodev    = sysdat + 0x85;
	sd_lcb        = sysdat + 0x86;
	sd_popen_max  = sysdat + 0x8A;
	sd_plock_max  = sysdat + 0x8B;
	sd_cmode      = sysdat + 0x90;
        sd_xpcns      = sysdat + 0x9f;
}


/************************************************************************/
/* doxios: gets miscellaneous XIOS values from user */
/************************************************************************/
    BYTE *
doxios(bb,xmtitle)			/* submenu for XIOS value mods */
    BYTE *bb;				/* value passed in from menu */
    BYTE *xmtitle;
{
	BYTE cmds[CMDSLEN];		/* space for additional commands */
	EXTERN BYTE *clearit;

	if( bb != NULLPTR ) {		/* if any "command line" arguments */
	    domenu(bb);	                /* do what they've asked for */
	} else FOREVER {		/* work with user for a while */
		printf("%s",clearit);	/* clear screen */
		prtmenu(xmtitle);	/* display user options */
		printf(PROMPT);		/* prompt user */
		if( gets(cmds)!=cmds ||	/* on end of file */
		    *cmds==NULL )	/* or blank line of input */
		    break;		/* exit interaction with user */
		domenu(cmds);	        /* do what they've asked for */
	}				/*****************************/
	return NULLPTR;			/* standard return... */
}



/************************************************************************/
/* dosysp: gets miscellaneous system paramaters from user */
/************************************************************************/
    BYTE *
dosysp(bb,smtitle)			/* submenu for SYSP value mods */
    BYTE *bb;				/* value passed in from menu */
    BYTE *smtitle;
{
	BYTE cmds[CMDSLEN];		/* space for additional commands */
	EXTERN MENU *syspmenu;		/* list of menu items for sysp menu */
	EXTERN BYTE *clearit;

	if( bb != NULLPTR ) {		/* if any "command line" arguments */
	    domenu(bb,syspmenu);	/* do what they've asked for */
	} else FOREVER {		/* work with user for a while */
		printf("%s",clearit);	/* clear screen */
		prtmenu(smtitle,syspmenu);	/* display user options */
		printf(PROMPT);		/* prompt user */
		if( gets(cmds)!=cmds ||	/* on end of file */
		    *cmds==NULL )	/* or blank line of input */
		    break;		/* exit interaction with user */
		domenu(cmds,syspmenu);	/* do what they've asked for */
	}				/*****************************/
	return NULLPTR;			/* standard return... */
}



/*************************************************************************/
/* dorsp: handles rsp editting */
/*************************************************************************/
    BYTE *
dorsp(parm,xmtitle)			/* handle RSP list editting */
    BYTE *parm;				/* command value parameter */
    BYTE *xmtitle;
{
	BYTE cmds[CMDSLEN];		/* space for additional commands */
	EXTERN MENU *rspmenu;		/* list of menu items for rsp menu */
	EXTERN BYTE *clearit;

	if( parm != NULLPTR ) {		/* if any "command line" arguments */
	    domenu(parm,rspmenu);	/* do what they've asked for */
	} else FOREVER {		/* work with user for a while */
		printf("%s",clearit);	/* clear screen */
		rsp_display(rsps);	/* display what they have to work with */
		prtmenu(xmtitle,rspmenu);	/* display user options */
		printf(PROMPT);		/* prompt user */
		if( gets(cmds)!=cmds ||	/* on end of file */
		    *cmds==NULL )	/* or blank line of input */
		    break;		/* exit interaction with user */
		domenu(cmds,rspmenu);	/* do what they've asked for */
	}				/*****************************/
	return NULLPTR;			/* standard return... */
}

/*************************************************************************/
    BYTE *
rspinclude(rs)
    BYTE *rs;
{
	BYTE *bp, *cp, *ep;		/* ptrs to take apart command str */
	DLIST *dd;

	if( rs==NULLPTR || *rs==NULL ) 
	    return "You need to say 'include=xx.rsp,yy.rsp,zz.rsp'";
	for( ep=rs; *ep; ++ep ) ;	/* end of string pointer */
	for( bp=rs; bp<ep; bp=cp+1 ){	/* process each rsp spec */
	    for( cp=bp; *cp && *cp!=','; ++cp ) ;	/* find eos or comma */
	    if( *cp == ',' ) *cp = '\0'; /* null terminate the spec */
	    if( (dd=dirsearch(bp)) == NULLPTR )
		return "Can't find RSP to include";
	    rspjoin(dd);
	}
	return NULLPTR;
}

rspjoin(dd)
    DLIST *dd;
{
	DLIST *rs;

	if( rsps == NULLPTR )
	    rsps = dd;
	else {
	    for( rs=rsps; rs->dlnext != NULLPTR; rs=rs->dlnext ) ;
	    rs->dlnext = dd;
	}
}



/*************************************************************************/
    BYTE *
rspexclude(rs)
    BYTE *rs;
{
	BYTE *bp, *cp, *ep;
	BOOLEAN rspzap();

	if( rs==NULLPTR || *rs==NULL )
	    return "You need to say 'exclude=aa.rsp,bb.rsp,cc.rsp'";
	for( ep=rs; *ep; ++ep ) ;	/* find eos */
	for( bp=rs; bp<ep; bp=cp+1 ){	/* process each rsp spec */
	    for( cp=bp; *cp && *cp!=','; ++cp ) ;	/* find eos or comma */
	    if( *cp == ',' ) *cp = '\0'; /* null terminate the spec */
	    if( !rspzap(bp) )
		return "Can't find RSP to exclude";
	}
	return NULLPTR;
}

    BOOLEAN
rspzap(nm)
    BYTE *nm;
{
	DLIST **qs, *ps, *rs;

	if( *nm == '*' ) {		/* zap entire list? */
	    for( ps=rsps; ps!=NULLPTR; ps=rs ) {
		rs = ps->dlnext;
		free(ps);
	    }
	    rsps = NULLPTR;
	    return TRUE;
	}
	for( qs= &rsps; *qs!=NULLPTR; qs = &rs->dlnext ) {	/* zap part of list */
	    rs = *qs;
	    if( namematch(nm,rs->dlname) ) {
		*qs = rs->dlnext;
		free(rs);
		return TRUE;
	    }
	}
	return FALSE;			/* thru loop, can't find it */
}

namematch(s1,s2)
    BYTE *s1, *s2;
{
	for( ; *s1 && *s2; ++s1, ++s2 ) 
	    if( toupper(*s1) != toupper(*s2) )
		return FALSE;
	if( *s1 || *s2 )
	     return FALSE;
	else return TRUE;
}


/**************************************************************************/
#define PAGEWIDTH 72
rsp_display(rp)
DLIST *rp;
{
	WORD ii;

	printf("\nRSPs to be included are:\n");
	for( ii=0; rp!=NULLPTR; rp = rp->dlnext ) {
	    if( (ii += 15) > PAGEWIDTH ) {
		printf("\n");
		ii=15;
	    }
	    printf("   %12.12s",rp->dlname);
	}
	printf("\n");
}


/**************************************************************************/
/* grpseek: seeks in MOD file 'fd' to the start of a code/data group, */
/*	returns length (in bytes) of that group */
/*************************************************************************/
    LONG
grpseek(fde,gt)
    WORD fde;				/* file descriptor */
    WORD gt;				/* group type to look for */
{
	WORD gc;			/* group counter */
	LONG dps;			/* pgphs to group base in file */
	GROUP grp;			/* structure for Cmd Hdr info */

	if( lseek(fde,0L,0) < 0 )	/* seek to beginning of file */
	    CRASH("grp cmd header seek failed\n");
	dps=8;				/* pgphs to start of load group */
	for( gc=0; gc<8; gc++ ) {	/* search up to 8 groups in cmd hdr */
	    read(fde,&grp,sizeof(grp));	/* read each group in header */
	    if( grp.gtype == gt )	/* the group we want */
		    break;
	    dps += grp.glen;		/* add in size of previous group */
	}
	if( grp.gtype != gt )		/* did we find the group? */
	    return ((LONG) FAILURE);	/* nope, so fail */
	dps = dps << 4;			/* pgphs xfm to bytes */
	if( lseek(fde,dps,0) < 0 )	/* seek to start of data group */
	    CRASH("grp group seek failed\n");
	return ((LONG)((grp.glen)<<4));	/* return group size in bytes */
}


/**************************************************************************/
/* xfer rtns: transfer a group from file 'module' to file NEWSYS (fns) 
/*	returns number of paragraphs in the group
/**************************************************************************/
    UWORD
xfergrp(module,group_type)
    BYTE *module;			/* module name */
    WORD group_type;			/* code? or data? */
{
	REG WORD fm;			/* file descriptor for module */
	REG UWORD rv,xferpart_grp();	/* return value */

	if( (fm=BROPEN(module)) < 0 ) 	/* open the module for binary reading */
		CRASH("can't find a system module\n");
	rv = xferpart_grp(fm,group_type,0);
	close(fm);
	return rv;
}

/**************************************************************************/
    UWORD
xferpart_grp(fm,gt,skip)
    WORD fm;				/* file descriptor for module */
    WORD gt;				/* group type: code or data */
    UWORD skip;				/* num pgphs to skip in this group */
{
	REG WORD ii;			/* counter */
	REG LONG xx, yy;		/* long counters */
	BYTE xferbuf[BUFSIZ];		/* a transfer buffer */
	GROUP grp;			/* a group area */

	if( lseek(fm,0L,0) < 0 )	/* seek to beginning of file */
	    CRASH("grp cmd header seek failed\n");
	yy = skip+8;			/* num pgphs to skip + in cmd hdr */
	for( ii=0; ii<8; ++ii ) {	/* read group hdrs in cmd hdr */
	    read(fm,&grp,sizeof(grp));
	    if( grp.gtype == gt )
		break;
	    yy += grp.glen;		/* count #pgphs in this group to skip */
	}
	if( grp.gtype!=gt ) CRASH("module doesn't have group\n");
	xx = yy << 4;			/* cvt to bytes */
	if( lseek(fm,xx,0)<0 ) CRASH("couldn't seek in module\n");
	xx = grp.glen; xx -= skip; 	/* pgphs to xfer */
	for( yy=xx<<4; yy>0; yy-=ii ){	/* xfer bytes to NEWSYS */
	    ii = ( yy > BUFSIZ ? BUFSIZ : yy );	/* num bytes per xfer */
	    if( read(fm,xferbuf,ii) != ii ) CRASH("not enough bytes in module\n");
	    write(fns,xferbuf,ii);
	}
	return (UWORD)(xx);		/* convert to pgphs */
}



/**************************************************************************/
/* help: prints out a help message */
/**************************************************************************/

BYTE *helpm1[] = {
	"\n\n",
#ifdef MCPM
        "\t\t*** GENCMPM Help Function ***",
        "\t\t=============================",
        "     GENCMPM lets you edit and/or generate a system image from",
#else
	"\t\t*** GENCCPM Help Function ***",
	"\t\t============================",
	"     GENCCPM lets you edit and/or generate a system image from",
#endif
	"operating system modules on the default drive.  A detailed",
	"explanation of each parameter may be found in the Concurrent CP/M-86",
	"System Guide, Section 2.\n",
#ifdef MCPM
        "     GENCMPM assumes the default values shown within square",
#else
	"     GENCCPM assumes the default values shown within square",
#endif
	"brackets.  All numbers are Hexadecimal.  To change a parameter,",
	"enter the parameter name followed by '=' and the new value.  Type",
	"<CR> (a carriage return) to enter the assignment.  You can make",
	"multiple assignments if you separate them by a space.  No spaces",
	"are allowed within an assignment.  Example:\n",
	"CHANGES?  verbose=N sysdrive=A: openmax=1A <CR>\n",
	"Parameter names may be shortened to the minimum combination of",
	"letters unique to the currently displayed menu.  Example:\n",
	"CHANGES?  v=N sysd=A: op=1a <CR>\n",
	NULLPTRI
};
BYTE *helpm2[] = {
	"\n\n",
	"Sub-menus (the last few options without default values) are accessed",
	"by typing the sub-menu name followed by <CR>.  You may enter",
	"multiple sub-menus, in which case each sub-menu will be displayed",
	"in order.  Example:\n",
	"CHANGES?  help sysparams rsps <CR>\n",
	"Enter <CR> alone to exit a menu, or a parameter name, '=' and the",
	"new value to assign a parameter.  Multiple assignments may be",
	"entered, as in response to the Main Menu prompt.\n",
	NULLPTRI
};


    BYTE *
help(tx)				/* print a helpful message to user */
    BYTE *tx;
{
	REG BYTE **cpp;		/* pointer to help strings */

	for( cpp=helpm1; *cpp != NULLPTR; cpp++ )
		printf("%s\n",*cpp);
	press_return();
	for( cpp=helpm2; *cpp != NULLPTR; cpp++ )
		printf("%s\n",*cpp);
	press_return();
	return NULLPTR;
}

/************************************************************************/
/* memory list allocation */
/**************************************************************************/


    BYTE *
domem(cmdval,memtitle)
    BYTE *cmdval;
    BYTE *memtitle;
{
	WORD cntmlist();		/* does list validation */
	BOOLEAN okl;			/* okay list */
	BYTE cmds[CMDSLEN];		/* place to put input */
	EXTERN MLIST *memroot;		/* memory list root */
	EXTERN MENU *memmenu;		/* memory edit menu */
	EXTERN BYTE *clearit;

	okl = (cntmlist(memroot) > 0);	/* overlap chk & condense */
	if(cmdval!=NULLPTR) {
	    domenu(cmdval,memmenu);	/* do the commands */
	    okl = (cntmlist(memroot)>0);/* check the results */
	}
	if( !okl || cmdval==NULLPTR ) 	/* do we need to interact with user? */
	    FOREVER {			/* display, the edit */
		printf("%s",clearit);	/* clear the screen (maybe) */
		dspmlist(memroot);	/* here's what it looks like */
		prtmenu(memtitle,memmenu);	/* here's how to edit */
		printf(PROMPT);		/* what to do? */
		if( gets(cmds)!=cmds ||	*cmds==NULL ) {	/* see if the want to get out */
		    if(okl)
			 break;
		    else {
			printf("Please adjust memory partitions\n");
			press_return();
			continue;
		    }
		}
		domenu(cmds,memmenu);	/* edit the memory list */
		okl = (cntmlist(memroot)>0);/* validate the memory list */
	    }
	return NULLPTR;			/* standard return */
}


/**************************************************************************/
    BYTE *
addmem(ms)				/* add memory partitions */
    BYTE *ms;				/* memory partition spec */
{
	REG BYTE *pl;			/* pointer to last addr spec */
	REG BYTE *ps;			/* pointer to size spec */
	UWORD f, l, qq, s;		/* values of spec */
	MLIST *intomlist(), *mnew;	/* where to put them */
	EXTERN MLIST *memroot;		/* memory partitions list */
	BYTE *badspec = "Add memory partition spec should look like:\n\tadd=first,last,size";
	BYTE *badval = "Spec: add=first,last,size\n\twhere last>first, size>80";
	BYTE *warnbig = "Warning: size too big, it will be truncated";
	BYTE *warnleft = "Warning: region not evenly divided by size, will adjust last partition";
	BYTE *rval;
	BYTE *index();

	rval = NULLPTR;			/* default return value */
	if( (pl=index(ms,','))==0)	/* look for ptr to last addr */
		return badspec;
	else	*pl++ = NULL;		/* terminate first addr spec */
	if( (ps=index(pl,','))==0)	/* look for ptr to size */
		return badspec;
	else	*ps++ = NULL;
	f=atoih(ms);			/* convert first addr spec */
	s=atoih(ps);			/* convert size spec */
	if( *pl=='+' )			/* last addr relative to first? */
		l=atoih(pl+1)+f;	/* yes, so calculate */
	else if( *pl==NULL  ||  *pl=='*' ) {
	    if( *pl=='*' )		/* last addr multiple of size? */
		 l=atoih(pl+1);		/* spec multiple */
	    else l=1;			/* default multiple=1 */
	    l = f+l*s;			/* s in paragraphs */
	} else	l=atoih(pl);		/* no, it's absolute address */
	if( l<=f  ||  s==0 )		/* 1st validation check */
		return badval;		
	if( s > l-f ) {			/* 2nd validation check */
	    rval = warnbig;
	    s = l-f;
	}
	if( s<MINKMEM )			/* 3rd validation check */
		return "Memory partition must be at least 80 paragraphs";
	qq = ((l-f)/s)*s;		/* make sure last == first + xx * s */
	if( qq != (l-f) ) {			/* 4th validation check */
		rval = warnleft;
		l = f+qq;
	}
	mnew = (MLIST *)malloc(sizeof(*mnew));	/* make room */
	mnew->mlfirst = f;
	mnew->mllast = l;
	mnew->mlsize = s;
	memroot=intomlist(memroot,mnew);	/* sorted insertion into list */
	return rval;			/* everything okay */
}


/**************************************************************************/
    BYTE *
delmem(ms)				/* delete memory partition */
    BYTE *ms;
{
	REG WORD df, dl;		/* delete first, last */
	MLIST **mm, **mn, *mo, *mp, *mq;	/* necessary list ptrs */
	EXTERN MLIST *memroot;		/* memory partitions list */
	BYTE *pl;
	WORD i;
	BYTE *badspec="To delete a memory partition, say\n\tdelete=1  or  delete=1-3\n";

	if( *ms == '*' ) {		/* delete all? */
	    for( mp=memroot; mp!=NULLPTR; mp=mq ) {
		mq = mp->mlnext;	/* save the pointer before we free it */
		free(mp);
	    }
	    memroot = NULLPTR;
	    return NULLPTR;		/* everything okay */
	}
	if( (pl=index(ms,'-')) == 0 )
	       	pl=ms;			/* point last to first */
	else 	*pl++ = NULL; 		/* else point to after 'dash' */
	df=atoi(ms);			/* decimal conversion */
	dl=atoi(pl);
	for( i=1, mm=&memroot, mn=NULLPTR, mo=memroot;
	    i<=dl && mo!=NULLPTR;
	    ++i ) {
		if( i==df ) mn=mm;	/* found first, pt to ptr to */
		mm = &mo->mlnext;	/* ptr to ptr */
		mo = mo->mlnext;
	}
	if( mn==NULLPTR || i<=dl )	/* check range of deletions */
	    return badspec;
	mp = *mn;			/* save ptr to deleted list */
	*mn = mo;			/* cut them suckers out of the list */
	for( ; mp!=mo; mp=mq ) {
	    mq = mp->mlnext;
	    free(mp);	
	}
	return NULLPTR;			/* everything ok */
}



/**************************************************************************/
dspmlist(mroot)
    MLIST *mroot;
{
	MLIST *mn, *mo;
	REG WORD i, nump;

	printf("\n\n");
	printf("          Addresses          Partitions    (in paragraphs)\n");
	printf(" #      Start    Last       Size      Qty \n");
	for( i=1, mn=mroot; mn!=NULLPTR; ++i, mn=mo ) {
	    nump = (mn->mllast-mn->mlfirst) / mn->mlsize;
	    printf("%2.2d.     %4.4xh    %4.4xh      %4.4xh   %4.4xh ",
		    i,mn->mlfirst,mn->mllast,mn->mlsize,nump);
	    if( (mo=mn->mlnext) != NULLPTR ) {	/* do some checking */
		if( mn->mllast > mo->mlfirst ) 
		    printf("**overlaps** ");
		if( mn->mlsize > mn->mllast-mn->mlfirst )
		    printf("**partition too big** ");
	    }
	    printf("\n");
	}
}


/**************************************************************************/
    WORD
cntmlist(mroot)			/* check for overlaps, condense if possible */
    MLIST *mroot;		/* returns number mem partitions, 0 if bad list */
{
	MLIST *mn, *mo;			/* ptr within list */
	REG BOOLEAN olap;		/* partitions overlap or bad size */
	REG WORD nump;			/* number of partitions this block */
	REG WORD totnump;		/* keep the count going */

	totnump = 0;			/* total number partitions */
	olap = FALSE;			/* innocent until proven guilty */
	for( mn=mroot; mn!=NULLPTR && (mo=mn->mlnext)!=NULLPTR; ) {
	    if( mn->mlsize==mo->mlsize ){ 	/* partitions same size? */
	      if( mn->mlfirst==mo->mlfirst && 	/* same partitions? */
		  mn->mllast==mo->mllast ) {
			mn->mlnext=mo->mlnext;
			free(mo);	/* mo is redundant, zap it */
			continue;
	      } else 
	      if( mn->mllast==mo->mlfirst  ||	/* adjacent partition? */
		 mn->mllast==(mo->mlfirst-1) ) {
			mn->mllast=mo->mllast;	/* make 1st partition bigger */
			mn->mlnext=mo->mlnext;	/* zap out mo from list */
			free(mo);	/* we don't need it any more */
			continue;
	      }
	    }
	    if( mn->mllast > mo->mlfirst ) 
		olap=TRUE;		/* partitions overlap */
	    nump = (mn->mllast-mn->mlfirst) / mn->mlsize;
	    if( nump<=0 ) 
		olap=TRUE;		/* size > partition */
	    else totnump += nump;	/* incr this count */
	    mn=mo;			/* continue through loop */
	}
	if( mn!=NULLPTR ) {		/* catch the last in the loop */
	    nump = (mn->mllast-mn->mlfirst) / mn->mlsize;
	    if( nump<=0 ) 
		olap=TRUE;		/* size > partition */
	    else totnump += nump;	/* incr this count */
	}
	if( olap ) 
	     return 0;			/* boo boo, return 0 */
	else return totnump;		/* return num partitions */
}



/**************************************************************************/
    MLIST *
intomlist(mroot,mi)		/* insert mi into list mroot sorted */
    MLIST *mroot;
    MLIST *mi;
{
	MLIST **mm;			/* ptr to ptr to list */
	MLIST *mn;			/* ptr to list */

	for( mm=&mroot, mn=mroot;
	    mn != NULLPTR  &&  mi->mlfirst > mn->mlfirst; ){
		mm = &mn->mlnext;
		mn = mn->mlnext;
	}
	mi->mlnext = mn;
	*mm = mi;
	return mroot;
}

/**************************************************************************/
    UWORD		/* returns new size of partition, 0 if not enough room */
trimit(start,last,olds)
    UWORD start;			/* start address of partition */
    UWORD last;				/* end address of partition */
    UWORD olds;				/* old partition size */
{
	REG UWORD ss;

	ss = last-start;		/* size is in paragraphs */
	if( ss<MINKMEM ) return 0;	/* sorry, too small */
	else if( ss<olds ) return ss-1;	/* needs to be smaller */
	else return olds;		/* fine just the way it is */
}

    WORD
onbounds(start,size,nst)
    UWORD start;			/* original starting boundary */
    UWORD size;				/* partition size */
    UWORD nst;				/* new starting address */
{					/* returns addr>nst multiple of size from start */
	REG UWORD rr;

	for( rr=start; rr<nst; rr += size )
		;
	return rr;
}

/**************************************************************************/
/* trimlist: trims the memory list according to OS bounds */
/**************************************************************************/

    BOOLEAN				/* returns TRUE iff any adjustments */
trimlist(osstart,osend)
    UWORD osstart;			/* starting pgph */
    UWORD osend;			/* ending pgph */
{
	EXTERN MLIST *memroot;
	REG MLIST **mm, *mn, *mo;	/* ptrs into memlist */
	REG UWORD ss, zz;		/* temp vars */
	LOCAL WORD adjusts;		/* number of collisions */

	adjusts = 0;			/* we haven't adjusted anything yet */
	for( mm=&memroot, mn=memroot; mn!=NULLPTR; mn=*mm ) {
	    if( mn->mlfirst < osstart ) { 
		if( mn->mllast >= osstart ) { /** collision **/
		    ++adjusts;		/* we're doing some adjusting */
		    if( mn->mllast > osend ) {	/* real big mem part? */
			mo = (MLIST *)malloc(sizeof *mo); /* set it up now, adjust it later */
			mo->mlfirst = onbounds(mn->mlfirst,mn->mlsize,osstart);
			mo->mllast = mn->mllast;
			mo->mlsize = mn->mlsize;
			mo->mlnext = mn->mlnext;
			mn->mlnext = mo;
		    }
		    if( (zz=trimit(mn->mlfirst,osstart-1,mn->mlsize)) > 0 ) {
			mn->mllast = osstart-1;
			mn->mlsize = zz;	/* in case it was trimmed */
		    } else {		/* partitions too small, delete it */
			*mm = mn->mlnext;
			free(mn);
			continue;	/* don't increment list ptr */
		    }
		}			/**** end collision ****/

	    } else if( mn->mlfirst < osend ) { /** collision **/
		++adjusts;		/* we're doing some adjusting */
		if( mn->mllast <= osend ) {	/* does part cross past os? */
		    *mm = mn->mlnext;	/* no, delete it */
		    free(mn);
		    continue;
		}
		ss = onbounds(mn->mlfirst,mn->mlsize,osend);
		if( (zz=trimit(osend,ss,mn->mlsize)) > 0 ){/* set up a smaller mem part */
		    mo = (MLIST *)malloc(sizeof *mo);
		    mo->mlfirst = osend+1;
		    mo->mllast = ss;
		    mo->mlsize = zz;
		    mo->mlnext = mn;
		    *mm = mo;
		    mm = &mo->mlnext;
		}
		if( (zz=trimit(ss,mn->mllast,mn->mlsize)) > 0 ){
		    mn->mlfirst = ss;
		    mn->mlsize = zz;
		} else {		/* partition too small, delete */
		    *mm = mn->mlnext;
		    free(mn);
		    continue;		/* don't bottom out on this loop */
		}
	    }				/**** end collision ****/

	    mm = &mn->mlnext;		/* set up for next partition check */
	}				/**** end FOR loop ***/
	return adjusts>0;		/* returns TRUE if we adjusted */
}




