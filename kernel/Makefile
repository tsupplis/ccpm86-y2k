ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd

SNIPPETS=bdos.con sup.con rtm.con cio.con mem.con sysdat.con xios.con

all: $(SNIPPETS)

bdos.con: bdos.obj
	$(LINK86) $@=$< '[data[origin[0]]]'
	@cmdinfo $@

xios.con: xios.h86
	$(GENCMD) $< 8080
	mv $(basename $@).cmd $@
	@cmdinfo $@

%.h86: %.loc
	$(INTEL_OH86) $^
	mv $*.hex $*.h86

%.con: %.h86
	$(GENCMD) $^
	mv $(basename $@).cmd $@
	@cmdinfo $@

%.h86: %.a86
	$(ASM86) $^
	
%.obj: %.a86
	$(RASM86) $^ 

clean:
	rm -f *.obj *.h86 *.sym *.lst *.con *.cmd *.loc *.lnk

