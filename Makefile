DISTDIR=dist
Y2KDISTZIP=y2k-dist.zip
DISKSZIP=disks.zip
DIST_DISKS=ccpm86.img ccpmtools.img ccpmdev.img ccpmmaint.img
SOURCEDIST=$(DISTDIR)/ccpm86-source.tar.gz

.PHONY: all commands kernel boot media check check-qemu test dist source-dist clean

all: media

commands:
	$(MAKE) -C commands all

kernel: commands
	$(MAKE) -C kernel all

boot:
	$(MAKE) -C boot all

media: commands kernel boot
	$(MAKE) -C media all

check: all
	$(MAKE) -C boot check
	$(MAKE) -C commands/sysdisk check
	$(MAKE) -C media check-media

check-qemu: all
	$(MAKE) -C media check-qemu

test: all
	$(MAKE) -C media test

# Include tracked inputs and new sources, but no ignored build products.
# Skip deleted paths awaiting a commit.
source-dist:
	mkdir -p $(DISTDIR)
	@git ls-files --cached --others --exclude-standard | LC_ALL=C sort -u | \
	  while IFS= read -r source; do \
	    if test -f "$$source"; then \
	      printf '%s\n' "$$source"; \
	    fi; \
	  done > $(DISTDIR)/source-files.txt
	COPYFILE_DISABLE=1 tar -czf $(SOURCEDIST) -T $(DISTDIR)/source-files.txt

# Build first, then evaluate archive dependencies after the recursive builds.
# Validation is explicit through check/check-qemu, not part of packaging.
dist: all
	$(MAKE) $(DISKSZIP) $(Y2KDISTZIP)

# Release artifacts live at the repository root.
$(DIST_DISKS): %: media/%
	cp $< $@

$(DISKSZIP): $(DIST_DISKS) LICENSE.md Makefile
	$(RM) $@
	COPYFILE_DISABLE=1 zip -X -j $@ $(DIST_DISKS) LICENSE.md

$(Y2KDISTZIP): commands/sdir/sdir.cmd commands/tod/tod.cmd \
    commands/date/date.cmd commands/show/show.cmd LICENSE.md Makefile
	$(RM) $@
	COPYFILE_DISABLE=1 zip -X -j $@ commands/sdir/sdir.cmd \
	  commands/tod/tod.cmd commands/date/date.cmd commands/show/show.cmd LICENSE.md

clean:
	@set -e; for d in commands kernel boot media; do $(MAKE) -C $$d clean; done
	rm -rf $(DISTDIR) dist.zip $(Y2KDISTZIP) $(DISKSZIP) $(DIST_DISKS)
