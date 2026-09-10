SUBDIRS=tools asm ddt86 xios31

DISTDIR=dist
DISTZIP=dist.zip
Y2KDISTZIP=y2k-dist.zip

all clean:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@; \
	done

dist:
	rm -rf $(DISTDIR) $(DISTZIP)
	mkdir -p $(DISTDIR)
	@for d in $(SUBDIRS); do \
		find $$d -maxdepth 1 -name '*.cmd' -exec cp {} $(DISTDIR)/ \; ; \
	done
	COPYFILE_DISABLE=1 zip -X -j $(DISTZIP) $(DISTDIR)/*.cmd
	COPYFILE_DISABLE=1 zip -X -j $(Y2KDISTZIP) $(DISTDIR)/sdir.cmd \
	 $(DISTDIR)/tod.cmd \
	 $(DISTDIR)/date.cmd \
	 $(DISTDIR)/show.cmd \


.PHONY: all clean dist $(SUBDIRS)
