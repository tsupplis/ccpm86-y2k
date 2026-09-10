/**************************************************************************
 *	G E N C C P M -  generates an operating system
 *
 *	Written by Bill Fitler Nov. 1982
 *
 * GENCCPM is designed to edit the system image for the operating system
 * and construct an initialized, loadable image for execution.
 * GENCCPM was originally designed for Concurrent CP/M-86 v2.0, and was
 * modelled after the old GENSYS.
 *
 * Revision history:
 *	10 Feb 84 whf: bug fixes: addmem, fixupxios_info, and qbuf limits (glp)
 *	08 Jan 84 glp: added support for Extended DPB
 *      15 Dec 83 fmb: changed ncondev,nciodev initialization
 *	 5 Oct 83 whf: converted to DRC, broke apart modules, added ADDRs
 *       9 Sep 83 fmb: compiler bug fix for daend in gentables
 *       4 Aug 83 fmb: version number -> 3.1
 *	14 Jun 83 whf: fix flags & large xios bugs
 *       8 Jun 83 fmb: handles Multi-User Concurrent  3.1
 *      31 May 83 fmb: added sd_xpcns init for Multi-User CCP/M 3.1 .
 *      23 Feb 83 whf: add patch module
 *	25 Jan 83 whf: combine all modules for easier maintenance
 *	20 Jan 83 whf: handles SYSDAT overflow
 *	19 Nov 82 whf: added BDOS buffering generation.
 *
 ************************************************************************/


/*******************************/
/* for serialization purposes: */
char *copynote = "COPYRIGHT (C) 1983, DIGITAL RESEARCH XXXX-0000-654321 $";
/****************************************************/
/* up front for patching: Note the internal EOS (0) */
char *clearx = "\033H\033E\0\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
/****************************************************/

/****** include files ********/
#define MAINMODULE 1
#include <genccpm.h>
/*****************************/

#ifdef CCPM
BYTE *edsysver = "GENCCPM  v2.1  [2/7/84]\n";
#endif
#ifdef CMPM
BYTE *edsysver = "GENCCPM  v3.1  [2/7/84]\n";
#endif

BYTE mainmtitle[50] = "";		/* room for title of main menu */
MENU *mainmenu = NULLPTRI;		/* pointers to main menu items    */
MENU *memmenu=NULLPTRI;			/* memory management menu */
MENU *syspmenu=NULLPTRI;		/* system parameters menu */
MENU *rspmenu=NULLPTRI;			/* rsp edit menu */

/**************************************************************************/
/* main module								  */
/**************************************************************************/

main(ac,av)
WORD ac;
BYTE **av;
{
	BYTE cmds[CMDSLEN];		/* input area of commands */
	BOOLEAN sysvalid();		/* is the system data valid? */

	fprintf(stderr,edsysver);
	if( isatty(1) )			/* standard output to a tty? */
		clearit = clearx;	/* yes, do clear screens */
	else	clearit = "";		/* no, don't do clear screens */
	init_sysdat();			/* initialize system data */
	init_edsys();			/* initialize menus */
	if( ac>1 ) {			/* if any command line arguments */
	    copyargs(ac-1,av+1,cmds);	/* simplify cmd parser */
	    verbose = TRUE;		/* turn on wordiness by default */
	    domenu(cmds,mainmenu);	/* do what they've asked for */
	} 
	FOREVER {			/* work with user for a while */
		if(doit) {		/* does user want to quit? */
		    if( sysvalid() )	/* has evrything been defined? */
			break;		/* let them if everything okay */
		    press_return();	/* let them view the message(s) */
		}
		printf("%s",clearit);	/* clear the screen */
		prtmenu(mainmtitle,mainmenu);	/* display user options */
		printf(PROMPT);		/* prompt user */
		if( gets(cmds)!=cmds ){	/* on end of file */
		    close(0);		/* standard input */
		    open("CON:",0);	/* reopen to the console */
		    close(1);		/* standard output: assume redirection */
		    open("CON:",1);	/* reopen to console */
		    clearit = clearx;	/* do the clear screen routine */
		    continue;		/* force user to input blank line */
		}
		if( *cmds==NULL ) 	/* blank line of input */
		    continue;		/* don't bother with blank lines here */
		domenu(cmds,mainmenu);	/* do what they've asked for */
	    }				/*****************************/
					/* we've got user input */
	make_sysfile();			/* construct system image/use old one*/
	gentables();			/* generate the tables */
	fixups();			/* odds and ends on the system image */
	wrapup();			/* close files, etc. */
}

/* copyargs: copies args to cbuf for uniform parsing */
copyargs(ac,av,cbuf)
WORD ac;
BYTE **av;
BYTE *cbuf;
{
	for( *cbuf=NULL; ac>0; --ac, ++av ) {
		strcat(cbuf,*av);
		strcat(cbuf," ");
	}
}

/**************************************************************************/
/* init_sysdat: reads SYStem DATa info from:
/*	1. SYSFILE, if it exists
/*	2. SYSMOD, o.w. if this is the case, it also checks 
/*		for existence of all other required modules
/**************************************************************************/
VOID init_sysdat()
{
	REG WORD sf;			/* file descriptor for SysData read */
	LONG grpseek();			/* seek the data group */
	UWORD goffs;			/* loc of group */
	EXTERN DLIST *rsps, *dirsearch(); /* directory search fn */
	BYTE *addmem();			/* for initializing memory */
	BYTE *chkmodules(), *xlist[80];	/* list of missing modules */

	printf("GENerate SYStem image for %s\n",VERLABEL);
	init_sds(sdat_area);		/* initialize program ptrs */
			/***************************************************/
	if(chkmodules(xlist)==NULLPTR  &&  (sf=BROPEN(SYSMOD)) > 0 ) {
	    printf("Constructing new %s file\n",SYSFILE);
					/*** procedureize this? for cmd use */
	    dogensys=TRUE;		/* no SYSFILE, read from modules */
	    if(grpseek(sf,GTYPDATA)<0)	/* seek start of data group */
		CRASH("no data group in SYSDAT module\n");
	    read(sf,sdat_area,SDLEN);	/* read in the data group */
	    /**** may have to init arbitrarily some MAX values here ****/
	    xtrapds = *sd_pul;		/* default number of extra process descriptors */
	    totqcbs = *sd_qul;		/* default number of qcbs */
	    totopen = 0x40;		/* arbitrary total open files & locked recs */
	    qbuflen = *sd_qmalen;	/* init'ed to bytes */
	    rsps = dirsearch("*.RSP");	/* init RSP list */
	    addmem("400,6000,400");	/* start out with this memory allocation */
	    close(sf);			/* don't need it for a while */

	    sf=BROPEN(XIOSMOD);		/* let's go read the XIOS info */
	    if( grpseek(sf,GTYPDATA) <= 0 ) /* if there is not a data group */
		grpseek(sf,GTYPCODE);	/* use the code group instead */
	    goffs = lseek( sf,0L,1 );	/* where did we end up? */
	    initxios_info(sf,goffs);	/* grab info from there */
	    close(sf);			/* don't need it for a while */

	} else if( FALSE  &&		/****** temporarily disable this option!!! ***/
		(sf=BROPEN(SYSFILE)) >= 0 ){ /* does SYSFILE exist? *******/
	    printf("Editting %s file\n",SYSFILE);
	    dogensys = FALSE;		/* tell them not to gen a new system */
	    grpseek(sf,GTYPDATA);	/* get data module in OS */
	    read(sf,sdat_area,SDLEN);	/* read in the info */
	    /**** may have to trace down some lists for MAX values ****/
	    close(sf);			/* don't need it for a while */
	} else {	/***************************************************/
	    if( xlist == NULLPTR ) {
		fprintf(stderr,"Can't find %s module\n",SYSMOD);
	    } else {
		fprintf(stderr,"Can't find these modules:\n%s\n",xlist);
	    }
	    fprintf(stderr,"Please find the correct modules\n");
	    exit(1);
	}				/**** end module check ****/
	if( VERSION != *sd_mpmvn ){	/* a little version number checking */
	    USERR("%s works on OS version %x\n",edsysver,VERSION);
	    USERR("Sys Data area found was from OS version %x\n",*sd_mpmvn);
	    USERR("Please find correct .SYS or %s files\n",MODEXT);
	    exit(1);
	}
}


/**************************************************************************/
#define RDAC 4				/* Read Access mode for 'access' */
    BYTE *
chkmodules(badbuf)			/* place to put missing module names */
BYTE *badbuf;
{

	*badbuf=0;	
	if( access(SYSMOD,RDAC) ) {strcat(badbuf,SYSMOD);strcat(badbuf," ");}
	if( access(SUPMOD,RDAC) ) {strcat(badbuf,SUPMOD);strcat(badbuf," ");}
	if( access(RTMMOD,RDAC) ) {strcat(badbuf,RTMMOD);strcat(badbuf," ");}
	if( access(MEMMOD,RDAC) ) {strcat(badbuf,MEMMOD);strcat(badbuf," ");}
	if( access(CIOMOD,RDAC) ) {strcat(badbuf,CIOMOD);strcat(badbuf," ");}
	if( access(BDOSMOD,RDAC) ) {strcat(badbuf,BDOSMOD);strcat(badbuf," ");}
	if( access(XIOSMOD,RDAC) ) {strcat(badbuf,XIOSMOD);strcat(badbuf," ");}
	if( *badbuf ) 
		return badbuf;
	else	return NULLPTR;
}




/**************************************************************************/
/* init_edsys: initializes program variables and menus
/**************************************************************************/

VOID init_edsys()
{
	MENU *m;			/* mainmenu pointer */
	MENU *me;			/* memory partitions menu */
	MENU *s;			/* sys params menu */
	MENU *r;			/* rsp menu */
	MENU *bldmenu();		/* func to build the menu list */
	BYTE *drvmsg;			/* destination drive message */
	BYTE *delmsg;			/* delete old SYSFILE msg */
	BYTE getdrive();		/* get default drive */
	BYTE *help();			/* func to help user */
	BYTE *doxios();			/* func to do xios menu */
	BYTE *dosysp();			/* func to do sys param menu */
	BYTE *dorsp();			/* func to display & handle RSPs */
	BYTE *dolbl();			/* func to modify version label */
	BYTE *domem();			/* handle memory partitions */
	BYTE *doxbufs();		/* func to construct bdos buffers */
	BYTE *addmem();			/* add mem part */
	BYTE *delmem();			/* delete mem part */
	BYTE *rspinclude();		/* include a bunch of RSPs */
	BYTE *rspexclude();		/* exclude a bunch of RSPs */
	BYTE *doexit();			/* exit the main menu */

	sprintf(mainmtitle,"*** %s %s Main Menu ***",VERLABEL,PROGNAME);
	destdrv = getdrive();		/* returns the default drive */
	drvmsg = malloc(50);
	sprintf(drvmsg,"%s Output To (Destination) Drive",SYSFILE);
	delmsg = malloc(55);
	sprintf(delmsg,"Delete (instead of rename) old %s file\n",SYSFILE);
	m=NULLPTR;  me=NULLPTR; s=NULLPTR; r=NULLPTR;	/* init menu pointers */

	/***** initialize menus *****/
	m=bldmenu(m,MPROC,help,"help","GENCCPM Help");
	m=bldmenu(m,MBOOL,&verbose,"verbose","More Verbose GENCCPM Messages");
	m=bldmenu(m,MDRIV,&destdrv,"destdrive",drvmsg);
	m=bldmenu(m,MBOOL,&doclean,"deletesys",delmsg);


	m=bldmenu(m,MPROC,dosysp,"sysparams","Display/Change System Parameters");

	s=bldmenu(s,MDRIV,sd_srchdisk,"sysdrive","System Drive");
	s=bldmenu(s,MDRIV,sd_tempdisk,"tmpdrive","Temporary File Drive");
	s=bldmenu(s,MBOOL,sd_dayfile,"cmdlogging","Command Day/File Logging at Console");
	s=bldmenu(s,MBOOL,sd_cmode,"compatmode","CP/M FCB Compatibility Mode");
	s=bldmenu(s,MWORD,sd_mmp,"memmax","Maximum Memory per Process (paragraphs)");
	s=bldmenu(s,MBYTE,sd_popen_max,"openmax","Open Files per Process Maximum");
	s=bldmenu(s,MBYTE,sd_plock_max,"lockmax","Locked Records per Process Maximum\n");

	s=bldmenu(s,MWORD,sd_mpmseg,"osstart","Starting Paragraph of Operating System");
	s=bldmenu(s,MWORD,&totopen,"nopenfiles","Number of Open File and Locked Record Entries");
	s=bldmenu(s,MBYTE,&xtrapds,"npdescs","Number of Process Descriptors");
	s=bldmenu(s,MBYTE,&totqcbs,"nqcbs","Number of Queue Control Blocks");
	s=bldmenu(s,MWORD,&qbuflen,"qbufsize","Queue Buffer Total Size in bytes");
        s=bldmenu(s,MBYTE,sd_nflags,"nflags","Number of System Flags");


	m=bldmenu(m,MPROC,domem,"memory","Display/Change Memory Allocation Partitions");
	me=bldmenu(me,MPROC,addmem,"add","ADD Memory Partition(s)");
	me=bldmenu(me,MPROC,delmem,"delete","DELETE Memory Partition(s)");


	m=bldmenu(m,MPROC,doxbufs,"diskbuffers","Display/Change Disk Buffer Allocation");

	if( dogensys )
	    m=bldmenu(m,MPROC,dolbl,"oslabel","Display/Change Operating System Label");

	m=bldmenu(m,MPROC,dorsp,"rsps","Display/Change RSP list\n");
	r=bldmenu(r,MPROC,rspinclude,"include","Include RSPs");
	r=bldmenu(r,MPROC,rspexclude,"exclude","Exclude RSPs");


	m=bldmenu(m,MPROC,doexit,"gensys","I'm finished changing things, go GEN a SYStem\n");


	mainmenu=m;			/* assign now to longer name */
	memmenu=me;			/*  yah yah */
	syspmenu=s;			/*   "   "  */
	rspmenu=r;			/*   "   "  */
}


    BYTE *
doexit()				/* this is really a boolean... */
{
	doit = TRUE;
	return NULLPTR;
}

/**************************************************************************/
/* sysvalid: validates system data.
/*	also calculates certain SYSDAT variables, in the course of validation.
/**************************************************************************/
#define BYTE_ADJUST(xx) if( xx > 0xFF ) xx = 0xFF
#define ALIGN(xx) (((xx)+0xF) & 0xFFF0)

    BOOLEAN
sysvalid()
{
	REG BOOLEAN isv;		/* is valid? */
	BOOLEAN xiosvalid();		/* check xios info */
	LOCAL WORD nmparts;		/* number of memory partitions */

	isv = TRUE;			/* innocent until proven guilty */
	if( totopen < *sd_popen_max ) {
		USERR("'nopenfiles' is less than 'openmax'. Please adjust.\n");
		isv = FALSE;
	}
	if( totopen < *sd_plock_max ) {
		USERR("'nopenfiles' is less than 'lockmax'. Please adjust.\n");
		isv = FALSE;
	}
	if( (nmparts=cntmlist(memroot)) <= 0 ) {
		USERR("Memory Partitions need to be adjusted\n");
		isv = FALSE;
	}
	if( !xiosvalid() ) {		/* xios info needs work? */
		USERR("Disk Performance Buffers need to be adjusted\n");
		isv=FALSE;
	}
	if( !isv )
		USERR("Please correct the System Parameters\n");
	return isv;
}



/**************************************************************************/
/* make_sysfile: responsible for preparing SYSfile image for table generation
/* and 
/*	if( dousesys )
/*		doctors up SYSFILE for appropriate table generation and fixups
/*	else	constructs a SYSfile from assembled modules
/**************************************************************************/
VOID make_sysfile()
{
	BYTE fbuf[20];			/* file name buffer */

	PUTDRV(fbuf,NEWSYS);
	unlink(fbuf);			/* this is the file we'll be using */
	if( (fns=BWCREAT(fbuf))<0 ) {	/* try to open for output */
	    USERR("can't open new .SYS file (%s)\n",fbuf);
	    exit(1);
	}
	if( dogensys )
		gensys();		/* make a new SYS file from modules */
	else	edsys();		/* edit the old SYS file */
}


/**************************************************************************/
/* gensys: generates a new SYS file image from modules
/**************************************************************************/
    VOID
gensys()
{
	WORD fxm;			/* file descriptor for xios mod */
	BYTE fbuf[20];

	if( doclean ) {			/* clean up the SYSFILE first? */
	    PUTDRV(fbuf,SYSFILE);
	    unlink(fbuf);		/* delete if there... */
	}
	if(verbose) printf("Generating new SYS file\n");
	FCHECK(fns);			/* File error check */
	write(fns,cmdhdr,SECSIZ);	/* write out a (dummy) command hdr */
	genfix(); 			/* remember the sysdat & sup fixups */
	clsup = xfergrp(SUPMOD,GTYPCODE);
	clos_label = writelbl();	/* write the OS label (returns 0 if none exists) */
	if( clos_label != 0 )
		*sd_verptr = clsup<<4;	/* fill in offset of label rel sup code seg */
	genfix(); clrtm = xfergrp(RTMMOD,GTYPCODE);
	genfix(); clmem = xfergrp(MEMMOD,GTYPCODE);
	genfix(); clcio = xfergrp(CIOMOD,GTYPCODE);
	genfix(); clbdos= xfergrp(BDOSMOD,GTYPCODE);
	clnet = 0;

	fxm = BROPEN(XIOSMOD);		/* look at XIOS module */
	if( grpseek(fxm,GTYPDATA) >= 0){	/* is there a data group?*/
		USERR("XIOS has separate code and data (small model)");
		USERR("This is not supported in this O.S.: use 8080 model.");
		exit(1);
	} else {
		pure_xios = FALSE;
		clxios = 0;
	}
	cltotal = clsup+clos_label+clrtm+clmem+clcio+clbdos+clnet+clxios;
	/*** finished writing the code portion of NEWSYS ***/

	fldstart = lseek(fns,0L,1);	/* where in file SYSDAT gets written */
	dsstart = *sd_mpmseg + cltotal;	/* where in memory SYSDAT gets loaded */
	dlsysdat = xfergrp(SYSMOD,GTYPDATA);	/* system data area, fixed later */

	if( pure_xios )
		dlxios = xferpart_grp(fxm,GTYPDATA,dlsysdat);
	else { genfix(); dlxios = xferpart_grp(fxm,GTYPCODE,dlsysdat);	}
	close(fxm);			/* don't need this file descriptor */
	dltotal = dlsysdat+dlxios;	/* this will change later */
	/*** finished writing STATIC data portion of NEWSYS ***/

	/*** fill in sysdat here ***/
	sd_supmod->ep_eseg  = sd_supmod->ep_iseg  = *sd_mpmseg;
	sd_rtmmod->ep_eseg  = sd_rtmmod->ep_iseg  = sd_supmod->ep_eseg+clsup+clos_label;
	sd_memmod->ep_eseg  = sd_memmod->ep_iseg  = sd_rtmmod->ep_eseg+clrtm;
	sd_ciomod->ep_eseg  = sd_ciomod->ep_iseg  = sd_memmod->ep_eseg+clmem;
	sd_bdosmod->ep_eseg = sd_bdosmod->ep_iseg = sd_ciomod->ep_eseg+clcio;
	sd_xiosmod->ep_eseg = sd_xiosmod->ep_iseg = sd_bdosmod->ep_eseg+clbdos;
	if( pure_xios )			/* compute xios offsets */
		sd_xiosmod->ep_ioff = 0;	/* separate code */
	else	sd_xiosmod->ep_ioff = dlsysdat<<4;	/* mixed code & data, after sysdat */
	sd_xiosmod->ep_eoff = sd_xiosmod->ep_ioff + 3;
}

/****************************************************************************/
genfix()		/* subroutine to remember SYSDAT & Super fixups */
{
	LONG fa;

	fa = lseek(fns,0L,1);		/* where are we in this file? */
	fixlater(fa+6,F_PUT,&dsstart);	/* fix the SYSDAT address */
	fixlater(fa+8,F_PUT,&(sd_supmod->ep_eoff));	/* Supervisor entry offset */
	fixlater(fa+10,F_PUT,&(sd_supmod->ep_eseg));	/*     "        "   segment */
}


/**************************************************************************/
/* edsys: edit the old SYS file image.
/*	- copies old sys file into new
/*	- if( donew_xios ) will patch in a new XIOS from module
/**************************************************************************/
    VOID
edsys()
{
	if(verbose) printf("Editting old SYS file\n");
	write(fns,cmdhdr,SECSIZ);	/* leave room for command hdr */
	cltotal = xfergrp(SYSFILE,GTYPCODE);
	dltotal = xfergrp(SYSFILE,GTYPDATA);
	if( donew_xios ) {
	    ; /* need to read 'xt' from XIOS module */
	} else {
	    ; /* need to init 'xt' from sysdat */
	}
}



/**************************************************************************/
/* fixups: inserts updated SYSDAT page into NEWSYS file, and does some
/*	address fixups
/**************************************************************************/
VOID fixups()
{
	REG WORD ii;			/* counter */
	ADDR gg;			/* temp var */
	GROUP *cg, *dg;			/* group ptrs */
	UWORD genmfl();			/* (re)Gen Memory Free List */

	if(verbose) printf("Doing fixups\n");
	FCHECK(fns);			/* File error check */
					/*** fixup the memory list ***/
	if(verbose) {
	    printf("SYS image load map:\n");
	    printf("        Code starts at %4.4x\n",*sd_mpmseg);
	    printf("        Data starts at %4.4x\n",dsstart);
	    printf("       Tables start at %4.4x\n",(dsotables>>4)+dsstart);
	    printf("         RSPs start at %4.4x\n",(dsotables>>4)+dsstart+dltables);
	    printf(" XIOS buffers start at %4.4x\n",rsvd_seg);
	    printf("          End of OS at %4.4x\n",*sd_endseg);
	}

	if( trimlist(*sd_mpmseg,*sd_endseg) ) {	/* mem part list trimmed? */
	    printf("Trimming memory partitions. New list:");
	    dspmlist(memroot);
	    lseek( fns, locmfl, 0 );
	    gg = (dlsysdat+dlxios)<<4;
	    genmfl(gg);			/*****/
	}
					/*** fixup XIOS info ***/
	fixupxios_info(); 		/* fixes the buffer info (DPH info) */
					/*** fixup the command header ***/
	for( ii=0; ii<SECSIZ; ii++ )	/* zero out command header */
	    cmdhdr[ii] = '\0';
	cg = cmdhdr;			/* code group first */
	cg->gtype = GTYPCODE;		/* init all code vars */
	cg->glen = cltotal;		/* total length of code groups */
	cg->gmin = cltotal;		/* min length of code group */
	cg->gabase = *sd_mpmseg;	/* loads at absolute address */
	dg = cmdhdr + sizeof(*cg);	/* data group next */
	dg->gtype = GTYPDATA;		/* init all data vars */
	dg->glen = dltotal;		/* total length of data group+tables */
	dg->gmin = dltotal;		/* min length of data group */
	dg->gabase = cg->gabase+cltotal;/* abs addr of start of system data */
	lseek( fns, 0L, 0 );		/* seek the beginning of NEWSYS */
	write(fns,cmdhdr,SECSIZ);	/* write out the command header */

					/*** fixup the sysdat page ***/
	lseek(fns,fldstart,0);		/* seek there in file */
	write(fns,sdat_area,SDLEN);	/* put it into the file */

					/*** take care of the little fixes ***/
	fixfile();			/* fixes from the list */
}

/**************************************************************************/
/* wrapup: closes files
/**************************************************************************/
VOID wrapup()
{
	BYTE buf1[20], buf2[20];

	if(verbose) printf("Wrapping up\n");
	FCHECK(fns);			/* File error check */
	lseek(fns,0L,2);		/* flush NEWSYS buffers? */
	if(strlen(edsysver) != write(fns,edsysver,strlen(edsysver))) {
	    USERR("WRITE FAILURE - the disk may be too full\n");
	    exit(1);
	}
	close(fns);			/* all finished with NEWSYS */
	PUTDRV(buf1,OLDSYS); unlink(buf1);
	PUTDRV(buf2,SYSFILE); rename(buf2,buf1);
	PUTDRV(buf1,NEWSYS); rename(buf1,buf2);
}


