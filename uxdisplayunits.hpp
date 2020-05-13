/**
\author Anthony Matarazzo
\file uxworkstate.hpp
\date 5/12/20
\version 1.0
 \details The class holds the context of parameters for
 any drawing operations that occur. Such as AREA, or STRING.
 These objects have named slot positions within the by
 contextUnit.


*/
#pragma once

namespace uxdevice {

/**
\brief base class for all display units. defaulted
is the isOutput function. Drawing object should override
this and return true. This enables the checking of the surface
for errors after invocation.

*/
class DisplayUnit {
public:
  virtual ~DisplayUnit() {}
  virtual void invoke(DisplayContext &context){};
  virtual bool isOutput(void) { return false; }
};

/**
\brief base class for objects that produce image drawing commands
The isOutput is overriden to return true. As well the object uses
the render work list to determine if a particular image is on screen.

*/

class DrawingOutput {
public:
  DrawingOutput(){};
  virtual ~DrawingOutput(){};
  bool hasInkExtents = false;
  cairo_rectangle_int_t cairoInkRectangle = cairo_rectangle_int_t();
  cairo_region_overlap_t overlap = CAIRO_REGION_OVERLAP_OUT;
  DisplayContext::RenderList renderRectangles = DisplayContext::RenderList();
  cairo_rectangle_int_t *inkExtents(void) { return &cairoInkRectangle; }
  bool bCompleted = false;
  bool visible(DisplayContext &context);
};

class CLEARUNIT : public DisplayUnit {
public:
  CLEARUNIT(void **_ptr) : p(_ptr) {}

  CLEARUNIT &operator=(const CLEARUNIT &other) {
    p = other.p;
    return *this;
  }
  CLEARUNIT(const CLEARUNIT &other) { *this = other; }
  void invoke(DisplayContext &context) { *p = nullptr; }
  void **p = nullptr;
};

class ANTIALIAS : public DisplayUnit {
public:
  ANTIALIAS(antialias _antialias)
      : setting(static_cast<cairo_antialias_t>(_antialias)) {}
  void invoke(DisplayContext &context) {
    cairo_set_antialias(context.cr, setting);
  }

  cairo_antialias_t setting;
};

class AREA : public DisplayUnit {
public:
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
  cairo_rectangle_int_t cairoInkRectangle = cairo_rectangle_int_t();
  cairo_rectangle_int_t *inkExtents(void) { return &cairoInkRectangle; };
  void invoke(DisplayContext &context) {
    cairoInkRectangle = {(int)x, (int)y, (int)w, (int)h};
    context.setUnit(this);
  }
};

class STRING : public DisplayUnit {
public:
  STRING(const std::string &s) : data(s) {}
  ~STRING() {}
  std::string data;
  void invoke(DisplayContext &context) { context.setUnit(this); }
};

class FONT : public DisplayUnit {
public:
  FONT(const std::string &s)
      : description(s), pointSize(DEFAULT_TEXTSIZE), bProvidedName(false),
        bProvidedSize(false), bProvidedDescription(true) {}
  FONT(const std::string &s, const double &pt)
      : description(s), pointSize(pt) {}

  FONT &operator=(const FONT &other) = delete;
  FONT(const FONT &);

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
  void invoke(DisplayContext &context) {
    if (!fontDescription)
      fontDescription = pango_font_description_from_string(description.data());
    context.setUnit(this);
  }
};

class PEN : public DisplayUnit, public Paint {
public:
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
  void invoke(DisplayContext &context) { context.setUnit(this); }
};

class BACKGROUND : public DisplayUnit, public Paint {
public:
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
  void invoke(DisplayContext &context) { context.setUnit(this); }
};

class ALIGN : public DisplayUnit {
public:
  ALIGN(alignment _aln) : setting(_aln) {}
  ~ALIGN() {}
  void emit(PangoLayout *layout);
  alignment setting = alignment::left;
  void invoke(DisplayContext &context) { context.setUnit(this); }
};

class EVENT : public DisplayUnit {
public:
  EVENT(eventHandler _eh) : fn(_eh){};
  ~EVENT() {}
  eventHandler fn;
  void invoke(DisplayContext &context) { context.setUnit(this); }
};

class TEXTSHADOW : public DisplayUnit, public Paint {
public:
  TEXTSHADOW(const Paint &c, int r, double xOffset, double yOffset)
      : Paint(c), radius(r), x(xOffset), y(yOffset) {}
  TEXTSHADOW(u_int32_t c, int r, double xOffset, double yOffset)
      : Paint(c), radius(r), x(xOffset), y(yOffset) {}
  TEXTSHADOW(double _r, double _g, double _b, int r, double xOffset,
             double yOffset)
      : Paint(_r, _g, _b), radius(r), x(xOffset), y(yOffset) {}
  TEXTSHADOW(double _r, double _g, double _b, double _a, int r, double xOffset,
             double yOffset)
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
      : Paint(cx0, cy0, radius0, cx1, cy1, radius1, cs), radius(r), x(xOffset),
        y(yOffset) {}

  ~TEXTSHADOW() {}
  void invoke(DisplayContext &context) { context.setUnit(this); }

public:
  unsigned short radius = 3;
  double x = 1, y = 1;
};

class TEXTOUTLINE : public Paint, public DisplayUnit {
public:
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
  TEXTOUTLINE(double x0, double y0, double x1, double y1, const ColorStops &cs,
              double _lineWidth)
      : Paint(x0, y0, x1, y1, cs), lineWidth(_lineWidth) {}
  TEXTOUTLINE(double cx0, double cy0, double radius0, double cx1, double cy1,
              double radius1, const ColorStops &cs, double _lineWidth)
      : Paint(cx0, cy0, radius0, cx1, cy1, radius1, cs), lineWidth(_lineWidth) {
  }
  void emit(cairo_t *cr) {
    Paint::emit(cr);
    cairo_set_line_width(cr, lineWidth);
  }
  void emit(cairo_t *cr, double x, double y, double w, double h) {
    Paint::emit(cr);
    cairo_set_line_width(cr, lineWidth);
  }

  void invoke(DisplayContext &context) { context.setUnit(this); }

public:
  double lineWidth = .5;
};

class TEXTFILL : public Paint, public DisplayUnit {
public:
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
  void invoke(DisplayContext &context) { context.setUnit(this); }
};

class IMAGE : public DisplayUnit {
public:
  IMAGE(const std::string &data) : _data(data) {}
  IMAGE(const IMAGE &other) { *this = other; }
  IMAGE &operator=(const IMAGE &other) {
    _image = cairo_surface_reference(other._image);
    return *this;
  }
  ~IMAGE() {
    if (_image)
      cairo_surface_destroy(_image);
  }

  void invoke(DisplayContext &context);
  cairo_surface_t *_image = nullptr;
  std::string _data = "";
  bool bIsSVG = false;
  bool bLoaded = false;
};

class DRAWTEXT : public DisplayUnit, public DrawingOutput {
public:
  DRAWTEXT(void) : beginIndex(0), endIndex(0), bEntire(true) {}
  DRAWTEXT(std::size_t _b, std::size_t _e)
      : beginIndex(_b), endIndex(_e), bEntire(false) {}
  DRAWTEXT(const DRAWTEXT &other) = delete;
  DRAWTEXT &operator=(const DRAWTEXT &other) = delete;
  bool isOutput(void) { return true; }
  ~DRAWTEXT() {
    if (textImage)
      cairo_surface_destroy(textImage);

    if (layout)
      g_object_unref(layout);
  }
  bool setLayoutOptions(DisplayContext &context);
  void drawShadowOffscreen(DisplayContext &context);
  void drawTextOffscreen(DisplayContext &context);

  bool bTextImageCompleted = false;

  std::size_t beginIndex = 0;
  std::size_t endIndex = 0;
  bool bEntire = true;
  cairo_surface_t *textImage = nullptr;
  PangoLayout *layout = nullptr;
  PangoRectangle ink_rect = PangoRectangle();
  PangoRectangle logical_rect = PangoRectangle();

  void invoke(DisplayContext &context);
};

class DRAWIMAGE : public DisplayUnit, public DrawingOutput {
public:
  DRAWIMAGE(const AREA &a) : src(a) { bEntire = false; }
  DRAWIMAGE(void) {}
  ~DRAWIMAGE() {}
  void invoke(DisplayContext &context);
  bool isOutput(void) { return true; }

private:
  AREA src = AREA();
  bool bEntire = true;
};

class DRAWAREA : public DisplayUnit, public DrawingOutput {
public:
  DRAWAREA() {}
  ~DRAWAREA() {}
  void invoke(DisplayContext &context);

private:
};

/**
\internal
\brief call previously bound function with the cairo context.
*/
typedef std::function<void(cairo_t *cr)> CAIRO_FUNCTION;
class FUNCTION : public DisplayUnit {
public:
  FUNCTION(CAIRO_FUNCTION _func) : func(_func) {}
  ~FUNCTION() {}
  void invoke(DisplayContext &context) { func(context.cr); }

private:
  CAIRO_FUNCTION func;
};

} // namespace uxdevice
