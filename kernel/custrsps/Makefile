ASM86=cpm86_asm86
GENCMD=cpm86_gencmd
.PHONY: all clean
all: cls.rsp sysdisk.rsp
%.rsp: %.h86
	$(GENCMD) $<
	mv $*.cmd $@
	cmdinfo $@
%.h86: %.a86
	$(ASM86) $<
clean:
	rm -f *.rsp *.cmd *.h86 *.lst *.sym
