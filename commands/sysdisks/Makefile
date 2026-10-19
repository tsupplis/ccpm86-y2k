ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd

TOOLS=sysdisk.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

sysdisk.cmd: sysdisk.h86
	$(GENCMD) $<
	cmdinfo $@

%.h86: %.a86
	$(ASM86) $^

clean:
	rm -f *.sys *.loc *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
	rm -f *.loc *.mp? *.lnk

