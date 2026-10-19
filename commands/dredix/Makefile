DRCC=drccpm_cc
DRLINK=drccpm_link
DRASM86=pcdev_rasm86
CMDINFO=cmdinfo

TOOLS=dredix.cmd

all: $(TOOLS)

dredix.cmd: ed.obj common.obj ekey.obj ebuf.obj ecmdglb.obj ecmdio.obj epat.obj \
 emem.obj ecmdblk.obj ewv.obj ehelp.obj elesson.obj ewdv.obj ecmdsta.obj \
 vd.obj vk.obj wmca.obj wwcall.obj util.obj wmca.obj wwcall.obj
	$(DRLINK) '$(basename $@)[input]'
	$(CMDINFO) $@
    
%.obj: %.c
	$(DRCC) $<

%.obj: %.a86
	$(DRASM86) $<

clean:
	$(RM) -f *.obj *.lst *.sym *.cmd
