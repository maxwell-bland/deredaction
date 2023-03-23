
/**
 * Locates all the redactions in a PDF or PNG using OCR.
 *
 * Calls into the tesseract library and operates as described
 * by section 3.1.1 of the "Kerning Considered Harmful" paper and appendix
 * algorithm 1.
 *
 * Author: Maxwell Bland
 */

#ifndef KERNING_LOCATOR_H
#define KERNING_LOCATOR_H

#include "Inverter.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <leptonica/allheaders.h>
#include <memory>
#include <string>
#include <tesseract/baseapi.h>

class Locator {
public:
  static const uint WHITE_RGBA = 0xFFFFFFFF;
  static const uint BLACK_RGBA = 0x000000FF;

  // "We then check each matrix, M for a rectangle of black pixels acting as a
  // “border” around some fraction α of other pixels in M"
  const float alpha = 0.5;
  // "As a fallback, we also check whether the fraction of black pixels b in M
  // satisfies b≥β for some β, computed empirically from the norm of real
  // redactions."
  const float beta = 0.5;
  // beta but for white pixels
  const float gamma = 0.99;

  struct RedactionLocation {
    int paragraph, line, firstLoc, secondLoc;
    int leftJust, rightJust; // endpoints of line in pixels
    BOX *bbox;
  };

private:
  char *fileName;

  /* The image operating on */
  Pix *image;

  /* Tesseract state */
  tesseract::TessBaseAPI *api;
  tesseract::ResultIterator *ri;

public:
  /**
   * Initializes the tesseract API and results list, sets
   * the file being analyzed.
   */
  Locator(char *fileName);
  ~Locator();

  /**
   * Locates all the redactions for the current file, returns
   * an array for each page with redactions and the image for
   * the page.
   */
  std::vector<std::pair<Pix *, std::vector<Locator::RedactionLocation>>>
  findRedactions();

private:
  /**
   * Handles the PNG file in which redactions will be searched for. Returns
   * a pix image for the file and a set of redactions
   */
  std::pair<Pix *, std::vector<Locator::RedactionLocation>>
  handleFile(char *pngName);

  /**
   * Adds word bounding boxes for the line pointed to by the curret ri to the
   * array arr.
   */
  void boundingBoxesForLine(BOXA *arr);

  /**
   * Bounds the y dimensions of a submatrix for redaction identification by
   * gradually expanding from a height of 1 while checking black pixels along
   * the top and bottom border, accounting for discontinuities to avoid
   * capturing text on the lines above and below the current line. Results are
   * stored in y1 and y2.
   */
  void expandBorder(PIX *img, BOX *left, BOX *right, int *y1, int *y2);

  /**
   * Looks for the largest box of black pixels in a submatrix of
   * an image specified by M. If box covers as many pixels as the aera
   * threshold, returns 0.
   */
  BOX *findBox(PIX *img, BOX *M, int areaThresh);

  /**
   * Get percentage of black pixels in a box
   */
  float getPercentBlackPixels(PIX *image, BOX *M);

  /**
   * Identifies any redactions on a line defined by the set of bounding boxes.
   * Boxes should contain the words on the line as well as the justification
   * boundaries.
   */
  Locator::RedactionLocation identifyRedactions(PIX *image, BOXA *boxes);

  /**
   * Handles finding redactions on a page; for each line, for each pair of
   * words on that line (and the justification boundaries), we look for
   * redaction between those words.
   */
  std::vector<Locator::RedactionLocation> searchFile();
};

#endif
