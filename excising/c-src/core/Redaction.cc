#include "Redaction.h"

void Redaction::initNoisyCharacters(const Locator::RedactionLocation &loc,
		int height, Inverter &inverter) {
	double xMin, xMax, yMin, yMax, x2Min, x2Max, y2Min, y2Max;
	leftWord->getBBox(&xMin, &yMin, &xMax, &yMax);
	rightWord->getBBox(&x2Min, &y2Min, &x2Max, &y2Max);
}

int Redaction::initSurroundingWords(const Locator::RedactionLocation &loc,
		Inverter &inverter) {
	/* TODO just use the GFXState. */
	// Used for locating words to the right and left
	int height = loc.bbox->y + loc.bbox->h;
	// Get the metadata for the word to the left of the redaction
	int x_1 = loc.bbox->x - 1;
	while (x_1 > loc.leftJust) {
		for (int y = loc.bbox->y; y <= height; y++) {
			auto ws = inverter.getWordsAtPixel(x_1, y);
			if (ws.size()){
				leftWord = ws.back();
				break;
			}
		}
		if (leftWord)
			break;

		x_1--;
	}
	// For the right word too
	int x_2 = loc.bbox->x + loc.bbox->w + 1;
	while (x_2 < loc.rightJust) {
		for (int y = loc.bbox->y; y <= height; y++) {
			auto ws = inverter.getWordsAtPixel(x_2, y);
			if (ws.size()){
				rightWord = ws[0];
				break;
			}
		}
		if (rightWord)
			break;

		x_2++;
	}
	return height;
}

Redaction::Redaction(Inverter inverter, int pgNum,
		Locator::RedactionLocation loc) {
	// Save and restore later
	int origPgNum = inverter.getPageNumber();
	inverter.setPageNumber(pgNum);
	this->pgNum = pgNum;

	// Used for locating words to the right and left
	int height = initSurroundingWords(loc, inverter);
	// Initialize the target for guessing
	// TODO: make skipping space character optional
	// target = rightWord->getRis()[1]->getX();
	// inject = leftWord->getRis().back()->getPreState();

	inverter.setPageNumber(origPgNum);
}

#ifdef PRINT_REDACTION_WIDTHS
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Please supply input file!" << std::endl;
    exit(1);
  }

  Inverter inv(argv[1]);
  Locator loc(argv[1]);
  auto redactions = loc.findRedactions();

  int pgNum = 1;
  for (auto p : redactions) {
    pixDestroy(&p.first);
    for (auto rLoc : p.second) {
      Redaction redaction(inv, pgNum, rLoc);
      std::cout << std::string(redaction) << std::endl;
    }
    pgNum++;
  }
}
#endif
