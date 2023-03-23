#!/usr/bin/env bash
pip3 install PyMuPDF
cd PDF-Text-Analysis/lib/poppler
mkdir build
cd build
cmake ..
make -j8
cd ../../../../
cd PDF-Text-Analysis/adjustment/
make VectorString.o
cd ..
make pts
cd ..
