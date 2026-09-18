INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LOC86=intel_loc86
INTEL_LINK86=intel_link86
INTEL_OH86=intel_oh86
ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd
PLI86=drplicpm_pc
PLILINK86=drplicpm_link
PLIOBJFIX=drpli_objfix

TOOLS=initdir.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

initdir.cmd: initdir.obj initdira.obj diomod.obj
	$(PLILINK86) initdir [locals],initdira,diomod
	cmdinfo $@

initdir.obj: initdir.pli
	$(PLI86) $< \$$dl
	$(PLIOBJFIX) $@

diomod.obj: diomod.a86
	$(RASM86) $^

initdira.obj: initdira.a86
	$(RASM86) $^

%.h86: %.loc
	$(INTEL_OH86) $^
	mv $*.hex $*.h86

%.obj: %.a86
	$(INTEL_ASM86) $^

%.h86: %.a86
	$(ASM86) $^

%.obj: %.pli
	$(PLI86) $^ \$$dl

%.obj: %.plm
	$(INTEL_PLM86) $^ debug 'optimize(3)' xref

clean:
	rm -f *.sys *.loc *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
	rm -f *.loc *.mp? *.lnk

