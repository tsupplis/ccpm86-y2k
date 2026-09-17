RASM86=pcdev_rasm86
DRC86=drccpm_cc
DRCLINK86=drccpm_link

TOOLS=chset.cmd

SNIPPETS=

all: $(TOOLS) $(SNIPPETS)

chset.cmd: main.obj chset.obj function.obj cpmif.obj
	$(DRCLINK86) chset[map]=main,chset,function,cpmif

cpmif.obj: cpmif.a86
	$(RASM86) $^

%.obj: %.c
	$(DRC86) -o$@ $^
clean:
	rm -f *.sys *.loc *.cmd *.obj *.h86 *.sym *.lin *.lst *.prn ccpmtest.img
	rm -f *.loc *.mp? *.lnk *.map
