
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
#include "GlobalParams.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "Page.h"
#include "Gfx.h"
#include "goo/GooString.h"
#include "poppler-config.h"
#include "TextOutputDev.h"

class PopplerWrapper {
public:
  double x_resolution = 72.0;
  double y_resolution = 72.0;

private:
  GooString *pdfName = nullptr;
  PDFDoc* doc;
  TextOutputDev *textOut;
  int pageNumber = 1;
  int numPages = 0;

public:
  PopplerWrapper(char *inputFile);
  ~PopplerWrapper() {
    delete textOut;
    delete pdfName;
    delete doc;
  }
  /* pgNum must be above zero and <= to the # of pages in the PDF.  */
  void setPageNumber(int pgNum);
  int getPageNumber() { return pageNumber; }
  int getNumPages() { return numPages; }
  TextOutputDev* getTextOut() {
    return textOut;
  }
  std::vector<GfxRenderInfo *> getTextInfo() {
    return textOut->getRenderInfo();
  }
  /* Creates a new Gfx based upon the current pg # */
  Gfx * getGfx();

  /**
   * Prints nicely formatted GFX render info coordinates according to the CTM
   */
  void print_infos(std::vector<GfxRenderInfo *> infos);
private:
  /**
   * Populates an "output device", i.e. a PNG or a poppler text interface
   * abstraction, with the data from a given PDF document for a given page.
   * optionally, it is possible to crop to the portion of the page inspected.
   */
  void populateOutputs(PDFDoc *doc, OutputDev *outputDev, int pg, int x, int y,
                       int w, int h, double pg_w, double pg_h);
};

#endif
