SUBDIRS=con rsp1 rsp2

DISTDIR=dist
DISTZIP=dist.zip
Y2KDISTZIP=y2k-dist.zip

all clean:
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@; \
	done

.PHONY: all clean dist $(SUBDIRS)
