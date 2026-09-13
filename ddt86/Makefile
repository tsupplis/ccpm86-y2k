INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LINK86=intel_link86
INTEL_LOC86=intel_loc86
INTEL_OH86=intel_oh86
CPM_ASM86=cpm_asm86
CPM_GENCMD=cpm_gencmd
CPM_RASM86=pcdev_rasm86
CPM_PLI86=drpli_pc
CPM_PLILINK86=drpli_link
CPM_DRC86=drccpm_cc
CPM_DRCLINK86=drccpm_link

TOOLS=gentab.cmd ddt86.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

# Final ddt86.cmd: pip-concatenate the three h86 files then gencmd (cpm section)
ddt86.cmd: ddt86m.h86
	$(CPM_GENCMD) $< 8080 'code[m366]'
	mv ddt86m.cmd ddt86.cmd

ddt86m.h86: ddt86.h86 dis86.h86 ass86.h86
	doscat $^ > $@

# ddt86 base: cpm section — asm86 ddt86 $$fi
ddt86.h86: ddt86.a86
	$(CPM_ASM86) ddt86 '$$fi'

# dis86 overlay (is14 section): plm86+asm86 -> link86+plm86.lib -> loc86 -> oh86
dis86.obj: dis86.plm
	$(INTEL_PLM86) $^ debug 'pagewidth(100)' xref

ins86.obj: ins86.plm
	$(INTEL_PLM86) $^ debug 'pagewidth(100)' xref

dislnk86.obj: dislnk86.asm
	$(INTEL_ASM86) $^ debug

dis86.lnk: dislnk86.obj dis86.obj ins86.obj
	$(INTEL_LINK86) dislnk86.obj, dis86.obj, ins86.obj to $@

dis86.abs: dis86.lnk
	$(INTEL_LOC86) $< to $@ 'ad(sm(dats(0),code(0h)))' 'od(sm(dats,code,const,data))'

dis86.h86: dis86.abs
	$(INTEL_OH86) $< to $@

# ass86 overlay (is14 section): plm86+asm86 -> link86+po(dis86.abs)+plm86.lib -> loc86 -> oh86
ass86.obj: ass86.plm
	$(INTEL_PLM86) $^ debug 'pagewidth(100)' xref

asmtab.obj: asmtab.plm
	$(INTEL_PLM86) $^ debug 'pagewidth(100)' xref

asslnk86.obj: asslnk86.asm
	$(INTEL_ASM86) $^ debug

ass86.lnk: asslnk86.obj asmtab.obj ass86.obj dis86.abs
	$(INTEL_LINK86) asslnk86.obj, asmtab.obj, ass86.obj, 'po(dis86.abs)' to $@

ass86.abs: ass86.lnk
	$(INTEL_LOC86) $< to $@ 'ad(sm(dats(0),code(0h)))' 'od(sm(dats,code,const,data))'

ass86.h86: ass86.abs
	$(INTEL_OH86) $< to $@

op86.dat: gentab.cmd

gentab.cmd: gentab.obj
	$(CPM_PLILINK86) gentab=gentab '[data[ori[0]]'

cpmif.obj: cpmif.a86
	$(CPM_RASM86) $^

%.obj: %.a86
	$(INTEL_ASM86) $^

%.obj: %.c
	$(CPM_DRC86) -o$@ $^

%.obj: %.pli
	$(CPM_PLI86) $^ \$$d

%.obj: %.plm
	$(INTEL_PLM86) $^ debug 'optimize(2)' 'pagewidth(100)' xref

clean:
	rm -f *.sys *.cmd *.obj *.exe *.map *.h86 *.abs *.lnk *.sym 
	rm -f *.lin *.lst *.prn ccpmtest.img ddt86m.h86 *.abs *.lnk *.mp1

ccpmtest.img: $(TOOLS) Makefile startup.0
	cp cpmbase.img ccpmtest.img
	cpmcp -f ibmpc-514ss ccpmtest.img ddt86.cmd 0:
	cpmcp -f ibmpc-514ss ccpmtest.img startup.0 0:
	cpmls -F -f ibmpc-514ss ccpmtest.img 0:*.*


test: ccpmtest $(TOOLS)

ccpmtest: ccpmtest.img $(TOOLS)
	@./ccpm86
