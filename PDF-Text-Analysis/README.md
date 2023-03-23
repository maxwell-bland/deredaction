# PDF Glyph Positioning, Text Analysis, and Parsing Library, PDF CMap Parsing Library

An highly accurate PDF text analysis library built around Poppler. I built this since 
IText and other PDF analysis toolkits in python actually are imprecise and tend to use
coordinate systems that are not fine grained in their representation of glyph positions.

The primary method of using this is via the `pts` script, which supplies a wealth of 
text positioning information to stdout by default. In the `parsing` directory there 
are a number of Ruby scripts for translating this UNIX-stype output to first-class 
objects. There is also the `cmap` script which can retrieve character map information
from PDFs. At the time of writing, this was the only open source one available, though 
who knows what related work has come out since then. 

To build it, just compile the poppler version under `/lib/poppler` and also compile
the top level scripts in this directory (the Makefile should take care of this).

I have the feeling this might have the strongest correctness and precision guarantees 
of any PDF text analysis library around, since it was used in a very sensitive project...

## Structure

```
core: the wrapper for analyzing the PDF structure recorded in the modified poppler
adjustment: a datatype for recording glyph displacements
lib: the modified poppler
parsing: ruby scripts for parsing glyph positioning information and cmap data
utils: contains the actual analysis utilities (for this public version, just the print glyph information script)
```
