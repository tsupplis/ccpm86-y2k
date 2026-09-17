INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LOC86=intel_loc86
INTEL_LINK86=intel_link86
INTEL_OH86=intel_oh86
GENCMD=cpm86_gencmd

TOOLS=sdir.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

sdir.cmd: sdir.h86
	$(GENCMD) $< 'data[b1000 m3c5 x800]'
	cmdinfo $@

sdir.loc: scd.obj sdir.obj search.obj scan.obj sort.obj disp.obj util.obj \
    timest.obj dpb.obj
	$(INTEL_LINK86) scd.obj, search.obj, scan.obj, sort.obj, disp.obj, \
	util.obj, timest.obj, dpb.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

%.h86: %.loc
	$(INTEL_OH86) $^
	mv $*.hex $*.h86

%.obj: %.a86
	$(INTEL_ASM86) $^

%.h86: %.a86
	$(ASM86) $^

%.obj: %.plm
	$(INTEL_PLM86) $^ debug 'optimize(3)' xref

clean:
	rm -f *.sys *.loc *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
	rm -f *.loc *.mp? *.lnk

