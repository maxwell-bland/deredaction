/**
 * This class specifies a redaction within a PDF document, including a subpixel
 * accurate width, whether it contains an annotation or the secret text is in
 * the document, toUnicode restrictions, etc
 */

#ifndef KERNING_REDACTION_H
#define KERNING_REDACTION_H
#include "Gfx.h"
#include "TextOutputDev.h"
#include "Locator.h"

class Redaction {
private:
	/**
	 * Represents the location that a particular guess for the
	 * word of the redaction should hit in the render space
	 * of the page. Currently, the space character before
	 * the word to the right of the redaction.
	 */
	double target;
	GfxState *inject;

	/**
	 * Contains the actual metadata characters associated with this
	 * redaction object.
	 */
	std::vector<GfxRenderInfo> allChars;
	// Words internal to the redaction
	std::vector<GfxRenderInfo> inChars;

	TextWord *leftWord;
	TextWord *rightWord;
	int pgNum;

public:
	/**
	 * Initializes the redaction, calculating subpixel accurate width, etc.
	 * sets the metadata-precise width of a particular redaction, bounded
	 * by the xMin and xMax values of the redactionLocation
	 */
	Redaction(Inverter inverter, int pgNum, Locator::RedactionLocation loc);

	operator std::string() const {
		std::string strRepr;
		return strRepr;
	}

	double getTarget() {
		return target;
	}

	const std::vector<GfxRenderInfo>& getAllChars() const {
		return allChars;
	}

	GfxState* getInjectionState() {
		return inject;
	}

	/**
	 * Initializes state variables with a potentially noisy representation of
	 * the characters in the redaction.
	 */
	void initNoisyCharacters(const Locator::RedactionLocation &loc, int height,
			Inverter &inverter);
	/**
	 * Initializes words to the left and right of the redacction
	 */
	int initSurroundingWords(const Locator::RedactionLocation &loc,
			Inverter &inverter);

private:
	/**
	 * Initializes the width of this redaction by subtracting the X coordinate
	 * from the
	 */
	void initTarget();
};

#endif
