PYTHON ?= python3
FORMAT = ccpm86-320
TWIST = $(PYTHON) ../tools/cpm86twist.py
CPMCP = cpmcp -f $(FORMAT)

.PHONY: all prepare test check check-media check-qemu clean
.DELETE_ON_ERROR:
all: ccpm86.img ccpmtools.img

SYSTEM_COMMANDS = ../commands/chset/chset.cmd ../commands/date/date.cmd \
    ../commands/era/era.cmd ../commands/era/eraq.cmd \
    ../commands/help/help.cmd ../commands/help/help.hlp \
    ../commands/initdir/initdir.cmd ../commands/ren/ren.cmd \
    ../commands/sdir/sdir.cmd ../commands/set/set.cmd ../commands/show/show.cmd \
    ../commands/submit/submit.cmd \
    ../commands/tod/tod.cmd ../commands/type/type.cmd \
    ../commands/vcmode/vcmode.cmd ../commands/dredix/dredix.cmd

BASE_SYSTEM_COMMANDS = ../base/config.cmd ../base/sysdisk.cmd
BASE_TOOLS_COMMANDS = $(filter-out $(BASE_SYSTEM_COMMANDS),$(wildcard ../base/*.cmd))

TOOLS_COMMANDS = ../commands/asm86/asm86.cmd ../commands/asm80/asm80.cmd \
    ../commands/ddt86/ddt86.cmd ../commands/ddt86/gentab.cmd \
    ../commands/gencmd/gencmd.cmd ../commands/genccpm/genccpm.cmd \
    ../commands/hexcom/hexcom.cmd ../commands/tod/tod.cmd ../boot/tcopy.cmd \
    ../commands/systat/systat.cmd

COMMAND_DIRS = chset date era help initdir pip ren sdir set show submit systat tod \
    type vcmode asm86 asm80 ddt86 gencmd genccpm hexcom dredix

# Recurse so building directly in media/ picks up changes to included sources.
prepare:
	@set -e; for d in $(COMMAND_DIRS); do $(MAKE) -C ../commands/$$d all; done
	$(MAKE) -C ../commands/help help.hlp
	$(MAKE) -C ../kernel/con all
	$(MAKE) -C ../kernel/rsp1 all
	$(MAKE) -C ../kernel/rsp2 all
	$(MAKE) -C ../kernel/sysgen all
	$(MAKE) -C ../boot tcopy.cmd

# base.img is the empty template with the original working boot track.
ccpm86base.img: base.img diskdefs ../tools/cpm86twist.py Makefile prepare
	cp base.img $@

ccpm86.img: ccpm86base.img startup.0 $(BASE_SYSTEM_COMMANDS) Makefile prepare
	$(TWIST) untwist ccpm86base.img $@.logical.tmp
	$(CPMCP) $@.logical.tmp ../kernel/sysgen/ccpm.sys startup.0 $(SYSTEM_COMMANDS) $(BASE_SYSTEM_COMMANDS) 0:
	$(CPMCP) $@.logical.tmp ../commands/pip/pip4.cmd 0:PIP.CMD
	cpmls -F -f $(FORMAT) $@.logical.tmp '0:*.*'
	$(TWIST) twist $@.logical.tmp $@.tmp
	mv $@.tmp $@
	$(RM) $@.logical.tmp

ccpmtools.img: ccpm86base.img startup.0 $(BASE_TOOLS_COMMANDS) Makefile prepare
	$(TWIST) untwist ccpm86base.img $@.logical.tmp
	$(CPMCP) $@.logical.tmp ../kernel/sysgen/ccpm.sys startup.0 $(TOOLS_COMMANDS) $(BASE_TOOLS_COMMANDS) 0:
	$(CPMCP) $@.logical.tmp ../commands/pip/pip4.cmd 0:PIP.CMD
	cpmls -F -f $(FORMAT) $@.logical.tmp '0:*.*'
	$(TWIST) twist $@.logical.tmp $@.tmp
	mv $@.tmp $@
	$(RM) $@.logical.tmp

ccpmtest.img: ccpm86base.img
	cp $< $@

# Verify the embedded kernel and boot track independently of image assembly.
check-media: all
	@set -e; for image in ccpm86.img ccpmtools.img; do \
	    $(TWIST) untwist $$image $$image.verify.tmp; \
	    $(CPMCP) $$image.verify.tmp 0:CCPM.SYS $$image.kernel.tmp; \
	    cmp ../kernel/sysgen/ccpm.sys $$image.kernel.tmp; \
	    dd if=$$image of=$$image.boot.tmp bs=4096 count=1 2>/dev/null; \
	    dd if=base.img of=$$image.baseboot.tmp bs=4096 count=1 2>/dev/null; \
	    cmp $$image.boot.tmp $$image.baseboot.tmp; \
	    test "$$(wc -c < $$image)" -eq 327680; \
	    $(RM) $$image.verify.tmp $$image.kernel.tmp $$image.boot.tmp $$image.baseboot.tmp; \
	    echo "$$image: kernel, boot track and size verified"; \
	done

check: test

check-qemu: check-media
	$(PYTHON) ../tools/boot_check.py

test: ccpm86.img ccpmtools.img
	./ccpm86

clean:
	$(RM) ccpm.sys ccpm86base.img ccpm86.img ccpmtools.img ccpmtest.img *.img.*.tmp *.img.tmp
