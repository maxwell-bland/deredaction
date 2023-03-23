/*
 *  Created on: Oct 22, 2020
 *      Author: bland
 */

#ifndef WIDTHMATCHER_H_
#define WIDTHMATCHER_H_

class PrintInjector : public Injector {
public:
	const double localityRange = 0.00001;
	const double epsilon = 0.000001;

	PrintInjector(Inverter* inv, int num_procs, float x, float y, FakeCMap fCMap_)
    : Injector(inv, x, y, fCMap_), num_procs(num_procs) {}

	void guess();

	/**
	 * Reads words to guess on a loop from stdin until EOF
	 */
	void readWords();

	/**
	 * This function spawns off a bunch of child processes and populates
	 * a result set that includes all the potential guess words and their
	 * widths.
	 */
	std::set<std::pair<double, std::wstring>> getWordWidths();

	/**
	 * Forks off a child process to split up computation of guess widths, and
	 * communicates results back to the parent.
	 *
	 * Returns the pipe that the child will write to.
	 *
	 * We have to use this rather than threads because poppler stores a bunch of
	 * state about the text output device, and without spending hours analyzing
	 * and creating some copy function, it isn't really possible to multithread
	 * the PDF lib.
	 */
	int multiprocWidths(int start, int end);
	std::vector<double> getRenderWidths(int start, int end);

	/**
	 * Injects the given string into the PDF state and then print, for each
	 * character, a space-seperated list of the index, the state X value and the
	 * character itself.
	 *
	 * @return non-zero on error
	 */
	int injectAndPrintResults(AdjAlgs::VectorString* vs);
	/**
	 * Injects the vector string into the PDF state and updates it with the proper
	 * state x values for each character.
	 *
	 * @return non-zero on error
	 */
	int getRenderWidth(AdjAlgs::VectorString* vs);

	void setAdjEps(double adj) { adj_epsilon = adj; }

private:
	/* Guesses for redacted content sorted by width */
	std::pair<std::wstring, float> widthSortedGuesses;

	/* Words read in until EOF */
	std::vector<std::wstring> words;

	/* The number of processes to use when parallelizing guessing */
	int num_procs;

	double adj_epsilon;
};

#endif /* WIDTHMATCHER_H_ */
