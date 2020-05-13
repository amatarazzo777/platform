/**
\author Anthony Matarazzo
\file uxevent.hpp
\date 5/12/20
\version 1.0
 \details  paint class

*/
#pragma once

namespace uxdevice {
/**
\class Paint

\brief interface for the paint class.

*/
enum class paintType { none, color, pattern, image };
enum class gradientType { none, linear, radial };

class ColorStop {
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

class Paint : public Matrix {
public:
  Paint(u_int32_t c);
  Paint(double r, double g, double b);
  Paint(double r, double g, double b, double a);
  Paint(const std::string &n);
  Paint(const std::string &n, double width, double height);

  Paint(double x0, double y0, double x1, double y1, const ColorStops &_cs);
  Paint(double cx0, double cy0, double radius0, double cx1, double cy1,
        double radius1, const ColorStops &cs);
  Paint(const Paint &other) { *this = other; }
  Paint &operator=(const Paint &other) {
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
    return *this;
  }
  virtual ~Paint();

  virtual void emit(cairo_t *cr);
  virtual void emit(cairo_t *cr, double x, double y, double w, double h);
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
  std::string _description = "";

  gradientType _gradientType = gradientType::none;

  // linear gradient pattern space coordinates
  double _x0 = 0, _y0 = 0, _x1 = 0, _y1 = 0;

  // radial gradient pattern space coordinates
  double _cx0 = 0, _cy0 = 0, _radius0 = 0, _cx1 = 0, _cy1 = 0, _radius1 = 0;
  ColorStops _stops = {};

  filterType _filter = filterType::fast;
  extendType _extend = extendType::repeat;
  double _width = -1;
  double _height = -1;

  cairo_pattern_t *_pattern = nullptr;
  cairo_surface_t *_image = nullptr;
  PangoColor _pangoColor = {0, 0, 0};
  bool _bLoaded = false;
};

} // namespace uxdevice
