INTEL_OH86=intel_oh86
ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd

TOOLS=submit.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

submit.cmd: submit.h86
	$(GENCMD) $<
	cmdinfo $@

%.h86: %.a86
	$(ASM86) $^

clean:
	rm -f *.sys *.loc *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
	rm -f *.loc *.mp? *.lnk

