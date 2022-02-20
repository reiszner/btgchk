# Tool to check and repair broken FlightGear BTG files.

## Installation

1. Download or `git clone` this repository into a folder of your choice.
2. Install the archive library headers - use this console command in Ubuntu:
```
sudo apt install libarchive-dev
```
3. `cd` into the folder you put this repository into, then run
```
cc airport.c bsphere.c btgchk.c btgio.c check.c color.c coord.c edge.c element.c file.c geometry.c header.c normal.c object.c point.c property.c raw.c seam.c texcoo.c triangle.c vertex.c -o btgchk -lm -larchive
```
to compile. After that you can run it with `./btgchk`.
