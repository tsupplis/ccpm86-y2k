PYTHON ?= python3
ASM86 = cpm86_asm86
H862BIN = $(PYTHON) ../tools/h862bin.py

.PHONY: all check clean
.DELETE_ON_ERROR:
all: boottrack.bin

%.h86: %.a86
	$(ASM86) $<

%.bin: %.h86 ../tools/h862bin.py
	$(H862BIN) $< $@

# The working 320 KB disk's reserved track, built entirely from source:
# E5 filler, boot code at 000h, sides marker at 1FFh, loader at 900h.
boottrack.bin: bootsect.bin loader.bin Makefile
	@test "$$(wc -c < bootsect.bin)" -eq 147
	@test "$$(wc -c < loader.bin)" -eq 846
	$(PYTHON) -c "import sys; sys.stdout.buffer.write(b'\xe5' * 4096)" > $@.tmp
	dd if=bootsect.bin of=$@.tmp bs=1 seek=0 conv=notrunc 2>/dev/null
	printf '\001' | dd of=$@.tmp bs=1 seek=511 conv=notrunc 2>/dev/null
	dd if=loader.bin of=$@.tmp bs=1 seek=2304 conv=notrunc 2>/dev/null
	mv $@.tmp $@

# base.img remains the independent, original reference, not an assembly input.
check: boottrack.bin ../media/base.img
	@test "$$(wc -c < boottrack.bin)" -eq 4096
	@dd if=../media/base.img bs=4096 count=1 2>/dev/null | cmp boottrack.bin -
	@echo 'Boot track: all 4096 bytes match the original'

clean:
	$(RM) *.bin *.h86 *.sym *.lst boottrack.bin.tmp
