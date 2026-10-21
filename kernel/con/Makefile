ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd

SNIPPETS=bdos.con sup.con rtm.con cio.con sysdat.con xios.con mem.con

all: $(SNIPPETS)

# Included assembly sources must invalidate their parent object.
xios.h86: xbegin.lib xchar.lib xconout.lib xfloppy.lib xhard.lib xrtc.lib xinit.lib \
  pcmode.lib xpcmode.lib
bdos.obj: $(wildcard *.bdo) uda.cic xioscb.def
sup.h86: $(wildcard *.sup) patch.cod
rtm.h86: $(wildcard *.rtm) s8087.lib
cio.h86: $(wildcard *.cio) console.cio chario.cio uda.cic
mem.h86: $(wildcard *.mem)
sysdat.h86: sysdat.dat

bdos.con: bdos.obj
	$(LINK86) $@=$< '[data[origin[0]]]'
	@cmdinfo $@

xios.con: xios.h86
	$(GENCMD) $< 8080
	mv $(basename $@).cmd $@
	@cmdinfo $@

%.h86: %.loc
	$(INTEL_OH86) $^
	mv $*.hex $*.h86

%.con: %.h86
	$(GENCMD) $^
	mv $(basename $@).cmd $@
	@cmdinfo $@

%.h86: %.a86
	$(ASM86) $<
	
%.obj: %.a86
	$(RASM86) $<

clean:
	rm -f *.obj *.h86 *.sym *.lst *.con *.cmd *.loc *.lnk *.lst

