CXX=g++ -O3 -g
RM=rm -f
POPPLER_LIBS := -I ../lib/poppler/ -I ../lib/poppler/poppler -I ../lib/poppler/build/poppler
LIBS := ../location/libkerning-location.so -llept -ltesseract ../lib/poppler/build/libpoppler.so -Wl,-R ../lib/poppler/build/

all: ms-tj-2012 ms-tj-2020 adj-diff
	echo "DONE"

VectorString.o: VectorString.cc
	$(CXX) -c VectorString.cc 

ms-tj-2012-nobound: Office.h Office.cc VectorString.o
	$(CXX) $(CFLAGS) -DNOBOUND_ADJ -DALG_2012 -DMS_TJ -o ms-tj-2012-nobound Office.cc VectorString.o

ms-tj-2012: Office.h Office.cc VectorString.o ms-tj-2012-nobound
	$(CXX) $(CFLAGS) -DALG_2012 -DMS_TJ -o ms-tj-2012 Office.cc VectorString.o

ms-tj-2020-nobound: Office.h Office.cc VectorString.o
	$(CXX) $(CFLAGS) -DNOBOUND_ADJ -DMS_TJ -o ms-tj-2020-nobound Office.cc VectorString.o

ms-tj-2020: Office.h Office.cc VectorString.o ms-tj-2020-nobound
	$(CXX) $(CFLAGS) -DMS_TJ -o ms-tj-2020 Office.cc VectorString.o

adj-diff: AdjDiff.cc VectorString.o
	$(CXX) $(CFLAGS) -o adj-diff AdjDiff.cc VectorString.o


clean: 
	rm -rf *.o  ms-tj-* ms-tj-break adj-diff
