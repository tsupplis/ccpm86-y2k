;
; This sub file compiles, links and generates in hex the VOUT rsp
; for Concurrent CP/M version 3.1
; It is meant for use under UDI.
;
; After the last step in this file, do:
;	GENCMD VOUT DATA[B1000]
;	REN VOUT.RSP=VOUT.CMD
;
asm86 rhvout.a86
asm86 pxios.a86
plm86 vout.p86 xref debug optimize(3) 
link86 rhvout.obj, pxios.obj, vout.obj to rhvout.lnk
rename rhvout.lnk to vout.lnk
loc86 vout.lnk od(sm(code,dats,data,const,stack)) &
     ad(sm(code(0),dats(10000h))) ss(stack(0)) to vout.dat
oh86 vout.dat
rename vout.hex to vout.h86

