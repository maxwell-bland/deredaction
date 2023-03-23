#!/usr/bin/env bash
# Script implementing the cut method for redactions: converts an input pdfs into
# a 3-color variant in grayscale and then attempts to walk a rectangle covering
# greater than 70% of the width of the width of each large shift in pixels.
#
# Note how it does *not* crop the PNG. This allows us to avoid several types of
# false positives if you think about it.
#
# Duplicate of the (later added and cleaner) script in the tagging directory
DIR=$(realpath $(dirname ${BASH_SOURCE[0]}))
rm "$1".cut_meth "$1".cut.pdf 2>/dev/null
tf=$(mktemp -u)
"$DIR"/camlpdf -draft "$1" -o "$tf".pdf-0
"$DIR"/../painting/remove-tjs.sh "$tf".pdf-0 "$tf".pdf
convert -background 'rgb(255,255,255)' -colorspace gray -normalize \
        -density 300 -quality 100 -depth 8 "$tf".pdf "$tf".ppm
"$PTS_PATH" "$1" 1 | tee /dev/stderr | "$DIR"/large_shifts.rb | tee /dev/stderr |
    tee "$1".large_shifts | tee /dev/stderr |
    perl -lane 'print "$1 $3 $2" if /\[(.*?),(.*?),.*?,.*?\[.*?,.*?,(.*?),/;' |
    "$DIR"/find_redaction_box.py "$tf".ppm | 
    tee "$1".redaction_boxes | 
    sort | uniq | grep -v '^-$' | tee "$1".cut_meth | tee /dev/stderr |
    "$DIR"/../painting/add-rects-png.rb "$1" "$1".cut.png
if [ ! -s "$1".cut_meth ]; then
  rm "$1".large_shifts
  rm "$1".redaction_boxes
  rm "$1".cut_meth
fi
rm "$tf".pdf-0 "$tf".pdf "$tf".ppm
