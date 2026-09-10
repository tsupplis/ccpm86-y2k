PLM86=intel_plm86
ASM86=intel_asm86
DRASM86=cpm_asm86
GENCMD=cpm_gencmd
RASM86=pcdev_rasm86
LINK86=pcdev_linkcmd
LIB86=pcdev_lib86

TOOLS=asm86.cmd

# c86lnk (CP/M entry stub) and mainp (PUBLIC ASM86 label it jumps to) are
# linked directly; everything else is resolved on demand from asm86.lib.
LIB_OBJS=brexpr.obj cm.obj cm2.obj cmac1.obj cmac2.obj cmac3.obj cmac4.obj \
	cmac5.obj cmsubr.obj dline.obj ermod.obj expr.obj files.obj global.obj \
	instr.obj io.obj mnem1.obj mnem2.obj mnem3.obj mnem4.obj outp.obj \
	predef.obj print.obj pseud1.obj pseud2.obj scan.obj subr1.obj subr2.obj \
	symb.obj text.obj

all: $(TOOLS)

asm86.cmd: c86lnk.obj mainp.obj asm86.lib
	$(LINK86) asm86=c86lnk,mainp,'asm86.lib[search,data[ori[0],max[1000],add[200]]]'

# LIB-86's command line can't hold this many modules, so drive it from a
# response file (see doc/plm.pdf sec. 8.5): LIB86 asm86 [INPUT] reads
# asm86.inp, whose last byte must be the CP/M EOF marker (Ctrl-Z, 1AH).
asm86.lib: asm86.inp $(LIB_OBJS)
	rm -f asm86.lib
	$(LIB86) asm86 [input]

asm86.inp:
	printf 'asm86.lib = brexpr,cm,cm2,cmac1,cmac2,cmac3,cmac4,cmac5,\n' > asm86.inp
	printf 'cmsubr,dline,ermod,expr,files,global,instr,io,\n' >> asm86.inp
	printf 'mnem1,mnem2,mnem3,mnem4,outp,predef,print,pseud1,\n' >> asm86.inp
	printf 'pseud2,scan,subr1,subr2,symb,text\n' >> asm86.inp
	unix2dos asm86.inp

%.h86: %.a86
	$(DRASM86) $^

%.obj: %.a86
	$(ASM86) $^

%.obj: %.plm
	$(PLM86) $^ debug 'optimize(3)' xref

clean:
	rm -f *.sys *.cmd *.obj *.h86 *.sym *.lin *.lst *.lib *.inp

