ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd
H862BIN=h862bin
BIN2ASM=bin2asm

TOOLS=dskmaint.cmd dskmorg.cmd dform.cmd
IMAGES=bootsect.bin loader.bin
INCS=bootsect.inc loader.inc
REF=../../base/dskmaint.cmd

all: $(TOOLS) $(IMAGES)

dskmaint.h86: dskmaint.a86 $(INCS)
dskmorg.h86: dskmorg.a86 $(INCS)

# GENCMD creates all four historical groups, including the empty EXTRA
# descriptor and the initialized 256-byte STACK. XFFFF is in paragraphs.
dskmaint.cmd dskmorg.cmd: CMDOPTS=extra[m200 xffff]

dskmaint.cmd: dskmaint.h86 Makefile
	$(GENCMD) dskmaint '$(CMDOPTS)'
	cmdinfo $@
dskmorg.cmd: dskmorg.h86 Makefile
	$(GENCMD) dskmorg '$(CMDOPTS)'
	cmdinfo $@

%.h86: %.a86
	unix2dos $<
	rm -f $@
	$(ASM86) $*
	@test -f $@

%.bin: %.h86
	$(H862BIN) $< $@

# bin2asm is already provided on PATH; labels belong to the including file.
%.inc: %.bin
	$(BIN2ASM) -m -f $@ $<
	unix2dos $@

dform.obj: dform.a86 ccpmio.a86 $(INCS)
	unix2dos $<
	rm -f $@
	@$(RASM86) $< $$ pz sz > dform.log 2>&1; cat dform.log
	@grep -qE 'Number of errors: +0\.' dform.log || { rm -f $@; exit 1; }
dform.cmd: dform.obj Makefile
	$(LINK86) dform '[$$sz,cod[ori[0]],dat[ori[0],max[0]]]'
	@python3 -c "p='$@';b=open(p,'rb').read();open(p,'wb').write(b[:128+int.from_bytes(b[1:3],'little')*16])"
	cmdinfo $@

check: check-dsk check-dform
check-dsk: dskmaint.cmd dskmorg.cmd $(IMAGES)
	cmp dskmaint.cmd $(REF)
	cmp dskmorg.cmd $(REF)
	@echo 'PASS: CCP/M DSKMAINT matches all groups, headers and padding'
check-dform: dform.cmd $(IMAGES)
	python3 ../../tools/dfmtest.py

clean:
	rm -f *.cmd *.obj *.h86 *.lst *.sym *.log *.bin.boot *.mp? *.lnk
	rm -f $(IMAGES) $(INCS)
.PHONY: all check check-dsk check-dform clean
