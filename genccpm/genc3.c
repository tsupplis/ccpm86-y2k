/*********************************************************************/
/*********************************************************************/
/* genc3.c
/* EXPORTS gentable() - the routine which builds all the tables
/*********************************************************************/
/*********************************************************************/
#ifndef MAINMODULE
#include <genccpm.h>
#endif


MLOCAL UWORD rsp_last;		/* link to previous RSP */
MLOCAL UWORD rsp_seg;		/* where next RSP seg gets written */

#define ALIGN(xx) (((xx)+0xF)&0xFFF0)
#define BYTE_ADJUST(yy) if(yy>0xFF) yy=0xFF


/*************************************************************************/
    UWORD				/* return new offset */
segwrite(nbs)				/* pad to paragraph boundary */
    UWORD nbs;				/* old offset */
{
	REG UWORD nps;

	if( nbs & 0xF ) {		/* does it need padding? */
		nps = 0x10 - (nbs & 0xF);	/* number pad bytes needed */
		write(fns,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",nps);
		return (nbs+nps);
	} else	return nbs;
}

    UWORD				/* return num paragraphs */
padwrite(nbs)				/* pad to paragraph boundary */
    UWORD nbs;				/* num bytes needing padding */
{
	return segwrite(nbs) >> 4;		/* convert to pgphs */
}

    UWORD
datwrite(from,size)
    BYTE *from;
    WORD size;
{
	if( write(fns,from,size) != size ) {
	    fprintf(stderr,"ERROR: Write to new SYS file failed\n");
	    fprintf(stderr,"Out of room on disk?\n");
	    exit(1);
	}
	return size;
}

/**************************************************************************/
MLOCAL LONG oldladdr=0;		/* save previous address for comparison */
#define BIGWORD 0x0000ffffL
chkaddr(uaddr)			/* check for SYSDAT overflow */
    UWORD uaddr;
{
	LONG laddr;

	laddr = BIGWORD & uaddr;
	if( laddr < oldladdr ) {
	    USERR("ERROR - System Data Area has grown too large\n");
	    USERR("Try reducing the size of some of the tables\n");
	    exit(1);
	}
	oldladdr = laddr;	/* save for wrap around compare */
}
	/* this is a kluge: I did it because I couldn't get the LONG cast */
	/* to work without doing sign extension... whf 1/20/83 */


/**************************************************************************/
/* EXPORT gentables: generates various tables in system data area,
/*	and handles RSPs
/**************************************************************************/
VOID gentables()
{
	LOCAL WORD nmparts;		/* number of memory partitions */
	LOCAL WORD nsat_items;		/* number of SAT items in qmau */
	LOCAL UWORD damds;		/* offset of beginning of tables */
	LOCAL UWORD daqbuf;		/* offset of beginning of qbuf */
	LOCAL LONG daend;		/* sys data offset: end of data area */
	LOCAL UWORD doffset;		/* current offset into data area */
	LOCAL UWORD lastlink;		/* helps link lists together */
	LOCAL WORD count;		/* counts num list items being generated */
	LOCAL UWORD ucount;		/* unsigned counter */
	LOCAL MLIST *mn;		/* for trapsing down memroot */
	LOCAL WORD nump;		/* local number partitions */
	LOCAL UWORD saddr;		/* local starting addr */
	LOCAL WORD length;		/* local partition length */
	BYTE buf[SECSIZ];		/* an area for writing */
	LOCAL LONG dmagic;		/* fixup location for xt_alloc info */
	UWORD genmfl();			/* gen a memory free list */
	UWORD genxios();		/* go gen the xios disk buffers */
	DLIST *rs;			/* ptr to rsp list */
	MD md;				/* sample memory descriptor */
	PD pd;				/*   "    process descriptor */
	QCB qcb;			/*   "    queue control block */
	CCB ccb;			/*   "    channel control block */
	LOCK lock;			/*   "    lock list item */
	FLAG flg;			/*   "    flag item */
	SATITEM sat;			/*   "    sub allocation table item */

	if(verbose) printf("Generating tables\n");
	FCHECK(fns);			/* File error check */

	/* pre-calculate certain vars */
	bufs_seg = 0;			/* total buf segs allocated in rsvd_seg */
	if( (nmparts=cntmlist(memroot)) <= 0 ) 
		CRASH("invalid memory list, aborting\n");
	totpds = xtrapds + nmparts;	/* 1 pd per memory partition, plus xtras */
	BYTE_ADJUST(totpds);
	totmds = (3*totpds) + nmparts + (nmparts>>1); /* 3 mds per process, plus */
					/* 1 per partition, plus extra for */
					/* memory allocator worst case */
	BYTE_ADJUST(totmds);
	nsat_items = (1+ (1+2*totqcbs)); /* 1 for header entry plus worst case */

	dltotal = dlsysdat+dlxios;	/* keep track of total data length in pgphs */
	doffset = dltotal << 4;		/* calculate current data offset */
	dsotables = doffset;		/* mark the start of tables */
	chkaddr(doffset);		/* init the function */

					/* MEMORY FREE LIST */
	locmfl = lseek(fns,0L,1);	/* remember where we are writing this */
	doffset = genmfl(doffset);	/* gen the mem free list */

	lastlink = 0;			/* UNUSED MEMORY DESCRIPTOR LIST */
	zfill(&md,sizeof md);		/* init the mem descriptor */
	for( count=totmds-nmparts; count>0; --count ) {	/* generate rest of mds */
	    md.md_link = lastlink;
	    lastlink = doffset;
	    doffset += datwrite(&md,sizeof(md));
	}
	*sd_mdul = lastlink;		/* store ptr to mds unused */

	lastlink = 0;			/* UNUSED PROCESS DESCRIPTOR LIST */
	zfill(&pd,sizeof(pd));
	for( count=totpds; count>0; --count ) {
	    pd.pd_link = lastlink;
	    lastlink = doffset;
	    doffset += datwrite(&pd,sizeof(pd));
	}
	*sd_pul = lastlink;		/* store ptr to pds unused */

	lastlink = 0;			/* UNUSED QUEUE CONTROL BLOCKS */
	zfill(&qcb,sizeof(qcb));
	for( count=totqcbs; count>0; --count ) {
	    qcb.qc_link = lastlink;
	    lastlink = doffset;
	    doffset += datwrite(&qcb,sizeof(qcb));
	}
	*sd_qul = lastlink;		/* store ptr to unused qcb list */

	lastlink = 0;			/* UNUSED LOCKED ITEMS LIST */
	zfill(&lock,sizeof(lock));
	for( count=totopen; count>0; --count ) {
	    lock.lo_link = lastlink;
	    lastlink = doffset;
	    doffset += datwrite(&lock,sizeof(lock));
	}
	*sd_lul = lastlink;		/* store ptr to unused lock items list */

	*sd_ncns = xt.xt_nvcns;         /* transfer info from XIOS to SYSDAT*/
	*sd_ncondev = xt.xt_nccbs;
	*sd_ccb = xt.xt_ccb;
	*sd_nlst = xt.xt_nlcbs;
	*sd_nlstdev = xt.xt_nlcbs;
	*sd_lcb = xt.xt_lcb;
	*sd_nciodev = *sd_ncondev + *sd_nlstdev;
	*sd_tickspsec = xt.xt_ticks_sec;

        *sd_xpcns = xt.xt_xpcns;        /* # physical consoles;FMB */
	*sd_flags = doffset;		/* FLAG TABLE */
	flg.fl_status = -1;
	flg.fl_pd = -1;
	for( count= *sd_nflags; count>0; --count ) {
	    doffset += datwrite(&flg,sizeof(flg));
	}



	doffset = genxios(doffset);	/* go gen the xios buffering info */
	chkaddr(doffset);		/* is table space too big? */

	doffset = segwrite(doffset);	/* Q Sub Alloc Tab HEADER */
	*sd_qmastart = dsstart + (doffset>>4);	/* fill in seg addr */
	daqbuf = doffset + sizeof(sat)*nsat_items;
	zfill(&sat,sizeof(sat));
	sat.sa_start = --nsat_items;	/* decr before write */
	doffset += datwrite(&sat,sizeof(sat));	/* write the header */
					/* Q Sub Alloc Tab ENTRIES */
	sat.sa_start = daqbuf;
	sat.sa_length = qbuflen;	/* byte value */
	*sd_qmalen = qbuflen>>4;	/* pgph value */
	doffset += datwrite(&sat,sizeof(sat));	/* write the first sat entry */

	zfill(&sat,sizeof(sat));
	for( count= --nsat_items; count>0; --count ) {
	    doffset += datwrite(&sat,sizeof(sat));/* write the rest of the sat entries */
	}

	zfill(buf,SECSIZ);		/* Q BUFFER AREA */
	for( ucount=qbuflen; ucount>0; ucount -= length ) {
	    length = (ucount>SECSIZ ? SECSIZ : ucount);
	    doffset += datwrite(buf,length); 
	}
	
	chkaddr(doffset);		/* too big? */
	daend = padwrite(doffset);	/* end Q BUF AREA and TABLES */

	dltables = daend - dltotal;	/* compute length of tables in pgphs */
	dltotal = daend;		/* keep track of total data length */

	/*** now fill in the rsps ***/
	if(verbose) printf("Appending RSPs to system file\n");
	FCHECK(fns);			/* File error check */
	rsp_last = 0;
	rsp_seg = dsstart + daend;
	for( rs=rsps; rs!=NULLPTR; rs=rs->dlnext ) /* follow the rsp list */
	    xfer_rsp(rs->dlname);
	*sd_rspseg = rsp_last;
	dlrsps = rsp_seg - (dsstart + daend);
	dltotal += dlrsps;
	rsvd_seg = rsp_seg;
	length = xt.xt_alloc;		/* how much memory is XIOS requesting */
	xt.xt_alloc = rsvd_seg+bufs_seg; /* this is the place they can use */
	dmagic = fldstart;		/* location of SYSDAT in SYS file */
	dmagic += XTABALLOC;		/* add in where xt_alloc goes to */
	fixlater(dmagic,F_PUT,&(xt.xt_alloc));	/* fix this number later on */
	bufs_seg += length;		/* allocate requested space */
	*sd_endseg = rsvd_seg+bufs_seg;	/* this is where we reserve to */
}


/**************************************************************************/
/* genmfl: writes out a memory free list, assuming file 'fns' is seeking
/*	correctly into file.
/**************************************************************************/
    UWORD
genmfl(doffset)
    UWORD doffset;
{
	REG MLIST *mn;			/* ptr into EDSYS mem list */
	LOCAL WORD nump;
	LOCAL UWORD lastlink, length, saddr; /* counter vars */
	MD md;				/* mem desc struct */

	*sd_mfl = doffset;		/* gen this list forward */
	zfill(&md,sizeof(md));		/* init the data structure */
	for( mn=memroot; mn!=NULLPTR; mn=mn->mlnext ) {
	    nump = (mn->mllast - mn->mlfirst) / mn->mlsize;
	    saddr = mn->mlfirst;	/* use this starting address */
	    length = md.md_length = mn->mlsize; /* size in pgphs */
	    for( ; nump>0; --nump ) {	/* for each partition */
		doffset += sizeof(md);	/* incr by sizeof object */
		lastlink = doffset;	/* record address of where this will go */
		if( nump == 1 )	{	/* on this group's last partition */
		    length = md.md_length = mn->mllast-saddr; /* stick in rmdr */
		    if( mn->mlnext == NULLPTR ) /* on the very last partition */
			lastlink=0;	/* set end of list */
		}
		md.md_link = lastlink;	/* thread the list */
		md.md_start = saddr;	/* set the starting addr */
		write(fns,&md,sizeof(md));	/* put it into the NEWSYS file */
		saddr += length;	/* next md starts here */
	    }
	}				/* finished with memory partitions */
	return doffset;
}


/**************************************************************************/
    VOID
zfill(addr,num)
    BYTE *addr;
    WORD num;
{
	for( ; num>0; --num )
		*addr++ = '\0';
}


/**************************************************************************/
/**************************************************************************/
#define RSPHDR struct rsp_header
RSPHDR {
	UWORD rs_link;			/* link to other RSPs */
	WORD rs_sdatvar;		/* SYSDAT offset: num copies */
	BYTE rs_ncopies;		/* local: num copies */
	BYTE rs_space[11];		/* fill up rest of header */
};

RSPHDR *rhdr;				/* RSP's header */
PD *rpd;				/* RSP's process descriptor area */
UDA *ruda;				/* RSP's uda area */
#define RHDRSIZE (sizeof(RSPHDR)+sizeof(PD)+sizeof(UDA))
#define RHDRPGPHS (RHDRSIZE>>4)
MLOCAL BYTE rh[RHDRSIZE];		/* where to read the RSP header */
MLOCAL BYTE rspnbuf[9];			/* room for copy of pd name */
MLOCAL BYTE *rspname;			/* name of RSP */


/**************************************************************************/
xfer_rsp(rspnm)
    BYTE *rspnm;			/* name of rsp */
{
	LOCAL WORD fr;			/* rsp file descriptor */
	LOCAL WORD ncs;			/* number of copies */

	rhdr = rh;			/* init these vars */
	rpd = rh+sizeof(*rhdr);		/* points to process descriptor */
	ruda = rh+sizeof(*rhdr)+sizeof(*rpd);	/* points to user data area */
	if( (fr=BROPEN(rspnm)) < 0 ) {
	    printf("can't open RSP %s\n",rspnm);
	    exit(1);
	}
	if( grpseek(fr,GTYPDATA) > 0L){	/* is there a data group? */
	    ncs = grab_hdr(fr);
	    if( rpd->pd_mem == 0 )
		 xfer_separate_rsp(fr,ncs);
	    else xfer_pure_rsp(fr,ncs);
	} else {
	    grpseek(fr,GTYPCODE);
	    ncs = grab_hdr(fr);
	    if( rpd->pd_mem == 0 )
		 xfer_mixed_rsp(fr,ncs);
	    else {
		USERR("RSP %s has non zero MEM field and no data group\n",
			rspnm);
	    }
	}
	close(fr);
}


/**************************************************************************/
    WORD
grab_hdr(fr)
    WORD fr;
{
	REG WORD rv;			/* return val */
	EXTERN BYTE sdat_area[];

	rspname = NULLPTR;		/* blank out this name */
	read(fr,rh,RHDRSIZE);
	strncpy(rspnbuf,rpd->pd_name,8);/* store this in case we need it */
	if( rhdr->rs_sdatvar != 0 ) {
		rspname = rspnbuf;
		rv = sdat_area[rhdr->rs_sdatvar] - 1;
	} else {
	    if( rhdr->rs_ncopies > 0 )
		rspname = rspnbuf;
	    rv = rhdr->rs_ncopies;
	}
	return rv;
}


/**************************************************************************/
xfer_separate_rsp(fr,ncs)		/* (many) data and (many) code segs */
    WORD fr;				/* file desc of RSP file */
    WORD ncs;				/* number of copies */
{
	WORD ii;			/* counter */

	for( ii=0; ii<=ncs; ++ii) {
	    ruda->ud_dsinit = ruda->ud_esinit = ruda->ud_ssinit = rsp_seg;
	    ruda->ud_csinit = rsp_seg + (grpseek(fr,GTYPDATA)>>4);
	    rhdr->rs_ncopies = ii;
	    if( rspname ) 		/* do we need to insert a modfied name? */
		putname(rpd->pd_name,rspname,ii);
	    rhdr->rs_link = rsp_last;	/* link together the list */
	    rsp_last = rsp_seg;		/* remember this for next time */
	    write(fns,rh,RHDRSIZE);	/* write out the RSP header */
	    rsp_seg += RHDRPGPHS + xferpart_grp(fr,GTYPDATA,RHDRPGPHS);
	    rsp_seg += xferpart_grp(fr,GTYPCODE,0);
	}
}


/**************************************************************************/
xfer_pure_rsp(fr,ncs)			/* (many) data segs and one code segs */
    WORD fr;				/* file desc of RSP file */
    WORD ncs;				/* number of copies */
{
	WORD ii;			/* counter */

	ruda->ud_csinit = rsp_seg + (1+ncs) * (grpseek(fr,GTYPDATA) >> 4);
	for( ii=0; ii<=ncs; ++ii) {
	    ruda->ud_dsinit = ruda->ud_esinit = ruda->ud_ssinit = rsp_seg;
	    rhdr->rs_ncopies = ii;
	    if( rspname ) 		/* do we need to insert a modfied name? */
		putname(rpd->pd_name,rspname,ii);
	    rpd->pd_mem = 8; 		/* magic ptr to md in rsphdr */
	    rhdr->rs_link = rsp_last;	/* link together the list */
	    rsp_last = rsp_seg;		/* remember this for next time */
	    write(fns,rh,RHDRSIZE);	/* write out the RSP header */
	    rsp_seg += RHDRPGPHS + xferpart_grp(fr,GTYPDATA,RHDRPGPHS);
	}
	rsp_seg += xferpart_grp(fr,GTYPCODE,0);
}


/**************************************************************************/
xfer_mixed_rsp(fr,ncs)			/* (many) mixed code/data segs */
    WORD fr;				/* file desc of RSP file */
    WORD ncs;				/* number of copies */
{
	WORD ii;			/* counter */

	for( ii=0; ii<=ncs; ++ii) {
	    ruda->ud_csinit = ruda->ud_dsinit = ruda->ud_esinit = 
		ruda->ud_ssinit = rsp_seg;
	    rhdr->rs_ncopies = ii;
	    if( rspname ) 		/* do we need to insert a modfied name? */
		putname(rpd->pd_name,rspname,ii);
	    rhdr->rs_link = rsp_last;	/* link together the list */
	    rsp_last = rsp_seg;		/* remember this for next time */
	    write(fns,rh,RHDRSIZE);	/* write out the RSP header */
	    rsp_seg += RHDRPGPHS + xferpart_grp(fr,GTYPCODE,RHDRPGPHS);
	}
}


/**************************************************************************/
putname(to,from,num)
    BYTE *to;
    BYTE *from;
    WORD num;
{
	BYTE buf[15];
	BYTE *bp;
	WORD jj;

	for( bp=buf, jj=0; *from  &&  *from != ' '  &&  jj<8; ++jj )
		*bp++ = *from++;
	if( jj>6 )
		bp = buf+7;
	sprintf(bp,"%x      ",num);
	for( bp=buf, jj=0; jj<8; jj++ )
		*to++ = *bp++;
}


/************************************************************************/
/* dolbl: modify operating system version label */
/************************************************************************/
#define MAXLBL 83

    BYTE *
dolbl(txt,lbtitle)
    BYTE *txt;				/* cmd val */
    BYTE *lbtitle;
{
	DLIST *sl, *psl;		/* ptrs to system labels */
	BYTE mb[MAXLBL];		/* max size of each line */
	EXTERN BYTE *clearit;

	sl = psl = syslbls;		/* init ptrs */
	printf("%s\n%s\n",clearit,lbtitle);
	printf("Current message is:\n");
	if( syslbls==NULLPTR )
		printf("<null>\n");
	else for( ; sl != NULLPTR; sl = (psl=sl)->dlnext ) 
		printf("%s\n",sl->dlname);
	printf("\nAdd lines to message.  Terminate by entering only RETURN:\n");
	FOREVER {
	    if( gets(mb) != mb  ||  *mb==NULL )
		break;
	    sl = (DLIST *)malloc(sizeof(*sl));	/* alloc a list item */
	    sl->dlname = malloc(sizeof(mb));
	    strcpy(sl->dlname,mb);	/* copy message to new space */
	    sl->dlnext = NULLPTR;
	    if( psl==NULLPTR )		/* new list? */
		syslbls = psl = sl;	/* this will be the first line */
	    else {
		psl->dlnext = sl; 
		psl = psl->dlnext; 
	    }
	}
	return NULLPTR;
}

/************************************************************************/
/* writelbl: writes the OS label to the NEWSYS file, returning the length
/*	of the label (in pgphs)
/************************************************************************/
    WORD
_wrlbl(str)				/* subroutine to writelbl */
    BYTE *str;
{
	REG WORD slen;

	slen=strlen(str);		/* length of string to be written */
	write(fns,str,slen);		/* write string to file NEWSYS */
	write(fns,"\015\012",2);	/* cr,lf */
	return slen+2;
}

    WORD
writelbl()				/* returns num pgphs written */
{
	REG WORD cc;			/* count of chars output */
	DLIST *sl;			/* list ptr */

	cc = 0;

	cc += _wrlbl("");		/* crlf */
	cc += _wrlbl(version);		/* write out the version label */
	cc += _wrlbl(copyright);	/* followed by copyright notice */
	for( sl=syslbls; sl!=NULLPTR; sl=sl->dlnext )
	    cc += _wrlbl(sl->dlname);	/* followed by all lines in OS label */
	cc += _wrlbl("$");		/* terminate message for printstr fn */
	return padwrite(cc);		/* writes enough zeroes to pad to pgph bdry */
}


/***************************************************************************/
/* fixup routines: remembers and posthumously patches the SYS file
/***************************************************************************/

FIX *fixroot = NULLPTRI;		/* list of fixes */

fixlater(fa,ft,fp)			/* records the fix in the list */
    LONG fa;				/* addr within NEWSYS to fix */
    WORD ft;				/* type of fix: F_ADD or F_PUT */
    WORD *fp;				/* ptr to val used in fixing */
{
	FIX *fx;

	fx = (FIX *)malloc(sizeof *fx);	/* alloc a list item */
	fx->f_addr = fa;
	fx->f_type = ft;
	fx->f_ptr = fp;
	fx->f_next = fixroot;		/* insert at head of list */
	fixroot = fx;
}

fixfile()				/* does the fixes in the file */
{
	FIX *fx;
	WORD fval;			/* the value to stuff */
	LONG lastval;

	lastval=lseek(fns,0L,2);       /* end of file */

	for( fx=fixroot; fx!=NULLPTR; fx = fx->f_next ) {
	    if(fx->f_addr > lastval) { /* oops! past eof */
		printf("fixup addressing internal error: %X %x %x\n",fx->f_addr,fx->f_type,fx->f_ptr);
		continue;
	    }
	    if( lseek(fns,fx->f_addr,0) < 0L )
		CRASH("fixup seek failure\n");
	    if( fx->f_type == F_ADD ) {	/* F_ADD the value to what's there */
		read(fns,&fval,2);
		fval += *(fx->f_ptr);
		lseek(fns,-2L,1);	/* backup to write */
	    } else			/* must be a F_PUT */
		fval = *(fx->f_ptr);
	    write(fns,&fval,2);
	}
}




