INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LOC86=intel_loc86
INTEL_LINK86=intel_link86
INTEL_OH86=intel_oh86
GENCMD=cpm86_gencmd

TOOLS=pip3.cmd pip4.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

pip4.cmd: pip4.h86
	$(GENCMD) $< 'data[b1000 m280 xfff]'
	cmdinfo $@

pip4.loc: pip4.obj inpout.obj scd1.obj
	$(INTEL_LINK86) scd1.obj, inpout.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

pip3.cmd: pip3.h86
	$(GENCMD) $< 'data[b1000 m280 xfff]'
	cmdinfo $@

pip3.loc: pip3.obj inpout.obj scd1.obj
	$(INTEL_LINK86) scd1.obj, inpout.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

%.h86: %.loc
	$(INTEL_OH86) $^
	mv $*.hex $*.h86

%.obj: %.a86
	$(INTEL_ASM86) $^

%.obj: %.plm
	$(INTEL_PLM86) $^ debug 'optimize(3)' xref

clean:
	rm -f *.sys *.loc *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
	rm -f *.loc *.mp? *.lnk

