/* Translate CP/M console key codes to the editor's IBM-style key stream. */
#include "stdio.h"
#include "common.h"

static WORD pending;

BYTE
VKgetkey()
{
    WORD c, scan;
    BYTE *p;
    static BYTE letters[] = "qwertyuiopasdfghjklzxcvbnm";
    static BYTE scans[] = {16,17,18,19,20,21,22,23,24,25,
                           30,31,32,33,34,35,36,37,38,
                           44,45,46,47,48,49,50};
    if (pending) {
        c = pending; pending = 0;
        return(c);
    }
    do { c = __BDOS(6, 0xfd) & 255; } while (c == 0);
    scan = 0;
    if (c >= 0xbb && c <= 0xc4) scan = c & 127; /* F1..F10 */
    else if (c >= 0xc7 && c <= 0xd3) scan = c & 127; /* keypad */
    else if (c >= 0xe1 && c <= 0xfa) {
        /* Alt letters share codes with modified F keys on this console.
         * Prefer the editor's Alt command shortcuts (Alt-Q, Alt-W, ...). */
        for (p = letters; *p; p++)
            if (*p == (c & 127)) scan = scans[p - letters];
    }
    if (c == 8 || c == 127) return(4); /* existing KBDEL action */
    if (scan) { pending = scan; return(0); }
    return(c);
}

VOID
VKinit()
{
    pending = 0;
    __BDOS(6, 27); __BDOS(6, '6'); /* disable PFK expansion */
}

VOID
VKfin()
{
    __BDOS(6, 27); __BDOS(6, '7'); /* restore normal console keys */
    pending = 0;
}
