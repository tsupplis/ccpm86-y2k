/*************************************************************************/
/*************************************************************************/
/* genccpm.h  -  the include module(s) for genccpm.c
/*************************************************************************/
/*************************************************************************/

/***** include files *****/
#include <portab.h>
#include <stdio.h>
#include <ctype.h>
#include <cpm.h>
/*************************/

/* New types for GENCCPM: */
#define ADDR UWORD			/* offset in system image */
#define SEG  UWORD			/* segment in system image */
#define FPTR LONG			/* pointer into file */

/*******************************************/
/* compiler differences                    */
/* configured for Digital Research's C     */
#define BROPEN(fn) openb(fn,0)		/* binary open for read */
#define BWCREAT(fn) creatb(fn,1)	/* binary creat for write */
LONG lseek();
BYTE *malloc();
BYTE *gets();
#define NULLPTRI 0			/* NULLPTR initializer */

/* compile time options: assume the "-dXXX" flag is used, where XXX is: */

/* #define CCPM				/* this is a CCP/M EDSYS */
#define CMPM				/* this is an CMP/M EDSYS */

/**************************************************************************/
#ifndef CCPM
#ifndef CMPM
	ERROR: you must specify one of 'CMPM' or 'CCPM'
#endif
#endif
#ifdef CCPM
#ifdef CMPM
	ERROR: these compile options are mutually exclusive: only 1 allowed!
#endif
#endif



/**************************************************************************/
/**************************************************************************/
/* menu.h : include with progs using menu rtns 
/**************************************************************************/
/**************************************************************************/

#define MENU struct menuitem
MENU {
	WORD mutype;		/* type of menu item */
	BYTE *muiptr;		/* ptr to information for menu item */
	BYTE *muname;		/* name of menu item */
	BYTE *mudesc;		/* description of menu item */
	MENU *munext;		/* ptr to next menu item */
};

#define MBOOL 1
#define MBYTE 2
#define MWORD 3
#define MTEXT 4
#define MPROC 5
#define MDRIV 6



/**************************************************************************/
/**************************************************************************/
/* cpm.h - support for cpm types of calls 
/**************************************************************************/
/**************************************************************************/
#define SECSIZ 128
#define BUFSIZ 512	/* should be in stdio.h */

/**************************************************************************/
/* directory search support */
#define DLIST struct dliststr		/* directory list structure */
DLIST {
	BYTE *dlname;			/* file name */
	DLIST *dlnext;			/* next item in list */
};

/**************************************************************************/
/* .CMD header record Group structure */
#define GROUP struct cmdgrp
GROUP {					/* what a Cmd Hdr group looks like */
	BYTE gtype;			/* type of group (see below) */
	UWORD glen;			/* length in paragraphs of group */
	UWORD gabase;			/* pgph addr for non-relocatable grp */
	UWORD gmin;			/* min pgphs to alloc to group */
	UWORD gmax;			/* max pgphs to alloc to group */
};
#define GTYPCODE 1			/* Code Group */
#define GTYPDATA 2			/* Data Group */



/**************************************************************************/
/**************************************************************************/
/* mpm.h - data structures peculiar to MP/M & Concurrent 
/**************************************************************************/
/**************************************************************************/
#define MINKMEM 80		/* minimum memory allocation in paragraphs */

#define XIOSTAB struct xios_table
XIOSTAB {
	UBYTE xt_tick;
	UBYTE xt_ticks_sec;
	UBYTE xt_door;
	UBYTE xt_rsvd;
        UBYTE xt_xpcns;            /* # physical consoles; FMB */
	UBYTE xt_nvcns;
	UBYTE xt_nccbs;
	UBYTE xt_nlcbs;
	UWORD xt_ccb;
	UWORD xt_lcb;
	UWORD xt_dphtab[16];
	UWORD xt_alloc;
};
#define XTABLEN (sizeof(XIOSTAB))
#define XTABLOC   (0X0c0cL)
#define XTABALLOC (0X0c38L)

#define MD struct mem_descriptor
MD {
	UWORD md_link;
	UWORD md_start;
	UWORD md_length;
	UWORD md_plist;
	UWORD md_unused;
};

#define PD struct proc_descriptor
PD {
	UWORD pd_link;
	UWORD pd_thread;
	UBYTE pd_stat;
	UBYTE pd_prior;
	UWORD pd_flag;
	BYTE pd_name[8];
	UWORD pd_uda;
	UBYTE pd_dsk;
	UBYTE pd_user;
	UBYTE pd_ldsk;
	UBYTE pd_luser;
	UWORD pd_mem;
	BYTE pd_restofit[24];
};

#define LOCK struct lock_item
LOCK {
	UWORD lo_link;
	UWORD lo_restofit[4];
};

#define UDA struct uda_item
UDA {
	BYTE ud_1space[0x50];
	UWORD ud_csinit;
	UWORD ud_dsinit;
	UWORD ud_esinit;
	UWORD ud_ssinit;
	BYTE ud_2space[0xA8];		/* uda is 0x100 bytes long */
};

#define QCB struct qcb_descriptor
QCB {
	UWORD qc_link;
	BYTE qc_restofit[26];
};

#define FLAG struct flag_item
FLAG {
	UBYTE fl_status;
	UWORD fl_pd;
};

#define CCB struct ccb_item
CCB {
	UWORD cc_attach;
	UWORD cc_queue;
	UBYTE cc_flag;
	UBYTE cc_startcol;
	UBYTE cc_column;
	UBYTE cc_nchar;
	UBYTE cc_mimic;
	UBYTE cc_msource;
	UBYTE cc_type;
	UBYTE cc_xdev;
};

#define SATITEM struct sat_item
SATITEM {
	UWORD sa_start;
	UWORD sa_length;
	UBYTE sa_nall;
};




/**************************************************************************/
/**************************************************************************/
/* here's the body of edsys.h
/**************************************************************************/
/**************************************************************************/
#ifdef CCPM
#define PROGNAME "GENCCPM"
#define VERLABEL "Concurrent CP/M-86 2.1"
#define VERSION	0x1421
#else
#define PROGNAME "GENCCPM"
#define VERLABEL "Concurrent CP/M-86 3.1"
#define VERSION 0x1431
#endif
#define SYSFILE "CCPM.SYS"
#define NEWSYS	"CCPM.$Y$"
#define OLDSYS	"CCPM.OLD"
#define MODEXT	".CON"
#define SUPMOD	"SUP.CON"
#define RTMMOD	"RTM.CON"
#define MEMMOD	"MEM.CON"
#define CIOMOD	"CIO.CON"
#define BDOSMOD	"BDOS.CON"
#define NETMOD	"NET86.CON"
#define SYSMOD	"SYSDAT.CON"
#define XIOSMOD	"XIOS.CON"




/**************************************************************************/
#define USERR printf			/* how to handle user errors */
#define CRASH(fn) {fprintf(stderr,fn);exit(1);}
#define FCHECK(sp) /* disable */
	/* if(ferror(sp)){printf("Error on output: out of space?");exit(1);} */
#define DBGPRT printf
#define CMDSLEN 80
#define PROMPT "Changes? "


/* memory list definition */
#define MLIST struct memorylist
MLIST {
	UWORD mlfirst;			/* addr of 1st pgph */
	UWORD mllast;			/* addr of last pgph */
	UWORD mlsize;			/* partition size in K bytes */
	MLIST *mlnext;			/* ptr to next item in list */
};

/* ENTRY point structure decls */
#define ENTRY struct entrypoint
ENTRY {
	UWORD ep_eoff;			/* entry offset */
	UWORD ep_eseg;			/* entry segment */
	UWORD ep_ioff;			/* init offset */
	UWORD ep_iseg;			/* init segment */
};

/* FIXup structure decls */
#define FIX struct fixitem
FIX {
	LONG f_addr;			/* offset in file */
	WORD f_type;			/* either F_ADD or F_PUT */
	WORD *f_ptr;			/* ptr to value to add or put */
	FIX *f_next;			/* next fixup item */
};
#define F_ADD 1				/* add value of value to word in file*/
#define F_PUT 2				/* put value ... */


/**************************************************************************/
/* Program Globals: 
/*	Keep all the globals defined in one file, for ease of access
/*	Definitions first, then declarations
/**************************************************************************/

/******** definitions for the MAINMODULE **********/
#ifdef MAINMODULE

/**************************************************************************/
/* Edsys Program Globals
/**************************************************************************/
    GLOBAL BYTE *copyright = "Copyright (C) 1983, Digital Research";
    GLOBAL BYTE *version = VERLABEL;
    GLOBAL BYTE *clearit;		/* Home cursor & erase screen */
    GLOBAL BOOLEAN verbose=TRUE;	/* flag to tell EDSYS to be wordy */
    GLOBAL BOOLEAN doit=FALSE;		/* flag: tell EDSYS to do the rest of it */
    GLOBAL WORD fns = -1;		/* File descriptor for New System file */
    GLOBAL BOOLEAN dogensys = FALSE;	/* Gen System from scratch? */
    GLOBAL BOOLEAN doclean = FALSE;	/* Clean SYSFILE first from destination drive? */
    GLOBAL BOOLEAN donew_xios = FALSE;	/* edit in a NEW XIOS? */
    GLOBAL BOOLEAN pure_xios = FALSE;	/* XIOS groups are pure (separate) code & data? */
    GLOBAL BYTE destdrv;		/* where the SYSFILE goes */

/**************************************************************************/
/* Sysfile Globals
/**************************************************************************/
    GLOBAL WORD totmds;			/* total number of memory descriptors */
    GLOBAL WORD totpds;			/* total number of process descriptors */
    GLOBAL WORD xtrapds;		/* number of pds wanted by user */
    GLOBAL WORD totqcbs;		/* tot num q control blocks */
    GLOBAL UWORD qbuflen;		/* length in bytes of q buf area */
    GLOBAL WORD totopen;		/* total open files and locked records */

    GLOBAL DLIST *rsps=NULLPTRI;	/* list of RSP file names */
    GLOBAL DLIST *syslbls=NULLPTRI;	/* lines of OS label */
    GLOBAL MLIST *memroot=NULLPTRI;	/* memory partitions */
    GLOBAL XIOSTAB xt;			/* xios info table */

#define SDLEN 0x210
    GLOBAL BYTE sdat_area[SDLEN];	/* where to store the SYSDAT info */
    GLOBAL BYTE cmdhdr[SECSIZ];		/* command header for NEWSYS */

/* module length information: lengths are in paragraphs */
    GLOBAL UWORD clsup;			/* code length: supervisor */
    GLOBAL UWORD clos_label;		/* code length: OS label */
    GLOBAL UWORD clrtm;			/*  "     "   : real time monitor */
    GLOBAL UWORD clmem;			/*  "     "   : memory manager */
    GLOBAL UWORD clcio;			/*  "     "   : char i/o */
    GLOBAL UWORD clbdos;		/*  "     "   : bdos */
    GLOBAL UWORD clnet;			/*  "     "   : network handler */
    GLOBAL UWORD clxios;		/*  "     "   : xios */
    GLOBAL UWORD cltotal;		/*  "     "    of all modules */
    GLOBAL UWORD dsstart;		/* starting segment of system data */
    GLOBAL UWORD dlsysdat;		/* data length: system data */
    GLOBAL UWORD dlxios;		/* length of xios in data seg */
    GLOBAL UWORD dltables;		/* length of tables */
    GLOBAL UWORD dsotables;		/* starting offset of tables */
    GLOBAL UWORD dlrsps;		/* length of rsps */
    GLOBAL UWORD dltotal;		/* length of data, xios, tables & rsps */
    GLOBAL UWORD rsvd_seg;		/* start of OS reserved (buffer) area (not in load image) */
    GLOBAL UWORD bufs_seg;		/* ptr to allocated buffer at rsvd_seg */

/*** fixup locations ***/
    GLOBAL LONG fldstart;		/* location of SYSDAT start */
    GLOBAL LONG locmfl;			/* location in file of mem free list */

/**************************************************************************/
/* system data variables for MP/M-86 or Concurrent CP/M-86 */
/**************************************************************************/
    GLOBAL ENTRY * sd_supmod;
    GLOBAL ENTRY * sd_rtmmod;
    GLOBAL ENTRY * sd_memmod;
    GLOBAL ENTRY * sd_ciomod;
    GLOBAL ENTRY * sd_bdosmod;
    GLOBAL ENTRY * sd_xiosmod;
    GLOBAL ENTRY * sd_netmod;
    GLOBAL UWORD * sd_mpmseg;
    GLOBAL UWORD * sd_rspseg;
    GLOBAL UWORD * sd_endseg;
    GLOBAL UBYTE * sd_module_map;
    GLOBAL UBYTE * sd_ncns;
    GLOBAL UBYTE * sd_nlst;
    GLOBAL UBYTE * sd_nccb;
    GLOBAL UBYTE * sd_nflags;
    GLOBAL UBYTE * sd_srchdisk;
    GLOBAL UWORD * sd_mmp;
    GLOBAL UBYTE * sd_nslaves;
    GLOBAL UBYTE * sd_dayfile;
    GLOBAL UBYTE * sd_tempdisk;
    GLOBAL UBYTE * sd_tickspsec;
    GLOBAL UWORD * sd_lul;
    GLOBAL UWORD * sd_ccb;
    GLOBAL UWORD * sd_flags;
    GLOBAL UWORD * sd_mdul;
    GLOBAL UBYTE * sd_nxmds;
    GLOBAL UWORD * sd_mfl;
    GLOBAL UBYTE * sd_nmparts;
    GLOBAL UWORD * sd_pul;
    GLOBAL UBYTE * sd_nxpd;
    GLOBAL UWORD * sd_qul;
    GLOBAL UBYTE * sd_nqds;
    GLOBAL UWORD * sd_qmau;
    GLOBAL UWORD * sd_qmastart;		/* segment start of q buffer */
    GLOBAL UWORD * sd_qmalen;		/* length of q buffer */
    GLOBAL UWORD * sd_verptr;		/* pts to SUP segment ver string */
    GLOBAL UWORD * sd_vn;		/* F.12 version num */
    GLOBAL UWORD * sd_mpmvn;		/* F.163 version num */
    GLOBAL UBYTE * sd_ncondev;
    GLOBAL UBYTE * sd_nlstdev;
    GLOBAL UBYTE * sd_nciodev;
    GLOBAL UWORD * sd_lcb;
    GLOBAL UBYTE * sd_plock_max;
    GLOBAL UBYTE * sd_popen_max;
    GLOBAL UBYTE * sd_cmode;
    GLOBAL UBYTE * sd_xpcns;

#else

/****** declarations for non MAINMODULE *******/

/**************************************************************************/
/* Program Globals
/**************************************************************************/
    EXTERN BYTE *copyright;
    EXTERN BYTE *version;
    EXTERN BYTE *clearit;		/* Home cursor & erase screen */
    EXTERN BOOLEAN verbose;		/* flag to tell EDSYS to be wordy */
    EXTERN BOOLEAN doit;		/* flag: tell EDSYS to do the rest of it */
    EXTERN WORD fns;			/* File descriptor for New System file */
    EXTERN BOOLEAN dogensys;		/* Gen System from scratch? */
    EXTERN BOOLEAN doclean;		/* Clean SYSFILE first from destination drive? */
    EXTERN BOOLEAN donew_xios;		/* edit in a NEW XIOS? */
    EXTERN BOOLEAN pure_xios;		/* XIOS groups are pure (separate) code & data? */
    EXTERN BYTE destdrv;		/* where the SYSFILE goes */

/**************************************************************************/
/* Sysfile Globals
/**************************************************************************/
    EXTERN WORD totmds;			/* total number of memory descriptors */
    EXTERN WORD totpds;			/* total number of process descriptors */
    EXTERN WORD xtrapds;		/* number of pds wanted by user */
    EXTERN WORD totqcbs;		/* tot num q control blocks */
    EXTERN UWORD qbuflen;		/* length in bytes of q buf area */
    EXTERN WORD totopen;		/* total open files & locked records */

    EXTERN DLIST *rsps;			/* list of RSP file names */
    EXTERN DLIST *syslbls;		/* lines of OS label */
    EXTERN MLIST *memroot;		/* memory partitions */
    EXTERN XIOSTAB xt;			/* xios info table */

    EXTERN BYTE sdat_area[];		/* where to store the SYSDAT info */
    EXTERN BYTE *cmdhdr;		/* command header for NEWSYS */

    EXTERN UWORD clsup;			/* code length: supervisor */
    EXTERN UWORD clos_label;		/* code length: OS label */
    EXTERN UWORD clrtm;			/*  "     "   : real time monitor */
    EXTERN UWORD clmem;			/*  "     "   : memory manager */
    EXTERN UWORD clcio;			/*  "     "   : char i/o */
    EXTERN UWORD clbdos;		/*  "     "   : bdos */
    EXTERN UWORD clnet;			/*  "     "   : network handler */
    EXTERN UWORD clxios;		/*  "     "   : xios */
    EXTERN UWORD cltotal;		/*  "     "    of all modules */
    EXTERN UWORD dsstart;		/* starting segment of system data */
    EXTERN UWORD dlsysdat;		/* data length: system data */
    EXTERN UWORD dlxios;		/* length of xios in data seg */
    EXTERN UWORD dltables;		/* length of tables */
    EXTERN UWORD dsotables;		/* starting offset of tables */
    EXTERN UWORD dlrsps;		/* length of rsps */
    EXTERN UWORD dltotal;		/* length of data, xios, tables & rsps */
    EXTERN UWORD rsvd_seg;		/* start of OS reserved (buffer) area (not in load image) */
    EXTERN UWORD bufs_seg;		/* ptr to allocated buffer at rsvd_seg */

/*** fixup locations ***/
    EXTERN LONG fldstart;		/* location of SYSDAT start */
    EXTERN LONG locmfl;			/* location in file of mem free list */

/***********************************************************/
/* system data variables for MP/M-86 or Concurrent CP/M-86 */
    EXTERN ENTRY * sd_supmod;
    EXTERN ENTRY * sd_rtmmod;
    EXTERN ENTRY * sd_memmod;
    EXTERN ENTRY * sd_ciomod;
    EXTERN ENTRY * sd_bdosmod;
    EXTERN ENTRY * sd_xiosmod;
    EXTERN ENTRY * sd_netmod;
    EXTERN UWORD * sd_mpmseg;
    EXTERN UWORD * sd_rspseg;
    EXTERN UWORD * sd_endseg;
    EXTERN UBYTE * sd_module_map;
    EXTERN UBYTE * sd_ncns;
    EXTERN UBYTE * sd_nlst;
    EXTERN UBYTE * sd_nccb;
    EXTERN UBYTE * sd_nflags;
    EXTERN UBYTE * sd_srchdisk;
    EXTERN UWORD * sd_mmp;
    EXTERN UBYTE * sd_nslaves;
    EXTERN UBYTE * sd_dayfile;
    EXTERN UBYTE * sd_tempdisk;
    EXTERN UBYTE * sd_tickspsec;
    EXTERN UWORD * sd_lul;
    EXTERN UWORD * sd_ccb;
    EXTERN UWORD * sd_flags;
    EXTERN UWORD * sd_mdul;
    EXTERN UBYTE * sd_nxmds;
    EXTERN UWORD * sd_mfl;
    EXTERN UBYTE * sd_nmparts;
    EXTERN UWORD * sd_pul;
    EXTERN UBYTE * sd_nxpd;
    EXTERN UWORD * sd_qul;
    EXTERN UBYTE * sd_nqds;
    EXTERN UWORD * sd_qmau;
    EXTERN UWORD * sd_qmastart;		/* segment start of q buffer */
    EXTERN UWORD * sd_qmalen;		/* length of q buffer */
    EXTERN UWORD * sd_verptr;		/* pts to SUP segment ver string */
    EXTERN UWORD * sd_vn;		/* F.12 version num */
    EXTERN UWORD * sd_mpmvn;		/* F.163 version num */
    EXTERN UBYTE * sd_ncondev;
    EXTERN UBYTE * sd_nlstdev;
    EXTERN UBYTE * sd_nciodev;
    EXTERN UWORD * sd_lcb;
    EXTERN UBYTE * sd_plock_max;
    EXTERN UBYTE * sd_popen_max;
    EXTERN UBYTE * sd_cmode;
    EXTERN UBYTE * sd_xpcns;

#endif


#define PUTDRV(dd,nn) (sprintf(dd,"%c:%s",'A'+destdrv,nn))
