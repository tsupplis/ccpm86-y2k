
TOOLS=

SNIPPETS= ccpm.sys

CONS=bdos.con cio.con mem.con rtm.con sup.con sysdat.con xios.con
RSPS=abort.rsp clock.rsp dir.rsp echo.rsp pin.rsp serin.rsp serout.rsp tmp.rsp vout.rsp

all: $(TOOLS) $(SNIPPETS)


bdos.con: ../con/bdos.con
	@cp $< .

cio.con: ../con/cio.con
	@cp $< .

mem.con: ../con/mem.con
	@cp $< .

rtm.con: ../con/rtm.con
	@cp $< .

sup.con: ../con/sup.con
	@cp $< .

sysdat.con: ../con/sysdat.con .
	@cp $< .

xios.con: ../con/xios.con .
	@cp $< .

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

ccpm.sys: genccpm.in $(CONS) $(RSPS)
	@emu2 ../../commands/genccpm/genccpm.cmd < genccpm.in
	@cmdinfo ccpm.sys

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
