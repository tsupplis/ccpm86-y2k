INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LOC86=intel_loc86
INTEL_LINK86=intel_link86
INTEL_OH86=intel_oh86
GENCMD=cpm86_gencmd

TOOLS=pip3.cmd pip4.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

# BBB and MMM for gencmd's data[bBBB,mMMM,xfff], derived from pip4.mp2:
#   BBB = start of dats segment / 16 (e.g. 1000)
#   MMM = (start of memory segment - start of dats segment) / 16
PARA = sub(/H$$/,"",s); sub(/.$$/,"",s); sub(/^0+/,"",s); print tolower(s); exit
PIP4_BASE = $(shell awk '/DATS +DATA/{s=$$1; $(PARA)}' pip4.mp2)
PIP4_MEM  = $(shell awk '/MEMORY +MEMORY/{s=$$1; $(PARA)}' pip4.mp2)
PIP4_MIN  = $(shell printf '%x' $$((0x$(PIP4_MEM)-0x$(PIP4_BASE))))

pip4.cmd: pip4.h86
	@test -n "$(PIP4_BASE)" -a -n "$(PIP4_MEM)" || \
	  { echo "pip: cannot derive DATS/MEMORY from pip4.mp2"; exit 1; }
	# $(GENCMD) $< 'data[b1000 m280 xfff]'
	$(GENCMD) $< 'data[b$(PIP4_BASE) m$(PIP4_MIN) xfff]'
	cmdinfo $@

pip4.loc: pip4.obj inpout.obj scd1.obj
	$(INTEL_LINK86) scd1.obj, inpout.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

# BBB and MMM for gencmd's data[bBBB,mMMM,xfff], derived from pip3.mp2:
#   BBB = start of dats segment / 16 (e.g. 1000)
#   MMM = (start of memory segment - start of dats segment) / 16
PIP3_BASE = $(shell awk '/DATS +DATA/{s=$$1; $(PARA)}' pip3.mp2)
PIP3_MEM  = $(shell awk '/MEMORY +MEMORY/{s=$$1; $(PARA)}' pip3.mp2)
PIP3_MIN  = $(shell printf '%x' $$((0x$(PIP3_MEM)-0x$(PIP3_BASE))))

pip3.cmd: pip3.h86
	@test -n "$(PIP3_BASE)" -a -n "$(PIP3_MEM)" || \
	  { echo "pip: cannot derive DATS/MEMORY from pip3.mp2"; exit 1; }
	# $(GENCMD) $< 'data[b1000 m280 xfff]'
	$(GENCMD) $< 'data[b$(PIP3_BASE) m$(PIP3_MIN) xfff]'
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

