DRASM86=cpm86_asm86
GENCMD=cpm86_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd

TOOLS=tcopy.cmd

# initdir.cmd

SNIPPETS=boot.sys load.sys lbdos.sys  lbdos3.sys

all: $(TOOLS) $(SNIPPETS)

tcopy.cmd: tcopy.h86
	$(GENCMD) $<
	
lbdos.sys: lbdos.h86
	$(GENCMD) $< 8080
	mv $(basename $@).cmd $@
	cmdinfo $@

lbdos3.sys: lbdos3.obj
	$(LINK86) $@=$(basename $<) '[data[ori[0],max[0]]]'
	cmdinfo $@

load.sys: load.h86
	$(GENCMD) $< 8080
	mv $(basename $@).cmd $@
	cmdinfo $@

boot.sys: boot.h86
	$(GENCMD) $< 8080
	mv $(basename $@).cmd $@
	cmdinfo $@

%.h86: %.loc
	$(INTEL_OH86) $^
	mv $*.hex $*.h86

%.h86: %.a86
	$(DRASM86) $^

%.obj: %.a86
	$(RASM86) $^

clean:
	rm -f $(TOOLS) $(SNIPPETS) *.obj *.cmd *.h86 *.sym *.lst ccpmtest.img

ccpmtest.img: $(TOOLS) Makefile startup.0
	cp cpmbase.img ccpmtest.img
	cpmcp -f ibmpc-514ss ccpmtest.img *.cmd 0:
	cpmcp -f ibmpc-514ss ccpmtest.img startup.0 0:
	cpmls -F -f ibmpc-514ss ccpmtest.img 0:*.*


test: ccpmtest $(TOOLS)

ccpmtest: ccpmtest.img $(TOOLS)
	@./ccpm86

