DISTDIR=dist
DISTZIP=dist.zip
Y2KDISTZIP=y2k-dist.zip
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
	$(MAKE) -C commands/sysdisk check
	$(MAKE) -C media check-media

check-qemu: all
	$(MAKE) -C media check-qemu

test: all
	$(MAKE) -C media test

# Include tracked inputs and new sources, but no ignored build products.
# Skip deleted paths awaiting a commit and the legacy generated boot/track0.
source-dist:
	mkdir -p $(DISTDIR)
	@git ls-files --cached --others --exclude-standard | LC_ALL=C sort -u | \
	  while IFS= read -r source; do \
	    if test -f "$$source" && test "$$source" != boot/track0; then \
	      printf '%s\n' "$$source"; \
	    fi; \
	  done > $(DISTDIR)/source-files.txt
	COPYFILE_DISABLE=1 tar -czf $(SOURCEDIST) -T $(DISTDIR)/source-files.txt

# Keep source-built commands, reference inputs, and vendor tools distinct.
dist: check
	rm -rf $(DISTDIR)
	mkdir -p $(DISTDIR)/commands $(DISTDIR)/base $(DISTDIR)/dev $(DISTDIR)/media
	find commands -name '*.cmd' -exec cp {} $(DISTDIR)/commands/ \;
	cp boot/tcopy.cmd $(DISTDIR)/commands/
	cp commands/pip/pip4.cmd $(DISTDIR)/commands/pip.cmd
	cp commands/help/help.hlp $(DISTDIR)/commands/
	cp base/*.cmd $(DISTDIR)/base/
	cp dev/*.cmd dev/LICENSE.md dev/msbasic-patch.md $(DISTDIR)/dev/
	cp media/ccpm86.img media/ccpmtools.img media/ccpmdev.img $(DISTDIR)/media/
	cp README.md PATCHES.md LICENSE.md $(DISTDIR)/
	rm -f $(DISTZIP) $(Y2KDISTZIP)
	cd $(DISTDIR) && COPYFILE_DISABLE=1 zip -X -r ../$(DISTZIP) commands base dev media README.md PATCHES.md LICENSE.md
	COPYFILE_DISABLE=1 zip -X -j $(Y2KDISTZIP) commands/sdir/sdir.cmd \
	  commands/tod/tod.cmd commands/date/date.cmd commands/show/show.cmd
	$(MAKE) source-dist

clean:
	@set -e; for d in commands kernel boot media; do $(MAKE) -C $$d clean; done
	rm -rf $(DISTDIR) $(DISTZIP) $(Y2KDISTZIP)
