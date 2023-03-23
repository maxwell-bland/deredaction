CXX=g++ -g -O3 -std=c++14
RM=rm -f

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MKFILE_DIR := $(dir $(MKFILE_PATH))

INCLUDE_DIRS := -I $(MKFILE_DIR)/lib/poppler/poppler -I $(MKFILE_DIR)/lib/poppler/build/poppler -I $(MKFILE_DIR)/lib/poppler -I . -I $(MKFILE_DIR)/adjustment
OBJS := core/Inverter.o core/Locator.o core/Redaction.o adjustment/VectorString.o
LIBS := -llept -ltesseract $(MKFILE_DIR)/lib/poppler/build/libpoppler.so -Wl,-R $(MKFILE_DIR)/lib/poppler/build/ 

all: guess-inject
	make -C adjustment clean
	make -C adjustment
	make -C core get-states

VectorString.o:
	make -C adjustment VectorString.o

libkerning-core.so:
	make -C core libkerning-core.so

guess-inject: libkerning-core.so VectorString.o
	$(CXX) -DWIDTH_MATCH $(INCLUDE_DIRS) -o guess-inject $(OBJS) PrintInjector.cc Injector.cc FakeCMap.h lib/IntervalTree.h $(LIBS)

clean:
	make -C core clean
	make -C adjustment clean
	rm -rf *.o width-match
