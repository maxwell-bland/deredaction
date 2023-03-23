# Contributing to Kerning

## Welcome

Of course, keep good style, so that it is easier to maintain and fix things.

We won't bother with making issues and pull requests since we are a small team.

## Install

Unfortunately, we had to modify Poppler's PDF API to get certain portions of the code 
working, thus, you should install it from the `poppler` directory.

You should also use the _best_ quality tesseract OCR training data:

```
sudo su

cd /usr/share/tesseract-ocr/4.00/tessdata/
cp eng.traineddata eng.traineddata.bkp
wget 'https://github.com/tesseract-ocr/tessdata_best/blob/master/eng.traineddata?raw=true' eng.traineddata
```

```
make
```

Should hopefully just work.
Then the neat scripts for doing things are in `misc/eval`, which gets various tools copied into its subdirectories.

Next, the last line in the PDF file you are analyzing should contain a comment with two words.
The first is the font type. 
Check the WidthDict.h class in `adjustment-algorithms` for a list of supported fonts. 
That directory also has scripts and guides for adding new supported fonts using windbg.
The second is the targeting method. 
`PAGECAPTURE` targets the first glyph of the word immediately following a redaction.
`FULLLINE` matches the x value of the entire line (i.e. try to match all the adjustments on the line exactly).
NOTE: you will need to uncompress the PDF (`podofouncompress`) to do this or just set the variables manually.

```
%PDF-1.7
% TNR-12PT PAGECAPTURE
```

For large PDFs you will probably want to extract a subpage range:

```
pdftk [pdf name] cat [page number range] output [output file name]
```

TODO is a third word specifying which kerning algorithm to use (Word justified, Word regular, Kerning pairs, No Adjustment).

Then `misc/eval/run` should more or less work, unless there are some gotchas with the specific pdf you are working with, i.e. specific glyphs that are always accompanied by an adjustment, TJ breaks, substitued CID values, etc. (there are other scripts, in various places, that will allow you to handle these additional cases (subsituted CIDs need to modify the inject function in `lib/poppler/poppler/Gfx.cc`).

## Developing for Kerning

We'll followed a layered design where each component solves a particular problem.

Efficiency is key! All scripts should be nearly instantaneous after pre-processing.

## Organization

The Kerning codebase is subdivided by subproblem, with each subproblem handled in its own subdirectory. To get an understanding of the functionality of each submodule, either check the header of the `__init__.py` file in that directory or see how they are coordinated in the root `__init__.py` script.

The `utilities` directory contains classes for generic problems such as PDF manipulation, and are organized by the nature of the task.

The `evaluation` directory contains scripts and files related to performing the evaluation in the paper, including constructing the synthetic dataset, testing the overall system, etc.

The `background` directory contains scripts for the background of the paper; it includes a npm project which has APIs with input: some text on the page, and output of the font type that text is rendered in, as well as a human-readable specification of the text objects that compose that object.

Large amounts of data, such as corpora, are efficiently cached within their related subdirectory.

## Contributors

Maxwell Bland

## TODO

Figure out how the shifts are determined during with justified alignment. 
	- Model the algorithm's dataflow for writing those shift values.
	- Analyze and correctly break based upon the writing of TJ values inside ghidra
	- Determine why the shifts are off by 22, 18, 21 in the justified alignment text for inter-word spacing.
