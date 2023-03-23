#include "Locator.h"

Locator::Locator(char *fileName) {
	this->fileName = fileName;

	// Initialize tesseract-ocr with English, without specifying tessdata path
	api = new tesseract::TessBaseAPI();
	if (api->Init(NULL, "eng")) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}

	ri = nullptr;
	image = nullptr;
}

Locator::~Locator() {
	api->End();
	delete api;
}

std::vector<std::pair<Pix*, std::vector<Locator::RedactionLocation>>> Locator::findRedactions() {
	std::vector<std::pair<Pix*, std::vector<Locator::RedactionLocation>>> redactions;
	// Check extension
	std::string fileNameStr(fileName);
	transform(fileNameStr.begin(), fileNameStr.end(), fileNameStr.begin(),
			[](unsigned char c) {
				return tolower(c);
			});
	std::string last = fileNameStr.substr(fileNameStr.length() - 4);

	if (last == ".pdf") {
		// Convert each page to png and locate redactions
		Inverter inv(fileName);
		for (int i = 1; i <= inv.getNumPages(); i++) {
			// Construct temporary filename
			char fn[100];
			snprintf(fn, sizeof(fn), ".tmp-%d.png", i);

			// Convert to png and set current page
			inv.setPageNumber(i);
			inv.savePng(fn);
			redactions.push_back(handleFile(fn));
		}
	} else {
		redactions.push_back(handleFile(fileName));
	}

	return redactions;
}

std::pair<Pix*, std::vector<Locator::RedactionLocation>> Locator::handleFile(
		char *pngName) {
	image = pixRead(pngName);
	api->SetImage(image);
	api->Recognize(0);
	ri = api->GetIterator();
	if (ri == 0) {
		std::cerr << "Failed to initialize Tesseract results iterator!"
				<< std::endl;
		exit(1);
	}
	return std::pair<Pix*, std::vector<Locator::RedactionLocation>>(image,
			searchFile());
}

void Locator::boundingBoxesForLine(BOXA *arr) {
	bool wordsDone = 0;
	while (!wordsDone) {
		if (ri->IsAtFinalElement(tesseract::RIL_TEXTLINE,
				tesseract::RIL_WORD)) {
			wordsDone = 1;
		}
		int x1, y1, x2, y2;
		ri->BoundingBox(tesseract::RIL_WORD, &x1, &y1, &x2, &y2);
		boxaAddBox(arr, boxCreate(x1, y1, x2 - x1, y2 - y1), L_INSERT);

		if (!wordsDone) {
			ri->Next(tesseract::RIL_WORD);
		}
	}
}

void Locator::expandBorder(PIX *img, BOX *left, BOX *right, int *y1, int *y2) {
	// Saved seperately in case no black pixels are found
	int top = std::min(left->y, right->y);
	int bottom = std::max(left->y + left->h, right->y + right->h);
	int north = top;
	int south = bottom;
	int west = left->x + left->w;
	int width = right->x - west;
	north = south = ((north + south) / 2);
	auto bottomPixels = std::vector<uint>(width);
	auto contVect = std::vector<bool>(width);
	uint pixel; // for checking continuities

	// Expand top
	for (int i = 0; i < width; i++) {
		uint pixel;
		pixGetPixel(img, west + i, north, &pixel);
		if (pixel == BLACK_RGBA) {
			contVect[i] = true;
		} else {
			contVect[i] = false;
		}
	}

	do {
		for (int i = 0; i < width; i++) {
			if (contVect[i]) {
				pixGetPixel(img, west + i, north - 1, &pixel);
				if (pixel != BLACK_RGBA) {
					contVect[i] = false;
				} else {
					for (int j = i - 1; j >= 0; j--) {
						pixGetPixel(img, west + j, north - 1, &pixel);
						if (pixel == BLACK_RGBA) {
							contVect[j] = true;
						} else {
							break;
						}
					}
					for (int j = i + 1; j < width; j++) {
						pixGetPixel(img, west + j, north - 1, &pixel);
						if (pixel == BLACK_RGBA) {
							contVect[j] = true;
							i++;
						} else {
							break;
						}
					}
				}
			}
		}
		north--;
	} while (find(contVect.begin(), contVect.end(), true) != contVect.end());

	// Expand bottom
	for (int i = 0; i < width; i++) {
		uint pixel;
		pixGetPixel(img, west + i, south, &pixel);
		if (pixel == BLACK_RGBA) {
			contVect[i] = true;
		} else {
			contVect[i] = false;
		}
	}

	do {
		for (int i = 0; i < width; i++) {
			if (contVect[i]) {
				pixGetPixel(img, west + i, south + 1, &pixel);
				if (pixel != BLACK_RGBA) {
					contVect[i] = false;
				} else {
					for (int j = i - 1; j >= 0; j--) {
						pixGetPixel(img, west + j, south + 1, &pixel);
						if (pixel == BLACK_RGBA) {
							contVect[j] = true;
						} else {
							break;
						}
					}
					for (int j = i + 1; j < width; j++) {
						pixGetPixel(img, west + j, south + 1, &pixel);
						if (pixel == BLACK_RGBA) {
							contVect[j] = true;
							i++;
						} else {
							break;
						}
					}
				}
			}
		}
		south++;
	} while (find(contVect.begin(), contVect.end(), true) != contVect.end());

	north++;
	south--;
	if (north == south) {
		*y1 = top;
		*y2 = bottom;
	} else {
		*y1 = north - 2;
		*y2 = south + 2;
	}
}

BOX* Locator::findBox(PIX *img, BOX *M, int areaThresh) {
	img = pixCopy(0, img);
	int r = M->y, c = M->x;
	int r_init, c_init; // Beginning of a potential box
	r_init = r;
	c_init = c;
	int southBound = M->y + M->h;
	int eastBound = M->x + M->w;
	BOX *blackBox = 0;
	int aMax = areaThresh;
	uint pixel;
	while (r < southBound) {
		do {
			r_init = r;
			c_init = c;
			pixGetPixel(img, c_init, r_init, &pixel);

			if (c < eastBound) {
				c++;
			} else if (r < southBound) {
				r++;
				c = M->x;
			} else {
				break;
			}
		} while (pixel != BLACK_RGBA);

		r = r_init;
		c = c_init;

		pixGetPixel(img, c, r + 1, &pixel); // Go down
		while (pixel == BLACK_RGBA) {
			r++;
			pixGetPixel(img, c, r + 1, &pixel);
		}
		int bot = r + 1;

		pixGetPixel(img, c + 1, r, &pixel); // Go right
		while (pixel == BLACK_RGBA) {
			c++;
			pixGetPixel(img, c + 1, r, &pixel);
		}
		int right = c + 1;

		pixGetPixel(img, c, r - 1, &pixel); // Go up
		while (pixel == BLACK_RGBA) {
			r--;
			pixGetPixel(img, c, r - 1, &pixel);
		}
		pixGetPixel(img, c - 1, r, &pixel); // Go left
		while (pixel == BLACK_RGBA) {
			c--;
			pixGetPixel(img, c - 1, r, &pixel);
		}

		if (r_init == r && c_init == c) {
			int w = right - c_init;
			int h = bot - r_init;
			if (w * h > aMax) {
				aMax = w * h;
				if (blackBox)
					boxDestroy(&blackBox);
				blackBox = boxCreate(c_init, r_init, w, h);
			}
		} else {
			r = r_init;
			c = c_init;
		}

		pixSetPixel(img, c, r, WHITE_RGBA);
	}

	pixDestroy(&img);

	return blackBox;
}

float Locator::getPercentBlackPixels(PIX *image, BOX *M) {
	int southBound = M->y + M->h;
	int eastBound = M->x + M->w;
	uint pixel;
	float count = 0;

	for (int i = M->x; i < eastBound; i++) {
		for (int j = M->y; j < southBound; j++) {
			pixGetPixel(image, i, j, &pixel); // Go down
			if (pixel == BLACK_RGBA) {
				count++;
			}
		}
	}

	return count / (M->h * M->w);
}

Locator::RedactionLocation Locator::identifyRedactions(PIX *image,
		BOXA *boxes) {
	// for each pair of words (as well as the justification boundaries)
	int numBoxes = boxaGetCount(boxes);
	// For the word bounding boxes
	BOX *wi;
	BOX *wj;

	// Get justification boundaries
	wi = boxaGetBox(boxes, 0, L_COPY);
	wj = boxaGetBox(boxes, numBoxes - 1, L_COPY);
	int leftBorder = wi->x + wi->w;
	int rightBorder = wj->x;

	// For the potential redaction bounding box
	int x1, x2, y1, y2;
	for (int i = 0; i < numBoxes; i++) {
		boxDestroy(&wi);
		wi = boxaGetBox(boxes, i, L_COPY);
		for (int j = i + 1; j < numBoxes; j++) {
			boxDestroy(&wj);
			wj = boxaGetBox(boxes, j, L_COPY);
			x1 = wi->x + wi->w;
			x2 = wj->x;
			// Handles the case where the justification is equal to the
			// side of the word bounding box
			if (x2 - x1 < 1)
				continue;

			expandBorder(image, wi, wj, &y1, &y2);

			int h = y2 - y1;
			int w = x2 - x1;
			BOX *Mbox = boxCreate(x1, y1, w, h);
			int thresh = alpha * h * w;
			BOX *blackBox = findBox(image, Mbox, thresh);
			if (blackBox) {
				std::cout << "REDACTION FOUND, ALPHA: "
						<< float(blackBox->w * blackBox->h) / (w * h)
						<< std::endl;
				boxDestroy(&Mbox);
				return {-1, -1, i, j, leftBorder, rightBorder, blackBox};
			} else {
				float b = getPercentBlackPixels(image, Mbox);
				if (b >= 1 - beta) {
					std::cout << "REDACTION FOUND, BETA: " << b << std::endl;
					boxDestroy(&blackBox);
					return {-1, -1, i, j, leftBorder, rightBorder, Mbox};
				} else if ((1 - b) >= gamma) {
					// std::cout << "POTENTIAL WHITESPACE REDACTION: (1 - b): "
					// 		<< (1 - b) << " Width: " << w << std::endl;
				}
			}
			boxDestroy(&Mbox);
			boxDestroy(&blackBox);
		}
	}
	return {-1, -1, -1, -1, leftBorder, rightBorder, nullptr};
}

std::vector<Locator::RedactionLocation> Locator::searchFile() {
	int paragraphs = 0;
	int lineNum = 0;
	std::vector<Locator::RedactionLocation> res;
	// for each paragraph
	do {
		// get the justification
		int left, y1, right, y2;
		ri->BoundingBox(tesseract::RIL_PARA, &left, &y1, &right, &y2);

		// for each line in that paragraph
		bool paraDone = 0;
		while (!paraDone) {
			if (ri->IsAtFinalElement(tesseract::RIL_PARA,
					tesseract::RIL_TEXTLINE)) {
				paraDone = 1;
			}

			// Identify the redactions
			BOXA *wordBoxes = boxaCreate(1);
			boxaAddBox(wordBoxes, boxCreate(left, y1, 1, y2 - y1), L_INSERT);
			boundingBoxesForLine(wordBoxes);
			boxaAddBox(wordBoxes, boxCreate(right, y1, 1, y2 - y1), L_INSERT);
			auto redaction = identifyRedactions(image, wordBoxes);

			if (redaction.bbox) {
				redaction.paragraph = paragraphs;
				redaction.line = lineNum;
				res.push_back(redaction);
			}

			boxaDestroy(&wordBoxes);

			if (!paraDone) {
				ri->Next(tesseract::RIL_TEXTLINE);
				lineNum++;
			}
		}

		paragraphs++;
		lineNum = 0;
	} while (ri->Next(tesseract::RIL_PARA));

	return res;
}

#ifdef PAINT_REDACTIONS
/**
 * Paints red boxes over redactions in a file
 */
int main(int argc, const char *argv[]) {
  int numImages = 1; // Number of images to look for redactions in

  if (argc != 2) {
    fprintf(stderr, "Please supply an input file.\n");
    exit(1);
  }

  Locator loc((char *)argv[1]);
  auto redactions = loc.findRedactions();

  int i = 0;
  for (auto p : redactions) {
    // Paint all redactions on the image
    if (p.second.size()) {
      for (auto r : p.second) {
        BOXA *boxa = boxaCreate(1);
        boxaAddBox(boxa, r.bbox, L_INSERT);
        p.first = pixPaintBoxa(p.first, boxa, 0xFF0000FF);
      }

      // Save all the images
      char fn[100];
      snprintf(fn, sizeof(fn), "out-%d.png", i);
      pixWrite(fn, p.first, IFF_PNG);
      pixDestroy(&p.first);
      i++;
    }
  }

  return 0;
}
#endif
