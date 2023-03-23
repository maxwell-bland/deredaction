/*
 * VectorString.cc
 *
 *  Created on: Aug 11, 2020
 *      Author: bland
 */
#include <vector>
#include <iostream>
#include <iterator>
#include <string>
#include <regex>
#include "VectorString.h"

namespace AdjAlgs {

	VectorString::VectorString(std::wstring s) {
		std::wsmatch m;
		std::wregex e(L"(\\(TJBREAK\\)|\\(-?\\d+\\.?\\d*\\))");

		if (s.front() == L'[' && s.back() == L']') {
			s = s.substr(1, s.npos);
			s.pop_back();
		}

		int i = 0;
		auto itr = s.begin();
		while (std::regex_search(s, m, e)) {
			auto x = m[0];
			while (itr != x.first) {
				displacements.push_back(0);
				characters += *itr;
				itr++;
				i++;
			}
			std::wstring adj = x.str();
			displacements[i - 1] = std::stod(adj.substr(1, adj.npos - 2));
			s = m.suffix().str();
			itr = s.begin();
		}

		characters += s;
		displacements.resize(characters.size(), 0);
		displacementDiffs.resize(characters.size(), 0);
		stateXs.resize(characters.size() * 2, 0);
	}

} // namespace AdjAlgs
