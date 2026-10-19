#include "stdio.h"



#include "common.h"



#define C_GET 0x99
#define C_RAWIO 6

#define	IO_KEYBD 32
#define	ENABLE 1
#define	DISABLE 2

static WORD vc_number;



BYTE
VKgetkey()
/****************************************************************/
/*								*/
/*	Virtual Keyboard GET KEY routine -  get the raw		*/
/*		keyboard input.					*/
/*								*/
/****************************************************************/
{
	static BYTE oldc = '\0';
	UWORD ax;
	BYTE ah, al;


	if (oldc != '\0') {
		al = oldc;
		oldc = '\0';
		return(al);
	}

	ax = RAWIO();

	ah = (ax >> 8) & 0xFF;

	al = ax & 0xFF;

	if (al == 0)
		oldc = ah;

	return(al);
}



VOID
VKfin()
/****************************************************************/
/*								*/
/*	Virtual Keyboard FINish routine - restore keyboard	*/
/*								*/
/****************************************************************/
{
	UWORD ax, bx, cx, dx;

	ax = IO_KEYBD;			/* XIOS function 32		*/
	bx = 0;
	cx = DISABLE;			/* disable PC-MODE keyboard	*/
	dx = vc_number;			/* default			*/
	IO_CALL(&ax, &bx, &cx, &dx);	/*		DO IT		*/
}



VOID
VKinit()
/****************************************************************/
/*								*/
/*	Virtual Keyboard INITilization routine			*/
/*								*/
/****************************************************************/
{
	UWORD ax, bx, cx, dx;

	vc_number = __BDOS(C_GET, 0x0);	/* Get the virtual console #	*/

	ax = IO_KEYBD;			/* direct I/O system #32	*/
	bx = 0;
	cx = ENABLE;			/* enable PC-MODE keyboard	*/
	dx = vc_number;
	IO_CALL(&ax, &bx, &cx, &dx);	/*		DO IT		*/
}
