#!/usr/bin/env bash
# The primary issue is that text is intersecting a fill; we can make the fill and the text resolve to black.
# We locate the redactions by producing two files, one with TJs and one without TJs, then convert them
# to PPM and subtract them from eachother. We then take the  x_i, x_f, y coordinates for each word and iterate 
# across this word, looking for any change in the color. If none is found, the word is considered redacted, 
# and the original x_i, x_f, and y coordinates are noted down for later use.
DIR=$(realpath $(dirname ${BASH_SOURCE[0]}))

# NOTE: could add -draft to remove images
"$DIR"/lib/camlpdf -blacktext "$1" -o "$1"-a
gs -q -o "$1"-b -sDEVICE=pdfwrite -dFILTERTEXT "$1"-a

pdftoppm -singlefile -r 300 "$1"-a "$1"-a
pdftoppm -singlefile -r 300 "$1"-b "$1"-b

"$DIR"/get_word_coords.rb "$1"-a |
  "$DIR"/trivial_redaction_loc.py "$1"-a.ppm "$1"-b.ppm

rm "$1"-a "$1"-b "$1"-a.ppm "$1"-b.ppm
