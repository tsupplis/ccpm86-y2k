DRC86=drccpm_cc
DRCLINK86=drccpm_link

TOOLS=genccpm.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

genccpm.cmd: genc1.obj genc2.obj genc3.obj genc4.obj genc5.obj genc6.obj
	$(DRCLINK86) genccpm[map]=genc1,genc2,genc3,genc4,genc5,genc6 [libsyms]

%.h86: %.a86
	$(DRASM86) $^

%.obj: %.a86
	$(ASM86) $^

%.obj: %.c
	$(DRC86) -o$@ $^

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
