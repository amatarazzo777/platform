/**
\author Anthony Matarazzo
\file uxdisplaycontext.hpp
\date 5/12/20
\version 1.0
 \details CLass provides the painting object interface which invokes the
 appropriate cairo api. The notable virtual method is the emit function
 which applies the cairo source setting.

*/
#include "uxdevice.hpp"

/**
\brief color stops interface
*/
uxdevice::ColorStop::ColorStop(u_int32_t c) : ColorStop(-1, c) {
  _bAutoOffset = true;
}

uxdevice::ColorStop::ColorStop(double o, u_int32_t c) {
  _bRGBA = false;
  _bAutoOffset = false;
  _offset = o;
  _r = static_cast<u_int8_t>(c >> 16) / 255.0;
  _g = static_cast<u_int8_t>(c >> 8) / 255.0;
  _b = static_cast<u_int8_t>(c) / 255.0;
  _a = 1.0;
}

uxdevice::ColorStop::ColorStop(double r, double g, double b)
    : _bAutoOffset(true), _bRGBA(false), _offset(-1), _r(r), _g(g), _b(b),
      _a(1) {}

uxdevice::ColorStop::ColorStop(double o, double r, double g, double b)
    : _bAutoOffset(false), _bRGBA(false), _offset(o), _r(r), _g(g), _b(b),
      _a(1) {}

uxdevice::ColorStop::ColorStop(double o, double r, double g, double b, double a)
    : _bAutoOffset(false), _bRGBA(true), _offset(o), _r(r), _g(g), _b(b),
      _a(a) {}

uxdevice::ColorStop::ColorStop(const std::string &s) : ColorStop(-1, s) {
  _bAutoOffset = true;
}
uxdevice::ColorStop::ColorStop(const std::string &s, double a)
    : ColorStop(-1, s, a) {
  _bAutoOffset = true;
}
uxdevice::ColorStop::ColorStop(double o, const std::string &s) {
  _bAutoOffset = false;
  _bRGBA = false;
  _offset = o;
  parseColor(s);
}

uxdevice::ColorStop::ColorStop(double o, const std::string &s, double a) {
  _bAutoOffset = false;
  _bRGBA = true;
  _offset = o;
  _a = a;
  parseColor(s);
}

void uxdevice::ColorStop::parseColor(const std::string &s) {
  PangoColor pangoColor;
  if (pango_color_parse(&pangoColor, s.data())) {
    _r = pangoColor.red / 65535.0;
    _g = pangoColor.green / 65535.0;
    _b = pangoColor.blue / 65535.0;
  }
}

/**
\brief color given as a uint32 value
*/
uxdevice::Paint::Paint(u_int32_t c) {

  _r = static_cast<u_int8_t>(c >> 16) / 255.0;
  _g = static_cast<u_int8_t>(c >> 8) / 255.0;
  _b = static_cast<u_int8_t>(c) / 255.0;
  _a = 1.0;
  _type = paintType::color;
  _bLoaded = true;
}

uxdevice::Paint::Paint(double r, double g, double b)
    : _r(r), _g(g), _b(b), _a(1.0), _type(paintType::color), _bLoaded(true) {}

uxdevice::Paint::Paint(double r, double g, double b, double a)
    : _r(r), _g(g), _b(b), _a(a), _type(paintType::color), _bLoaded(true) {}

// color given as a description
uxdevice::Paint::Paint(const std::string &n)
    : _description(n), _bLoaded(false) {}

uxdevice::Paint::Paint(const std::string &n, double width, double height)
    : _description(n), _width(width), _height(height), _bLoaded(false) {}

// specify a linear gradient
uxdevice::Paint::Paint(double x0, double y0, double x1, double y1,
                       const ColorStops &cs)
    : _gradientType(gradientType::linear), _x0(x0), _y0(y0), _x1(x1), _y1(y1),
      _stops(cs), _bLoaded(false) {}

// specify a radial gradient
uxdevice::Paint::Paint(double cx0, double cy0, double radius0, double cx1,
                       double cy1, double radius1, const ColorStops &cs)
    : _gradientType(gradientType::radial), _cx0(cx0), _cy0(cy0),
      _radius0(radius0), _cx1(cx1), _cy1(cy1), _radius1(radius1), _stops(cs),
      _bLoaded(false) {}

uxdevice::Paint::~Paint() {
  if (_pattern)
    cairo_pattern_destroy(_pattern);
  if (_image)
    cairo_surface_destroy(_image);
}

/**
\brief The routine handles the creation of the pattern or surface.
Patterns can be an image file, a description of a linear, actual parameters
of linear, a description of a radial, the actual radial parameters stored.
SVG inline or a base64 data set.

*/
bool uxdevice::Paint::create(void) {
  // already created,
  if (_bLoaded)
    return true;

  if (_description.size() == 0 && _stops.size() == 0) {
    return false;
  }

  // if a description was provided, determine how it should be interpreted
  _image = readImage(_description, _width, _height);

  if (_image) {
    _width = cairo_image_surface_get_width(_image);
    _height = cairo_image_surface_get_height(_image);
    _pattern = cairo_pattern_create_for_surface(_image);
    cairo_pattern_set_extend(_pattern, CAIRO_EXTEND_REPEAT);
    cairo_pattern_set_filter(_pattern, CAIRO_FILTER_FAST);
    _type = paintType::pattern;
    _bLoaded = true;

    // determine if the description is another form such as a gradient or
    // color.
  } else if (_description.size() > 0) {

    if (isLinearGradient(_description)) {
      _gradientType = gradientType::linear;

    } else if (isRadialGradient(_description)) {
      _gradientType = gradientType::radial;

    } else if (patch(_description)) {

    } else if (pango_color_parse(&_pangoColor, _description.data())) {
      _r = _pangoColor.red / 65535.0;
      _g = _pangoColor.green / 65535.0;
      _b = _pangoColor.blue / 65535.0;
      _a = 1;
      _type = paintType::color;
      _bLoaded = true;
    }
  }

  // still more processing to do, -- create gradients
  // the parsing above for gradients populates this data.
  // so this logic is used in duel form. When gradients may be
  // named as a string, or provided in complete API form.
  // the logic below fills in the offset values automatically distributing
  // equally across the noted offset. offsets are provided from 0 - 1
  // and name the point within the line.
  if (!_bLoaded && _stops.size() > 0) {

    if (_gradientType == gradientType::linear) {
      _pattern = cairo_pattern_create_linear(_x0, _y0, _x1, _y1);

    } else if (_gradientType == gradientType::radial) {
      _pattern = cairo_pattern_create_radial(_cx0, _cy0, _radius0, _cx1, _cy1,
                                             _radius1);
    }
    // provide auto offsets
    if (_pattern) {
      bool bDone = false;
      bool bEdgeEnd = false;

      // first one, if auto offset set to
      //   0 - the beginning of the color stops
      ColorStopsIterator it = _stops.begin();
      if (it->_bAutoOffset) {
        it->_bAutoOffset = false;
        it->_offset = 0;
      }
      double dOffset = it->_offset;

      while (!bDone) {

        // find first color stop with a defined offset.
        ColorStopsIterator it2 =
            find_if(it + 1, _stops.end(),
                    [](auto const &o) { return !o._bAutoOffset; });

        // not found, the last item in color stops did not have a value,
        // assign it 1.0
        if (it2 == _stops.end()) {
          bEdgeEnd = true;
          bDone = true;
          // very last one has a setting
        } else if (it2 == _stops.end() - 1) {
          bDone = true;
        }

        // distribute offsets equally across range
        int nColorStops = std::distance(it, it2);
        if (bEdgeEnd)
          nColorStops--;

        if (nColorStops > 0) {
          double incr = 0;
          if (bEdgeEnd) {
            incr = (1 - it->_offset) / nColorStops;
          } else {
            incr = (it2->_offset - it->_offset) / nColorStops;
            nColorStops--;
          }

          dOffset = it->_offset;
          while (nColorStops) {
            it++;
            dOffset += incr;
            it->_offset = dOffset;
            it->_bAutoOffset = false;
            nColorStops--;
          }
        }
        // forward to next range
        it = it2;
      }
      // add the color stops
      std::for_each(_stops.begin(), _stops.end(), [=](auto &n) {
        if (n._bRGBA)
          cairo_pattern_add_color_stop_rgba(_pattern, n._offset, n._r, n._g,
                                            n._b, n._a);
        else
          cairo_pattern_add_color_stop_rgb(_pattern, n._offset, n._r, n._g,
                                           n._b);
      });

      cairo_pattern_set_extend(_pattern, CAIRO_EXTEND_REPEAT);
      _type = paintType::pattern;
      _bLoaded = true;
    }
  }

  return _bLoaded;
}

/**
 parse web formats

linear-gradient(to bottom, #1e5799 0%,#2989d8 50%,#207cca 51%,#2989d8
51%,#7db9e8 100%); linear-gradient(to right, #1e5799 0%,#2989d8 50%,#207cca
51%,#2989d8 51%,#7db9e8 100%); linear-gradient(135deg, #1e5799 0%,#2989d8
50%,#207cca 51%,#2989d8 51%,#7db9e8 100%); linear-gradient(45deg, #1e5799
0%,#2989d8 50%,#207cca 51%,#2989d8 51%,#7db9e8 100%);

radial-gradient(ellipse at center, #1e5799 0%,#2989d8 50%,#207cca
51%,#2989d8 51%,#7db9e8 100%);

*/
const std::string_view sLinearPattern = "linear-gradient";
const std::string_view sRadialPattern = "radial-gradient";

bool uxdevice::Paint::isLinearGradient(const std::string &s) {

  if (s.compare(0, sLinearPattern.size(), sLinearPattern) != 0)
    return false;

  return true;
}

bool uxdevice::Paint::isRadialGradient(const std::string &s) {

  if (s.compare(0, sRadialPattern.size(), sRadialPattern) != 0)
    return false;

  return true;
}

bool uxdevice::Paint::patch(const std::string &s) { return false; }

/**
\brief The routine is called by area, text or other rendering attribute
areas when the color style is needed for painting. The cairo context is
passed. The paint is loaded if need be. File processing or simply parsing
the color name string.

*/
void uxdevice::Paint::emit(cairo_t *cr) {
  if (!isLoaded())
    create();

  if (isLoaded()) {
    switch (_type) {
    case paintType::none:
      break;
    case paintType::color:
      cairo_set_source_rgba(cr, _r, _g, _b, _a);
      break;
    case paintType::pattern:
      if (_pattern) {
        cairo_pattern_set_matrix(_pattern, &_matrix);
        cairo_set_source(cr, _pattern);
      }
      break;
    case paintType::image:
      if (_image) {
        cairo_pattern_set_matrix(_pattern, &_matrix);
        cairo_set_source_surface(cr, _image, 0, 0);
      }
      break;
    }
  }
}

void uxdevice::Paint::emit(cairo_t *cr, double x, double y, double w,
                           double h) {
  if (!isLoaded()) {
    create();

    // adjust to user space
    if (_type == paintType::pattern || _type == paintType::image)
      translate(-x, -y);
  }

  if (isLoaded()) {
    switch (_type) {
    case paintType::none:
      break;
    case paintType::color:
      cairo_set_source_rgba(cr, _r, _g, _b, _a);
      break;
    case paintType::pattern:
      if (_pattern) {
        cairo_pattern_set_matrix(_pattern, &_matrix);
        cairo_set_source(cr, _pattern);
      }
      break;
    case paintType::image:
      if (_image) {
        cairo_pattern_set_matrix(_pattern, &_matrix);
        cairo_set_source_surface(cr, _image, 0, 0);
      }
      break;
    }
  }
}
