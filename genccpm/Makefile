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

TOOLS=genccpm.cmd

# initdir.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

genccpm.cmd: genc1.obj genc2.obj genc3.obj genc4.obj genc5.obj genc6.obj
	$(DRCLINK86) genccpm[map]=genc1,genc2,genc3,genc4,genc5,genc6

%.h86: %.a86
	$(DRASM86) $^

%.obj: %.a86
	$(ASM86) $^

%.obj: %.c
	$(DRC86) -o$@ $^

%.obj: %.pli
	$(PLI86) $^ \$$dl

%.obj: %.plm
	$(PLM86) $^ debug 'optimize(3)' xref

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
