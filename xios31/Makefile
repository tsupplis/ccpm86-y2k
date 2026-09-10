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

SNIPPETS=pcxios.h86

all: $(SNIPPETS)

%.h86: %.a86
	$(DRASM86) $^

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn

