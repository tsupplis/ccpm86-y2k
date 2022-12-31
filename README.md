# Concurrent CP/M-86/Concurrent DOS Y2K Patches

This projects regroups patch versions of Concurrent Y2K OS utilties.

Currently the supported OSes are
- Concurrent CP/M-86 3.1
- Concurrent DOS 3.2
- Concurrent DOS 4.1
- DOS Plus 1.2

| Utility      | CCP/M 3.1 | CDOS 3.2  | CDOS 4.1  | DOS Plus 1.2|
|--------------|-----------|-----------|-----------|-------------| 
| SDIR         | Fixed     | Fixed     | Fixed     | Fixed       |
| DATE         | Fixed     | Compliant | Compliant | Compliant   |
| SHOW         | Fixed     | Fixed     | Compliant | Compliant   |

- Concurrent DOS XM 6.21 seems fully ocmpliant

You can go to github.com/tsupplis/cpm86-kernel and github.com/tsupplis/cpm86-hacking for CP/M-86 1.1 Y2K compliance.

## Where to find CP/M-86 and Concurrent CP/M-86?

The source for CP/M-86 dnd Concurrent CP/M-86 oc, sources and binaries is http://www.cpm.z80.de.

A cleaned-up distribution and kernel is available at https://github.com/tsupplis/cpm86-kernel. This distribution is working well in virtual environments, patched with all known patches, 'y2k' friendly (it contains the version of tod which sources are in this project) and AT friendly.

## Sources and Build

To come .... the sources are in PLM on http://www.cpm.z80.de. I may be complicated to standardize a toolset.