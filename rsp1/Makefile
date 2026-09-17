INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LOC86=intel_loc86
INTEL_LINK86=intel_link86
INTEL_OH86=intel_oh86
ASM86=cpm86_asm86
GENCMD=cpm86_gencmd

SNIPPETS= serin.rsp serout.rsp tmp.rsp pin.rsp vout.rsp 

all: $(SNIPPETS)

pin.rsp: pin.h86
	$(GENCMD) $< 'data[b1000]'
	mv $(basename $@).cmd $@
	cmdinfo $@

pin.loc: rhpin.obj pxios.obj pin.obj 
	$(INTEL_LINK86) rhpin.obj, pxios.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(0))'  to $(basename $@).loc

vout.rsp: vout.h86
	$(GENCMD) $< 'data[b1000]'
	mv $(basename $@).cmd $@
	cmdinfo $@

vout.loc: rhvout.obj pxios.obj vout.obj 
	$(INTEL_LINK86) rhvout.obj, pxios.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(0))'  to $(basename $@).loc

%.h86: %.loc
	$(INTEL_OH86) $^
	mv $*.hex $*.h86

%.rsp: %.h86
	$(GENCMD) $^
	mv $(basename $@).cmd $@
	@cmdinfo $@

%.h86: %.a86
	$(ASM86) $^
	
%.obj: %.a86
	$(INTEL_ASM86) $^

%.obj: %.p86
	$(INTEL_PLM86) $^ debug 'optimize(3)' xref

%.obj: %.plm
	$(INTEL_PLM86) $^ debug 'optimize(3)' xref

clean:
	rm -f *.obj *.h86 *.sym *.lst *.rsp *.cmd *.loc *.lnk *.mp?

