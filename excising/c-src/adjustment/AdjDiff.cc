/**
 * Reads in a prefix, a suffix vector string and then after that
 * a list of additional vector strings. For each, it gets the 
 * absolute value of the difference between the prefix/suffix
 * strings and the guess string, and prints it to stdout.
 */
#include <string>
#include <iostream>
#include <vector>
#include "VectorString.h"
int main() {
	setlocale(LC_ALL, "");
	std::wcin.imbue(std::locale("en_US.UTF-8"));
	std::wcerr.imbue(std::locale("en_US.UTF-8"));
	std::wcout.imbue(std::locale("en_US.UTF-8"));
	std::wstring lTJs, rTJs;
	std::getline(std::wcin, lTJs);
	std::getline(std::wcin, rTJs);
	AdjAlgs::VectorString lTJ(lTJs);
	AdjAlgs::VectorString rTJ(rTJs);
	for (std::wstring line; std::getline(std::wcin, line);) {
		if (line != L"") {
			AdjAlgs::VectorString* guessVs = new AdjAlgs::VectorString(line);
			double pre_sub_adj =
				guessVs->pre_l1(lTJ, false) + guessVs->suf_l1(rTJ, false);
			std::wcout << pre_sub_adj << std::endl;
		}
	}
}
