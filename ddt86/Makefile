PLM86=intel_plm86
ASM86=intel_asm86
DRASM86=cpm_asm86
GENCMD=cpm_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd
PLI86=drpli_pc
PLILINK86=drpli_link
DRC86=drccpm_cc
DRCLINK86=drccpm_link

TOOLS=ddt86.cmd dis86.cmd gentab.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

ddt86.cmd: ddt86.h86
	$(GENCMD) $<

dis86.cmd: dis86.obj ins86.obj asmtab.obj ass86.obj dislnk86.obj asslnk86.obj
	$(LINK86) dis86=dis86,ins86,asmtab,ass86,dislnk86,asslnk86 '[data[ori[0]]'

gentab.cmd: gentab.obj
	$(PLILINK86) gentab=gentab '[data[ori[0]]'

asslnk86.obj: asslnk86.asm
	$(ASM86) $^

dislnk86.obj: dislnk86.asm
	$(ASM86) $^

cpmif.obj: cpmif.a86
	$(RASM86) $^

%.h86: %.a86
	$(DRASM86) $^

%.obj: %.a86
	$(ASM86) $^

%.obj: %.c
	$(DRC86) -o$@ $^

%.obj: %.pli
	$(PLI86) $^ \$$dl

%.obj: %.plm
	$(PLM86) $^ debug 'optimize(2)' xref

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img

