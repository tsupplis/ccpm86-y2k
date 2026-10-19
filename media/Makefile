PYTHON ?= python3
FORMAT = ccpm86-320
TWIST = $(PYTHON) ../tools/cpm86twist.py
CPMCP = cpmcp -f $(FORMAT)

.PHONY: all prepare test check check-media check-qemu clean
.DELETE_ON_ERROR:
IMAGES = ccpm86.img ccpmtools.img ccpmdev.img ccpmmaint.img
all: $(IMAGES)

SYSTEM_COMMANDS = ../commands/chset/chset.cmd ../commands/date/date.cmd \
    ../commands/era/era.cmd ../commands/era/eraq.cmd \
    ../commands/help/help.cmd ../commands/help/help.hlp \
    ../commands/ed/ed.cmd ../commands/ren/ren.cmd \
    ../commands/sdir/sdir.cmd ../commands/set/set.cmd ../commands/show/show.cmd \
    ../commands/submit/submit.cmd ../commands/systat/systat.cmd \
    ../commands/tod/tod.cmd ../commands/type/type.cmd \
    ../commands/vcmode/vcmode.cmd ../commands/sysdisk/sysdisk.cmd \
    ../commands/setdef/setdef.cmd

# SYSDISK is source-built; other base files are selected explicitly.
BASE_SYSTEM_COMMANDS = ../base/config.cmd \
    ../base/function.cmd ../base/pcetime.cmd ../base/setup.cmd

WINDOW_COMMANDS = ../base/window.cmd ../base/wmenu.cmd
WINDOW_SAMPLES = ../base/carddeck.sub ../base/clear.sub \
    ../base/glass.sub ../base/wordproc.sub

TOOLS_COMMANDS = ../commands/asm86/asm86.cmd ../commands/ddt86/ddt86.cmd \
    ../commands/gencmd/gencmd.cmd ../commands/genccpm/genccpm.cmd \
    ../commands/tod/tod.cmd ../commands/dredix/dredix.cmd \
    $(WINDOW_COMMANDS) $(WINDOW_SAMPLES)

MAINT_COMMANDS = ../base/dskmaint.cmd ../base/hdmaint.cmd ../commands/initdir/initdir.cmd \
    ../commands/tod/tod.cmd

# Data-only developer disk; omit the older MBASIC 5.22 reference binary.
DEV_COMMANDS = ../dev/rasm86.cmd ../dev/link86.cmd ../dev/lib86.cmd \
    ../dev/xref86.cmd ../dev/sid86.cmd ../dev/cbas86.cmd ../dev/crun86.cmd \
    ../dev/pbasic.cmd ../dev/mbasic86.cmd ../commands/asm80/asm80.cmd \
    ../commands/hexcom/hexcom.cmd ../commands/ddt86/gentab.cmd

COMMAND_DIRS = chset date era help initdir pip ren sdir set show submit systat tod \
    type vcmode asm86 asm80 ddt86 gencmd genccpm hexcom dredix ed sysdisk setdef

# Recurse so building directly in media/ picks up changes to included sources.
prepare:
	@set -e; for d in $(COMMAND_DIRS); do $(MAKE) -C ../commands/$$d all; done
	$(MAKE) -C ../commands/help help.hlp
	$(MAKE) -C ../commands/sysdisk check
	$(MAKE) -C ../kernel/con all
	$(MAKE) -C ../kernel/rsp1 all
	$(MAKE) -C ../kernel/rsp2 all
	$(MAKE) -C ../kernel/sysgen all
	$(MAKE) -C ../boot check

# prepare also builds and checks the source boot track for a direct media build.
../boot/boottrack.bin: prepare
	@test -f $@

# Keep the empty filesystem template, but always install the source-built boot.
ccpm86base.img: base.img ../boot/boottrack.bin diskdefs ../tools/cpm86twist.py Makefile prepare
	cp base.img $@
	dd if=../boot/boottrack.bin of=$@ bs=4096 count=1 conv=notrunc 2>/dev/null

ccpm86.img: ccpm86base.img startup.0 $(BASE_SYSTEM_COMMANDS) Makefile prepare
	$(TWIST) untwist ccpm86base.img $@.logical.tmp
	$(CPMCP) $@.logical.tmp ../kernel/sysgen/ccpm.sys startup.0 $(SYSTEM_COMMANDS) $(BASE_SYSTEM_COMMANDS) 0:
	$(CPMCP) $@.logical.tmp ../commands/pip/pip4.cmd 0:PIP.CMD
	cpmls -F -f $(FORMAT) $@.logical.tmp '0:*.*'
	$(TWIST) twist $@.logical.tmp $@.tmp
	mv $@.tmp $@
	$(RM) $@.logical.tmp

ccpmtools.img: ccpm86base.img startup.0 Makefile prepare
	$(TWIST) untwist ccpm86base.img $@.logical.tmp
	$(CPMCP) $@.logical.tmp ../kernel/sysgen/ccpm.sys startup.0 $(TOOLS_COMMANDS) 0:
	$(CPMCP) $@.logical.tmp ../commands/pip/pip4.cmd 0:PIP.CMD
	cpmls -F -f $(FORMAT) $@.logical.tmp '0:*.*'
	$(TWIST) twist $@.logical.tmp $@.tmp
	mv $@.tmp $@
	$(RM) $@.logical.tmp

ccpmdev.img: ccpm86base.img $(DEV_COMMANDS) Makefile prepare
	$(TWIST) untwist ccpm86base.img $@.logical.tmp
	$(CPMCP) $@.logical.tmp $(DEV_COMMANDS) 0:
	cpmls -F -f $(FORMAT) $@.logical.tmp '0:*.*'
	$(TWIST) twist $@.logical.tmp $@.tmp
	mv $@.tmp $@
	$(RM) $@.logical.tmp

ccpmmaint.img: ccpm86base.img startup.0 $(MAINT_COMMANDS) Makefile prepare
	$(TWIST) untwist ccpm86base.img $@.logical.tmp
	$(CPMCP) $@.logical.tmp ../kernel/sysgen/ccpm.sys startup.0 $(MAINT_COMMANDS) 0:
	$(CPMCP) $@.logical.tmp ../commands/pip/pip4.cmd 0:PIP.CMD
	cpmls -F -f $(FORMAT) $@.logical.tmp '0:*.*'
	$(TWIST) twist $@.logical.tmp $@.tmp
	mv $@.tmp $@
	$(RM) $@.logical.tmp

ccpmtest.img: ccpm86base.img
	cp $< $@

# Check every packaged file, allowing only CP/M record padding after its bytes.
check-media: all
	@set -e; for image in $(IMAGES); do \
	    $(TWIST) untwist $$image $$image.verify.tmp; \
	    case $$image in \
	      ccpm86.img) sources='../kernel/sysgen/ccpm.sys startup.0 $(SYSTEM_COMMANDS) $(BASE_SYSTEM_COMMANDS)' ;; \
	      ccpmtools.img) sources='../kernel/sysgen/ccpm.sys startup.0 $(TOOLS_COMMANDS)' ;; \
	      ccpmdev.img) sources='$(DEV_COMMANDS)' ;; \
	      ccpmmaint.img) sources='../kernel/sysgen/ccpm.sys startup.0 $(MAINT_COMMANDS)' ;; \
	    esac; \
	    for source in $$sources; do \
	      file=$$(basename $$source); \
	      $(CPMCP) $$image.verify.tmp 0:$$file $$image.file.tmp; \
	      head -c "$$(wc -c < $$source)" $$image.file.tmp | cmp $$source -; \
	      $(RM) $$image.file.tmp; \
	    done; \
	    if test $$image != ccpmdev.img; then \
	      $(CPMCP) $$image.verify.tmp 0:PIP.CMD $$image.file.tmp; \
	      cmp ../commands/pip/pip4.cmd $$image.file.tmp; \
	    fi; \
	    dd if=$$image of=$$image.boot.tmp bs=4096 count=1 2>/dev/null; \
	    cmp $$image.boot.tmp ../boot/boottrack.bin; \
	    test "$$(wc -c < $$image)" -eq 327680; \
	    $(RM) $$image.verify.tmp $$image.file.tmp $$image.boot.tmp; \
	    echo "$$image: all payloads, boot track and size verified"; \
	done

check: test

check-qemu: check-media
	$(PYTHON) ../tools/boot_check.py

test: ccpm86.img ccpmtools.img
	./ccpm86

clean:
	$(RM) ccpm.sys ccpm86base.img $(IMAGES) ccpmtest.img *.img.*.tmp *.img.tmp
