#!/usr/bin/env python3
"""Read CCP/M 3.1 memory accounting from a paused physical-memory snapshot.

Offsets follow kernel/con/sd.lit and the generated mem.lst. Descriptor and SAT
layouts follow mem.def and mau.mem. No guest calls or memory writes are used.
"""
def accounting(memory, data_segment):
    base = data_segment * 16
    def word(address):
        if not 0 <= address <= len(memory) - 2:
            raise ValueError('Memory pointer outside captured RAM')
        return int.from_bytes(memory[address:address + 2], 'little')
    def chain(head):
        node, seen = word(base + head), set()
        while node:
            if node in seen or node > 0xfff6:
                raise ValueError('Invalid/cyclic memory descriptor list')
            seen.add(node)
            yield base + node
            node = word(base + node)
    free, allocated = [], []
    for node in chain(0x5a):  # Memory Free List: paragraph regions
        free.append((word(node + 2), word(node + 4)))
    for node in chain(0x76):  # Memory Allocation List: MAU SATs
        start, length = word(node + 2), word(node + 4)
        sat = start * 16
        if not 0 <= sat < len(memory):
            raise ValueError('SAT outside captured RAM')
        count = memory[sat]
        for index in range(1, count + 1):
            entry = sat + index * 5
            address, size = word(entry), word(entry + 2)
            if entry + 4 >= len(memory):
                raise ValueError('Truncated SAT')
            if address == 0:  # zero start terminates used SAT slots
                break
            references = memory[entry + 4]
            if address < start or address + size > start + length:
                raise ValueError('SAT region outside its MAU')
            if size:
                (allocated if references else free).append((address, size))
    return dict(free_paragraphs=sum(n for _, n in free),
                allocated_paragraphs=sum(n for _, n in allocated),
                unused_descriptors=sum(1 for _ in chain(0x58)),
                free_regions=sorted(free), allocated_regions=sorted(allocated))
