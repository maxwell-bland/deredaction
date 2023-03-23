# Location

Locating a redaction inside of a PDF

Input: A PDF Output: True and a list of bounding boxes of redactions, otherwise
False

## Installation 

We use the poppler and tesseract c++ APIs for OCR and PDF file parsing. These
can be installed following the tutorials at [the poppler
github](https://github.com/freedesktop/poppler) and [the tesseract
github](https://github.com/tesseract-ocr/tesseract).

Before running `make`, do `export $OS` so that it can be compiled on MacOS as
well as Linux.

Once these are installed, the makefiles of this repository should "just work",
though certain library paths may need to be edited.

## How Guessing Works

First, we extract the GfxState right before the redaction, by parsing the 
PDF. During this process, the positions of all glyphs on the page get stored.
We then inject our guess and see whether the width aligns well with the 
pre-existing "next glyph" of the redaction.

Note for later: we move one space in from the word to the left of the redaction, if
there are spaces in-between the words, and one space in from the right. We need
to measure how accurate this is with respect to the different redaction software.

Note for later: we need to extract out the kernings from the document to get accurate
shift amounts. Will need to measure this later.
