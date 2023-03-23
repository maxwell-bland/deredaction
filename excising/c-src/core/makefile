CXX=g++ -g -O3 -std=c++14
RM=rm -f

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(dir $(MKFILE_PATH))

POPPLER_DIR := $(CURRENT_DIR)../lib/poppler/
POPPLER_BUILD_DIR := $(POPPLER_DIR)build/

# Set compiler path specifications
POPPLER_INCLUDES = -I $(POPPLER_DIR) -I $(POPPLER_DIR)poppler -I $(POPPLER_BUILD_DIR)poppler
LDLIBS=-llept -ltesseract

ifeq ($(OS),OSX)
    CXX=g++ -std=c++14
    POPPLER_OBJS=$(POPPLER_BUILD_DIR)glib/libpoppler-glib.dylib $(POPPLER_BUILD_DIR)cpp/libpoppler-cpp.dylib $(CURRENT_DIR)../poppler/build/libpoppler.dylib 
    CPPFLAGS = $(shell printenv CPPFLAGS) $(POPPLER_INCLUDES) -fPIC
else
    POPPLER_OBJS=$(POPPLER_BUILD_DIR)cpp/libpoppler-cpp.so $(POPPLER_BUILD_DIR)libpoppler.so 
    CPPFLAGS = $(shell printenv CPPFLAGS) $(POPPLER_INCLUDES) -Wl,-R$(POPPLER_BUILD_DIR) -fPIC
endif


all: get-states

POPPLER_SRCS := $(POPPLER_DIR)poppler/*.cc $(POPPLER_DIR)poppler/*.h

libpoppler: $(POPPLER_SRCS)
	make CFLAGS="-O3" -C $(POPPLER_BUILD_DIR) poppler poppler-cpp

ADDIT_LIBS := $(LDLIBS) $(POPPLER_OBJS)
LOC_COMPILE = $(CXX) $(CPPFLAGS) $(LDFLAGS) 

$(CURRENT_DIR)/%.o: $(CURRENT_DIR)/%.cc libpoppler
	$(LOC_COMPILE) -c $<

get-states: libpoppler Inverter.cc
	$(LOC_COMPILE) -DPRINT_BOUNDING_BOXES -o get-states $(filter %.o,$^) Inverter.cc $(ADDIT_LIBS)

locate-lib libkerning-core.so: libpoppler Inverter.o Locator.o Redaction.o
	$(LOC_COMPILE) -shared -o libkerning-core.so $(filter %.o,$^)

clean:
	$(RM) *.o get-states *.png *.so
