INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LOC86=intel_loc86
INTEL_LINK86=intel_link86
INTEL_OH86=intel_oh86
GENCMD=cpm86_gencmd

TOOLS=sdir.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

# BBB and MMM for gencmd's data[bBBB,mMMM,xfff], derived from sdir.mp2:
#   BBB = start of dats segment / 16 (e.g. 1000)
#   MMM = (start of memory segment - start of dats segment) / 16
PARA = sub(/H$$/,"",s); sub(/.$$/,"",s); sub(/^0+/,"",s); print tolower(s); exit
GENCMD_BASE = $(shell awk '/DATS +DATA/{s=$$1; $(PARA)}' sdir.mp2)
GENCMD_MEM  = $(shell awk '/MEMORY +MEMORY/{s=$$1; $(PARA)}' sdir.mp2)
GENCMD_MIN  = $(shell printf '%x' $$((0x$(GENCMD_MEM)-0x$(GENCMD_BASE))))

sdir.cmd: sdir.h86 Makefile
	@test -n "$(GENCMD_BASE)" -a -n "$(GENCMD_MEM)" || \
	  { echo "sdir: cannot derive DATS/MEMORY from sdir.mp2"; exit 1; }
	# $(GENCMD) $< 'data[b1000 m3c5 x800]'
	$(GENCMD) $< 'data[b$(GENCMD_BASE) m$(GENCMD_MIN) x800]'
	cmdinfo $@

sdir.obj: sdirmain.plm

# PL/M modules include shared declarations as well as their own source.
$(patsubst %.plm,%.obj,$(filter-out sdirmain.plm mon.plm,$(wildcard *.plm))): $(wildcard *.lit) mon.plm

sdir.loc: scd.obj sdir.obj search.obj scan.obj sort.obj disp.obj util.obj \
    timest.obj dpb.obj
	$(INTEL_LINK86) scd.obj, search.obj, scan.obj, sort.obj, disp.obj, \
	util.obj, timest.obj, dpb.obj, $(basename $@).obj to $(basename $@).lnk
	@if grep -q 'UNRESOLVED EXTERNAL NAMES' sdir.mp1; then \
	  echo "sdir: unresolved symbols in sdir.mp1"; exit 1; fi
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
	$(INTEL_PLM86) $< debug 'optimize(3)' xref

clean:
	rm -f *.sys *.loc *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
	rm -f *.loc *.mp? *.lnk

