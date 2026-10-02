
TOOLS=

all: ccpm86.img


ccpm.sys: ../kernel/sysgen/ccpm.sys
	@cp $< .

ccpm86.img: ccpm.sys ccpm86base.img
	cp ccpm86base.img ccpm86.img
	cpmcp -f ibmpc-514ss ccpm86.img ccpm.sys 0:
	cpmcp -f ibmpc-514ss ccpm86.img startup.0 0:
	cpmls -F -f ibmpc-514ss ccpm86.img 0:*.*

ccpmtest.img: ccpm86base.img
	cp ccpm86base.img ccpmtest.img

test: ccpmtest

ccpmtest: ccpm86.img ccpmtest.img
	@./ccpm86

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn 
	rm -f *.con *.rsp
	rm -f ccpm86.img ccpmtest.img
