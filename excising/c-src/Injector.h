/**
 * Basic Naive Guesser. Takes in a text file with a list of potential text
 * guesses and a PDF, locates the redactions, and does a linear search of the
 * text list for inputs matching the PDF metadata width constraints exactly.
 */

#ifndef KERNING_NAIVEGUESSER_H
#define KERNING_NAIVEGUESSER_H

class Injector {
	/**
	 * Makes a duplicate of the current gfx render state for performing render
	 * operations on
	 */
	void copyState();

protected:
	/* Saved, and then used to inject guesses for the text state */
	Gfx* gfx = nullptr;
	/* Stores the initial state to re-initialize the injection state */
	GfxState* initialState = nullptr;
	GfxState* state = nullptr;
	/* Output device that records renders */
	TextOutputDev* out = nullptr;
	double x_i, y_i;
  FakeCMap fCMap;

public:
	const double DPI = 300.0;

	Injector(Inverter* inv, double x, double y, FakeCMap fCMap_) : x_i(x), y_i(y), fCMap(fCMap_) {
		gfx = inv->getGfx();
		out = inv->getTextOut();
		initialState = out->getFirstState();
		state = out->getRenderState(x, y);
		state->setCharSpace(0);
		state->setWordSpace(0);
	}

	~Injector() { 
		delete gfx;
		delete initialState; 
		delete state; 
		}

	/**
	 * Actually injects the string into the gfx state. Updates the
   * vector string with the state values for each injected
   * character returns non-zero if error
	 */
	int injectStringToGfx(AdjAlgs::VectorString* vs);
};

#endif
