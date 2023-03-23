
/**
 * The goal of this header is to take in a PDF, and generate a PNG, where
 * each pixel in the PNG can be "inverted", and mapped back ot the original
 * PDF textobject, for redaction location.
 *
 * Much code taken from pdftoppm.cc.
 * Author: Maxwell Bland
 */

#ifndef KERNING_INVERTER_H
#define KERNING_INVERTER_H
#include <iostream>
#include <math.h>
#include <limits.h>
#include <sstream>
#include "GlobalParams.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "Page.h"
#include "Gfx.h"
#include "SplashOutputDev.h"
#include "TextOutputDev.h"
#include "goo/GooString.h"
#include "poppler-config.h"
#include "splash/Splash.h"
#include "splash/SplashBitmap.h"

class Inverter {
public:
  double x_resolution = 300.0;
  double y_resolution = 300.0;

private:
  GooString *pdfName = nullptr;
  PDFDoc* doc;
  SplashOutputDev *splashOut; // PNG output
  TextOutputDev *textOut;     // TextObject interface
  int pageNumber = 1;
  int numPages = 0;
  // Words on the current page
  std::vector<TextWord *> wordList;
  // Used for entropy calculation

public:
  Inverter(char *inputFile);
  ~Inverter() {
    delete textOut;
    delete pdfName;
    delete doc;
  }
  /**
   * pgNum must be above zero and below or equal to the number of pages in the
   * PDF.
   */
  void setPageNumber(int pgNum);
  int getPageNumber() {
      return pageNumber;
  }

  /**
   * Prints all the bounding boxes for a the current page number
   */
  void printBoundingBoxes();

  void savePng(const char *outputFileName);

  int getNumPages() {
    return numPages;
  }

  /**
   * Maps a pixel back to a text word with a more precise width measurement.
   */
  std::vector<TextWord *> getWordsAtPixel(float x, float y);

  /**
   * Creates a new Gfx object using the text output based upon the current
   * page number state and returns a pointer to it.
   */
  Gfx * getGfx();

  TextOutputDev * getTextOut() {
	  return textOut;
  }

private:
  /**
   * Populates an "output device", i.e. a PNG or a poppler text interface
   * abstraction, with the data from a given PDF document for a given page.
   * optionally, it is possible to crop to the portion of the page inspected.
   */
  void populateOutputs(PDFDoc *doc, OutputDev *outputDev, int pg, int x, int y,
                       int w, int h, double pg_w, double pg_h);

  /**
   * Updates the set of words in this object to the current page.
   */
  void updateWordList();
	void initSplashOut();
	void initTextOut();
	void initDocument();
};

#endif
