#include "Inverter.h"
#include <vector>

Inverter::Inverter(char *inputFile) {
	pdfName = new GooString(inputFile);

	// Initialize poppler parameters
	globalParams = std::make_unique<GlobalParams>();

	// Create the poppler doc
	doc = PDFDocFactory().createPDFDoc(*pdfName, nullptr, nullptr);
	if (!doc->isOk()) {
		std::cerr << "Something is wrong with the input PDF!" << std::endl;
		exit(1);
	}

	numPages = doc->getNumPages();

	// Create image interface
        /*
              SplashColor paperColor;
              paperColor[0] = 255;
              paperColor[1] = 255;
              paperColor[2] = 255;
              splashOut = new SplashOutputDev(splashModeRGB8, 4, false,
           paperColor, true, splashThinLineDefault); splashOut->startDoc(doc);
         */

        // Create Text interface
	textOut = new TextOutputDev(NULL, true, false, false, false);

}

void Inverter::setPageNumber(int pgNum) {
	if (pgNum > 0 && pgNum <= numPages) {
		pageNumber = pgNum;
		auto pg_w = doc->getPageMediaWidth(pgNum);
		auto pg_h = doc->getPageMediaHeight(pgNum);
		pg_w = pg_w * (x_resolution / 72.0);
		pg_h = pg_h * (y_resolution / 72.0);
		// We are reusing the same output device, so we reset it
		textOut->resetRenderInfo();
		populateOutputs(doc, textOut, pgNum, 0, 0, 0, 0, pg_w, pg_h);
		// populateOutputs(doc, splashOut, pgNum, 0, 0, 0, 0, pg_w, pg_h);
		updateWordList();
	}
}

Gfx* Inverter::getGfx() {
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

void Inverter::printBoundingBoxes() {
	std::cout << "# Finding words ..." << std::endl;
	double xMin, yMin, xMax, yMax;
	double wordWidth;

	for (auto wp : wordList) {
		std::unique_ptr<GooString> gooWord { wp->getText() };
		std::cout << gooWord->c_str() << std::endl;
		wp->getBBox(&xMin, &yMin, &xMax, &yMax);
		fprintf(stdout, "%f, %f, %f, %f\n", xMin, yMin, xMax, yMax);

	}
	std::cout << "# Done!" << std::endl;
}

void Inverter::updateWordList() {
	wordList.clear();
	if (std::unique_ptr<TextWordList> words { textOut->makeWordList() }) {
		for (int i = 0; i < words->getLength(); i++) {
			wordList.push_back(words->get(i));
		}
	}
}

void Inverter::savePng(const char *outputFileName) {
	std::cout << "# Saving PNG ..." << std::endl;
	SplashBitmap *bitmap = splashOut->getBitmap();
	if (outputFileName != nullptr) {
		bitmap->writeImgFile(splashFormatPng, outputFileName, x_resolution,
				y_resolution);
	}
	std::cout << "# Done!" << std::endl;
}

void Inverter::populateOutputs(PDFDoc *doc, OutputDev *outputDev, int pg, int x,
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

std::vector<TextWord*> Inverter::getWordsAtPixel(float x, float y) {
	double xMin, yMin, xMax, yMax;
	std::vector<TextWord *> res;

	for (auto wp : wordList) {
		std::unique_ptr<GooString> gooWord { wp->getText() };
		wp->getBBox(&xMin, &yMin, &xMax, &yMax);
		if (x >= round(xMin) && x <= round(xMax) && y >= round(yMin)
				&& y <= round(yMax)) {
			res.push_back(wp);
		}
	}

	return res;
}

#ifdef PRINT_BOUNDING_BOXES
int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Please supply an input file.\n");
    exit(1);
  }

  setlocale(LC_CTYPE, "");
  std::wcin.imbue(std::locale("en_US.UTF-8"));
  std::wcerr.imbue(std::locale("en_US.UTF-8"));
  std::wcout.imbue(std::locale("en_US.UTF-8"));

  Inverter * inv = new Inverter(argv[1]);
  std::wcout.precision(14);

  std::wcout << "PAGEBREAK " << 1 << std::endl;
  for (int i = 1; i <= inv->getNumPages(); i++) {
    inv->setPageNumber(i);
    TextOutputDev * out = inv->getTextOut();
    auto infos = out->getRenderInfo();
    for (auto ri : infos) {
    	std::wcout << (std::wstring) *ri << std::endl;
    }
    std::wcout << "PAGEBREAK " << i + 1 << std::endl;
  }
}
#endif
