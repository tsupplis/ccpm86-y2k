
; This sub file compiles, links and generates in hex the PIN rsp
; for Concurrent CP/M version 3.1.
; It is meant for use under UDI.
;
; After the last step in this file, do:
;       GENCMD PIN DATA[B1000]
;       REN PIN.RSP=PIN.CMD
;
asm86 rhpin.a86
asm86 pxios.a86
plm86 pin.p86 xref debug optimize(3) 
link86 rhpin.obj, pxios.obj, pin.obj to rhpin.lnk
rename rhpin.lnk to pin.lnk
loc86 pin.lnk od(sm(code,dats,data,const,stack)) &
     ad(sm(code(0),dats(10000h))) ss(stack(0)) to pin.dat
oh86 pin.dat
rename pin.hex to pin.h86

