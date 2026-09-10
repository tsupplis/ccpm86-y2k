SUBDIRS=tools asm xios31

DISTDIR=dist
DISTZIP=dist.zip

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
	rm -rf $(DISTDIR)

.PHONY: all clean dist $(SUBDIRS)
