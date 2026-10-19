ASM86=cpm86_asm86
GENCMD=cpm86_gencmd

.PHONY: all clean
.DELETE_ON_ERROR:
all: setdef.cmd

setdef.cmd: setdef.h86 Makefile
	$(GENCMD) $<
	cmdinfo $@

setdef.h86: setdef.a86
	$(ASM86) $<

clean:
	rm -f setdef.cmd setdef.h86 setdef.sym setdef.lst
