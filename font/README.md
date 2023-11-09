# font

the default fonts used by the character unit:

- font0: 8x16 font, page 0
- font1: 8x16 font, page 1
- font2: 16x16 font

some tools are provided for converting the sources:

- convert\_glyphs.c: turns font source into binary or bitmap (for loading in GIMP)
- revert\_glyphs.c: turns GIMP raw image data export (indexed color) into font source
