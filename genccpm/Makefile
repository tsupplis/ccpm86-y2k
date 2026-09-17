DRC86=drccpm_cc
DRCLINK86=drccpm_link

TOOLS=genccpm.cmd

SNIPPETS= ccpm.sys

CONS=bdos.con cio.con mem.con rtm.con sup.con sysdat.con xios.con
RSPS=abort.rsp clock.rsp dir.rsp echo.rsp pin.rsp serin.rsp serout.rsp tmp.rsp vout.rsp

all: $(TOOLS) $(SNIPPETS)


bdos.con: 
	@cp ../kernel/bdos.con .

cio.con: 
	@cp ../kernel/cio.con .

mem.con: 
	@cp ../kernel/mem.con .

rtm.con: 
	@cp ../kernel/rtm.con .

sup.con: 
	@cp ../kernel/sup.con .

sysdat.con: 
	@cp ../kernel/sysdat.con .

xios.con: 
	@cp ../kernel/xios.con .

abort.rsp:
	@cp ../rsp*/abort.rsp .

clock.rsp:
	@cp ../rsp*/clock.rsp .

dir.rsp:
	@cp ../rsp*/dir.rsp .

echo.rsp:
	@cp ../rsp*/echo.rsp .

pin.rsp:
	@cp ../rsp*/pin.rsp .

serin.rsp:
	@cp ../rsp*/serin.rsp .

serout.rsp:
	@cp ../rsp*/serout.rsp .

tmp.rsp:
	@cp ../rsp*/tmp.rsp .

vout.rsp:
	@cp ../rsp*/vout.rsp .

ccpm.sys: genccpm.cmd genccpm.in $(CONS) $(RSPS)
	@cp ../kernel/*.con .
	#@cp ../rsp*/*.rsp .
	@emu2 genccpm.cmd < genccpm.in
	@cmdinfo ccpm.sys

genccpm.cmd: genc1.obj genc2.obj genc3.obj genc4.obj genc5.obj genc6.obj
	$(DRCLINK86) genccpm[map]=genc1,genc2,genc3,genc4,genc5,genc6 [libsyms]

%.h86: %.a86
	$(DRASM86) $^

%.obj: %.a86
	$(ASM86) $^

%.obj: %.c
	$(DRC86) -o$@ $^

ccpm86.img: ccpm.sys ccpm86base.img
	cp ccpm86base.img ccpm86.img
	cpmcp -f ibmpc-514ss ccpm86.img ccpm.sys 0:
	#cpmcp -f ibmpc-514ss ccpm86.img startup.0 0:
	cpmls -F -f ibmpc-514ss ccpm86.img 0:*.*

test: ccpmtest

ccpmtest: ccpm86.img 
	@./ccpm86

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn 
	rm -f *.con *.rsp
	rm -f ccpm86.img ccpmtest.img
