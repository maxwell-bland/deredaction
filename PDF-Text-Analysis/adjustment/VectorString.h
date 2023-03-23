/*
 * VectorString.h
 *
 *  Created on: Aug 11, 2020
 *      Author: bland
 */

#ifndef ADJALGS_VECTORSTRING_H_
#define ADJALGS_VECTORSTRING_H_

namespace AdjAlgs {

/**
 * Representation of a string with displacement vector values for
 * each position. Displacements are before the characters.
 */
class VectorString {
  std::wstring characters;
  std::vector<double> displacements;
  std::vector<int> displacementDiffs;
  /*
     This is initialized as an array of 0's, and when
     the vector string is injected into a PDF, the
     X values for the state before and after that
     character are both pushed into this array,
     so indexing is stateXs[char_index * 2 + [0 for pre 1 for post]]
  */
  std::vector<double> stateXs;

public:
  VectorString(std::wstring s);

  VectorString(VectorString& o)
    : characters(o.getString()), displacements(o.getDisplacements()), displacementDiffs(o.getDispDiffs()) {}

  void resetIndex(void);

  int size(void) { return characters.size(); }

  wchar_t getChar(int ind) { return characters[ind]; }

  double getDisplacement(int ind) { return displacements[ind]; }

  std::vector<double> getDisplacements() {
    return std::vector<double>(displacements);
  }

  std::vector<int> getDispDiffs() {
    return std::vector<int>(displacementDiffs);
  }

  void addDisplacement(int ind, double val) { displacements[ind] += val; }

  void setDisplacement(int ind, double val) { displacements[ind] = val; }

  void fixCharWidth(wchar_t to_change, double delta) {
    int i = 0;
    for (auto c : characters) {
      if (c == to_change) {
        displacements[i] += delta;
      }
      i++;
    }
  }

  operator std::wstring() {
    std::wstring res(L"[");
    int i = 0;
    for (auto c : characters) {
      if (displacementDiffs[i])
        res += L"\033[32m";
      res += c;
      if (displacements[i] != 0) {
        res += L"(" + std::to_wstring(displacements[i]) + L")";
      }
      if (displacementDiffs[i])
        res += L"\033[0m";
      i++;
    }
    res += L"]";
    return res;
  }

  std::wstring substr(int start, int len) {
    std::wstring res(L"[");
    for (int i = start; i < len; i++) {
      res += characters[i];
      if (displacements[i] != 0) {
        res += L"(" + std::to_wstring(displacements[i]) + L")";
      }
      i++;
    }
    res += L"]";
    return res;
  }

  std::wstring getString() { return std::wstring(characters); }

  void setStateX(int i, double xPre, double xPost) {
    stateXs[i * 2] = xPre;
    stateXs[i * 2 + 1] = xPost;
  }
  double getPreStateX(int i) { return stateXs[i * 2]; }
  double getPostStateX(int i) { return stateXs[i * 2 + 1]; }

  /**
   * Computes the l1 distance for each dimension between another
   * vectorstring and the prefix of this one, stores it in diffs
   */
  double pre_l1(VectorString& o, bool abs) {
    std::vector<double> v2 = o.getDisplacements();
    double result = 0;
    int len = o.size();

    /* If the prefix has a space character at the end we ignore the adjustment
     * on that space character */
    if (o.getChar(len - 1) == L' ') {
      len = len - 1;
    }

    for (int i = 0; i < len; i++) {
      double val = v2[i] - displacements[i];
      if (abs)
        if (val < 0)
          val = 0 - val;

      if (val) {
        displacementDiffs[i] = 1;
      } else {
        displacementDiffs[i] = 0;
      }
      result += val;
    }

    return result;
  }

  double suf_l1(VectorString& o, bool abs) {
    std::vector<double> v2 = o.getDisplacements();
    double result = 0;
    for (int i = 0; i < v2.size(); i++) {
      double val =
          v2[v2.size() - 1 - i] - displacements[displacements.size() - 1 - i];
      if (abs)
        if (val < 0)
          val = 0 - val;

      if (val) {
        displacementDiffs[displacementDiffs.size() - 1 - i] = 1;
      } else {
        displacementDiffs[displacementDiffs.size() - 1 - i] = 0;
      }

      result += val;
    }

    return result;
  }
};
} // namespace AdjAlgs

#endif /* DISPLACEMENT_VECTORSTRING_H_ */
