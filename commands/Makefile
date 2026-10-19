SUBDIRS=tod asm86 ddt86 submit chset initdir pip era sdir ed \
    set show date gencmd dir ren vcmode type help systat \
    genccpm asm80 hexcom dredix sysdisk setdef

all clean:
	set -e; for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@; \
	done

.PHONY: all clean dist $(SUBDIRS)
