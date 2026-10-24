ASM86=cpm86_asm86
GENCMD=cpm86_gencmd

TOOLS=put.cmd get.cmd rdr.cmd

all: $(TOOLS)

.PHONY: all clean
.DELETE_ON_ERROR:

%.cmd: %.h86 Makefile
	$(GENCMD) $<
	cmdinfo $@

%.h86: %.a86 putget.def putget.lib
	$(ASM86) $<

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lst *.prn *.loc *.lnk *.mp?
