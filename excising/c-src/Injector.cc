
#include <functional>
#include <string>
#include "lib/poppler/poppler/GfxState.h"
#include "poppler/Gfx.h"
#include "core/Inverter.h"
#include "adjustment/VectorString.h"
#include "FakeCMap.h"
#include "Injector.h"

void Injector::copyState() {
  GfxState* stateCopy = state->copy(true);
  /* Handles poppler bug that doesn't scale page by DPI */
  stateCopy->setPageWidth(initialState->getPageWidth() * (DPI / 72.0));
  stateCopy->setPageHeight(initialState->getPageHeight() * (DPI / 72.0));

  /* Doesn't actually read params 0 and 2 */
  out->startPage(0, stateCopy, nullptr);

  gfx->restoreStateStack(stateCopy);
}

int Injector::injectStringToGfx(AdjAlgs::VectorString* vs) {
  std::vector<GfxRenderInfo*> res;
  copyState();

  /* Loop through vector injecting characters and displacements */
  std::wstring inj = L"";
  int valid = 1;
  for (int i = 0; i < vs->size(); i++) {

    inj += fCMap.get(vs->getChar(i));

    double disp = vs->getDisplacement(i);
    if (disp) {
      if ((res = gfx->injectGuess(inj)).size() == 0)
        return 1;

      int j = i;

      gfx->injectTextShift(0, disp);

      GfxRenderInfo* shiftRi = out->popLastRender();
      for (auto ri : res) {
        if (ri && ri->postState) {
          if (shiftRi) {
            vs->setStateX(j, ri->x, ri->postState->getCurX() + shiftRi->dx);
          } else {
            vs->setStateX(j, ri->x, ri->postState->getCurX());
          }
        } else {
          vs->setStateX(j, -1, -1);
          valid = 0;
        }
        j--;
        delete ri;
      }
      delete shiftRi;

      inj = L"";
      res.clear();
    }
  }

  if (inj != L"") {
    res = gfx->injectGuess(inj);
    if (res.size() == 0)
      return 1;

    int j = vs->size() - 1;
    for (auto ri : res) {
      if (ri && ri->postState) {
        vs->setStateX(j, ri->x, ri->postState->getCurX());
      } else {
        vs->setStateX(j, -1, -1);
        valid = 0;
      }
      j--;
      delete ri;
    }
    res.clear();
  }

  if (!valid) {
    return 1;
  }

  return 0;
}
