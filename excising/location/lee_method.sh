#!/usr/bin/env bash
$DR_SRC/location/lee_method/find_bad_redactions.pl "$1" | 
    tee "$1".lee_meth |
    $DR_SRC/painting/add-rects-png.rb "$1" "$1".lee.png
if [ ! -s "$1".lee_meth ]; then
  rm "$1".lee_meth
fi
