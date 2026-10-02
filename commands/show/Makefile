INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LOC86=intel_loc86
INTEL_LINK86=intel_link86
INTEL_OH86=intel_oh86
GENCMD=cpm86_gencmd

TOOLS=show.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

# BBB and MMM for gencmd's data[bBBB,mMMM,xfff], derived from show.mp2:
#   BBB = start of dats segment / 16 (e.g. 1000)
#   MMM = (start of memory segment - start of dats segment) / 16
PARA = sub(/H$$/,"",s); sub(/.$$/,"",s); sub(/^0+/,"",s); print tolower(s); exit
GENCMD_BASE = $(shell awk '/DATS +DATA/{s=$$1; $(PARA)}' show.mp2)
GENCMD_MEM  = $(shell awk '/MEMORY +MEMORY/{s=$$1; $(PARA)}' show.mp2)
GENCMD_MIN  = $(shell printf '%x' $$((0x$(GENCMD_MEM)-0x$(GENCMD_BASE))))

show.cmd: show.h86
	@test -n "$(GENCMD_BASE)" -a -n "$(GENCMD_MEM)" || \
	  { echo "show: cannot derive DATS/MEMORY from show.mp2"; exit 1; }
	# $(GENCMD) $< 'data[b1000]'
	$(GENCMD) $< 'data[b$(GENCMD_BASE) m$(GENCMD_MIN) xfff]'
	cmdinfo $@

show.loc: show.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
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
