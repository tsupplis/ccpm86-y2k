
#include	<portab.h>
#include	"cpmfunc.h"

EXTERN	WORD	_EXTERR;		/* Has AX after an O.S. call	*/

#define		DMA_LEN		128
#define		FCB_LEN		36

#define		REC_LEN		23
#define		REC_DRIVE	0
#define		REC_FILE	1
#define		REC_TYPE	9
#define		REC_PASSWORD	12
#define		REC_FIELDS	20
#define		REC_8087	20
#define		REC_SHARED	21
#define		REC_SUSPEND	22
#define		REC_F_8087	0
#define		REC_F_SHARED	1
#define		REC_F_SUSPEND	2


#define		NOT_FOUND	-1
#define		LOW_TO_UP	('a' - 'A')
#define		EXIT_PROGRAM	-1
#define		CONTINUE	1

#define		BDOS_MASK	0x00ff
#define		OS_MASK		0xff00
#define		PASSWORD	0x07ff
#define		RO_ERROR	0x03ff
#define		RO_OR_PASS	0xfe

#define		BDOS_VER	0x0031
#define		OS_TYPE		0x1400

/***	FUNCTIONS THAT RETURN A POINTER    ***/

BYTE	*a_white_char();
BYTE	*check_format();
BYTE	*expand_file_name();
BYTE	*get_token();
BYTE	*search();
BYTE	*short_string();
BYTE	*getspass();

struct	_fcblst	*expfcb();
/**BYTE	*getpass();**/
BYTE	*strncpy();
BYTE	*index();
BYTE	*strncmp();
BYTE	*strcmp();
/*	strlen(); */


/***	ERROR MESSAGES    ***/

BYTE	*err_00_msg  =  "REQUIRES CONCURRENT CP/M-86 3.1$";
BYTE	*err_01_msg  =  "Invalid syntax -- one filespec only";
BYTE	*err_02_msg  =  "Use CMD or blank filetype in CHSET command line";
BYTE	*err_03_msg  =  "Invalid filespec";	/* too long */
BYTE	*err_04_msg  =  "Invalid filespec";	/* can't add .cmd */
BYTE	*err_05_msg  =  "Invalid syntax -- expected a '['";
BYTE	*err_06_msg  =  "Not a valid CHSET field";
BYTE	*err_07_msg  =  "Not a valid CHSET setting";
BYTE	*err_08_msg  =  "Duplicate field";
BYTE	*err_09_msg  =  "Invalid syntax -- expected a '='";
BYTE	*err_10_msg  =  "Invalid syntax -- expected ',' or ']'";
BYTE	*err_11_msg  =  "Wildcards are not allowed when using CHSET to change a setting";
BYTE	*err_12_msg  =  "Invalid filespec";
BYTE	*err_13_msg  =  "File not found";
BYTE	*err_14_msg  =  "Too many directory entries for query";



BYTE	*pass_prompt  =  "  Password? ";
BYTE	*set_to	      =  " set to ";
BYTE	*setting_is   =  " settings are ";
BYTE	*blanks	      =  "          ";		/* 10 blanks 		*/


/***	GLOBAL VARIABLES    ***/
	
BYTE	buff_01[DMA_LEN];	/* Buffer to hold current token		*/
BYTE	*token;			/* Points to buffer			*/

BYTE	buff_02[24];
BYTE	*file_spec;

BYTE	buff_03[FCB_LEN];
BYTE	*fcb;


BYTE	*cmd		=	".CMD" 					;
BYTE	*deli_01	=	"\t\\\" !@#$%^&*()_+{}~:|<>?-=[]`;',./" ;
BYTE	*deli_02	=	"\t =<>,|[]"				;


	struct	_fcblst
	{
		BYTE		fcb[FCB_LEN];
		struct _fcblst	*next_fcb;
	};

	struct  pfcb_str
	{
		BYTE	*filename;
		BYTE	*fcb_adr;
	};

	struct	abcd_str
	{
		WORD	start;
		WORD	end;
	};

	struct	fie1_str
	{
	   	BYTE	*fieldstr;
		WORD	field_nu;
	};

#define		EMPTY		-1	/* Can not be same as option number */
#define		FIE1_START	0
#define 	FIE1_END	2

	struct	fie1_str	fie1_tab[] =
	{
		"8*087",0,
		"SH*ARED",1,
		"SU*SPEND",2
	};

	struct	fie1_str	opt1_tab[] =
	{
		"ON*",0,
		"OF*F",1,		
		"OP*TIONAL",2
	};

	struct	abcd_str	xref_tab[] =
	{
		0,2,		/* on, off, optional			*/
		0,1,		/* on, off				*/
		0,1		/* on, off				*/
	};

	


VOID	main()

{
	BYTE				*dma;
	WORD				dma_len;


	if ( (ok_ver(BDOS_VER,OS_TYPE) == FALSE) )
	{			/* Not the right system			*/
	    c_writestr(err_00_msg);		/* delimited with $	*/
	    p_termcpm();			/* Terminate		*/
	}
	token = &buff_01[0];
	file_spec = &buff_02[0];
	fcb = &buff_03[0];
	f_errmode( 0xfe );	/* Sets BDOS errors to verbose 		*/
	c_delimit( NULL );	/* Sets output delimiter to NULL	*/
	dma = f_dmaget();	/* Get DMA ptr				*/
	dma_len = *dma++;	/* Get length, move ptr to str portion	*/
	*(dma+dma_len) = NULL;
	upper_case(dma);	/* Change all lowercase to upper	*/
	if ( (dma_len == 0) )
	{			/* Have no tail				*/
	    display_help();
	}
      else
	{
	    dma = a_white_char(dma);	/* DMA now points to a white char */
	    if ( (*dma == '[') )
	    {			/* Give help on [...			*/
		display_help();
	    }
	  else
	    {
		set_or_show(dma);	/* Does return on success 	*/
	    }
	}
}	/*** MAIN ***/

VOID	show_mode(string,token,file_spec)

	BYTE			*string;
	BYTE			*token;
	BYTE			*file_spec;

	{

	 WORD			error;
	 BYTE			buffer[REC_LEN];	/* record buffer */
	 BYTE			*record;
	 BYTE			*pass;	/* pts to password from C RTL	*/
	 WORD			first;	/* flag for search first or next */
	 struct	_fcblst		*files;	/* linklist of file names	*/


	    if ( (*token == NULL) )
	    {		/* user type 'chset $...' so token will be null	*/
		print_err(err_04_msg,EXIT_PROGRAM);
	    }
	    record = &buffer[0];
	    string = a_white_char(string);
	    if ( (*string != NULL) )
	    {		/* extra characters on line ???two filespecs???	*/
		print_err(err_01_msg,EXIT_PROGRAM);
	    }
	    files = expfcb(file_spec);	/* Returns linklist		*/
	    while ( (files != NULL) )
	    {
		copy(files->fcb,record,20);
		error = get_settings(record);
		if ( error == PASSWORD )
		{
		    passget(record);
		    error = get_settings(record);
		}
		if ( (error == 0) )
		{
		    print(record,setting_is);
		}
	      else
		{		/* extra line after bdos error		*/
		    c_writestr("\n\r");
		}
		files = files->next_fcb;
	    }
	}



VOID	set_mode(string,token,file_spec)

	BYTE			*string;
	BYTE			*token;
	BYTE			*file_spec;

	{

	 BYTE			buffer[REC_LEN];	/* record buffer  */
	 BYTE			*record;
	 BYTE			i;
	 WORD			error;
	 BYTE			*pass;	/* pts to password from C RTL	*/
	 struct	_fcblst		*files;	/* linklist of files		*/


	    record = &buffer[0];
	    if ( (index(file_spec,'?') == NULL) &&
		 (index(file_spec,'*') == NULL) )
	    {			/* O.K. no wild cards are present 	*/
	        string = a_white_char(string);	/* Move to a white char	*/
	        if ( (*string == '[') )
	        {			/* O.K. start symbol 		*/
		    string = check_format(string,token,record);
		    string = a_white_char(string);	/* White char	*/
		    if ( (*string == NULL) || (*string == ']') )
		    {		/* COMMAND OPTIONS HAVE BEEN ACCEPTED 	*/
			files = expfcb(file_spec);	/* Linklist	*/
			/* FCB has dfffffffftttpppppppp			*/
			for ( i=0; i<20; i++)
			{
			    *(record+i) = *(files->fcb+i);
			}
			error = set_fields(record);
			if ( error == PASSWORD )
			{
			    passget(record);
			    error = set_fields(record);
			}
			if ( error == 0)
			{
			    print(record,set_to);
			}
		      else
			{		/* extra line after bdos error		*/
			    c_writestr("\n\r");
			}
		    }
		  else
		    {  
		        print_err(err_10_msg,EXIT_PROGRAM);
		    }
	        }
	       else
	        {			/* Illegal start symbol		*/
		    print_err(err_05_msg,EXIT_PROGRAM);
	        }
	    }
	  else
	    {				/* No wild cards are allowed	*/
		print_err(err_11_msg,EXIT_PROGRAM);
	    }
	}

BYTE	*check_format(string,token,record)

	BYTE			*string;
	BYTE			*token;
	BYTE			*record;	/* Record to set fields in */

	{

	 WORD			field;
	 WORD			option;
	 WORD			start;
	 WORD			end;

	    *(record+REC_8087) = EMPTY;		/* 8087 field		*/
	    *(record+REC_SHARED) = EMPTY;   /* SHARED field of record	*/
	    *(record+REC_SUSPEND) = EMPTY;  /* SUSPEND field of record	*/
	    string++;
	    FOREVER		/* Exits on an error or success		*/
	    {
/*****			string++;   	****/
		string = get_token(string,deli_01,token);
		field = fisttonu(token,fie1_tab,FIE1_START,FIE1_END);
		if ( (field != NOT_FOUND) )
		{				/* O.K. field		*/
		    string = a_white_char(string);
		    if ( (*string++ == '=') )
		    {				/* O.K. seperator	*/
			string = get_token(string,deli_01,token);
			start = xref_tab[field].start;
			end = xref_tab[field].end;
			option = fisttonu(token,opt1_tab,start,end);
			if ( (option != NOT_FOUND) )
			{	/* Have 'field=option' check and record */
			    if (((*(record+REC_FIELDS+field)) == ((BYTE)EMPTY)))
			    {		/* EVERTHING O.K. on this pass */
				*(record+REC_FIELDS+field) = option;
				string = a_white_char(string);
				if ( (*string == ']') || (*string == '\0') )
	/**************     if ( (*string != ',') ) ***********/
				{	/* No more 'field=option' */
				    return(string);
				}
			      else
				{	/* accept ',' as well as ' '	*/
				    if (*string == ',') string++;
				}
			    }
			  else
			    {		/* Duplication error	*/
				print_err(err_08_msg,EXIT_PROGRAM);
			    }
			}
		      else
			{
			    print_err(err_07_msg,EXIT_PROGRAM);
			}
		    }
		  else
		    {				/* Expected an '='	*/
			print_err(err_09_msg,EXIT_PROGRAM);
		    }
		}
	      else
		{			/* Token is not legal field 	*/
		    print_err(err_06_msg,EXIT_PROGRAM);
		}
	    }
	}


VOID	set_or_show(string,file_spec)

	BYTE			*string;
	BYTE			*file_spec;

	{

	    string = get_token(string,deli_02,token);
	    add_cmd(token,file_spec);
	    if ( (index(string,'[') == NULL) )	/* i.e. did not find it	*/
	    {		/* User wishes to set */
		show_mode(string,token,file_spec);
	    }
	     else
	    {		/* User wishes to show		*/
		set_mode(string,token,file_spec);
	    }
	}


BYTE	*get_token(string,delimiters,token)

	BYTE			*string;
	BYTE			*delimiters;
	BYTE			*token;

	{

	 BYTE			*index;

	    string = a_white_char(string);
	    while ( (*string != NULL) )
	    {
		for ( index=delimiters; *index != NULL; *index++ )
		{
		    if (*index == *string)
		    {		/* Found a delimiter, prepare to return	*/
			*token = NULL;
			return(string);
		    }
		}
		*token++ = *string++;
	    }
	    *token = NULL;
	    return(token);
	}



VOID	display_help()

	{

	    c_writestr("Syntax:\n\r\n\r");
	    c_writestr("    CHSET {d:}filename{.CMD}\n\r");
	    c_writestr("    CHSET {d:}filename{.CMD} [field=setting{,field=setting...}]\n\r");
	    c_writestr("    CHSET [HELP]\n\r\n\r");
	    c_writestr("Fields and Settings:\n\r\n\r");
	    c_writestr("    8087        ON or OPT or OFF\n\r");
	    c_writestr("    SHARED      ON or OFF \n\r");
	    c_writestr("    SUSPEND     ON or OFF \n\r\n\r");
	    c_writestr("Examples:\n\r\n\r");
	    c_writestr("    CHSET qwe.cmd [shared=on]          ; Sets the shared field of qwe.cmd\n\r");
	    c_writestr("    CHSET editor [sus=on]              ; Sets editor.cmd to suspend\n\r");
	    c_writestr("    CHSET pie [80=opt]                 ; Sets the 8087 field to optional\n\r");
	    c_writestr("    CHSET *                            ; Displays settings for all CMD files\n\r");
	    c_writestr("    CHSET qwerty                       ; Display current settings of qwerty.cmd\n\r");
/***
	    c_writestr("    CHSET calc.cmd [8=on,su=off,sh=on]\n\r");
	    c_writestr("    CHSET calc1 [8=off,su=on,sh=off]\n\r");
	    c_writestr("    CHSET a*\n\r");
***/
	}




VOID	add_cmd(token,file_spec)

	BYTE			*token;		/* current file_spec */
	BYTE			*file_spec;

	{
	 BYTE			*found;
	 BYTE			*spec_index = file_spec;

	    if ( (strlen(token) < 24) )
	    {
	        found = search(".",token);
		if ( (*found == NULL) )
		{			/* We must add '.CMD' */
		    if ( (strlen(token) < 20) )
		    {
 		        found = search(";",token);	/* found pts to NULL or ';' */
		        while ( (*token != *found) )
		        {
		            *spec_index++ = *token++;
		        }
		        found = strncpy(spec_index,cmd,4);
		        spec_index = spec_index + 4;
		        while ( (*token != NULL) )
		        {
		            *spec_index++ = *token++;
		        }
		        *spec_index = NULL;
	            }
	          else
		    {		/* File spec to long to add '.cmd'	*/
			print_err(err_04_msg,EXIT_PROGRAM);
		    }
		}
	      else
		{		/* User has specified extension		*/
		    found = strncmp(found,cmd,4);	/* Is ext '.CMD' */
		    if ( (found == NULL) )
		    {				/* Extension is 'CMD'	*/
		        strncpy(file_spec,token,24);
		    }
		  else
		    {				/* Illegal extension	*/
		        print_err(err_02_msg,EXIT_PROGRAM);
		    }
	        }
	    }
	   else
	    {		/* File spec is too long to be in correct form	*/
		print_err(err_03_msg,EXIT_PROGRAM);
	    }
	}


VOID	print_err(string,mode)

	BYTE			*string;
	WORD			mode;

	{

	    c_writestr(string);
	    c_writestr("\n\r");
	    if ( (mode == EXIT_PROGRAM) )
	    {
		p_termcpm();
	    }
	}


BYTE	*search(sub_string,string)

/******

    OUTPUT
	returns ptr to first character of match or ptr to NULL if no match.

    RESTRICTIONS
	Sub_string is defined as 1+ characters ending with a NULL.

*******/

	BYTE			*sub_string;	/* String searching for	*/
	BYTE			*string;	/* String looking in	*/

	{

	 BYTE			*tmp1;
	 BYTE			*tmp2;

	    while ( (*string != NULL) )
	    {
		tmp1 = sub_string;
		tmp2 = string;
		while ( (*tmp1++ == *tmp2++) )
		{
		    if ( (*tmp1 == NULL) )
		    {				/* Found a match	*/
			return(string);
		    }
		    if ( (*tmp2 == NULL) )
		    {		/* Tmp1 is longer so no match possible	*/
			return(tmp2);
		    }
		}
		string++;
	    }
	    return(string);
	}


VOID	upper_case(string)

	BYTE			*string;

	{

	    while ( ((*string) != NULL) )
	    {
		if ( ('a' <=  (*string)) && ((*string) <= 'z') )
		{
		    *string = (*string) - LOW_TO_UP;
		}
		string++;
	    }
	}


WORD	fisttonu(string,table,start,end)

/****
        Will return NOT_FOUND if given a NULL string
****/

	BYTE			*string;
	struct fie1_str		table[];
	WORD			start;
	WORD			end;

	{
	 struct fie1_str	*loop	=   &table[start];
	 WORD			cnt	=   start;

	    if ( (*string == NULL) )
	    {			/* Special case		*/
		return(NOT_FOUND);
	    }
       	    while ( (cnt <= end) )
	    {
	        if ( (abrmatch(string,loop->fieldstr)) > -10 )
	        {
	            return(loop->field_nu);
	        }
	      else
	        {
	            cnt++;
	            loop++;
	        }
	    }
	    return(NOT_FOUND);
	}


WORD	abrmatch(word1,word2)

	BYTE				*word1;
	BYTE				*word2;

	{
	WORD				state	=   -30;

	    while ( (*word2 != NULL) )
	    {
	        if ( (*word2 == '*') )
	        {
	            if ( (*word1 == NULL) )
		    {
			if ( (*++word2 == NULL) )
			{
			    return(20);
			}
		      else
			{
			    return(10);
			}
		    }
		    *word2++;
		    state = -10;
	        }
	      else
	        {
	            if ( (*word1 != *word2) )
	            {
			if ( (*word1 == NULL) && (state == -10) )
			{
			    return(0);
			}
		      else
			{
			    return(state);
			}
	            }
		  else
		    {
			*word1++;
			*word2++;
		    }
	        }
	    }
		/*** at end of word2 ***/
	    if ( (*word1 == *word2) )
	    {	/*** at end of word1 also ***/
	        return(20);	/* perfect match */
	    }
	  else
	    {	/*** word1 has more letters left ***/
		return(-20);
	    }
	}


BYTE	*a_white_char(string)

	BYTE			*string;

	{

	    while ( (*string == ' ') || (*string == '\t') )
	    {
		string++;
	    }
	    return(string);
	}

VOID	print(record,string)

	BYTE			*record;
	BYTE			*string;

	{

	 BYTE			print_buffer[80];
	 BYTE			*prt_buf;
	 WORD			index;
	 WORD			i;

	    prt_buf = &print_buffer[0];
	    prt_buf = expand_file_name(record,prt_buf);
	    strcpy(prt_buf,string);	/* Gets string			*/
	    prt_buf = prt_buf + strlen(string);
	/*** Now have D:FILENAME.TYP'string' ***/
	    *prt_buf++ = '[';
	    record = record + REC_FIELDS;	/* Pts to fields	*/
	    for ( i = FIE1_START; i <= FIE1_END; i++,record++)
	    {
		if ( (*record != (BYTE)EMPTY) )
		{	/* Only print if field is not empty		*/
		    prt_buf = short_string(prt_buf,fie1_tab[i].fieldstr);
		    *prt_buf++ = '=';
		    index = xref_tab[i].start + (*record);
		    prt_buf = short_string(prt_buf,opt1_tab[index].fieldstr);
		    *prt_buf++ = ',';
		}
	    }
	    *(--prt_buf) = ']';		/* write over last ','		*/
	    *(++prt_buf) = '\n';
	    *(++prt_buf) = '\r';
	    *(++prt_buf) = NULL;
	    c_writestr(&print_buffer[0]);
	}

BYTE	*short_string(buffer,string)

	BYTE			*buffer;	/* Destination		*/
	BYTE			*string;	/* Source string	*/

/***	

	Copies all characters of a string except '*' and the NULL
    terminator to a buffer.

	OUTPUT
		A ptr to the location one past the last copied
	    character in the buffer.

***/

	{

	    while ( *string != NULL )
	    {
		if ( *string != '*')
		{		/* Copy character			*/
		    *buffer++ = *string++;
		}
	      else
		{
		    string++;
		}
	    }
	     return(buffer);
	}


/*----------------------------------------------------------------------*\
 |	NAME	:  expfcb						|
 |	CREATED	:  10-August-83		LAST MODIFIED:  12-September-83 |
 |	FUNCTION:  Expfcb expands the input filespec into a list of	|
 |		   FCB's						|
 |	INPUT	:  filespec -- ptr to filespec given on command line	|
 |	OUTPUT	:  Returns ptr to list of FCB's created from filespec	|
\*----------------------------------------------------------------------*/

struct	_fcblst   *expfcb( filespec )
	BYTE		*filespec;
{
/****	BYTE		fcb[FCB_LEN];	/* file control block buffer	*/
	BYTE		dma[DMA_LEN];	/* DMA buffer			*/
	WORD		dindex;		/* DMA buffer index		*/
	WORD		findex;		/* FCB buffer index		*/
	WORD		fcount;		/* file count			*/
	BYTE		*save_dma;	/* DMA offset save area		*/
	struct		_fcblst  *fhead;/* fcb list head ptr		*/
	struct		_fcblst  *ftail;/* fcb list tail ptr		*/
	struct		_fcblst  *flist;/* fcb	list			*/
/****	struct		_pfcb	  pfcb;	/* parse FCB for F_PARSE	*/
	WORD			error_code;
	struct	pfcb_str	pfcb;

	    save_dma = f_dmaget();
	    pfcb.filename = filespec;
	    pfcb.fcb_adr = fcb;
	    error_code = f_parse(&pfcb);
	    if ( (error_code != FALSE) )
	    {				/* We got an error		*/
		print_err(err_12_msg,EXIT_PROGRAM);
	    }
	    fcb[12] = 0;			/* extent number	*/
	fcount = 0;
	fhead = malloc( sizeof( struct _fcblst ) );
	if (fhead == 0)
	{			/* Out of room	*/
	     print_err(err_14_msg,EXIT_PROGRAM);
	}
	f_dmaset( dma );
	dindex = f_sfirst( fcb );
	if ( dindex == 0x00ff )
	{
	    print_err(err_13_msg,EXIT_PROGRAM);
	}
	while( dindex != 0x00ff )
	{
	   dindex = (dindex << 5) + 1;		/* dindex * 32, skip	*/
	   fcount++;				/* user number field	*/ 
	   flist = malloc( sizeof( struct _fcblst ) );
	    if (flist == 0)
	    {			/* Out of room	*/
	         print_err(err_14_msg,EXIT_PROGRAM);
	    }
	   fhead->next_fcb = flist;
	   fhead = flist;
	   if( fcount == 1 )
	      ftail = flist;
	   flist->fcb[0] = fcb[0];		/* insert drive code	*/
	   for( findex=1; findex < FCB_LEN; findex++ )	/* copy FCB	*/
	      flist->fcb[findex] = dma[dindex++];
	   for( findex=16; findex < 24; findex++ )	/* copy password*/
	      flist->fcb[findex-4] = fcb[findex];
     	   dindex = f_snext( fcb );
	}
	flist->next_fcb = NULL;			/* end FCB list		*/
	f_dmaset( save_dma );			/* reset DMA address	*/
	return( ftail );			/* front of list	*/
}


VOID	passget(record)

	BYTE			*record;

	{

	 BYTE			*pass;
	 BYTE			buffer[15];
	 BYTE			*prt_buf;

	    c_writestr("\n\r");
	    prt_buf = &buffer[0];
	    prt_buf = expand_file_name(record,prt_buf);
	    *prt_buf = NULL;
	    c_writestr(&buffer[0]);
	    pass = getspass(pass_prompt);	/* C RTL routine	*/
	    c_writestr("\n\r");
	    upper_case(pass);		/* Convert to upper case	*/
	    strncpy(record+12,blanks,8);		/* Blank out	*/
	    strncpy(record+12,pass,strlen(pass));	/* Replace	*/
	}

/*----------------------------------------------------------------------*\
 |	NAME	:  ok_ver						|
 |	CREATED	:  5-August-83		LAST MODIFIED:  12-September-83 |
 |	FUNCTION:  Ok_ver checks to see that the correct BDOS and OS	|
 |		   are being used.					|
 |	INPUT	:  bdos_ver -- BDOS version number to look for.		|
 |		   os_ver   -- OS version number to look for.		|
 |	OUTPUT	:  Returns 1 for true, 0 for false.			|
\*----------------------------------------------------------------------*/

WORD	ok_ver( bdos_ver,os_ver )
	WORD			bdos_ver;	/* min. BDOS version	*/
	WORD			os_ver;		/* min. CP/M version	*/
{
	WORD	ver;		/* S_BDOSVER return value (version #)	*/

	s_bdosver();
	ver = _EXTERR;
	if(((ver & BDOS_MASK) < bdos_ver) || ((ver & OS_MASK) != os_ver))
	   return(FALSE);
	return(TRUE);
}

BYTE	*getspass(prompt)

	BYTE			*prompt;

	{

	 BYTE			pasword[8];
	 WORD			pindex,xindex,ch;

	    c_writestr(prompt);
	    for (xindex=0;xindex<8;xindex++)
	    {
		pasword[xindex] = ' ';
	    }
	    pindex = 0;
	    while (pindex<8)
	    {
		ch = c_rawio(0x00fd);		/* read a character	*/
		if ( ch > ' ')
		{
		    pasword[pindex++] = (BYTE)ch;
		}
	      else
		{
		    switch(ch)
		    {
			case '\003' :	p_termcpm();		/* ^C */
					break;
			case '\010' :	if (pindex > 0 )	/* backspace */
					    pasword[--pindex] = ' ';
					break;
			case '\015' :	c_writestr("\n\r");	/* CR */
					pindex = 8;
					break;
			case '\030' :	for( xindex=0;xindex<8;xindex++) /* ^X */
					    pasword[xindex] = ' ';
					pindex = 0;
					break;
			default     :	break;
		    }
		}
	    }
	    ch = c_stat();
	    return(&pasword[0]);
	}


BYTE	*expand_file_name(record,buffer)

	BYTE				*record;
	BYTE				*buffer;

	{
	 BYTE				*prt_buf;
	 WORD				i;

	    prt_buf = buffer;		/* pts to buffer to store name	*/
	    if (*record != 0)
	    {			/* not the default drive		*/
		*prt_buf++ = (*record) + 64;	/* store letter		*/
		*prt_buf++ = ':';
	    }
	    record++;		/* Move record ptr to filename		*/
	    for ( i=0; i<8 ; prt_buf++,i++,record++ )
	    {			/*  Get filename  7-bit  ASCII		*/
		*prt_buf = *record & 0x7f;
	    }
	    *prt_buf++ = '.';
	    for ( i=0; i<3 ; prt_buf++,i++,record++ )
	    {			/* Get type  7-bits ASCII		*/
		*prt_buf = *record & 0x7f;
	    }
	    return(prt_buf);
	}

		
VOID	patch_area()

	{

	 WORD			i;

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

	}

