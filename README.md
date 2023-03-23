# Story Beyond the Eye: Glyph Positions Break PDF Redaction

Hello there. This text is not written by an AI. This is a repository with the
code for the Story Beyond the Eye: Glyph Positions Break PDF Redaction paper.

Remediation efforts and questionable legality prevent me from releasing the
code publicly to automatically break redactions: regardless of how spicy the
results would be or how many sleepless nights were spent, I will not be caught
giving a fully automatic .45 to any small child.

But, if you are a world-weary scholar scraping what you can together for rent,
I'll give you all the code on valid scientific request, for reproduction
purposes and to make your research easier, so long as you agree to a few
promises, so as to not hurt your future career.

Without further ado, let's talk about what this repository contains.

## Glyph Positioning Recovery

A submodule linking to a highly precise PDF Glyph Positioning, Text Analysis,
and Parsing Library, PDF CMap Parsing Library. This compiles using just two
makefiles.  To build it, just compile the poppler version by `cd lib/poppler`,
then here's the instructions.

```
sudo apt install cmake libfreetype-dev libfontconfig-dev libopenjp2-7-dev
mkdir build
cd build
cmake ..
make
make install
# CMake configuration options can be set using the -D option. eg
#  cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=release
```

then `cd adjustment` and `make VectorString.o` then run `make pts` in the top level.

and kablam, you are done.

You may need some other standard dependencies, but it should just work if you
are familiar with Linux.

### Running

The primary method of using this is via the `pts` script, which supplies a
wealth of text positioning information to stdout by default. To test it out and
verify for yourself the promises of the paper are real, I've included a word
document from the washington post for you to look at (but not break).

In the `parsing` directory there are a number of Ruby scripts for translating
this UNIX-stype output to first-class objects. There is also the `cmap` script
which can retrieve character map information from PDFs. At the time of writing,
this was the only open source one available, though who knows what related work
has come out since then. 

```
./PDF-Text-Analysis/pts inconspicuous_pdf.pdf 1
```

### A Note on the Structure

- core: the wrapper for analyzing the PDF structure recorded in the modified poppler
- adjustment: a datatype for recording glyph displacements
- lib: the modified poppler
- parsing: ruby scripts for parsing glyph positioning information and cmap data
- utils: contains the actual analysis utilities (for this public version, just
  the print glyph information script)

## Defenses

I've included my scripts for locating redactions and fixing redactions under
the defenses submodule. These are straightforward enough to run, although you
may also need to download some dependencies. There are some test PDFs in this
repository as well that you may try the code out on.

### Building Nonexcising Redaction Location Algorithm

`cd` into the defenses directory and _copy over the PDF-Text-Analysis_ part we
compiled above into this directory using `cp -r ../PDF-Text-Analysis .` 

```
pip3 install PyMuPDF
```

### Running

For nonexcising redactions:

```
./trivial_redaction_loc.sh testcases/trivial_1.pdf
```

### Building Excising Redaction Location Algorithm

First, let me note I had to delete about 80%-90% of the code in this directory,
and there are still some remnants of the early code for deredaction.

Thankfully, I don't believe what remains is too easy to adapt, and unfortunately a lot
of it is tied up in the methods by which we correctly identify excising redactions.

Make sure you have java installed!

```
pdfseparate inconspicuous_pdf.pdf inconspicuous_pdf.pdf-%d
cd excising
export PTS_PATH=<path to pts>
./location/cut_method.sh ../inconspicuous_pdf.pdf-11
```

##### Alternative route

There is also the possiblility of using 

```
./location/ml ../../inconspicuous_pdf.pdf 10 
```

This has a more general method of identifying redactions but relies on the old
`c-src/core` directory: to compile this, compile the `lib/poppler` directory like 
before, cd into `c-src/core` and make.

#### Note 

Note this is not the full code to extract all the attack information used in
the paper. Think of it as an early version of that code with one leg cut off,
intended _solely_ for the location of potentally vulnerable excising
redactions. The final attack did not use any of the shifts returned by `ml`,
but a later version of the algorithm did use the "BBox" tags.

if you get `convert-im6.q16: attempt to perform an operation not allowed by the
security policy `PDF' @ error/constitute.c/IsCoderAuthorized/421` see
"https://stackoverflow.com/questions/52998331/imagemagick-security-policy-pdf-blocking-conversion"

# Conclusion

Feel free to let me know if anything goes wrong. For now my email is
`mb28@illinois.edu`.

There are also lots of small easter eggs buried around this repository that may
help future research.

#### A last note on weird output (not in the default scripts)

On (bad) advice, I was convinced to try out the ascii file, group, record, and unit separators:

```
034   28    1C    FS  (file separator)        134   92    5C    \  '\\'
035   29    1D    GS  (group separator)       135   93    5D    ]
036   30    1E    RS  (record separator)      136   94    5E    ^
037   31    1F    US  (unit separator)        137   95    5F    _
```

Output should still be readable, but you may need to translate these characters
by piping to the `tr` command. This is pretty straightforward I hope,
none-the-less.

