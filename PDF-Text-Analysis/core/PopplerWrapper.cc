#include <vector>
#include <iostream>
#include <math.h>
#include <limits.h>
#include <sstream>
#include "PopplerWrapper.h"

PopplerWrapper::PopplerWrapper(char *inputFile) {
	pdfName = new GooString(inputFile);
	globalParams = std::make_unique<GlobalParams>();
	doc = PDFDocFactory().createPDFDoc(*pdfName, nullptr, nullptr);
	if (!doc->isOk()) {
		std::cerr << "Something is wrong with the input PDF!" << std::endl;
		exit(1);
	}
	numPages = doc->getNumPages();
	textOut = new TextOutputDev(NULL, true, false, false, false);
}

void PopplerWrapper::setPageNumber(int pgNum) {
	if (pgNum > 0 && pgNum <= numPages) {
		pageNumber = pgNum;
		auto pg_w = doc->getPageMediaWidth(pgNum);
		auto pg_h = doc->getPageMediaHeight(pgNum);
		pg_w = pg_w * (x_resolution / 72.0);
		pg_h = pg_h * (y_resolution / 72.0);
		// We are reusing the same output device, so we reset it
		textOut->resetRenderInfo();
		populateOutputs(doc, textOut, pgNum, 0, 0, 0, 0, pg_w, pg_h);
	}
}

Gfx* PopplerWrapper::getGfx() {
	auto pg_w = doc->getPageMediaWidth(pageNumber);
	auto pg_h = doc->getPageMediaHeight(pageNumber);
	Page *page = doc->getPage(pageNumber);
	XRef *localXRef = doc->getXRef();
	Gfx *gfx = page->createGfx(textOut, x_resolution, y_resolution, 0, false,
			false, 0, 0, pg_w, pg_h, false, nullptr, nullptr, localXRef);
	Object obj = page->getContents().fetch(localXRef);
	gfx->display(&obj);
	return gfx;
}

void PopplerWrapper::populateOutputs(PDFDoc *doc, OutputDev *outputDev, int pg, int x,
		int y, int w, int h, double pg_w, double pg_h) {
	// Calculate dimensions, if cropping
	if (w == 0)
		w = (int) ceil(pg_w);
	if (h == 0)
		h = (int) ceil(pg_h);
	w = (x + w > pg_w ? (int) ceil(pg_w - x) : w);
	h = (y + h > pg_h ? (int) ceil(pg_h - y) : h);
	// write the text and splash output
	doc->displayPageSlice(outputDev, pg, x_resolution, y_resolution, 0, false,
			false, false, x, y, w, h, nullptr, nullptr, nullptr, nullptr);
}

void PopplerWrapper::print_infos(std::vector<GfxRenderInfo *> infos) {
  wchar_t strRepr[1024];
  double curQuant = -420420;
  double curCharSpace = -420420;
  double curWordSpace = -420420;
  double curHoriScale = -420420;
  double dx, dy;
  for (auto ri : infos) {

    if (ri->postState) {
      /* GfxState changes. Could simplify. */
      ri->state->textTransformDelta(
        0.001 * ri->state->getFontSize() * ri->state->getHorizScaling(), 0, &dx, &dy);
      /* TODO: this is a hack to find the font size quanta which assumes the
         only other text space transform is page rotation. We should think of
         a better way to figure out the quanta regardless of the current text
         space matrix*/
      if (!dx) {
        ri->state->textTransformDelta(
          0, 0.001 * ri->state->getFontSize() * ri->state->getHorizScaling(), &dx, &dy);
      }

      double quant = ri->state->transformWidth(dx);
      if ( quant != curQuant ) {
        curQuant = quant;
        swprintf(strRepr, 1024, L"QUANTA\t%.17g\n", curQuant);
        std::wcout << std::wstring(strRepr);
      }
      if ( ri->state->getHorizScaling() != curHoriScale ) {
        curHoriScale = ri->state->getHorizScaling();
        swprintf(strRepr, 1024, L"HSCALE\t%.17g\n", curHoriScale);
        std::wcout << std::wstring(strRepr);
      }
      if ( ri->state->getCharSpace() != curCharSpace ) {
        curCharSpace = ri->state->getCharSpace();
        swprintf(strRepr, 1024, L"CSPACE\t%.17g\n", curCharSpace);
        std::wcout << std::wstring(strRepr);
      }
      if ( ri->state->getWordSpace() != curWordSpace ) {
        curWordSpace = ri->state->getWordSpace();
        swprintf(strRepr, 1024, L"WSPACE\t%.17g\n", curWordSpace);
        std::wcout << std::wstring(strRepr);
      }

      /* TODO: maybe ri don't need x,y, we can just keep the state copy
         around */
      double x_i, y_i, x_f, y_f;
      ri->state->transform(ri->x, ri->y, &x_i, &y_i);
      ri->postState->transform(
        ri->postState->getCurX(), ri->postState->getCurY(),
        &x_f, &y_f);

      /* Regular Character Metadata */
      std::wstring desc = ri->getDesc();
      swprintf(strRepr, 1024,
               L"%ls\t% 19.17g\t% 19.17g\t% 19.17g\t% 19.17g\t%ls\n",
               desc.c_str(), x_i, y_i, x_f, y_f, ri->getFontInfo().c_str());
      std::wcout << std::wstring(strRepr);
    }
  }
}
