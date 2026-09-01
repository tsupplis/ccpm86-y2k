PLM86=intel_plm86
ASM86=intel_asm86
DRASM86=zxasm86
GENCMD=zxgencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd

TOOLS=date.cmd era.cmd eraq.cmd gencmd.cmd ed.cmd help.cmd show.cmd \
    set.cmd dir.cmd tod.cmd type.cmd pip3.cmd pip4.cmd sdir.cmd ren.cmd \
    submit.cmd vcmode.cmd systat.cmd tcopy.cmd 

SNIPPETS=boot.h86 load.h86 lbdos.h86  lbdos3.sys

all: $(TOOLS) $(SNIPPETS)

tcopy.cmd: tcopy.h86
	$(GENCMD) $<
	
submit.cmd: submit.h86
	$(GENCMD) $<
	
sdir.cmd: sdir.obj scd.obj search.obj scan.obj sort.obj disp.obj util.obj \
    timest.obj dpb.obj
	$(LINK86) sdir=scd,sdir,search,sort,disp,scan,timest,dpb,util '[data[ori[0]]'

lbdos3.sys: lbdos3.a86
	$(RASM86) lbdos3.a86
	$(LINK86) 'lbdos3.sys=lbdos3[data[ori[0],max[0]]]'

pip4.cmd: pip4.obj inpout.obj scd1.obj 
	$(LINK86) pip4=scd1,inpout,pip4 '[data[ori[0],max[1000],add[200]]]'

pip3.cmd: pip3.obj inpout.obj scd1.obj 
	$(LINK86) pip3=scd1,inpout,pip3 '[data[ori[0],max[1000],add[200] ]'

systat.cmd: systat.obj scd.obj
	$(LINK86) systat=scd,systat '[data[ori[0]]'

vcmode.cmd: vcmode.obj scd.obj
	$(LINK86) vcmode=scd,vcmode '[data[ori[0]]'

ren.cmd: ren.obj scd.obj
	$(LINK86) ren=scd,ren '[data[ori[0]]'

type.cmd: type.obj scd.obj
	$(LINK86) type=scd,type '[data[ori[0]]'

tod.cmd: tod.obj scd.obj
	$(LINK86) tod=scd,tod '[data[ori[0]]'

help.cmd: help.obj scd.obj
	$(LINK86) help=scd,help '[dat[ori[0]]'

dir.cmd: dir.obj scd.obj
	$(LINK86) dir=scd,dir '[dat[ori[0]]'

set.cmd: set.obj scd.obj
	$(LINK86) set=scd,set '[dat[ori[0]]'

show.cmd: show.obj scd.obj
	$(LINK86) show=scd,show '[dat[ori[0]]'

ed.cmd: ed.obj scd.obj
	$(LINK86) ed=scd,ed '[dat[ori[0]]'

eraq.cmd: eraq.obj scd.obj
	$(LINK86) eraq=scd,eraq '[dat[ori[0]]'

era.cmd: era.obj scd.obj
	$(LINK86) era=scd,era '[dat[ori[0]]'

date.cmd: date.obj scd.obj
	$(LINK86) date=scd,date '[dat[ori[0]]'

gencmd.cmd: gencmd.obj scd.obj
	$(LINK86) gencmd=scd,gencmd '[dat[ori[0]]'

%.h86: %.a86
	$(DRASM86) $^

%.obj: %.a86
	$(ASM86) $^

%.obj: %.plm
	$(PLM86) $^ debug 'optimize(3)' xref

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lin *.lst ccpmtest.img

ccpmtest.img: $(TOOLS) Makefile startup.0
	cp cpmbase.img ccpmtest.img
	cpmcp -f ibmpc-514ss ccpmtest.img $(TOOLS) 0:
	cpmcp -f ibmpc-514ss ccpmtest.img startup.0 0:
	cpmls -F -f ibmpc-514ss ccpmtest.img 0:*.*


test: ccpmtest $(TOOLS)

ccpmtest: ccpmtest.img $(TOOLS)
	@./ccpm86

