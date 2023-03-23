/*
 * BlackBoxReduction.cpp
 *
 * Uses a maximum adjustment value and a document with a redaction in it to
 * reduce the possibilities for a black-box line reconstruction attack
 * to just those which are reasonably within the redaction bounds.
 *
 *  Created on: Oct 22, 2020
 *      Author: bland
 */

#include <string>
#include <functional>
#include <set>
#include <sys/wait.h>
#include <iomanip>
#include "poppler/Gfx.h"
#include "core/Inverter.h"
#include "adjustment/VectorString.h"
#include "FakeCMap.h"
#include "Injector.h"
#include "PrintInjector.h"

void PrintInjector::readWords() {
	for (std::wstring line; std::getline(std::wcin, line);)
		if (line != L"")
			words.push_back(line);
}

int PrintInjector::getRenderWidth(AdjAlgs::VectorString* vs) {
	vs->addDisplacement(0, adj_epsilon);
	if (injectStringToGfx(vs))
		return 1;

	return 0;
}

int PrintInjector::injectAndPrintResults(AdjAlgs::VectorString* vs) {
	if (getRenderWidth(vs))
		return 1;

	std::wprintf(L"%.17g\n", adj_epsilon);
	for (int i = 0; i < vs->size(); i++) {
		std::wprintf(L"%d\t%lc\t%.17g\t%.17g\n", i, vs->getChar(i), vs->getPreStateX(i),
				vs->getPostStateX(i));
	}
	std::wcout << std::endl;

	return 0;
}

#ifdef WIDTH_MATCH
int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "USAGE: {input PDF} {adjustment epsilon}" << std::endl;
		exit(1);
	}

	setlocale(LC_ALL, "");
	std::wcin.imbue(std::locale("en_US.UTF-8"));
	std::wcerr.imbue(std::locale("en_US.UTF-8"));
	std::wcout.imbue(std::locale("en_US.UTF-8"));

	float x, y, adj_quantum;
	int pgnum;
	std::wstring prefix_TJ, suffix_TJ;

	std::wcin >> x;
	std::wcin >> y;
	std::wcin >> pgnum;
	std::wcin >> adj_quantum;

	int base_adj = std::stoi(argv[2]);

	std::wstring fakeCMap;
	/* Discard whitespace after adj_quantum */
	std::getline(std::wcin, fakeCMap); 
	std::getline(std::wcin, fakeCMap);

	/* TODO: remove this and just read in the adjustment directly with the line */
	std::getline(std::wcin, prefix_TJ);
	std::getline(std::wcin, suffix_TJ);
	AdjAlgs::VectorString preTJ(prefix_TJ);
	AdjAlgs::VectorString sufTJ(suffix_TJ);

	Inverter* inv = new Inverter(argv[1]);
	inv->setPageNumber(pgnum);
	PrintInjector bbr(inv, 1, x, y, FakeCMap(fakeCMap));

	for (std::wstring line; std::getline(std::wcin, line);) {
		if (line != L"") {
			AdjAlgs::VectorString* guessVs = new AdjAlgs::VectorString(line);

			/*
			   We scale in the absolute difference in overall
			   adjustment of the prefix and suffix in order to
			   normalize our target result for TJ Break errors.
			   Later we can validate this using other scripts.
			   */
			double pre_sub_adj =
				guessVs->pre_l1(preTJ, false) + guessVs->suf_l1(sufTJ, false);

			bbr.setAdjEps(base_adj + pre_sub_adj);
			if (bbr.injectAndPrintResults(guessVs)) {
				std::cout << std::endl;
				std::wcerr << "ERROR: Injection Failed " << line << std::endl;
			}
			delete guessVs;
		}
	}

	delete inv;
}
#endif
