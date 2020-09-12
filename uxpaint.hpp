/*
 * This file is part of the PLATFORM_OBJ distribution
 * {https://github.com/amatarazzo777/platform_obj). Copyright (c) 2020 Anthony
 * Matarazzo.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
\author Anthony Matarazzo
\file uxpaint.hpp
\date 9/7/20
\version 1.0
\brief
*/

#pragma once

namespace uxdevice {
/**
\class painter_brush_t

\brief interface for the paint class.

*/
enum class paintType { none, color, pattern, image_block };
enum class gradientType { none, linear, radial };

class color_stop_t {
public:
  color_stop_t(u_int32_t _c);
  color_stop_t(double _r, double _g, double _b);
  color_stop_t(const std::string &_s);
  color_stop_t(const std::string &_s, double _a);

  color_stop_t(double _o, u_int32_t _c);
  color_stop_t(double _o, double _r, double _g, double _b);
  color_stop_t(double _o, double _r, double _g, double _b, double _a);
  color_stop_t(double _o, const std::string &_s);
  color_stop_t(double _o, const std::string &_s, double _a);
  void parse_color(const std::string &_s);

  UX_HASH_OBJECT_MEMBERS(std::type_index(typeid(this)), bAutoOffset, bRGBA,
                         offset, r, g, b, a)

  bool bAutoOffset = false;
  bool bRGBA = false;
  double offset = 0;
  double r = 0;
  double g = 0;
  double b = 0;
  double a = 1;
};
typedef std::vector<color_stop_t> color_stops_t;
typedef std::vector<color_stop_t>::iterator color_stops_iterator_t;
} // namespace uxdevice

UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::color_stop_t);

/**
 parse web formats

linear-gradient(to bottom, #1e5799 0%,#2989d8 50%,#207cca 51%,#2989d8
51%,#7db9e8 100%);

linear-gradient(to right, #1e5799 0%,#2989d8 50%,#207cca
51%,#2989d8 51%,#7db9e8 100%);

linear-gradient(135deg, #1e5799 0%,#2989d8
50%,#207cca 51%,#2989d8 51%,#7db9e8 100%);

linear-gradient(45deg, #1e5799
0%,#2989d8 50%,#207cca 51%,#2989d8 51%,#7db9e8 100%);

radial-gradient(ellipsize at center, #1e5799 0%,#2989d8 50%,#207cca
51%,#2989d8 51%,#7db9e8 100%);

*/
namespace uxdevice {
class coordinate_t;

class painter_brush_t : public Matrix {
public:
  enum class paint_definition_class_t {
    none,
    descriptive,
    color,
    linear_gradient,
    radial_gradient,
    image_block_pattern
  };

  class paint_definition_base_t {
  public:
    paint_definition_base_t(paint_definition_class_t _ct,
                            const std::string &_description)
        : class_type(_ct), description(_description) {}
    paint_definition_base_t &operator=(const paint_definition_base_t &other) {

      return *this;
    }
    paint_definition_base_t() {}
    paint_definition_base_t(const paint_definition_base_t &other) {}
    paint_definition_base_t(paint_definition_base_t &&other) {}

    virtual ~paint_definition_base_t() {}
    virtual void emit(cairo_t *cr) {}
    virtual void emit(cairo_t *cr, double x, double y, double w, double h) {}
    virtual void emit(cairo_t *cr, const coordinate_t &a) {}

    bool is_color_description(void) {
      bool bret = false;
      if (pango_color_parse(&pango_color, description.data())) {
        bret = true;
      }
      return bret;
    }
    bool is_linear_gradient_description(void) {
      const std::string_view sLinearPattern = "linear-gradient";
      if (description.compare(0, sLinearPattern.size(), sLinearPattern) != 0)
        return false;

      return true;
    }
    bool is_radial_gradient_description(void) {
      const std::string_view sRadialPattern = "radial-gradient";
      if (description.compare(0, sRadialPattern.size(), sRadialPattern) != 0)
        return false;

      return true;
    }

    bool is_patch_description(void) { return false; }

    UX_HASH_OBJECT_MEMBERS(std::type_index(typeid(this)), class_type,
                           description, pango_color.red, pango_color.green,
                           pango_color.blue, is_loaded)

    paint_definition_class_t class_type = paint_definition_class_t::none;
    std::string description = {};
    Matrix matrix = {};
    PangoColor pango_color = {0, 0, 0};
    bool is_loaded = false;
  };
  typedef std::shared_ptr<paint_definition_base_t> data_storage_t;

  class descriptive_definition_t : public paint_definition_base_t {
  public:
    descriptive_definition_t() {}
    descriptive_definition_t(const std::string &_description)
        : paint_definition_base_t(paint_definition_class_t::descriptive,
                                  _description) {}
    descriptive_definition_t &operator=(const descriptive_definition_t &other) {

      return *this;
    }
    descriptive_definition_t(const descriptive_definition_t &other)
        : paint_definition_base_t(other) {}
    descriptive_definition_t(descriptive_definition_t &&other)
        : paint_definition_base_t(other) {}
    virtual ~descriptive_definition_t() {}

    UX_HASH_OBJECT_MEMBERS(paint_definition_base_t::hash_code(),
                           UX_HASH_TYPE_ID_THIS)
  };

  class color_definition_t : public paint_definition_base_t {
  public:
    color_definition_t() {}
    color_definition_t(const std::string &_description, double _r, double _g,
                       double _b, double _a)
        : paint_definition_base_t(paint_definition_class_t::color,
                                  _description),
          r(_r), g(_g), b(_b), a(_a) {}
    color_definition_t(const std::string &_description)
        : paint_definition_base_t(paint_definition_class_t::color,
                                  _description) {
      if (pango_color_parse(&pango_color, _description.data())) {
        r = pango_color.red / 65535.0;
        g = pango_color.green / 65535.0;
        b = pango_color.blue / 65535.0;
        a = 1;
        is_loaded = true;
      }
    }
    color_definition_t &operator=(const color_definition_t &other) {
      r = other.r;
      g = other.g;
      b = other.b;
      a = other.a;
      return *this;
    }
    color_definition_t(const color_definition_t &other)
        : paint_definition_base_t(other), r(other.r), g(other.g), b(other.b),
          a(other.a) {}
    color_definition_t(color_definition_t &&other)
        : paint_definition_base_t(other) {
      r = std::move(other.r);
      g = std::move(other.g);
      b = std::move(other.b);
      a = std::move(other.a);
    }

    virtual ~color_definition_t() {}
    virtual void emit(cairo_t *cr) { cairo_set_source_rgba(cr, r, g, b, a); }
    virtual void emit(cairo_t *cr, double x, double y, double w, double h) {
      cairo_set_source_rgba(cr, r, g, b, a);
    }

    UX_HASH_OBJECT_MEMBERS(paint_definition_base_t::hash_code(),
                           std::type_index(typeid(this)), r, g, b, a)

    double r = {};
    double g = {};
    double b = {};
    double a = {};
  };

  class linear_gradient_definition_t : public paint_definition_base_t {
  public:
    linear_gradient_definition_t() {}
    linear_gradient_definition_t(const std::string &_description, double _x0,
                                 double _y0, double _x1, double _y1,
                                 const color_stops_t &_cs,
                                 filter_options_t _filter,
                                 extend_options_t _extend)
        : paint_definition_base_t(paint_definition_class_t::linear_gradient,
                                  _description),
          x0(_x0), y0(_x0), x1(_x0), y1(_x0), color_stops(_cs), filter(_filter),
          extend(_extend) {}
    linear_gradient_definition_t(const std::string &_description)
        : paint_definition_base_t(paint_definition_class_t::linear_gradient,
                                  _description) {}
    linear_gradient_definition_t &
    operator=(const linear_gradient_definition_t &other) {

      return *this;
    }
    linear_gradient_definition_t(const linear_gradient_definition_t &other)
        : paint_definition_base_t(other) {}
    linear_gradient_definition_t(linear_gradient_definition_t &&other)
        : paint_definition_base_t(other) {}

    ~linear_gradient_definition_t() {
      if (pattern)
        cairo_pattern_destroy(pattern);
    }
    virtual void emit(cairo_t *cr) {
      cairo_pattern_set_matrix(pattern, &matrix._matrix);
      cairo_set_source(cr, pattern);
    }
    virtual void emit(cairo_t *cr, double x, double y, double w, double h) {
      cairo_pattern_set_matrix(pattern, &matrix._matrix);
      cairo_set_source(cr, pattern);
    }

    UX_HASH_OBJECT_MEMBERS(UX_HASH_VECTOR_OBJECTS(color_stops),
                           paint_definition_base_t::hash_code(),
                           std::type_index(typeid(this)), x0, y0, x1, y1,
                           filter, extend, pattern)

    double x0 = {};
    double y0 = {};
    double x1 = {};
    double y1 = {};
    color_stops_t color_stops = {};
    filter_options_t filter = {};
    extend_options_t extend = {};
    cairo_pattern_t *pattern = {};
  };

  class radial_gradient_definition_t : public paint_definition_base_t {
  public:
    radial_gradient_definition_t() {}
    radial_gradient_definition_t(const std::string &_description, double _cx0,
                                 double _cy0, double _radius0, double _cx1,
                                 double _cy1, double _radius1,
                                 const color_stops_t &_cs,
                                 filter_options_t _filter,
                                 extend_options_t _extend)
        : paint_definition_base_t(paint_definition_class_t::radial_gradient,
                                  _description),
          cx0(_cx0), cy0(_cy0), radius0(_radius0), cx1(_cx1), cy1(_cy1),
          radius1(_radius1), color_stops(_cs), filter(_filter),
          extend(_extend) {}
    ~radial_gradient_definition_t() {
      if (pattern)
        cairo_pattern_destroy(pattern);
    }
    radial_gradient_definition_t(const std::string &_description)
        : paint_definition_base_t(paint_definition_class_t::radial_gradient,
                                  _description) {}
    radial_gradient_definition_t &
    operator=(const radial_gradient_definition_t &other) {

      return *this;
    }
    radial_gradient_definition_t(const radial_gradient_definition_t &other)
        : paint_definition_base_t(other) {}
    radial_gradient_definition_t(radial_gradient_definition_t &&other)
        : paint_definition_base_t(other) {}

    virtual void emit(cairo_t *cr) {
      cairo_pattern_set_matrix(pattern, &matrix._matrix);
      cairo_set_source(cr, pattern);
    }
    virtual void emit(cairo_t *cr, double x, double y, double w, double h) {
      cairo_pattern_set_matrix(pattern, &matrix._matrix);
      cairo_set_source(cr, pattern);
    }

    UX_HASH_OBJECT_MEMBERS(UX_HASH_VECTOR_OBJECTS(color_stops),
                           paint_definition_base_t::hash_code(),
                           UX_HASH_TYPE_ID_THIS, cx0, cy0, radius0, cx1, cy1,
                           radius1, filter, extend, pattern)

    double cx0 = {};
    double cy0 = {};
    double radius0 = {};
    double cx1 = {};
    double cy1 = {};
    double radius1 = {};
    color_stops_t color_stops = {};
    filter_options_t filter = {};
    extend_options_t extend = {};
    cairo_pattern_t *pattern = {};
  };

  class image_block_pattern_source_definition_t
      : public paint_definition_base_t {
  public:
    image_block_pattern_source_definition_t() {}
    image_block_pattern_source_definition_t(const std::string &_description,
                                            double _width, double _height)
        : paint_definition_base_t(paint_definition_class_t::image_block_pattern,
                                  _description),
          width(_width), height(_height) {}
    image_block_pattern_source_definition_t(const std::string &_description,
                                            double _width, double _height,
                                            cairo_surface_t *_image,
                                            filter_options_t _filter,
                                            extend_options_t _extend)
        : paint_definition_base_t(paint_definition_class_t::image_block_pattern,
                                  _description),
          width(_width), height(_height), image_block(_image), filter(_filter),
          extend(_extend) {
      pattern = cairo_pattern_create_for_surface(image_block);
      cairo_pattern_set_extend(pattern, static_cast<cairo_extend_t>(extend));
      cairo_pattern_set_filter(pattern, static_cast<cairo_filter_t>(filter));
    }
    image_block_pattern_source_definition_t &
    operator=(const image_block_pattern_source_definition_t &other) {

      return *this;
    }
    image_block_pattern_source_definition_t(
        const image_block_pattern_source_definition_t &other)
        : paint_definition_base_t(other) {}
    image_block_pattern_source_definition_t(
        image_block_pattern_source_definition_t &&other)
        : paint_definition_base_t(other) {}

    ~image_block_pattern_source_definition_t() {
      if (image_block)
        cairo_surface_destroy(image_block);
      if (pattern)
        cairo_pattern_destroy(pattern);
    }
    virtual void emit(cairo_t *cr) {
      cairo_pattern_set_matrix(pattern, &matrix._matrix);
      cairo_set_source(cr, pattern);
    }
    virtual void emit(cairo_t *cr, double x, double y, double w, double h) {
      cairo_pattern_set_matrix(pattern, &matrix._matrix);
      cairo_set_source(cr, pattern);
    }

    UX_HASH_OBJECT_MEMBERS(paint_definition_base_t::hash_code(),
                           std::type_index(typeid(this)), width, height,
                           image_block, pattern, filter, extend)

    double width = {};
    double height = {};
    cairo_surface_t *image_block = {};
    cairo_pattern_t *pattern = {};
    filter_options_t filter = {};
    extend_options_t extend = {};
  };

  painter_brush_t() {}
  /**
  \brief color given as a uint32 value
  */
  painter_brush_t(u_int32_t c) {

    u_int8_t r = static_cast<u_int8_t>(c >> 16) / 255.0;
    u_int8_t g = static_cast<u_int8_t>(c >> 8) / 255.0;
    u_int8_t b = static_cast<u_int8_t>(c) / 255.0;
    u_int8_t a = 1.0;

    data_storage =
        std::make_shared<color_definition_t>("u_int32_t RGB", r, g, b, a);
  }

  painter_brush_t(double r, double g, double b)
      : data_storage(
            std::make_shared<color_definition_t>("RGB", r, g, b, 1.0)) {}

  painter_brush_t(double r, double g, double b, double a)
      : data_storage(std::make_shared<color_definition_t>("RGBA", r, g, b, a)) {
  }

  // color given as a description
  painter_brush_t(const std::string &n)
      : data_storage(std::make_shared<descriptive_definition_t>(n)) {}

  painter_brush_t(const std::string &n, double width, double height)
      : data_storage(std::make_shared<image_block_pattern_source_definition_t>(
            n, width, height)) {}

  painter_brush_t(double x0, double y0, double x1, double y1,
                  const color_stops_t &cs)
      : data_storage(std::make_shared<linear_gradient_definition_t>(
            "linear_gradient", x0, y0, x1, y1, cs, filter_options_t::fast,
            extend_options_t::off)) {}

  // specify a radial gradient
  painter_brush_t(double cx0, double cy0, double radius0, double cx1,
                  double cy1, double radius1, const color_stops_t &cs)
      : data_storage(std::make_shared<radial_gradient_definition_t>(
            "radial_gradient", cx0, cy0, radius0, cx1, cy1, radius1, cs,
            filter_options_t::fast, extend_options_t::off)) {}

  painter_brush_t(const painter_brush_t &other) { *this = other; }
  painter_brush_t &operator=(const painter_brush_t &other) {
    Matrix::operator=(other);
    data_storage = other.data_storage;
    return *this;
  }
  painter_brush_t(painter_brush_t &&other) {
    data_storage = std::move(other.data_storage);
  }

  virtual ~painter_brush_t() {
    if (data_storage)
      data_storage.reset();
  }

  virtual void emit(cairo_t *cr);
  virtual void emit(cairo_t *cr, const coordinate_t &a);
  bool is_valid(void) { return data_storage != nullptr; }

private:
  bool create(void);

  bool is_linear_gradient(const std::string &s);
  bool is_radial_gradient(const std::string &s);
  bool patch(const std::string &s);

public:
  UX_HASH_OBJECT_MEMBERS(std::type_index(typeid(this)), data_storage)

  data_storage_t data_storage = {};
};

} // namespace uxdevice

UX_REGISTER_STD_HASH_SPECIALIZATION(
    uxdevice::painter_brush_t::paint_definition_base_t);
UX_REGISTER_STD_HASH_SPECIALIZATION(
    uxdevice::painter_brush_t::descriptive_definition_t);
UX_REGISTER_STD_HASH_SPECIALIZATION(
    uxdevice::painter_brush_t::color_definition_t);
UX_REGISTER_STD_HASH_SPECIALIZATION(
    uxdevice::painter_brush_t::linear_gradient_definition_t);
UX_REGISTER_STD_HASH_SPECIALIZATION(
    uxdevice::painter_brush_t::radial_gradient_definition_t);
UX_REGISTER_STD_HASH_SPECIALIZATION(
    uxdevice::painter_brush_t::image_block_pattern_source_definition_t);
