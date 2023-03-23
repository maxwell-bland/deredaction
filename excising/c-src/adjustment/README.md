# Adjustment Algorithms

This directory has implementations for the office adjustment and kerning pair adjustment algorithms.

## Install

```
You will need fontforge.
```

## Executive Summary

`ms-tj` does the basic adjustment
`ms-tj-break` will take in a line and a set of indices and figure out the right coordinates
if there is a TJ break at that index.

## How it works

- `ms-tj` takes in a list of `[char]\t[TTF width without left and right bearing` (potentially kerned), and then applies the MS Word WYSIWYG algorithm based upon these widths, and prints out the resulting TJ object. If a command line argument is supplied is applies the basic 1-inch margin text justification.
- `get-widths` takes in a string from the command line and formats it (kerned if a command line argument is supplied), for input to ms-tj (or whatever else you might want). This is just a wrapper for a fontforge script.

See https://stackoverflow.com/questions/4190667/how-to-get-width-of-a-truetype-font-character-in-1200ths-of-an-inch-with-python

## Notes

### 2/20/21

```
# The conversion to dots in microsoft word
(0x10ab * TTFWidth[i] * FONT_SIZE * 2) + 0x100000 >> 0x15
# The conversion to the TTF file width
(int) (TTFWidth[i] / 2048) * 1000
```

*However* it appears that at least for calibri 11-PT, fontSizeFloat (during
adjustment determination) is 11.04 rather than 11. FONT_SIZE in the conversion
above is 11 though. 

Note that it is somewhat critical to select the right exported font from the
PDF. I've had one case where the PDF extracted two calibri font dictionaries
(different font sizes), where one matched the MS Word widths for 11pt calibri
and the other did not. Seems you can probably just trust the one for the
section of text you are interested in.

### Earlier

Here is the current TODO, we want to handle TJ breaks:

```
string | tee >(./office-disp >3) | ./interim-dpi | paste -d'\n' - <3 | \
        while read tj; read interimDPIs; do
                cat $tj $interimDPIs <(./getBreaks) | ./breakTJS
        done
```

That is, the program should read in the output of office disp get the interim DPIs, and 
compute the proper adjustments for TJ breaks on the line according to the third input 
line. The above shell script will be optimized into a pipeline without a while in the 
final implementation (hopefully).

The determination of the adjustment value shifts from the text matrix set-values is 
entirely dependent on the original shift values of the line. Thus, a secondary 
analysis will be needed to determine the text state matrix values with respect to 
the shifts, that is, The analysis must be perfected with respect to the post-line 
adjustment values. 

Now, as to the design of such an efficient system. It would seem that the most 
natural manner in which to apply the readjustment of text would be post-hoc 
or pre-hoc? Is it possible to determine the resulting text matrix values 
after the shifts have been determined? Well, yes, it appears to simply be 
the total dots. That is, the application of the matrix shifts maps one to 
one with the readjustment of the DPI units after dotsToPdfUnits is applied.

This much is determinable via reason alone, from what has already been seen and 
understood. But no, the dotsToPdfUnits conversion is already on the raw value of 
the DPI unit widths, suggesting the text state procession can only be determined 
relative to the current instantiation of the PDF state. Is not, after all, the 
issue the non-correspondence between device widths and DPI widths?

Simply put, let us not be pedantic. We could attempt to reverse the precise 
text matrix out of the getString method but that seems pretentious. Instead,
it would be fitting for us to simply inject the ... rounded matrix 
modifications after retrieving them from the break operation, the text state 
values given by an unbroken line, and manage the difference? 

Simply: retrieve t_{x} for (index of the break + 1), and subtract the result 
from the value resulting from "ms-tj-break". Round the result to the nearest quanta,
add the corresponding adjustment, and voila! The correct body awaits analysis.
