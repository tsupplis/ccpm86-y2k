SUBDIRS=con rsp1 rsp2 rsp3 sysgen

DISTDIR=dist
DISTZIP=dist.zip
Y2KDISTZIP=y2k-dist.zip

all clean:
	set -e; for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@; \
	done

.PHONY: all clean dist $(SUBDIRS)
