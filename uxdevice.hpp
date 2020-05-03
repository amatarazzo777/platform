/**
\author Anthony Matarazzo
\file uxdevice.hpp
\date 11/19/19
\version 1.0
\brief interface for the platform.

*/
#pragma once

#define PI (3.14159265358979323846264338327f)

/**
\addtogroup Library Build Options
\brief Library Options
\details These options provide the selection to configure selection
options when compiling the source.
@{
*/

#define DEFAULT_TEXTFACE "arial"
#define DEFAULT_TEXTSIZE 12
#define DEFAULT_TEXTCOLOR 0

/**
\def USE_DEBUG_CONSOLE
*/
#define USE_DEBUG_CONSOLE

/** @} */

#include <algorithm>
#include <any>
#include <array>
#include <cstdint>

#if defined(_WIN64)
typedef unsigned char u_int8_t;
#endif

#include <assert.h>
#include <cctype>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

/*************************************
OS SPECIFIC HEADERS
*************************************/

#if defined(__linux__)
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include <sys/types.h>
#include <xcb/xcb_keysyms.h>

#elif defined(_WIN64)
// Windows Header Files:
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

// auto linking of direct x
#pragma comment(lib, "d2d1.lib")

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#endif

#include <cairo-xcb.h>
#include <cairo.h>

#include <glib.h>
#include <librsvg/rsvg.h>
#include <pango/pangocairo.h>

std::string _errorReport(std::string sourceFile, int ln, std::string sfunc,
                         std::string cond, std::string ecode);
typedef std::function<void(const std::string &err)> errorHandler;

namespace uxdevice {

class event;

/**
\enum eventType
\brief the eventType enumeration contains a sequenced value for all of the
events that can be dispatched by the system.
*/
enum class eventType : uint8_t {
  paint,
  focus,
  blur,
  resize,
  keydown,
  keyup,
  keypress,
  mouseenter,
  mousemove,
  mousedown,
  mouseup,
  click,
  dblclick,
  contextmenu,
  wheel,
  mouseleave
};

/// \typedef eventHandler is used to note and declare a lambda function for
/// the specified event.
typedef std::function<void(const event &et)> eventHandler;

/**
\class event

\brief the event class provides the communication between the event system and
the caller. There is one event class for all of the distinct events. Simply
different constructors are selected based upon the necessity of information
given within the parameters.
*/
using event = class event {
public:
  event(const eventType &et) {
    evtType = et;
    bVirtualKey = false;
  }
  event(const eventType &et, const char &k) {
    evtType = et;
    key = k;
    bVirtualKey = false;
  }
  event(const eventType &et, const unsigned int &vk) {
    evtType = et;
    virtualKey = vk;
    bVirtualKey = true;
  }

  event(const eventType &et, const short &mx, const short &my,
        const short &mb_dis) {
    evtType = et;
    mousex = mx;
    mousey = my;
    if (et == eventType::wheel)
      wheelDistance = mb_dis;
    else
      mouseButton = static_cast<char>(mb_dis);
    bVirtualKey = false;
  }
  event(const eventType &et, const short &w, const short &h) {
    evtType = et;
    width = w;
    height = h;
    mousex = w;
    mousey = h;
    bVirtualKey = false;
  }

  event(const eventType &et, const short &_x, const short &_y, const short &w,
        const short &h) {
    evtType = et;
    x = _x;
    y = _y;
    width = w;
    height = h;
    bVirtualKey = false;
  }
  event(const eventType &et, const short &distance) {
    evtType = et;
    wheelDistance = distance;
    bVirtualKey = false;
  }
  ~event(){};

public:
  eventType evtType;
  bool bVirtualKey;
  char key;
  unsigned int virtualKey;
  std::wstring unicodeKeys;
  short mousex;
  short mousey;
  char mouseButton;
  short width;
  short height;
  short wheelDistance;
  short x, y;
};

/**
 \details
*/
enum class antialias {
  def = CAIRO_ANTIALIAS_DEFAULT,

  /* method */
  none = CAIRO_ANTIALIAS_NONE,
  gray = CAIRO_ANTIALIAS_GRAY,
  subPixel = CAIRO_ANTIALIAS_SUBPIXEL,

  /* hints */
  fast = CAIRO_ANTIALIAS_FAST,
  good = CAIRO_ANTIALIAS_GOOD,
  best = CAIRO_ANTIALIAS_BEST
};

enum class filterType {
  fast = CAIRO_FILTER_FAST,
  good = CAIRO_FILTER_GOOD,
  best = CAIRO_FILTER_BEST,
  nearest = CAIRO_FILTER_NEAREST,
  bilinear = CAIRO_FILTER_BILINEAR,
  gaussian = CAIRO_FILTER_GAUSSIAN
};

enum class extendType {
  none = CAIRO_EXTEND_NONE,
  repeat = CAIRO_EXTEND_REPEAT,
  reflect = CAIRO_EXTEND_REFLECT,
  pad = CAIRO_EXTEND_PAD
};

enum class lineCap {
  butt = CAIRO_LINE_CAP_BUTT,
  round = CAIRO_LINE_CAP_ROUND,
  square = CAIRO_LINE_CAP_SQUARE
};

enum class lineJoin {
  miter = CAIRO_LINE_JOIN_MITER,
  round = CAIRO_LINE_JOIN_ROUND,
  bevel = CAIRO_LINE_JOIN_BEVEL
};
enum class areaType { none, circle, ellipse, rectangle, roundedRectangle };

enum op_t {
  opClear = CAIRO_OPERATOR_CLEAR,
  opSource = CAIRO_OPERATOR_SOURCE,
  opOver = CAIRO_OPERATOR_OVER,
  opIn = CAIRO_OPERATOR_IN,
  opOut = CAIRO_OPERATOR_OUT,
  opAtop = CAIRO_OPERATOR_ATOP,
  opDest = CAIRO_OPERATOR_DEST,
  opDestOver = CAIRO_OPERATOR_DEST_OVER,
  opDestIn = CAIRO_OPERATOR_DEST_IN,
  opDestOut = CAIRO_OPERATOR_DEST_OUT,
  opDestAtop = CAIRO_OPERATOR_DEST_ATOP,
  opXor = CAIRO_OPERATOR_XOR,
  opAdd = CAIRO_OPERATOR_ADD,
  opSaturate = CAIRO_OPERATOR_SATURATE,
  opMultiply = CAIRO_OPERATOR_MULTIPLY,
  opScreen = CAIRO_OPERATOR_SCREEN,
  opOverlay = CAIRO_OPERATOR_OVERLAY,
  opDarken = CAIRO_OPERATOR_DARKEN,
  opLighten = CAIRO_OPERATOR_LIGHTEN,
  opColorDodge = CAIRO_OPERATOR_COLOR_DODGE,
  opColorBurn = CAIRO_OPERATOR_COLOR_BURN,
  opHardLight = CAIRO_OPERATOR_HARD_LIGHT,
  opSoftLight = CAIRO_OPERATOR_SOFT_LIGHT,
  opDifference = CAIRO_OPERATOR_DIFFERENCE,
  opExclusion = CAIRO_OPERATOR_EXCLUSION,
  opHSLHUE = CAIRO_OPERATOR_HSL_HUE,
  opHSLSaturation = CAIRO_OPERATOR_HSL_SATURATION,
  opHSLColor = CAIRO_OPERATOR_HSL_COLOR,
  opHSLLuminosity = CAIRO_OPERATOR_HSL_LUMINOSITY
};
enum class alignment {
  left = PangoAlignment::PANGO_ALIGN_LEFT,
  center = PangoAlignment::PANGO_ALIGN_CENTER,
  right = PangoAlignment::PANGO_ALIGN_RIGHT,
  justified = 4
};
enum class ellipsize {
  none = PANGO_ELLIPSIZE_NONE,
  start = PANGO_ELLIPSIZE_START,
  middle = PANGO_ELLIPSIZE_MIDDLE,
  end = PANGO_ELLIPSIZE_END
};
enum class content {
  color = CAIRO_CONTENT_COLOR,
  alpha = CAIRO_CONTENT_ALPHA,
  all = CAIRO_CONTENT_COLOR_ALPHA
};

using bounds = class bounds {
public:
  double x = 0, y = 0, w = 0, h = 0;
};

using point = class point {
public:
  double x = 0, y = 0;
};

using Matrix = class Matrix {
public:
  Matrix() { cairo_matrix_init_identity(&_matrix); }
  void initIdentity(void) { cairo_matrix_init_identity(&_matrix); };
  void initTranslate(double tx, double ty) {
    cairo_matrix_init_translate(&_matrix, tx, ty);
  }
  void initScale(double sx, double sy) {
    cairo_matrix_init_scale(&_matrix, sx, sy);
  }
  void initRotate(double radians) {
    cairo_matrix_init_rotate(&_matrix, radians);
  }
  void translate(double tx, double ty) {
    cairo_matrix_translate(&_matrix, tx, ty);
  }
  void scale(double sx, double sy) { cairo_matrix_scale(&_matrix, sx, sy); }
  void rotate(double radians) { cairo_matrix_rotate(&_matrix, radians); }
  void invert(void) { cairo_matrix_invert(&_matrix); }
  void multiply(const Matrix &operand, Matrix &result) {
    cairo_matrix_multiply(&result._matrix, &_matrix, &operand._matrix);
  }
  void transformDistance(double &dx, double &dy) {
    double _dx = dx;
    double _dy = dy;
    cairo_matrix_transform_distance(&_matrix, &_dx, &_dy);
    dx = _dx;
    dy = _dy;
  }
  void transformPoint(double &x, double &y) {
    double _x = x;
    double _y = y;
    cairo_matrix_transform_point(&_matrix, &_x, &_y);
    x = _y;
    y = _y;
  }
  cairo_matrix_t _matrix;
};

/**
\class Paint

\brief interface for the paint class.

*/
enum class paintType { none, color, pattern, image };
enum class gradientType { none, linear, radial };

using ColorStop = class ColorStop {
public:
  ColorStop(u_int32_t c);
  ColorStop(double r, double g, double b);
  ColorStop(const std::string &s);
  ColorStop(const std::string &s, double a);

  ColorStop(double o, u_int32_t c);
  ColorStop(double o, double r, double g, double b);
  ColorStop(double o, double r, double g, double b, double a);
  ColorStop(double o, const std::string &s);
  ColorStop(double o, const std::string &s, double a);
  void parseColor(const std::string &s);

  bool _bAutoOffset = false;
  bool _bRGBA = false;
  double _offset = 0;
  double _r = 0;
  double _g = 0;
  double _b = 0;
  double _a = 1;
};
typedef std::vector<ColorStop> ColorStops;
typedef std::vector<ColorStop>::iterator ColorStopsIterator;

using Paint = class Paint : public Matrix {
public:
  Paint(u_int32_t c);
  Paint(double r, double g, double b);
  Paint(double r, double g, double b, double a);
  Paint(const std::string &n);
  Paint(const std::string &n, double width, double height);

  Paint(double x0, double y0, double x1, double y1, const ColorStops &_cs);
  Paint(double cx0, double cy0, double radius0, double cx1, double cy1,
        double radius1, const ColorStops &cs);

  Paint(const Paint &other) {
    _type = other._type;
    _description = other._description;

    _gradientType = other._gradientType;
    _width = other._width;
    _height = other._height;
    _stops = other._stops;

    if (other._pattern)
      _pattern = cairo_pattern_reference(other._pattern);

    if (other._image)
      _image = cairo_surface_reference(other._image);

    _r = other._r;
    _g = other._g;
    _b = other._b;
    _a = other._a;

    _matrix = other._matrix;
    _filter = other._filter;
    _extend = other._extend;

    _pangoColor = other._pangoColor;
    _bLoaded = other._bLoaded;
  }

  ~Paint();
  void emit(cairo_t *cr);
  void emit(cairo_t *cr, double x, double y, double w, double h);
  void filter(filterType ft) {
    if (_pattern)
      cairo_pattern_set_filter(_pattern, static_cast<cairo_filter_t>(ft));
  }
  void extend(extendType et) {
    if (_pattern)
      cairo_pattern_set_extend(_pattern, static_cast<cairo_extend_t>(et));
  }

private:
  bool create(void);
  bool isLoaded(void) const { return _bLoaded; }
  bool isLinearGradient(const std::string &s);
  bool isRadialGradient(const std::string &s);
  bool patch(const std::string &s);

private:
  double _r = 0.0, _g = 0.0, _b = 0.0, _a = 1.0;
  paintType _type = paintType::none;
  std::string _description;

  gradientType _gradientType = gradientType::none;

  // linear gradient pattern space coordinates
  double _x0 = 0, _y0 = 0, _x1 = 0, _y1 = 0;

  // radial gradient pattern space coordinates
  double _cx0 = 0, _cy0 = 0, _radius0 = 0, _cx1 = 0, _cy1 = 0, _radius1 = 0;
  ColorStops _stops;

  filterType _filter = filterType::fast;
  extendType _extend = extendType::repeat;
  double _width = -1;
  double _height = -1;

  cairo_pattern_t *_pattern = nullptr;
  cairo_surface_t *_image = nullptr;
  PangoColor _pangoColor;
  bool _bLoaded = false;
};

/**
\internal
\class platform
\brief The platform contains logic to connect the document object model to the
local operating system.
*/

class platform {
public:
  platform(const eventHandler &evtDispatcher, const errorHandler &fn);
  ~platform();
  void openWindow(const std::string &sWindowTitle, const unsigned short width,
                  const unsigned short height);
  void closeWindow(void);

  void render(const event &evt);
  void processEvents(void);
  void dispatchEvent(const event &e);

  void clear(void);
  void text(const std::string &s);
  void text(const std::stringstream &s);
  void image(const std::string &s);

  void pen(const Paint &p);
  void pen(u_int32_t c);
  void pen(const std::string &c);
  void pen(const std::string &c, double w, double h);
  void pen(double _r, double _g, double _b);
  void pen(double _r, double _g, double _b, double _a);
  void pen(double x0, double y0, double x1, double y1, const ColorStops &cs);
  void pen(double cx0, double cy0, double radius0, double cx1, double cy1,
           double radius1, const ColorStops &cs);

  void background(const Paint &p);
  void background(u_int32_t c);
  void background(const std::string &c);
  void background(const std::string &c, double w, double h);
  void background(double _r, double _g, double _b);
  void background(double _r, double _g, double _b, double _a);
  void background(double x0, double y0, double x1, double y1,
                  const ColorStops &cs);
  void background(double cx0, double cy0, double radius0, double cx1,
                  double cy1, double radius1, const ColorStops &cs);

  void textOutline(const Paint &p, double dWidth = 1);
  void textOutline(u_int32_t c, double dWidth = 1);
  void textOutline(const std::string &c, double dWidth = 1);
  void textOutline(const std::string &c, double w, double h, double dWidth = 1);
  void textOutline(double _r, double _g, double _b, double dWidth = 1);
  void textOutline(double _r, double _g, double _b, double _a,
                   double dWidth = 1);
  void textOutline(double x0, double y0, double x1, double y1,
                   const ColorStops &cs, double dWidth = 1);
  void textOutline(double cx0, double cy0, double radius0, double cx1,
                   double cy1, double radius1, const ColorStops &cs,
                   double dWidth = 1);

  void textOutlineNone(void);

  void textFill(const Paint &p);
  void textFill(u_int32_t c);
  void textFill(const std::string &c);
  void textFill(const std::string &c, double w, double h);
  void textFill(double _r, double _g, double _b);
  void textFill(double _r, double _g, double _b, double _a);
  void textFill(double x0, double y0, double x1, double y1,
                const ColorStops &cs);
  void textFill(double cx0, double cy0, double radius0, double cx1, double cy1,
                double radius1, const ColorStops &cs);

  void textFillNone(void);

  void textShadow(const Paint &p, int r = 3, double xOffset = 1.0,
                  double yOffset = 1.0);
  void textShadow(u_int32_t c, int r = 3, double xOffset = 1.0,
                  double yOffset = 1.0);
  void textShadow(const std::string &c, int r = 3, double xOffset = 1.0,
                  double yOffset = 1.0);
  void textShadow(const std::string &c, double w, double h, int r,
                  double xOffset, double yOffset);

  void textShadow(double _r, double _g, double _b, int r = 3,
                  double xOffset = 1.0, double yOffset = 1.0);
  void textShadow(double _r, double _g, double _b, double _a, int r = 3,
                  double xOffset = 1.0, double yOffset = 1.0);

  void textShadow(double x0, double y0, double x1, double y1,
                  const ColorStops &cs, int r = 3, double xOffset = 1.0,
                  double yOffset = 1.0);
  void textShadow(double cx0, double cy0, double radius0, double cx1,
                  double cy1, double radius1, const ColorStops &cs, int r = 3,
                  double xOffset = 1.0, double yOffset = 1.0);

  void textShadowNone(void);

  void textAlignment(alignment aln);
  void indent(double space);
  double indent(void);
  void ellipse(ellipsize e);
  ellipsize ellipse(void);
  void lineSpace(double dSpace);
  double lineSpace(void);
  void tabStops(const std::vector<double> &tabs);

  void font(const std::string &s);
  void area(double x, double y, double w, double h);
  void area(double x, double y, double w, double h, double rx, double ry);
  void area(double cx, double cy, double r) { areaCircle(cx, cy, r); }
  void areaCircle(double x, double y, double d);
  void areaEllipse(double cx, double cy, double rx, double ry);
  void areaLines(std::vector<double> lines);
  //void areaPath(std::vector<PathStep> path);


  void drawText(void);
  void drawImage(void);
  void drawArea(void);
  void antiAlias(antialias antialias);

  void save(void);
  void restore(void);

  void push(content _content = content::all);
  void pop(bool bToSource = false);

  void translate(double x, double y);
  void rotate(double angle);
  void scale(double x, double y);
  void transform(const Matrix &mat);
  void matrix(const Matrix &mat);
  void identity(void);
  void device(double &x, double &y);
  void deviceDistance(double &x, double &y);
  void user(double &x, double &y);
  void userDistance(double &x, double &y);

  void source(Paint &p);

  void cap(lineCap c);
  void join(lineJoin j);
  void lineWidth(double dWidth);
  void miterLimit(double dLimit);
  void dashes(const std::vector<double> &dashes, double offset);
  void tollerance(double _t);
  void op(op_t _op);

  void arc(double xc, double yc, double radius, double angle1, double angle2,
           bool bNegative = false);
  void curve(double x1, double y1, double x2, double y2, double x3, double y3,
             bool bRelative = false);

  void line(double x, double y, bool bRelative = false);
  void move(double x, double y, bool bRelative = false);
  void rectangle(double x, double y, double width, double height);
  point location(void);

  void mask(Paint &p);
  void mask(Paint &p, double x, double y);
  void paint(double alpha = 1.0);
  void stroke(bool bPreserve = false);
  bounds stroke(void);
  bool inStroke(double x, double y);
  void fill(bool bPreserve = false);
  bool inFill(double x, double y);

  bounds clip(void);
  void clip(bool bPreserve = false);
  bool inClip(double x, double y);

  static void blurImage(cairo_surface_t *img, int radius);
  static cairo_status_t read_contents(const gchar *file_name, guint8 **contents,
                                      gsize *length);

  static cairo_surface_t *
  imageSurfaceSVG(const char *filename, double width = -1, double height = -1);

private:
  void drawCaret(const int x, const int y, const int h);

  void messageLoop(void);
  void test(int x, int y);

  void flip(void);
  void resize(const int w, const int h);

#if defined(__linux__)

#elif defined(_WIN64)
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
  bool initializeVideo(void);
  void terminateVideo(void);
#endif

private:
  /**
   \details
  */

  class DisplayUnitContext;
#define IDX(NAME) platform::contextUnitIndex::NAME##_idx

  enum contextUnitIndex : std::size_t {
    AREA_idx,
    STRING_idx,
    IMAGE_idx,
    FONT_idx,
    ANTIALIAS_idx,
    TEXTOUTLINE_idx,
    TEXTFILL_idx,
    TEXTSHADOW_idx,
    PEN_idx,
    BACKGROUND_idx,
    ALIGN_idx,
    EVENT_idx,
    DRAWTEXT_idx,
    DRAWIMAGE_idx,
    DRAWAREA_idx,
    CLEAROPTION_idx,
    FUNCTION_idx,

    MAX_idx

  };
#define VIRTUAL_INDEX(NAME)                                                    \
  inline std::size_t index(void) { return contextUnitIndex::NAME##_idx; }

  using DisplayUnit = class DisplayUnit {
  public:
    virtual std::size_t index() = 0;
    virtual ~DisplayUnit() {}
    virtual void invoke(const DisplayUnitContext &context) = 0;
  };

  using CLEAROPTION = class CLEAROPTION : public DisplayUnit {
  public:
    VIRTUAL_INDEX(CLEAROPTION);
    CLEAROPTION(contextUnitIndex opt) : option(opt) {}
    void invoke(const DisplayUnitContext &context) {}
    contextUnitIndex option = MAX_idx;
  };

  using ANTIALIAS = class ANTIALIAS : public DisplayUnit {
  public:
    VIRTUAL_INDEX(ANTIALIAS);
    ANTIALIAS(antialias _antialias)
        : setting(static_cast<cairo_antialias_t>(_antialias)) {}
    void invoke(const DisplayUnitContext &context) {}
    void emit(cairo_t *cr) { cairo_set_antialias(cr, setting); }
    cairo_antialias_t setting;
  };

  using AREA = class AREA : public DisplayUnit {
  public:
    VIRTUAL_INDEX(AREA);

    AREA(void) {}
    AREA(double _cx, double _cy, double _r)
        : x(_cx), y(_cy), w(_r), h(_r), rx(_r), type(areaType::circle) {}
    AREA(areaType _type, double _x, double _y, double p3, double p4)
        : x(_x), y(_y), type(_type) {
      if (type == areaType::rectangle) {
        w = p3;
        h = p4;
      } else {
        rx = p3;
        ry = p4;
      }
    }
    AREA(double _x, double _y, double _w, double _h, double _rx, double _ry)
        : x(_x), y(_y), w(_w), h(_h), rx(_rx), ry(_ry),
          type(areaType::roundedRectangle) {}
    ~AREA() {}
    AREA(const AREA &other) { *this = other; };
    void shrink(double dWidth);
    double x = 0.0, y = 0.0, w = 0.0, h = 0.0, rx = -1, ry = -1;
    areaType type = areaType::none;

    void invoke(const DisplayUnitContext &context);
  };

  using STRING = class STRING : public DisplayUnit {
  public:
    VIRTUAL_INDEX(STRING);
    STRING(const std::string &s) : data(s) {}
    ~STRING() {}
    std::string data;
    void invoke(const DisplayUnitContext &context) {}
  };

  using FONT = class FONT : public DisplayUnit {
  public:
    VIRTUAL_INDEX(FONT);

    FONT(const std::string &s)
        : description(s), pointSize(DEFAULT_TEXTSIZE), bProvidedName(false),
          bProvidedSize(false), bProvidedDescription(true) {}
    FONT(const std::string &s, const double &pt)
        : description(s), pointSize(pt) {}
    ~FONT() {
      if (fontDescription)
        pango_font_description_free(fontDescription);
    }
    std::string description = DEFAULT_TEXTFACE;
    double pointSize = DEFAULT_TEXTSIZE;
    bool bProvidedName = false;
    bool bProvidedSize = false;
    bool bProvidedDescription = false;
    PangoFontDescription *fontDescription = nullptr;

    void invoke(const DisplayUnitContext &context);
  };

  using PEN = class PEN : public DisplayUnit, public Paint {
  public:
    VIRTUAL_INDEX(PEN);
    PEN(const Paint &c) : Paint(c) {}
    PEN(u_int32_t c) : Paint(c) {}
    PEN(double _r, double _g, double _b) : Paint(_r, _g, _b) {}
    PEN(double _r, double _g, double _b, double _a) : Paint(_r, _g, _b, _a) {}
    PEN(const std::string &n) : Paint(n) {}
    PEN(const std::string &n, double w, double h) : Paint(n, w, h) {}
    PEN(double x0, double y0, double x1, double y1, const ColorStops &cs)
        : Paint(x0, y0, x1, y1, cs) {}
    PEN(double cx0, double cy0, double radius0, double cx1, double cy1,
        double radius1, const ColorStops &cs)
        : Paint(cx0, cy0, radius0, cx1, cy1, radius1, cs) {}
    ~PEN() {}
    void invoke(const DisplayUnitContext &context) {}
  };

  using BACKGROUND = class BACKGROUND : public DisplayUnit, public Paint {
  public:
  public:
    VIRTUAL_INDEX(BACKGROUND);

    BACKGROUND(const Paint &c) : Paint(c) {}
    BACKGROUND(u_int32_t c) : Paint(c) {}
    BACKGROUND(double _r, double _g, double _b) : Paint(_r, _g, _b) {}
    BACKGROUND(double _r, double _g, double _b, double _a)
        : Paint(_r, _g, _b, _a) {}
    BACKGROUND(const std::string &n) : Paint(n) {}
    BACKGROUND(const std::string &n, double w, double h) : Paint(n, w, h) {}
    BACKGROUND(double x0, double y0, double x1, double y1, const ColorStops &cs)
        : Paint(x0, y0, x1, y1, cs) {}
    BACKGROUND(double cx0, double cy0, double radius0, double cx1, double cy1,
               double radius1, const ColorStops &cs)
        : Paint(cx0, cy0, radius0, cx1, cy1, radius1, cs) {}
    ~BACKGROUND() {}
    void invoke(const DisplayUnitContext &context) {}
  };

  using ALIGN = class ALIGN : public DisplayUnit {
  public:
    VIRTUAL_INDEX(ALIGN);

    ALIGN(alignment _aln) : setting(_aln) {}
    ~ALIGN() {}
    void emit(PangoLayout *layout);
    alignment setting = alignment::left;
    void invoke(const DisplayUnitContext &context) {}
  };

  using EVENT = class EVENT : public DisplayUnit {
  public:
    VIRTUAL_INDEX(EVENT);

    EVENT(eventHandler _eh) : fn(_eh){};
    ~EVENT() {}
    eventHandler fn;
    void invoke(const DisplayUnitContext &context) {}
  };

  using TEXTSHADOW = class TEXTSHADOW : public DisplayUnit, public Paint {
  public:
    VIRTUAL_INDEX(TEXTSHADOW);

    TEXTSHADOW(const Paint &c, int r, double xOffset, double yOffset)
        : Paint(c), radius(r), x(xOffset), y(yOffset) {}
    TEXTSHADOW(u_int32_t c, int r, double xOffset, double yOffset)
        : Paint(c), radius(r), x(xOffset), y(yOffset) {}
    TEXTSHADOW(double _r, double _g, double _b, int r, double xOffset,
               double yOffset)
        : Paint(_r, _g, _b), radius(r), x(xOffset), y(yOffset) {}
    TEXTSHADOW(double _r, double _g, double _b, double _a, int r,
               double xOffset, double yOffset)
        : Paint(_r, _g, _b, _a), radius(r), x(xOffset), y(yOffset) {}
    TEXTSHADOW(const std::string &n, int r, double xOffset, double yOffset)
        : Paint(n), radius(r), x(xOffset), y(yOffset) {}
    TEXTSHADOW(const std::string &n, double w, double h, int r, double xOffset,
               double yOffset)
        : Paint(n, w, h), radius(r), x(xOffset), y(yOffset) {}
    TEXTSHADOW(double x0, double y0, double x1, double y1, const ColorStops &cs,
               int r, double xOffset, double yOffset)
        : Paint(x0, y0, x1, y1, cs), radius(r), x(xOffset), y(yOffset) {}
    TEXTSHADOW(double cx0, double cy0, double radius0, double cx1, double cy1,
               double radius1, const ColorStops &cs, int r, double xOffset,
               double yOffset)
        : Paint(cx0, cy0, radius0, cx1, cy1, radius1, cs), radius(r),
          x(xOffset), y(yOffset) {}

    ~TEXTSHADOW() {}
    void invoke(const DisplayUnitContext &context) {}

  public:
    unsigned short radius = 3;
    double x = 1, y = 1;
  };

  using TEXTOUTLINE = class TEXTOUTLINE : public Paint, public DisplayUnit {
  public:
    VIRTUAL_INDEX(TEXTOUTLINE);
    TEXTOUTLINE(const Paint &c, double _lineWidth)
        : Paint(c), lineWidth(_lineWidth) {}
    TEXTOUTLINE(u_int32_t c, double _lineWidth)
        : Paint(c), lineWidth(_lineWidth) {}
    TEXTOUTLINE(double _r, double _g, double _b, double _lineWidth)
        : Paint(_r, _g, _b), lineWidth(_lineWidth) {}
    TEXTOUTLINE(double _r, double _g, double _b, double _a, double _lineWidth)
        : Paint(_r, _g, _b, _a), lineWidth(_lineWidth) {}
    TEXTOUTLINE(const std::string &n, double _lineWidth)
        : Paint(n), lineWidth(_lineWidth) {}
    TEXTOUTLINE(const std::string &n, double w, double h, double _lineWidth)
        : Paint(n, w, h), lineWidth(_lineWidth) {}
    TEXTOUTLINE(double x0, double y0, double x1, double y1,
                const ColorStops &cs, double _lineWidth)
        : Paint(x0, y0, x1, y1, cs), lineWidth(_lineWidth) {}
    TEXTOUTLINE(double cx0, double cy0, double radius0, double cx1, double cy1,
                double radius1, const ColorStops &cs, double _lineWidth)
        : Paint(cx0, cy0, radius0, cx1, cy1, radius1, cs),
          lineWidth(_lineWidth) {}

    void invoke(const DisplayUnitContext &context) {}

  public:
    double lineWidth = .5;
  };

  using TEXTFILL = class TEXTFILL : public Paint, public DisplayUnit {
  public:
    VIRTUAL_INDEX(TEXTFILL);
    TEXTFILL(const Paint &c) : Paint(c) {}
    TEXTFILL(u_int32_t c) : Paint(c) {}
    TEXTFILL(double _r, double _g, double _b) : Paint(_r, _g, _b) {}
    TEXTFILL(double _r, double _g, double _b, double _a)
        : Paint(_r, _g, _b, _a) {}
    TEXTFILL(const std::string &n) : Paint(n) {}
    TEXTFILL(const std::string &n, double w, double h) : Paint(n, w, h) {}
    TEXTFILL(double x0, double y0, double x1, double y1, const ColorStops &cs)
        : Paint(x0, y0, x1, y1, cs) {}
    TEXTFILL(double cx0, double cy0, double radius0, double cx1, double cy1,
             double radius1, const ColorStops &cs)
        : Paint(cx0, cy0, radius0, cx1, cy1, radius1, cs) {}

    ~TEXTFILL() {}
    void invoke(const DisplayUnitContext &context) {}
  };

  using DRAWTEXT = class DRAWTEXT : public DisplayUnit {
  public:
    VIRTUAL_INDEX(DRAWTEXT);

    DRAWTEXT(void) : beginIndex(0), endIndex(0), bEntire(true) {}
    DRAWTEXT(std::size_t _b, std::size_t _e)
        : beginIndex(_b), endIndex(_e), bEntire(false) {}
    ~DRAWTEXT() {
      if (textImage)
        cairo_surface_destroy(textImage);
      if (shadowImage)
        cairo_surface_destroy(shadowImage);
      if (layout)
        g_object_unref(layout);
    }
    std::size_t beginIndex = 0;
    std::size_t endIndex = 0;
    bool bEntire = true;
    cairo_surface_t *textImage = nullptr;
    cairo_surface_t *shadowImage = nullptr;
    PangoLayout *layout = nullptr;

    void invoke(const DisplayUnitContext &context);
  };

  using DRAWIMAGE = class DRAWIMAGE : public DisplayUnit {
  public:
    VIRTUAL_INDEX(DRAWIMAGE);
    DRAWIMAGE(const AREA &a) : src(a) { bEntire = false; }
    DRAWIMAGE(void) {}
    ~DRAWIMAGE() {}
    void invoke(const DisplayUnitContext &context);

  private:
    AREA src;
    bool bEntire = true;
  };

  using DRAWAREA = class DRAWAREA : public DisplayUnit {
  public:
    VIRTUAL_INDEX(DRAWAREA);
    DRAWAREA() {}
    ~DRAWAREA() {}
    void invoke(const DisplayUnitContext &context);

  private:
  };

  typedef std::function<void(cairo_t *cr)> CAIRO_FUNCTION;
  using FUNCTION = class FUNCTION : public DisplayUnit {
  public:
    VIRTUAL_INDEX(FUNCTION);
    FUNCTION(CAIRO_FUNCTION _func) : func(_func) {}
    ~FUNCTION() {}
    void invoke(const DisplayUnitContext &context);

  private:
    CAIRO_FUNCTION func;
  };

  using IMAGE = class IMAGE : public DisplayUnit {
  public:
    enum ImageSystemType { none, cairo_type, magick_type };

    VIRTUAL_INDEX(IMAGE);
    IMAGE(const std::string &_fileName) : fileName(_fileName) {}
    ~IMAGE() {
      if (image)
        cairo_surface_destroy(image);
    }
    cairo_surface_t *image = nullptr;
    void invoke(const DisplayUnitContext &context);
    std::string fileName;
    bool bLoaded = false;
  };

  class DisplayUnitContext {
  public:
    std::array<DisplayUnit *, contextUnitIndex::MAX_idx> currentUnit;

    DisplayUnitContext(void) {
      std::fill(currentUnit.begin(), currentUnit.end(), nullptr);
    }

#define INDEXED_ACCESSOR(NAME)                                                 \
  inline platform::NAME *NAME(void) const {                                    \
    return dynamic_cast<platform::NAME *>(                                     \
        currentUnit[contextUnitIndex::NAME##_idx]);                            \
  }                                                                            \
  inline bool validate##NAME(void) const {                                     \
    return dynamic_cast<platform::NAME *>(                                     \
               currentUnit[contextUnitIndex::NAME##_idx]) != nullptr;          \
  }

    INDEXED_ACCESSOR(AREA);
    INDEXED_ACCESSOR(STRING);
    INDEXED_ACCESSOR(IMAGE);
    INDEXED_ACCESSOR(FONT);
    INDEXED_ACCESSOR(ANTIALIAS);
    INDEXED_ACCESSOR(TEXTSHADOW);
    INDEXED_ACCESSOR(TEXTFILL);
    INDEXED_ACCESSOR(TEXTOUTLINE);

    INDEXED_ACCESSOR(PEN);
    INDEXED_ACCESSOR(BACKGROUND);
    INDEXED_ACCESSOR(ALIGN);
    INDEXED_ACCESSOR(EVENT);
    INDEXED_ACCESSOR(DRAWTEXT);
    INDEXED_ACCESSOR(DRAWIMAGE);
    INDEXED_ACCESSOR(DRAWAREA);

    INDEXED_ACCESSOR(FUNCTION);

    /**
      \def The set routine stores the passed object within the slot context.
      This reduces the number of parameters and offers a state.
    */
    void set(DisplayUnit *_ptr) { currentUnit[_ptr->index()] = _ptr; }
    void clear(contextUnitIndex idx) { currentUnit[idx] = nullptr; }

    short windowX = 0;
    short windowY = 0;
    unsigned short windowWidth = 0;
    unsigned short windowHeight = 0;
    bool windowOpen = false;
    cairo_t *cr = nullptr;

#if defined(__linux__)
    Display *xdisplay = nullptr;
    xcb_connection_t *connection = nullptr;
    xcb_screen_t *screen = nullptr;
    xcb_drawable_t window = 0;
    xcb_gcontext_t graphics = 0;

    xcb_visualtype_t *visualType = nullptr;

    // xcb -- keyboard
    xcb_key_symbols_t *syms = nullptr;

    cairo_surface_t *xcbSurface = nullptr;
    bool preclear = true;

#elif defined(_WIN64)
    HWND hwnd = 0;

    ID2D1Factory *pD2DFactory = nullptr;
    ID2D1HwndRenderTarget *pRenderTarget = nullptr;
    ID2D1Bitmap *pBitmap = nullptr;

#endif
  };

  DisplayUnitContext context;

private:
  errorHandler fnError;
  eventHandler fnEvents;
  std::vector<std::unique_ptr<DisplayUnit>> DL;

  std::vector<eventHandler> onfocus;
  std::vector<eventHandler> onblur;
  std::vector<eventHandler> onresize;
  std::vector<eventHandler> onkeydown;
  std::vector<eventHandler> onkeyup;
  std::vector<eventHandler> onkeypress;
  std::vector<eventHandler> onmouseenter;
  std::vector<eventHandler> onmouseleave;
  std::vector<eventHandler> onmousemove;
  std::vector<eventHandler> onmousedown;
  std::vector<eventHandler> onmouseup;
  std::vector<eventHandler> onclick;
  std::vector<eventHandler> ondblclick;
  std::vector<eventHandler> oncontextmenu;
  std::vector<eventHandler> onwheel;

  std::vector<eventHandler> &getEventVector(eventType evtType);
}; // namespace uxdevice
}; // namespace uxdevice
