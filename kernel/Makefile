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

SNIPPETS=bdos.con sup.con rtm.con cio.con mem.con sysdat.con xios.con

all: $(SNIPPETS)

bdos.con: bdos.h86
	$(GENCMD) bdos.cmd
	mv bdos.cmd bdos.con

xios.con: pcxios.h86
	$(GENCMD) pcxios.cmd
	mv pcxios.cmd xios.con

sup.con: sup.h86
	$(GENCMD) sup.cmd
	mv sup.cmd sup.con

rtm.con: rtm.h86
	$(GENCMD) rtm.cmd
	mv rtm.cmd rtm.con

mem.con: mem.h86
	$(GENCMD) mem.cmd
	mv mem.cmd mem.con

cio.con: cio.h86
	$(GENCMD) cio.cmd
	mv cio.cmd cio.con

sysdat.con: sysdat.h86
	$(GENCMD) sysdat.cmd
	mv sysdat.cmd sysdat.con

%.h86: %.a86
	$(DRASM86) $^
	
%.h86: %.a86
	$(DRASM86) $^

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn *.con *.cmd

