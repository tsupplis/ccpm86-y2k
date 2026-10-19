DRCC=drccpm_cc
DRLINK=drccpm_link
CMDINFO=cmdinfo

TOOLS=dredix.cmd

.DELETE_ON_ERROR:
.PHONY: all clean

all: $(TOOLS)

dredix.cmd: ed.obj common.obj ekey.obj ebuf.obj ecmdglb.obj ecmdio.obj epat.obj \
 emem.obj ecmdblk.obj ewv.obj ehelp.obj elesson.obj ewdv.obj ecmdsta.obj \
 vd.obj vk.obj dredix.inp Makefile
	$(DRLINK) '$(basename $@)[input]'
	$(CMDINFO) $@

$(patsubst %.c,%.obj,$(wildcard *.c)): $(wildcard *.h)

dredix.inp: Makefile
	printf '%s\r\n' 'dredix=ed,' 'common,' 'ekey,' 'ebuf,' 'ecmdglb,' \
	  'ecmdio,' 'epat,' 'emem,' 'ecmdblk,' 'ewv,' 'ehelp,' 'elesson,' \
	  'ewdv,' 'ecmdsta,' 'vd,' 'vk' > $@

%.obj: %.c
	$(DRCC) $<

clean:
	$(RM) -f *.obj *.lst *.sym *.cmd dredix.inp
