ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd

TOOLS=sysdisk.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

.PHONY: all check clean
.DELETE_ON_ERROR:

sysdisk.cmd: sysdisk.h86 Makefile
	$(GENCMD) $<
	cmdinfo $@

check: sysdisk.cmd sysdisk.ref
	cmp sysdisk.cmd sysdisk.ref
	@echo "SYSDISK: byte-identical to sysdisk.ref"

%.h86: %.a86
	$(ASM86) $^

clean:
	rm -f *.sys *.loc *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
	rm -f *.loc *.mp? *.lnk

