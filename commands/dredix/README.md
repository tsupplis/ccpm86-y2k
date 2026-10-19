# DREDIX on Concurrent CP/M-86 3.1

Build with `make`. The build generates its CRLF linker input file.
Only the BDOS keyboard and display backends are built.

The keyboard uses BDOS 6 and the console's non-expanded function-key codes.
F1/F2 select editor windows, F7 opens help, F8 marks blocks, Alt-W saves,
and Alt-X quits. Backspace, Delete, arrows, Home/End and Page Up/Down work
through the same translation layer. CP/M reserves Ctrl-keypad combinations
for console switching. Some modified function keys share input codes with
Alt letters; the editor gives Alt-letter shortcuts priority.

The display keeps an 80 by 24 character/attribute buffer in process memory.
BDOS 9 writes runs through the existing console escape protocol, including
colors, cursor position, visibility, and wrap control. Scrolling and block
highlighting update that local buffer; there are no private XIOS calls,
console structure pointers, or direct video-memory accesses in the program.
Exit restores the BDOS output mode and delimiter, enables function-key
expansion and wrapping, and restores normal attributes and cursor visibility.

The editor help/tutorial data (`HELP.EDX` and `LSN.EDX`) are not included in
these sources. F7 reports a missing help file if `HELP.EDX` is unavailable.
