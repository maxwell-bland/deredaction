CXX=g++ -g -O3 -std=c++14
RM=rm -f

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MKFILE_DIR := $(dir $(MKFILE_PATH))

INCLUDE_DIRS := -I $(MKFILE_DIR)/lib/poppler/poppler -I $(MKFILE_DIR)/lib/poppler/build/poppler -I $(MKFILE_DIR)/lib/poppler -I $(MKFILE_DIR)/adjustment -I $(MKFILE_DIR)/lib -I $(MKFILE_DIR)/core 
OBJS := adjustment/VectorString.o
LIBS := $(MKFILE_DIR)/lib/poppler/build/libpoppler.so -Wl,-R $(MKFILE_DIR)/lib/poppler/build/ 

all: clean pts
	make -C adjustment

VectorString.o:
	make -C adjustment VectorString.o

pts: utils/pts.cc
	make -C lib/poppler/build -j16
	$(CXX) -DWIDTH_MATCH $(INCLUDE_DIRS) -o pts $(OBJS) utils/pts.cc core/PopplerWrapper.cc $(LIBS)

clean:
	make -C adjustment clean
	rm -rf *.o gen-widths pts
