INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LOC86=intel_loc86
INTEL_LINK86=intel_link86
INTEL_OH86=intel_oh86
GENCMD=cpm86_gencmd

TOOLS=era.cmd eraq.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

# BBB and MMM for gencmd's data[bBBB,mMMM,xfff], derived from eraq.mp2:
#   BBB = start of dats segment / 16 (e.g. 1000)
#   MMM = (start of memory segment - start of dats segment) / 16
PARA = sub(/H$$/,"",s); sub(/.$$/,"",s); sub(/^0+/,"",s); print tolower(s); exit
ERAQ_BASE = $(shell awk '/DATS +DATA/{s=$$1; $(PARA)}' eraq.mp2)
ERAQ_MEM  = $(shell awk '/MEMORY +MEMORY/{s=$$1; $(PARA)}' eraq.mp2)
ERAQ_MIN  = $(shell printf '%x' $$((0x$(ERAQ_MEM)-0x$(ERAQ_BASE))))

eraq.cmd: eraq.h86
	@test -n "$(ERAQ_BASE)" -a -n "$(ERAQ_MEM)" || \
	  { echo "era: cannot derive DATS/MEMORY from eraq.mp2"; exit 1; }
	# $(GENCMD) $< 'data[b1000]'
	$(GENCMD) $< 'data[b$(ERAQ_BASE) m$(ERAQ_MIN) xfff]'
	cmdinfo $@

eraq.loc: eraq.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

# BBB and MMM for gencmd's data[bBBB,mMMM,xfff], derived from era.mp2:
#   BBB = start of dats segment / 16 (e.g. 1000)
#   MMM = (start of memory segment - start of dats segment) / 16
ERA_BASE = $(shell awk '/DATS +DATA/{s=$$1; $(PARA)}' era.mp2)
ERA_MEM  = $(shell awk '/MEMORY +MEMORY/{s=$$1; $(PARA)}' era.mp2)
ERA_MIN  = $(shell printf '%x' $$((0x$(ERA_MEM)-0x$(ERA_BASE))))

era.cmd: era.h86
	@test -n "$(ERA_BASE)" -a -n "$(ERA_MEM)" || \
	  { echo "era: cannot derive DATS/MEMORY from era.mp2"; exit 1; }
	# $(GENCMD) $< 'data[b1000]'
	$(GENCMD) $< 'data[b$(ERA_BASE) m$(ERA_MIN) xfff]'
	cmdinfo $@

era.loc: era.obj scd.obj
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

