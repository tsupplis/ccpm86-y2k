SUBDIRS=tod asm86 ddt86 submit chset initdir pip era sdir ed \
    set show date gencmd dir ren vcmode type help systat \
    genccpm

DISTDIR=dist
DISTZIP=dist.zip
Y2KDISTZIP=y2k-dist.zip

all clean:
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@; \
	done

.PHONY: all clean dist $(SUBDIRS)
