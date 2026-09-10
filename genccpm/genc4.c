/************************************************************************/
/************************************************************************/
/* genc4.c - This module exports the following routines:
/*
/* initxios_info(xf,goff)	'xf' is XIOSMOD file descriptor, 'goff'
/*	is offset of data group in XIOSMOD (including header);  
/*	this module inits the buffering info questions.
/* doxbufs(txt,title)	'txt' is a command, 'title' is the prompt for this fn;
/*	this routine is called within the MENU module, and prompts user
/*	for xios info.
/* xiosvalid()  returns a BOOLEAN value indicating whether the xios info
/*	is valid.
/* genxios(doffset)  generates the buffers for the xios, starting at 'doffset'
/*	in the system data area.
/* fixupxios_info()	writes DPH fixups required by genxios
/**************************************************************************/
/**************************************************************************/
#ifndef MAINMODULE
#include <genccpm.h>
#endif


/**************************************************************************/
/* edsdisk.h : defines the data structures used in this module
/**************************************************************************/

#define ADDR UWORD			/* Address (offset) within SysDat */
#define SEG  UWORD			/* Segment address (may be outside SysDat) */

#define DPH struct dph_item
DPH {					/* Disk Parameter Header structure */
	ADDR dph_xlt;			/* Translation Table Address */
	BYTE dph_space[6];		/* reserved & Media Flag */
	ADDR dph_dpb;			/* Disk Parameter Block Address */
	ADDR dph_csv;			/* CheckSum Vector Address */
	ADDR dph_alv;			/* Allocation Vector Address */
	ADDR dph_dirbcb;		/* Dir Buffer Control Block Header Address */
	ADDR dph_datbcb;		/* Data Buffer Control Block Header Address */
	SEG  dph_hstbl;			/* Hash Table Segment */
};

#define EDPB struct edbp_item
EDPB {				/* Extended Disk Parameter Block entries */
	WORD edpb_extflag;		/* Extended DPB flag */
	WORD edpb_nfats;		/* Number of FATS */
	WORD edpb_nfatrecs;		/* Number of records per FAT */
	WORD edpb_nclstrs;		/* Number of clusters */
	WORD edpb_clsize;		/* Cluster size */
	WORD edpb_fatadd;		/* FAT address */
};

#define DPB struct dpb_item
DPB {					/* Disk Parameter Block structure */
	WORD dpb_spt;			/* Sectors Per Track */
	BYTE dpb_bsh;			/* Allocation Block Shift Factor */
	BYTE dpb_blm;			/* Allocation Block Mask */
	BYTE dpb_exm;			/* Extent Mask */
	WORD dpb_dsm;			/* Disk Storage Maximum */
	WORD dpb_drm;			/* Directory Maximum */
	WORD dpb_dav;			/* Directory Allocation Vector */
	WORD dpb_cks;			/* Checksum Vector Size */
	WORD dpb_off;			/* Track Offset */
	WORD dpb_psh;			/* Physical Record Shift Factor */
	WORD dpb_phm;			/* Physical Record Mask */
};

#define BCBH struct bcbh_item
BCBH {					/* Buffer Control Block Header structure */
	ADDR bcbh_lr;			/* BCB List Root */
	BYTE bcbh_pm;			/* BCB Process Max */
};

#define BCB struct bcb_item
BCB {					/* Buffer Control Block structure */
	BYTE bcb_drv;			/* Drive */
	BYTE bcb_record[3];		/* Record Number */
	BYTE bcb_wflg;			/* Write Pending Flag */
	BYTE bcb_seq;			/* Sequential Access Counter */
	WORD bcb_track;			/* Logical Track Number */
	WORD bcb_sector;		/* Logical Translated Sector Number */
	UWORD bcb_bufptr;		/* Buffer Offset (for DIRBCB) / Segment (for DATBCB) */
	ADDR bcb_link;			/* Link to next BCB */
	WORD bcb_pdadr;			/* Process Descriptor Address */
};


#define DINFO struct dirbuf_information
DINFO {
	BOOLEAN d_modified;		/* DPH was modified? */
	BOOLEAN d_hashing;		/* user wants hashing? */
	WORD d_ndirbs;			/* Num dir buffers */
	WORD d_sdirbs;			/* Size dir buffers */
	WORD d_pmdirbs;			/* Process Max Number Dir Buffs */
	WORD d_ndatbs;			/* Num data buffers */
	WORD d_sdatbs;			/* Size data buffers */
	WORD d_pmdatbs;			/* Process Max Number Dir Buffs */
	WORD d_shash;			/* Size of hash table */
	WORD d_alvsize;			/* size of Alloc Vector area (in bytes ) */
	WORD d_blksize;			/* size of disk Allocation Block */
	WORD d_csvsize;			/* size of Checksum Vector area */
};

/* end of edsdisk.h ********************************************************/



/***************************************************************************/
WORD csfd;				/* command file descriptor */
WORD csgoff;				/* group offset within command file */

cseek(offs)				/* seek offset within Command File */
    ADDR offs;
{
	lseek(csfd,(LONG) offs+csgoff, 0);	/* compensate for cmd header */
}

/***************************************************************************/
/* Put special byte for buffer fills */
#define ZBYTE (doffset&0xFF)		/* the byte usually passed in */
    WORD
bufwrite(size,byt)
    WORD size;
    BYTE byt;
{
	WORD ss;

	for( ss=size; ss>0; --ss )
		write(fns,&byt,1);
	return size;
}


/***************************************************************************/
/* Module local variables */
#define DPHNUM 16			/* number of possible disks */
ADDR *dphtab;				/* table of offsets of Disk Parm Hdrs */
DPH *dh[DPHNUM];			/* table of Disk Parm Hdrs */
DINFO *di[DPHNUM];			/* table of info on DPHs */


/* Module local predicates */
#define ASKFOR(aa) ((aa) == 0xFFFF)	/* Is this one we're sposed to ask for? */
#define SHRMSK 0x8000			/* Is this shared with another drive? */
#define SHARED(bb) ((bb)!=0xFFFF  &&  (bb) & SHRMSK)	/* Is this nd??bs field shared with another disk? */
#define GETSHARED(cc) ((cc) & ~SHRMSK)	/* Get the drive we share with */



/***************************************************************************/
/* EXPORT
/* initxios_info: gets whatever information GENSYS needs from xios
/***************************************************************************/
initxios_info(xf,goff)			/* get info from xios file */
    WORD xf;				/* xios file descriptor */
    WORD goff;				/* group offset within xios file */
{
	REG WORD ii;			/* counter */
	DPB dd;				/* a place to keep this info */
	EDPB ee;			/* a place to keep this info */
	DPH *dph;
	DINFO *dinf;

	csfd = xf;			/* assign to global: set up cseek */
	csgoff = goff;			/* assign to global */
	cseek(XTABLOC);			/* go to where the XIOSTAB info is */
	read(xf,&xt,sizeof xt);		/* read in the xios info */
	dphtab = xt.xt_dphtab;		/* point to disk parm header table */

	for( ii=0; ii<DPHNUM; ++ii ) {	/* for each dph pointer */
	    if( dphtab[ii] == 0 ) {	/* is there a dph? */
		dh[ii]=NULLPTR;
		di[ii]=NULLPTR;		/* no tickee, no washee */
	    } else {			/* aha! let's set up this info */
		dh[ii] = (DPH *)malloc(sizeof(DPH));
		di[ii] = (DINFO *)malloc(sizeof(DINFO));	
		zfill(di[ii],sizeof(DINFO));
		cseek(dphtab[ii]);
		read(xf,dh[ii],sizeof(DPH));
	    }
	}				/* end of reading dph */
	for( ii=0; ii<DPHNUM; ++ii ) 	/* let's go thru it again */
	    if( dh[ii] ) {		/* if there's something there */
		dph = dh[ii];
		dinf = di[ii];
        	cseek(dph->dph_dpb);	/* point to the disk parm block */
	        read(xf,&ee,sizeof(EDPB));	/* Read as though EDPB */
                if (ee.edpb_extflag == 0xFFFF) {	/* Extended DPB */
		    cseek(dph->dph_dpb+12);	/* seek past extended info */
		} else {
		    cseek(dph->dph_dpb);
		}
        	read(xf,&dd,sizeof(DPB));	/* read DPH */
		dinf->d_blksize = 128 << dd.dpb_psh;
		dinf->d_csvsize = 0x7FFF & dd.dpb_cks;
		dinf->d_alvsize = (dd.dpb_dsm/8 + 1) * 2;
		dinf->d_shash = 4 * (1 + dd.dpb_drm);
		dinf->d_ndirbs = -1;	/* ask for these by name! */
		dinf->d_ndatbs = -1;
		dinf->d_hashing = TRUE;	/* default to hashing */
	    }				/* end of drive info init */
}



/************************************************************************/
/* EXPORT
/* doxbufs: queries user for BDOS buffering info;
/*	exits only when all data valid
/************************************************************************/
    BYTE *
doxbufs(txt,bdtitle)
    BYTE *txt;
    BYTE *bdtitle;
{
	BYTE cmds[CMDSLEN];
	BOOLEAN xiosvalid();
	EXTERN BYTE *clearit;

	FOREVER {
	    printf("%s",clearit);	/* clear screen */
	    printf("%s\n",bdtitle);
	    dbufs_display();			/* display */
	    printf("Drive (<CR> to exit) ? ");
	    if( gets(cmds)!=cmds || *cmds==NULL ) {	/* they hit <CR>? */
		if( xiosvalid() ) break;
		USERR("Please correct drive buffers information\n");
		press_return();
		continue;
	    }
	    askabout(cmds);		/* go do what they want */
	}
	return NULLPTR;			/* error msg for menu handler */
}

/************************************************************************/
/* dbufs_display: display disk buffering info 
/************************************************************************/
dbufs_display()
{
	REG UWORD ii, jj, tot;		/* counters */
	DPH *dph;
	DINFO *dinf;

	tot = 0;			/* init this */
	dcalcsizes();			/* check all the sizes */
	printf("\n\t*** Disk Buffering Information ***\n");
	printf("      Dir  Max/Proc   Data Max/Proc   Hash   Specified\n");
	printf("Drv   Bufs Dir Bufs   Bufs Dat Bufs   -ing   Buf Pgphs\n");
	printf("===   ==== ========   ==== ========   ====   =========\n");
	for( ii=0; ii<DPHNUM; ++ii ) {	/*** for each drive ***/
	    if( dphtab[ii]==0 ) continue; /* no DP Hdr for this drive */
	    dph = dh[ii]; dinf = di[ii];
	    printf(" %c:   ",'A'+ii);	/* print which drive */
	    dspdnums(dph->dph_dirbcb,dinf->d_ndirbs,dinf->d_pmdirbs);
	    dspdnums(dph->dph_datbcb,dinf->d_ndatbs,dinf->d_pmdatbs);
	    if( ! ASKFOR(dph->dph_hstbl) )
		 printf(" fixed  ");
	    else if( dinf->d_hashing )
		 printf(" yes    ");
	    else printf(" no     ");
	    jj = dspdsize(dph,dinf);
	    if( jj == -2 )
		 printf(" fixed   ");
	    else if( jj == -1 )
		 printf(" ??      ");
	    else {
		printf(" %4.4x   ",jj >> 4);
		tot += jj;
	    }
	    printf("\n");
	}				/*** end for each drive ***/
	printf("Total paragraphs allocated to buffers: %x\n",tot >> 4);
}


/***************************************************************************/
/* dspdsize: calculates a drive's user-requestable buffer size, in bytes
/***************************************************************************/
    WORD
dspdsize(dph,dinf)			/* calculate size requested */
    DPH *dph;				/* returns -1 if still needs info */
    DINFO *dinf;			/* returns -2 if pre-allocated */
{
	REG WORD siz;

	siz=0;
	if( !ASKFOR(dph->dph_dirbcb)  &&
	    !ASKFOR(dph->dph_datbcb)  &&
	    !ASKFOR(dph->dph_hstbl) )
		return -2;		/* nothing to say here */
	if( ASKFOR(dph->dph_dirbcb) ){
	    if( ASKFOR(dinf->d_ndirbs) )
		return -1;
	    if( !SHARED(dinf->d_ndirbs) )
		siz += dinf->d_sdirbs * dinf->d_ndirbs;
	}
	if( ASKFOR(dph->dph_datbcb) ){
	    if( ASKFOR(dinf->d_ndatbs) )
		return -1;
	    if( !SHARED(dinf->d_ndatbs) )
		siz += dinf->d_sdatbs * dinf->d_ndatbs;
	}
	if( ASKFOR(dph->dph_hstbl) ){
	    if( dinf->d_hashing )
		siz += dinf->d_shash;
	}
	return siz;
}


/***************************************************************************/
/* dspdnums: displays the data/dir buf numbers
/***************************************************************************/
dspdnums(nfix,ninput,npm)		/* display subroutine */
    WORD nfix;
    WORD ninput;
    WORD npm;
{
	if( !ASKFOR(nfix) )
	    printf("fixed           ");
	else if( SHARED(ninput) )
	    printf("shares %c:       ",'A'+GETSHARED(ninput));
	else {
	    if( ASKFOR(ninput) )
		 printf(" ??  ");
	    else printf(" %2.2x  ",ninput);
	    if( ASKFOR(npm) )
		 printf("  ??       ");
	    else printf("  %2.2x       ",npm);
	}
}


/***************************************************************************/
/* dcalcsizes: calculates buffer sizes across shared buffers
/***************************************************************************/
dcalcsizes()			/* make sure all the sizes are correct */
{
	REG WORD ii, jj;
	REG WORD *pkk;
	DPH *dph;
	DINFO *dinf;

	for( ii=0; ii<DPHNUM; ++ii )	/* init all sizes to zero */
	    if(dphtab[ii]) {
		dinf = di[ii];
		dinf->d_sdirbs = dinf->d_sdatbs = 0;
	    }
	for( ii=0; ii<DPHNUM; ++ii )		/* for each drive */
	    if(dphtab[ii]) {
		dph = dh[ii];
		dinf = di[ii];
		if( ASKFOR(dph->dph_dirbcb) ) 	/* max out the size of a dirbuf */
		    if( !ASKFOR(dinf->d_ndirbs) ) { /* if it's real data there */
			if( SHARED(dinf->d_ndirbs) ) {
			    jj = GETSHARED(dinf->d_ndirbs);
			    pkk = & ((di[jj])->d_sdirbs);
			} else 
			    pkk = & dinf->d_sdirbs;
			if( dinf->d_blksize > *pkk ) /* assign the maximum size found */
			    *pkk = dinf->d_blksize;
		    }
		if( ASKFOR(dph->dph_datbcb) ) 	/* max out the size of a data buf */
		    if( !ASKFOR(dinf->d_ndatbs) ) {
			if( SHARED(dinf->d_ndatbs) ) {
			    jj = GETSHARED(dinf->d_ndatbs);
			    pkk = & ((di[jj])->d_sdatbs);
			} else 
			    pkk = & dinf->d_sdatbs;
			if( dinf->d_blksize > *pkk )
			    *pkk = dinf->d_blksize;
		    }
	    }
}


/***************************************************************************/
/* ask user about a drive
/***************************************************************************/
askabout(drspec)			/* ask user about a drive */
    BYTE *drspec;
{
	REG WORD dr;
	DPH *dph;
	DINFO *dinf;
	BOOLEAN askhash();

	if( *drspec == '*' ) {
	    askstar();			/* get info, apply to all disks */
	    return;
	}
	dr = toupper(*drspec) - 'A';	/* which drive do they want to fix? */
	if( dr<0  ||  dr>=DPHNUM  ||  dphtab[dr]==0  ||  drspec[1] != ':') {
	    USERR("Problem with '%s':\n",drspec);
	    USERR("Please specify an existing drive between 'A:' and 'P:'\n");
	    press_return();
	    return;
	}
	dph = dh[dr];
	dinf = di[dr];
	if( dspdsize(dph,dinf) == -2 ) { /* is there anything to modify? */
	    USERR("All buffers for %c: are fixed within the XIOS module.\n",
		dr+'A');
	    USERR("You can't modify this fixed information in GENSYS.\n");
	    press_return();
	    return;
	}
	if( ASKFOR(dph->dph_dirbcb) ){
	    dinf->d_ndirbs = asknd(1,FALSE);	/* get directory info */
	    dinf->d_pmdirbs = askpm(1,dinf->d_ndirbs);
	}
	if( ASKFOR(dph->dph_datbcb) ){
	    dinf->d_ndatbs = asknd(2,(128==dinf->d_blksize)); /* get data info */
	    dinf->d_pmdatbs = askpm(2,dinf->d_ndatbs);
	}
	if( ASKFOR(dph->dph_hstbl) ){
	    dinf->d_hashing = askhash();
	}
}


/****************************************************************************/
/* askstar: asks buffering questions & applies to all ASKFOR/unanswered drives
/****************************************************************************/
askstar()
{
	REG WORD dr;
	DPH *dph;
	DINFO *dinf;
	WORD andirbs, andirpms, andatbs, andatpms;
	BOOLEAN ahash, askhash();

	andirbs = asknd(1,FALSE);	/* Number of directory buffers? */
	andirpms = askpm(1,andirbs);	/* Process Max Dir Bufs? */
	andatbs = asknd(2,FALSE);	/* Number of data buffers? */
	andatpms = askpm(2,andatbs);	/* Process Max Data Bufs? */
	ahash = askhash();		/* Hashing? */
	for( dr=0; dr<DPHNUM; ++dr )
	    if( dphtab[dr] ) {		/* for all existing drives */
		dph = dh[dr]; dinf = di[dr];
		if( ASKFOR(dph->dph_dirbcb) )
		    if( ASKFOR(dinf->d_ndirbs) ) {
			dinf->d_ndirbs = andirbs;
			dinf->d_pmdirbs = andirpms;
		    }
		if( ASKFOR(dph->dph_datbcb) )
		    if( ASKFOR(dinf->d_ndatbs) ) {
			dinf->d_ndatbs = andatbs;
			dinf->d_pmdatbs = andatpms;
		    }
		if( ASKFOR(dph->dph_hstbl) )
		    dinf->d_hashing = ahash;
	    }
}



/***************************************************************************/
/* asknd: ask the user for the number of dir/data buffers
/***************************************************************************/
    WORD
asknd(type,zero_ok)
    WORD type;				/* 1=dir, 2=data */
    BOOLEAN zero_ok;
{
	REG WORD val, jj;
	BYTE *name;
	BYTE cmds[CMDSLEN];
	DPH *dph;

	if( type==1 ) name="directory"; else name="data";
	FOREVER {
	    printf("Number of %s buffers, or drive to share with ? ",name);
	    gets( cmds );
	    if( cmds[1] != ':' ) {
		val = atoih(cmds);
		if( val > 0  &&  val <= 127 ) break;
		    if(*cmds == '0') {
			if( zero_ok ) break;
			USERR("Number of buffers must be greater than 0\n");
			press_return();
			continue;
		    }
	    } else {
		val = toupper(*cmds) - 'A';	/* which drive do they want? */
		if( val>=0 && val<DPHNUM && dphtab[val]!=0) { /* a drive maybe? */
		    dph = dh[val];
		    if( type==1 ) jj = dph->dph_dirbcb; 
		    else          jj = dph->dph_datbcb;
		    if( ASKFOR(jj) ) {
		    	if( type==1 ) jj = di[val]->d_ndirbs; 
		    	else	      jj = di[val]->d_ndatbs;
		    	if( !SHARED(jj) ) { 
			    val |= SHRMSK;	/* turn on sharing */
			    break;
		        }
		    }
		    USERR("Drive %c: is not available for sharing\n",'A'+val);
		    press_return();
		    continue;
		}
	    }
	    USERR("Please input a number, or an existing drive from 'A:' to 'P:'\n");
	    press_return();
	}
	return val;
}

/************************************/
/* askpm: ask about process maximum
/************************************/
    WORD
askpm(type,pmmax)
    WORD type;
    WORD pmmax;
{
	REG WORD val;
	BYTE *cmds[CMDSLEN];
	BYTE *name;

	switch(type) { case 1: name="directory"; break; case 2: name="data"; }
	if( SHARED(pmmax) || pmmax<=0 )
	    return 0;
	FOREVER {			/* get input from user */
	    printf("Maximum %s buffers per process [%x]? ",name,pmmax);
	    gets( cmds );
	    if( *cmds==NULL )
		 val = pmmax;		/* default value */
	    else val = atoih(cmds);
	    if( val > 0  &&  val <= pmmax ) break;
	    USERR("Maximum must be > zero and <= %x\n",pmmax);
	    press_return();
	}
	return val;
}

/*****************************************/
/* askhash: ask user if they'd like to hash
/*****************************************/
    BOOLEAN
askhash()
{
	REG WORD val;
	BYTE *cmds[CMDSLEN], *cp;

	FOREVER {
	    printf("Hashing [yes] ? ");
	    gets( cmds );
	    if( *cmds == NULL )
		return TRUE;
	    for( cp=cmds; *cp; ++cp ) {
		if( *cp==' '  ||  *cp=='\t' ) {
		    *cp = NULL;
		    break;
		}
		if( isupper(*cp) )
		    *cp = tolower(*cp);
	    }
	    if( (val=whichof(cmds,",yes,true,on,hashing,no,false,off")) > 0 )
		break;
	    USERR("Please answer 'hashing', 'yes', or 'no'.\n");
	    press_return();
	}
	return val<=4;			/* TRUE iff answer was one of 1st four */
}



/************************************************************************/
/* EXPORT
/* xiosvalid: validates that all the user-requestable info has been filled in
/************************************************************************/
    BOOLEAN
xiosvalid()
{
	REG WORD ii;
	REG WORD ndph;

	ndph = 0;
	for( ii=0; ii<DPHNUM; ++ii )
	    if( dphtab[ii]!=0 ) {
		if( dspdsize(dh[ii],di[ii]) == -1 )
		    return FALSE;
		++ndph;
	    }
	if( ndph==0 ) {
	    USERR("no dph information in xios header...");
	    exit(1);
	}
	return TRUE;
}



/***************************************************************************/
/* EXPORT
/* genxios: generates disk buffers (to be placed after the rest of the
/*	system tables.
/*************************************************************************/
#define OFF_BCBPTR 0xA
#define OFF_DPHHTAB 0x12
ADDR locrsvd_off;			/* local reserved offset, for DIRBUF fix */

    ADDR				/* returns final offset */
genxios(doffset)			/* generate the disk buffers */
    ADDR doffset;			/* offset in data file */
{
	REG WORD ii, jj;		/* counter */
	REG BOOLEAN dm;			/* modified flag */
	REG ADDR lastlink;		/* used for threading lists */
	ADDR locbufs_off;		/* local buffer offset for DIRBUF grouping */
	LONG dmagic;			/* where word needing fix goes into file */
	BCB bcb;			/* place for organizing, calc for fixups */
	BCBH bcbh;
	DPH *dph;
	DINFO *dinf;
	BOOLEAN xiosvalid();

	if( !xiosvalid() ) CRASH("invalid disk buffer information");

	for( ii=0; ii<DPHNUM; ++ii ) 	/* allocate CHECK SUM VECTORS */
	    if( dh[ii] ) {		/* if there's something to look at */
		dph = dh[ii];		/* convenient ptr */
		dinf = di[ii];
		if( ASKFOR(dph->dph_csv) ){
		    dinf->d_modified = TRUE;
		    if( dinf->d_csvsize == 0 )
			dph->dph_csv=0;
		    else {
			dph->dph_csv = doffset;
			doffset += bufwrite(dinf->d_csvsize,ZBYTE);
		    }
		}
	    }
	for( ii=0; ii<DPHNUM; ++ii ) 	/* allocate ALLOCATION TABLE VECTORS */
	    if( dh[ii] ) {		/* if there's something to look at */
		dph = dh[ii];		/* convenient ptr */
		dinf = di[ii];
		if( ASKFOR(dph->dph_alv) ){
		    dinf->d_modified = TRUE;
		    dph->dph_alv = doffset;
		    doffset += bufwrite(dinf->d_alvsize,ZBYTE);
		}
	    }
	locbufs_off = 0;		/* amount (bytes) of local dir buffer*/
	for( ii=0; ii<DPHNUM; ++ii ) 	/* allocate DIR BCBs & BUFFERS */
	    if( dh[ii] ) {		/* if there's something to look at */
		dph = dh[ii];		/* convenient ptr */
		dinf = di[ii];
		if( ASKFOR(dph->dph_dirbcb) ){	/* DIR BCBs */
		    dinf->d_modified = TRUE;
		    if( ! SHARED(dinf->d_ndirbs) ) { /* take care of sharing later */
			lastlink=0;		     /* linker */
			zfill(&bcb,sizeof(BCB));     /* init to zeroes */
			bcb.bcb_drv = 0xFF;
			for( jj=dinf->d_ndirbs; jj>0; --jj ){
			    bcb.bcb_link = lastlink;
			    /* pts to loc of buffer relative to locrsvd_off */
			    bcb.bcb_bufptr = locbufs_off;
			    locbufs_off += dinf->d_sdirbs; /* increase buf size */
			    /* calc where bcb will be written to, for fixlater */
			    dmagic = lseek(fns,0L,1) + OFF_BCBPTR;
			    fixlater(dmagic,F_ADD,&locrsvd_off);
			    /* finally, write out the bcb info */
			    lastlink = doffset;	     /* point to the bcb */
			    doffset += datwrite(&bcb,sizeof bcb);
			}
			bcbh.bcbh_lr = lastlink; /* fill in the bcb hdr */
			bcbh.bcbh_pm = dinf->d_pmdirbs;
			dph->dph_dirbcb = doffset; /* link to head of list */
			/* write out a buffer control block hdr */
			doffset += datwrite(&bcbh,sizeof bcbh);
		    }
		}
	    }
	for( ii=0; ii<DPHNUM; ++ii ) 	/* allocate DATA BCBs & BUFFERS */
	    if( dh[ii] ) {		/* if there's something to look at */
		dph = dh[ii];		/* convenient ptr */
		dinf = di[ii];
		/* the bufs can go out of the system page, but the bcbs need to be in */
		if( ASKFOR(dph->dph_datbcb) ){	/* DATA BCBs: !!temporary!!  */
		    dinf->d_modified = TRUE;
		    if( ! SHARED(dinf->d_ndatbs) ) { /* do sharing later */
			lastlink=0;		     /* linker */
			zfill(&bcb,sizeof(BCB));     /* init to zeroes */
			bcb.bcb_drv = 0xFF;
			for( jj=dinf->d_ndatbs; jj>0; --jj ){
			    bcb.bcb_link = lastlink;
			    bcb.bcb_bufptr = bufs_seg; /* point into reserved area */
			    bufs_seg += (dinf->d_sdatbs) >>4; /* cvt to pgphs */
			    /* calculate where the bcb.bcb_bufptr will be written in file */
			    dmagic = lseek(fns,0L,1) + OFF_BCBPTR;
			    fixlater(dmagic,F_ADD,&rsvd_seg); /* fix the bufptr */
			    lastlink = doffset;
			    /* write out a buffer control block */
			    doffset += datwrite(&bcb,sizeof bcb);
			}
			bcbh.bcbh_lr = lastlink; /* fill in the bcb hdr */
			bcbh.bcbh_pm = dinf->d_pmdatbs;
			dph->dph_datbcb = doffset; /* link to head of list */
			/* write out a buffer control block hdr */
			doffset += datwrite(&bcbh,sizeof bcbh);
		    }
		}
	    }
	/* now write the local directory buffers */
	locrsvd_off = doffset;	/* address of directory buffer area */
	doffset += bufwrite(locbufs_off,ZBYTE); /* write out the buffers */

	/* dmagic: magic number to add to dphtab[ii] */
	dmagic = OFF_DPHHTAB + fldstart;/* where in file does SYSDAT start */

	for( ii=0; ii<DPHNUM; ++ii ) 	/* allocate HASH TABLES */
	    if( dh[ii] ) {		/* if there's something to look at */
		dph = dh[ii];		/* convenient ptr */
		dinf = di[ii];
		if( ASKFOR(dph->dph_hstbl) ) {	/* HASH TABLE ALLOC */
		    dinf->d_modified = TRUE;
		    if( dinf->d_hashing ) {
			dph->dph_hstbl = bufs_seg; /* point into reserved area */
			bufs_seg += (dinf->d_shash) >>4; /* cvt bytes to pgphs */
			/* remember where the dph_hstbl word will be stored */
			/* so we can adjust bufs_seg by starting loc of rsvd_seg */
			fixlater(dmagic+dphtab[ii],F_ADD,&rsvd_seg);
		    } else dph->dph_hstbl = 0;
		}
	    }				/**** end of processing ****/

	for( ii=0; ii<DPHNUM; ++ii )	/* resolve shared buffers */
	    if( dh[ii] ) {
		if( ASKFOR(dh[ii]->dph_dirbcb) ) {
		    jj = GETSHARED(di[ii]->d_ndirbs);
		    dh[ii]->dph_dirbcb = dh[jj]->dph_dirbcb;
		}
		if( ASKFOR(dh[ii]->dph_datbcb) ) {
		    jj = GETSHARED(di[ii]->d_ndatbs);
		    dh[ii]->dph_datbcb = dh[jj]->dph_datbcb;
		}
	    }				/* end resolve shared buffers */

	return doffset;
}


/***************************************************************************/
/* EXPORT
/* fixupxios_info: does the fixup for the dph info in the xios
/*************************************************************************/
fixupxios_info()		/* call from fixups */
{
	REG WORD ii;			/* counter */
	REG LONG dmagic;		/* addr var */

	for( ii=0; ii<DPHNUM; ++ii )
	  if( dh[ii] )			/* if there's something to look at */
	    if( di[ii]->d_modified ) {
		dmagic = fldstart + dphtab[ii];
		lseek(fns,dmagic,0); /* where to write to */
		write(fns,dh[ii],sizeof(DPH));	/* what to write */
	    }
}




