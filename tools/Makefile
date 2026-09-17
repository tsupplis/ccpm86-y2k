INTEL_PLM86=intel_plm86
INTEL_ASM86=intel_asm86
INTEL_LOC86=intel_loc86
INTEL_LINK86=intel_link86
INTEL_OH86=intel_oh86
ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd
PLI86=drpli_pc
PLILINK86=drpli_link
DRC86=drccpm_cc
DRCLINK86=drccpm_link

TOOLS=date.cmd era.cmd eraq.cmd gencmd.cmd ed.cmd help.cmd show.cmd \
    set.cmd dir.cmd tod.cmd type.cmd pip3.cmd pip4.cmd sdir.cmd ren.cmd \
    submit.cmd vcmode.cmd systat.cmd chset.cmd initdir.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

submit.cmd: submit.h86
	$(GENCMD) $<
	cmdinfo $@

initdir.cmd: initdir.obj initdira.obj diomod.obj
	$(PLILINK86) initdir [locals],initdira,diomod
	cmdinfo $@

diomod.obj: diomod.a86
	$(RASM86) $^

initdira.obj: initdira.a86
	$(RASM86) $^

sdir.cmd: sdir.h86
	$(GENCMD) $< 'data[b1000 m3c5 x800]'
	cmdinfo $@

sdir.loc: scd.obj sdir.obj search.obj scan.obj sort.obj disp.obj util.obj \
    timest.obj dpb.obj
	$(INTEL_LINK86) scd.obj, search.obj, scan.obj, sort.obj, disp.obj, \
	util.obj, timest.obj, dpb.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

pip4.cmd: pip4.h86
	$(GENCMD) $< 'data[b1000 m280 xfff]'
	cmdinfo $@

pip4.loc: pip4.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

pip3.cmd: pip3.h86
	$(GENCMD) $< 'data[b1000 m280 xfff]'
	cmdinfo $@

pip3.loc: pip3.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

systat.cmd: systat.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

systat.loc: systat.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

vcmode.cmd: vcmode.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

vcmode.loc: vcmode.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

ren.cmd: ren.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

ren.loc: ren.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

chset.cmd: main.obj chset.obj function.obj cpmif.obj
	$(DRCLINK86) chset[map]=main,chset,function,cpmif

cpmif.obj: cpmif.a86
	$(RASM86) $^

type.cmd: type.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

type.loc: type.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

tod.cmd: tod.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

tod.loc: tod.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

help.cmd: help.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

help.loc: help.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

dir.cmd: dir.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

dir.loc: dir.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

set.cmd: set.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

set.loc: set.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

show.cmd: show.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

show.loc: show.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

eraq.cmd: eraq.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

eraq.loc: eraq.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

era.cmd: era.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

era.loc: era.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

date.cmd: date.h86
	$(GENCMD) $< 'data[b1000]'
	cmdinfo $@

date.loc: date.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

ed.cmd: ed.h86
	$(GENCMD) $< 'data[b1000 m80 xfff]'
	cmdinfo $@

ed.loc: ed.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

gencmd.cmd: gencmd.h86
	$(GENCMD) $< 'data[b1000 m86 xfff]'
	cmdinfo $@

gencmd.loc: gencmd.obj scd.obj
	$(INTEL_LINK86) scd.obj, $(basename $@).obj to $(basename $@).lnk
	$(INTEL_LOC86) $(basename $@).lnk 'od(sm(code,dats,data,stack,const))' \
		'ad(sm(code(0),dats(10000h)))' 'ss(stack(+32))'  to $(basename $@).loc

%.h86: %.loc
	$(INTEL_OH86) $^
	mv $*.hex $*.h86

%.obj: %.a86
	$(INTEL_ASM86) $^

%.h86: %.a86
	$(ASM86) $^

%.obj: %.c
	$(DRC86) -o$@ $^

%.obj: %.pli
	$(PLI86) $^ \$$dl

%.obj: %.plm
	$(INTEL_PLM86) $^ debug 'optimize(3)' xref

clean:
	rm -f *.sys *.loc *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
	rm -f *.loc *.mp? *.lnk

ccpmtest.img: $(TOOLS) Makefile startup.0
	cp cpmbase.img ccpmtest.img
	cpmcp -f ibmpc-514ss ccpmtest.img *.cmd 0:
	cpmcp -f ibmpc-514ss ccpmtest.img startup.0 0:
	cpmls -F -f ibmpc-514ss ccpmtest.img 0:*.*

test: ccpmtest $(TOOLS)

ccpmtest: ccpmtest.img $(TOOLS)
	@./ccpm86

