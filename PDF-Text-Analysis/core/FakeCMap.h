/**
 * Handles parsing out a one-line CMap supplied via the command line into 
 * an interval tree data structure and then translates non-ascii characters
 * to the right CID (Poppler CharCode) on the fly.
 *
 * The CMAP format as input is 
 * 0x12 0x33 ... 0x78|0x56 0x59 0xAB
 * where pre-pipe are pair-mappings (cid first) and post-pipe are range mappings.
 * Of course the numbers just get passed to stoi so the 0x is unneccessary.
 *
 * No spaces pre&post pipe!
 */
#ifndef FAKECMAP_H
#define FAKECMAP_H
#include <map>
#include <iostream>
#include "IntervalTree.h"

class FakeCMap {
	std::map<int,int> direct;
	IntervalTree<int, int> interv;

public:
  FakeCMap(std::wstring cmap) {
    int x,y,z;
    size_t i = 0, j = 0;
    std::vector<Interval<int, int>> ints;
    /* Expects a plain '|' if no cmap */
    if (cmap.size() != 1) {
      if (cmap[i] != L'|') {
        do {
          x = std::stoi(&cmap[i], &j, 16);
          i += j;
          y = std::stoi(&cmap[i], &j, 16);
          direct[y] = x;
          i += j;
        } while (cmap[i++] != L'|');
      } else {
        i++;
      }

      while (i < cmap.size()) {
        x = std::stoi(&cmap[i], &j, 16);
        i += j;
        y = std::stoi(&cmap[i], &j, 16);
        i += j;
        z = std::stoi(&cmap[i], &j, 16);
        i += j;
        ints.push_back(Interval<int, int>(z, z + y - x, x));
      }
    }
    interv = IntervalTree<int, int>(ints);
  }


	wchar_t get( wchar_t i ) {
		if (direct.find(i) != direct.end())
			return direct[i];
		else {
			auto results = interv.findOverlapping(i, i);
			if (results.size())
				return i - results[0].start + results[0].value;
		}

		return i;
	}

};

#endif
