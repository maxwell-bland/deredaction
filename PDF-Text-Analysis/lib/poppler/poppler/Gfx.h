//========================================================================
//
// Gfx.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2007 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2008 Brad Hards <bradh@kde.org>
// Copyright (C) 2008, 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009-2013, 2017, 2018 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009, 2010, 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2010 David Benjamin <davidben@mit.edu>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company,
// <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2019 Volker Krause <vkrause@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef GFX_H
#define GFX_H

#include "CharCodeToUnicode.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "Object.h"
#include "PopplerCache.h"
#include "poppler-config.h"

#include <locale>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

class GooString;
class PDFDoc;
class XRef;
class Array;
class Stream;
class Parser;
class Dict;
class Function;
class OutputDev;
class GfxFontDict;
class GfxFont;
class GfxPattern;
class GfxTilingPattern;
class GfxShadingPattern;
class GfxShading;
class GfxFunctionShading;
class GfxAxialShading;
class GfxRadialShading;
class GfxGouraudTriangleShading;
class GfxPatchMeshShading;
struct GfxPatch;
class GfxState;
struct GfxColor;
class GfxColorSpace;
class Gfx;
class PDFRectangle;
class AnnotBorder;
class AnnotColor;
class Catalog;
struct MarkedContentStack;

//------------------------------------------------------------------------

enum GfxClipType { clipNone, clipNormal, clipEO };

enum TchkType {
	tchkBool,		// boolean
	tchkInt,		// integer
	tchkNum,		// number (integer or real)
	tchkString, // string
	tchkName,		// name
	tchkArray,	// array
	tchkProps,	// properties (dictionary or name)
	tchkSCN,		// scn/SCN args (number of name)
	tchkNone		// used to avoid empty initializer lists
};

#define maxArgs 33

struct Operator {
	char name[4];
	int numArgs;
	TchkType tchk[maxArgs];
	void (Gfx::*func)(Object args[], int numArgs);
};

//------------------------------------------------------------------------

class GfxResources {
public:
	GfxResources(XRef *xref, Dict *resDict, GfxResources *nextA);
	~GfxResources();

	GfxResources(const GfxResources &) = delete;
	GfxResources &operator=(const GfxResources &other) = delete;

	GfxFont *lookupFont(const char *name);
	const GfxFont *lookupFont(const char *name) const;
	Object lookupXObject(const char *name);
	Object lookupXObjectNF(const char *name);
	Object lookupMarkedContentNF(const char *name);
	Object lookupColorSpace(const char *name);
	GfxPattern *lookupPattern(const char *name, OutputDev *out, GfxState *state);
	GfxShading *lookupShading(const char *name, OutputDev *out, GfxState *state);
	Object lookupGState(const char *name);
	Object lookupGStateNF(const char *name);

	GfxResources *getNext() const { return next; }

private:
	GfxFont *doLookupFont(const char *name) const;

	GfxFontDict *fonts;
	Object xObjDict;
	Object colorSpaceDict;
	Object patternDict;
	Object shadingDict;
	Object gStateDict;
	PopplerCache<Ref, Object> gStateCache;
	XRef *xref;
	Object propertiesDict;
	GfxResources *next;
};

// ============= Redaction Classes ============

/**
 * The different rendering modifications to the text space of the PDF that can
 * occur. Used in deredaction for classifying portions of the document
 * structure.
 */
enum class GfxTextRender {
  Char,     // A regular old character
  Merge,    // Used to combine two words together (not seen in practice)
  Null,     // Character value was null (ignored)
  Space,    // A space character (word spacing is added)
  Tinychar, // For tinychars limit
  Sanity,   // outside page bounds or character wrong size
  Shift, // Adjustment added to the next character for kerning or other purposes
  TJ // An end of a text object
};

/**
 * Creates an object containing the rendered character's information, used
 * by the kerning redaction system in width calculation, etc.
 */
class GfxRenderInfo {
public:
  GfxRenderInfo(GfxTextRender charType, double charVal, double xPos,
                double yPos, double spAmt, double width, int unicodeInd,
                double o_dx, double o_dy, GfxState *st)
    : state(nullptr), postState(nullptr), rct(charType), val(charVal), x(xPos), y(yPos), sp(spAmt), w(width),
      unicodeIndex(unicodeInd), dx(o_dx), dy(o_dy) {
    if (st != nullptr) {
      state.reset(st->copy(true));
    }
  }

  std::wstring getDesc() {
    std::wstring desc;

    switch (rct) {
    case GfxTextRender::Char:
      desc = L"CH[" + getChar() + L"]";
      break;
    case GfxTextRender::Merge:
      desc = L"MERGE";
      break;
    case GfxTextRender::Null:
      desc = L"NULL";
      break;
    case GfxTextRender::Space:
      desc = L"SP";
      wchar_t unicodeRepr[128];
      swprintf(unicodeRepr, 128, L"[%04X]", (int) val);
      desc += std::wstring(unicodeRepr);
      break;
    case GfxTextRender::Tinychar:
      desc = L"TINYCHAR";
      break;
    case GfxTextRender::Sanity:
      desc = L"SANITY";
      break;
    case GfxTextRender::Shift:
      desc = L"SH";
      wchar_t numRepr[128];
      swprintf(numRepr, 128, L"[%lf]", val);
      desc += std::wstring(numRepr);
      break;
    case GfxTextRender::TJ:
      desc = L"TJ";
      break;
    }
    
    return desc;
  }

  std::wstring getFontInfo () {
    std::wstring fontInfo = L"";
    if (state.get() != nullptr) {
      GfxFont *font = state->getFont();
      if (font) {
        const GooString *fName = font->getName();
        if (fName) {
          wchar_t strRepr[128];
            swprintf(strRepr, 128, L"%d,%s", font->getID()->num, fName->c_str());
          fontInfo += (std::wstring(strRepr) + L"[" + std::to_wstring(state->getFontSize())) + L"]";
        }
      }
    }

    return fontInfo;
  }

  std::wstring getChar() {
    setlocale(LC_CTYPE, "");
    switch (rct) {
    case GfxTextRender::Space:
      return L" ";
    default:
      wchar_t strRepr[128];
      swprintf(strRepr, 128, L"%lc",
               (val) ? (unsigned short)val : 0);
      return std::wstring(strRepr);
    }
  }

	GfxTextRender getType() { return rct; }

	double getVal() { return val; }

	void setVal(double v) { val = v; }

	unsigned short getUnicodeVal() {
		switch (rct) {
		case GfxTextRender::Space:
			return ' ';
		default:
			return getVal();
		}
	}

  std::shared_ptr<GfxState> state; // State in which the render info was recorded
  std::shared_ptr<GfxState> postState; // State after the render info was recorded
	GfxTextRender rct; // Type of render
	/* Character value, zero if not a character, or glyph adjustment value */
  /* These are public because for some reason my ****ing compiler is ****ing
   bugged and I hate whoever pushed the code that broke this. */
	double val;
	double x;						 // X coord (subpixel)
	double y;						 // Y coord (subpixel)
	double sp;					 // Spacing. For certain Enum types, this is the width
	double w;						 // Width of the character, if it is a character
	double unicodeIndex; // for multiply-encoded characters
	double dx;					 // X coord (subpixel)
  double dy;                                       // Y coord (subpixel)
  double getX() { return x + (unicodeIndex + 1) * dx; }
  double getXSp() { return x + (unicodeIndex + 1) * dx + sp; }
};

// ============================================

//------------------------------------------------------------------------
// Gfx
//------------------------------------------------------------------------

class Gfx {
public:
	// Constructor for regular output.
	Gfx(PDFDoc *docA, OutputDev *outA, int pageNum, Dict *resDict, double hDPI,
			double vDPI, const PDFRectangle *box, const PDFRectangle *cropBox,
			int rotate, bool (*abortCheckCbkA)(void *data) = nullptr,
			void *abortCheckCbkDataA = nullptr, XRef *xrefA = nullptr);

	// Constructor for a sub-page object.
	Gfx(PDFDoc *docA, OutputDev *outA, Dict *resDict, const PDFRectangle *box,
			const PDFRectangle *cropBox, bool (*abortCheckCbkA)(void *data) = nullptr,
			void *abortCheckCbkDataA = nullptr, Gfx *gfxA = nullptr);
#ifdef USE_CMS
	void initDisplayProfile();
#endif
	~Gfx();

	Gfx(const Gfx &) = delete;
	Gfx &operator=(const Gfx &other) = delete;

	XRef *getXRef() { return xref; }

	// Interpret a stream or array of streams.
	void display(Object *obj, bool topLevel = true);

	// Display an annotation, given its appearance (a Form XObject),
	// border style, and bounding box (in default user space).
	void drawAnnot(Object *str, AnnotBorder *border, AnnotColor *aColor,
								 double xMin, double yMin, double xMax, double yMax,
								 int rotate);

	// Save graphics state.
	void saveState();

	// Push a new state guard
	void pushStateGuard();

	// Restore graphics state.
	void restoreState();

	// Pop to state guard and pop guard
	void popStateGuard();

	// Get the current graphics state object.
	GfxState *getState() { return state; }

	bool checkTransparencyGroup(Dict *resDict);

	void drawForm(Object *str, Dict *resDict, const double *matrix,
								const double *bbox, bool transpGroup = false,
								bool softMask = false,
								GfxColorSpace *blendingColorSpace = nullptr,
								bool isolated = false, bool knockout = false,
								bool alpha = false, Function *transferFunc = nullptr,
								GfxColor *backdropColor = nullptr);

	void pushResources(Dict *resDict);
	void popResources();

	/**
	 * Nicely translate to a particular encoded GooObject string to a unicode
	 * string. (Based on the current font state).
	 */
	std::basic_string<unsigned short>
	gooStringToUnicode(const GooString *encoded);

	/**
	 * Encodes a GooString using the current state's font CMAP.
	 */
	std::vector<CharCode> gooStringToCharCode(const GooString *encoded);

	/**
	 *  Naughty naughty!
     *
     * @return the GfxRenderInfo* of each character in reverse order!
	 */
  std::vector<GfxRenderInfo*> injectGuess(std::wstring guess);

	/**
	 * TODO: move this back into a private context
	 */
  void recordNewTextShow();
  void restoreStateStack(GfxState *oldState);
  void injectTextShift(int wMode, const double num);

private:
  PDFDoc *doc;
  XRef *xref;           // the xref table for this PDF file
  Catalog *catalog;     // the Catalog for this PDF file
  OutputDev *out;       // output device
  bool subPage;         // is this a sub-page object?
  bool printCommands;   // print the drawing commands (for debugging)
  bool profileCommands; // profile the drawing commands (for debugging)
  bool commandAborted;  // did the previous command abort the drawing?
  GfxResources *res;    // resource stack
  int updateLevel;

  GfxState *state = nullptr;    // current graphics state
  int stackHeight;              // the height of the current graphics stack
  std::vector<int> stateGuards; // a stack of state limits; to guard
                                // against unmatched pops
  bool fontChanged;             // set if font or text matrix has changed
  GfxClipType clip;             // do a clip?
  int ignoreUndef;              // current BX/EX nesting level
  double baseMatrix[6];         // default matrix for most recent
  //   page/form/pattern
  int formDepth;
  bool ocState; // true if drawing is enabled, false if
  //   disabled

  MarkedContentStack *mcStack; // current BMC/EMC stack

  Parser *parser; // parser for page content stream(s)

  std::set<int> formsDrawing;    // the forms/patterns that are being drawn
  std::set<int> charProcDrawing; // the charProc that are being drawn

  bool // callback to check for an abort
      (*abortCheckCbk)(void *data);
  void *abortCheckCbkData;

  static const Operator opTab[]; // table of operators

  void go(bool topLevel);
  void execOp(Object *cmd, Object args[], int numArgs);
  const Operator *findOp(const char *name);
  bool checkArg(Object *arg, TchkType type);
  Goffset getPos();

  int bottomGuard();

  // graphics state operators
  void opSave(Object args[], int numArgs);
  void opRestore(Object args[], int numArgs);
  void opConcat(Object args[], int numArgs);
  void opSetDash(Object args[], int numArgs);
  void opSetFlat(Object args[], int numArgs);
  void opSetLineJoin(Object args[], int numArgs);
  void opSetLineCap(Object args[], int numArgs);
  void opSetMiterLimit(Object args[], int numArgs);
  void opSetLineWidth(Object args[], int numArgs);
  void opSetExtGState(Object args[], int numArgs);
  void doSoftMask(Object *str, bool alpha, GfxColorSpace *blendingColorSpace,
                  bool isolated, bool knockout, Function *transferFunc,
                  GfxColor *backdropColor);
  void opSetRenderingIntent(Object args[], int numArgs);

  // color operators
  void opSetFillGray(Object args[], int numArgs);
  void opSetStrokeGray(Object args[], int numArgs);
  void opSetFillCMYKColor(Object args[], int numArgs);
  void opSetStrokeCMYKColor(Object args[], int numArgs);
  void opSetFillRGBColor(Object args[], int numArgs);
  void opSetStrokeRGBColor(Object args[], int numArgs);
  void opSetFillColorSpace(Object args[], int numArgs);
  void opSetStrokeColorSpace(Object args[], int numArgs);
  void opSetFillColor(Object args[], int numArgs);
  void opSetStrokeColor(Object args[], int numArgs);
  void opSetFillColorN(Object args[], int numArgs);
  void opSetStrokeColorN(Object args[], int numArgs);

  // path segment operators
  void opMoveTo(Object args[], int numArgs);
  void opLineTo(Object args[], int numArgs);
  void opCurveTo(Object args[], int numArgs);
  void opCurveTo1(Object args[], int numArgs);
  void opCurveTo2(Object args[], int numArgs);
  void opRectangle(Object args[], int numArgs);
  void opClosePath(Object args[], int numArgs);

  // path painting operators
  void opEndPath(Object args[], int numArgs);
  void opStroke(Object args[], int numArgs);
  void opCloseStroke(Object args[], int numArgs);
  void opFill(Object args[], int numArgs);
  void opEOFill(Object args[], int numArgs);
  void opFillStroke(Object args[], int numArgs);
  void opCloseFillStroke(Object args[], int numArgs);
  void opEOFillStroke(Object args[], int numArgs);
  void opCloseEOFillStroke(Object args[], int numArgs);
  void doPatternFill(bool eoFill);
  void doPatternStroke();
  void doPatternText();
  void doPatternImageMask(Object *ref, Stream *str, int width, int height,
                          bool invert, bool inlineImg);
  void doTilingPatternFill(GfxTilingPattern *tPat, bool stroke, bool eoFill,
                           bool text);
  void doShadingPatternFill(GfxShadingPattern *sPat, bool stroke, bool eoFill,
                            bool text);
  void opShFill(Object args[], int numArgs);
  void doFunctionShFill(GfxFunctionShading *shading);
  void doFunctionShFill1(GfxFunctionShading *shading, double x0, double y0,
                         double x1, double y1, GfxColor *colors, int depth);
  void doAxialShFill(GfxAxialShading *shading);
  void doRadialShFill(GfxRadialShading *shading);
  void doGouraudTriangleShFill(GfxGouraudTriangleShading *shading);
  void gouraudFillTriangle(double x0, double y0, GfxColor *color0, double x1,
                           double y1, GfxColor *color1, double x2, double y2,
                           GfxColor *color2, int nComps, int depth,
                           GfxState::ReusablePathIterator *path);
  void gouraudFillTriangle(double x0, double y0, double color0, double x1,
                           double y1, double color1, double x2, double y2,
                           double color2, double refineColorThreshold,
                           int depth, GfxGouraudTriangleShading *shading,
                           GfxState::ReusablePathIterator *path);
  void doPatchMeshShFill(GfxPatchMeshShading *shading);
  void fillPatch(const GfxPatch *patch, int colorComps, int patchColorComps,
                 double refineColorThreshold, int depth,
                 const GfxPatchMeshShading *shading);
  void doEndPath();

  // path clipping operators
  void opClip(Object args[], int numArgs);
  void opEOClip(Object args[], int numArgs);

  // text object operators
  void opBeginText(Object args[], int numArgs);
  void opEndText(Object args[], int numArgs);

  // text state operators
  void opSetCharSpacing(Object args[], int numArgs);
  void opSetFont(Object args[], int numArgs);
  void opSetTextLeading(Object args[], int numArgs);
  void opSetTextRender(Object args[], int numArgs);
  void opSetTextRise(Object args[], int numArgs);
  void opSetWordSpacing(Object args[], int numArgs);
  void opSetHorizScaling(Object args[], int numArgs);

  // text positioning operators
  void opTextMove(Object args[], int numArgs);
  void opTextMoveSet(Object args[], int numArgs);
  void opSetTextMatrix(Object args[], int numArgs);
  void opTextNextLine(Object args[], int numArgs);

  // text string operators
  void opShowText(Object args[], int numArgs);
  void opMoveShowText(Object args[], int numArgs);
  void opMoveSetShowText(Object args[], int numArgs);
  void opShowSpaceText(Object args[], int numArgs);
  void doShowText(const GooString *s);
  void doIncCharCount(const GooString *s);

  // XObject operators
  void opXObject(Object args[], int numArgs);
  void doImage(Object *ref, Stream *str, bool inlineImg);
  void doForm(Object *str);

  // in-line image operators
  void opBeginImage(Object args[], int numArgs);
  Stream *buildImageStream();
  void opImageData(Object args[], int numArgs);
  void opEndImage(Object args[], int numArgs);

  // type 3 font operators
  void opSetCharWidth(Object args[], int numArgs);
  void opSetCacheDevice(Object args[], int numArgs);

  // compatibility operators
  void opBeginIgnoreUndef(Object args[], int numArgs);
  void opEndIgnoreUndef(Object args[], int numArgs);

  // marked content operators
  void opBeginMarkedContent(Object args[], int numArgs);
  void opEndMarkedContent(Object args[], int numArgs);
  void opMarkPoint(Object args[], int numArgs);
  GfxState *saveStateStack();
  bool contentIsHidden();
  void pushMarkedContent();
  void popMarkedContent();
  /**
     Converts a charcode set to a goostring. if the vect is CID based,
     i.e. in a TJ with <0055> for example, then it appends zeros
   */
  GooString *ccVectoToGooString(const std::vector<CharCode> &v2, bool useCID);
  double scaleCharacterDeltas(int wMode, double dx, int n, const char *p,
                              double &dy);
};

#endif
