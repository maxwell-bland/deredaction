#!/usr/bin/env bash
DIR="$(dirname "${BASH_SOURCE[0]}")"

# get number of pages in pdf
PAGES=$(pdfinfo "$1" | grep Pages | awk '{print $2}')

for i in $(seq 1 $PAGES); do
    # Rewrite the PDF page 
    "$DIR"/../PDF-Text-Analysis/pts "$1" $i | "$DIR"/pdf_rewrite.py /tmp/page_$i.pdf
done

pdfunite /tmp/page_*.pdf "$2"
rm /tmp/page_*.pdf
