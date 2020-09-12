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
\file uxdisplayunits.hpp
\date 9/7/20
\version 1.0
\brief
*/
/**
\author Anthony Matarazzo
\file uxpaint.hpp
\date 5/12/20
\version 1.0
 \details CLass provides the painting object interface which invokes the
 appropriate cairo api. The notable virtual method is the emit function
 which applies the cairo text_color_t setting.

*/
#include "uxdevice.hpp"

/**
\brief color stops interface
*/
uxdevice::color_stop_t::color_stop_t(u_int32_t _c) : color_stop_t(-1, _c) {
  bAutoOffset = true;
}

uxdevice::color_stop_t::color_stop_t(double _o, u_int32_t _c) {
  bRGBA = false;
  bAutoOffset = false;
  offset = _o;
  r = static_cast<u_int8_t>(_c >> 16) / 255.0;
  g = static_cast<u_int8_t>(_c >> 8) / 255.0;
  b = static_cast<u_int8_t>(_c) / 255.0;
  a = 1.0;
}

uxdevice::color_stop_t::color_stop_t(double _r, double _g, double _b)
    : bAutoOffset(true), bRGBA(false), offset(-1), r(_r), g(_g), b(_b), a(1) {}

uxdevice::color_stop_t::color_stop_t(double _offset, double _r, double _g,
                                     double _b)
    : bAutoOffset(false), bRGBA(false), offset(_offset), r(_r), g(_g), b(_b),
      a(1) {}

uxdevice::color_stop_t::color_stop_t(double _offset, double _r, double _g,
                                     double _b, double _a)
    : bAutoOffset(false), bRGBA(true), offset(_offset), r(_r), g(_g), b(_b),
      a(_a) {}

uxdevice::color_stop_t::color_stop_t(const std::string &_s)
    : color_stop_t(-1, _s) {
  bAutoOffset = true;
}
uxdevice::color_stop_t::color_stop_t(const std::string &_s, double _a)
    : color_stop_t(-1, _s, _a) {
  bAutoOffset = true;
}
uxdevice::color_stop_t::color_stop_t(double _o, const std::string &_s) {
  bAutoOffset = false;
  bRGBA = false;
  offset = _o;
  parse_color(_s);
}

uxdevice::color_stop_t::color_stop_t(double _o, const std::string &_s,
                                     double _a) {
  bAutoOffset = false;
  bRGBA = true;
  offset = _o;
  a = _a;
  parse_color(_s);
}

void uxdevice::color_stop_t::parse_color(const std::string &_s) {
  PangoColor pango_color;
  if (pango_color_parse(&pango_color, _s.data())) {
    r = pango_color.red / 65535.0;
    g = pango_color.green / 65535.0;
    b = pango_color.blue / 65535.0;
  }
}

/**
\brief The routine handles the creation of the pattern or surface.
Patterns can be an image_block_t file, a description of a linear, actual
parameters of linear, a description of a radial, the actual radial parameters
stored. SVG inline or a base64 data set.

*/
bool uxdevice::painter_brush_t::create(void) {

  // already created,
  if (data_storage->is_loaded)
    return true;
  if (!is_valid())
    return false;

  // if a description was provided, determine how it should be interpreted
  if (data_storage->class_type == paint_definition_class_t::descriptive) {
    double _width = {};
    double _height = {};

    auto _image = read_image(data_storage->description, _width, _height);

    if (_image) {

      data_storage = std::make_shared<image_block_pattern_source_definition_t>(
          data_storage->description, cairo_image_surface_get_width(_image),
          cairo_image_surface_get_height(_image), _image,
          filter_options_t::fast, extend_options_t::repeat);
      data_storage->is_loaded = true;
      // determine if the description is another form such as a gradient or
      // color.
    } else if (data_storage->is_linear_gradient_description()) {
      data_storage = std::make_shared<linear_gradient_definition_t>(
          data_storage->description);

    } else if (data_storage->is_radial_gradient_description()) {
      data_storage = std::make_shared<radial_gradient_definition_t>(
          data_storage->description);

    } else if (data_storage->is_patch_description()) {

    } else if (data_storage->is_color_description()) {
      data_storage =
          std::make_shared<color_definition_t>(data_storage->description);
    }
  }

  // early exit
  if (data_storage->is_loaded)
    return data_storage->is_loaded;

  // still more processing to do, -- create gradients
  // the parsing above for gradients populates this data.
  // so this logic is used in duel form. When gradients may be
  // named as a string, or provided in complete API form.
  // the logic below fills in the offset values automatically distributing
  // equally across the noted offset. offsets are provided from 0 - 1
  // and name the point within the line.

  color_stops_t *ptr_cs = {};
  cairo_pattern_t *ptr_cp = {};
  switch (data_storage->class_type) {
  case paint_definition_class_t::linear_gradient: {
    auto p =
        std::dynamic_pointer_cast<linear_gradient_definition_t>(data_storage);
    p->pattern = cairo_pattern_create_linear(p->x0, p->y0, p->x1, p->y1);
    ptr_cp = p->pattern;
    ptr_cs = &p->color_stops;
  } break;
  case paint_definition_class_t::radial_gradient: {
    auto p =
        std::dynamic_pointer_cast<radial_gradient_definition_t>(data_storage);
    p->pattern = cairo_pattern_create_radial(p->cx0, p->cy0, p->radius0, p->cx1,
                                             p->cy1, p->radius1);
    ptr_cp = p->pattern;
    ptr_cs = &p->color_stops;
  } break;
  case paint_definition_class_t::none: {
  } break;
  case paint_definition_class_t::descriptive: {
  } break;
  case paint_definition_class_t::color: {
  } break;
  case paint_definition_class_t::image_block_pattern: {
  } break;
  }

  // early exit
  if (!ptr_cs) {
    data_storage->is_loaded = false;
    return data_storage->is_loaded;
  }

  color_stops_t &stops = *ptr_cs;
  if (stops.size() > 0 && ptr_cp) {
    bool bDone = false;
    bool bEdgeEnd = false;

    // first one, if auto offset set to
    //   0 - the beginning of the color stops
    color_stops_iterator_t it = stops.begin();
    if (it->bAutoOffset) {
      it->bAutoOffset = false;
      it->offset = 0;
    }
    double dOffset = it->offset;

    while (!bDone) {

      // find first color stop with a defined offset.
      color_stops_iterator_t it2 = find_if(
          it + 1, stops.end(), [](auto const &o) { return !o.bAutoOffset; });

      // not found, the last item in color stops did not have a value,
      // assign it 1.0
      if (it2 == stops.end()) {
        bEdgeEnd = true;
        bDone = true;
        // very last one has a setting
      } else if (it2 == stops.end() - 1) {
        bDone = true;
      }

      // distribute offsets equally across range
      int ncolor_stops_t = std::distance(it, it2);
      if (bEdgeEnd)
        ncolor_stops_t--;

      if (ncolor_stops_t > 0) {
        double incr = 0;
        if (bEdgeEnd) {
          incr = (1 - it->offset) / ncolor_stops_t;
        } else {
          incr = (it2->offset - it->offset) / ncolor_stops_t;
          ncolor_stops_t--;
        }

        dOffset = it->offset;
        while (ncolor_stops_t) {
          it++;
          dOffset += incr;
          it->offset = dOffset;
          it->bAutoOffset = false;
          ncolor_stops_t--;
        }
      }
      // forward to next range
      it = it2;
    }
    // add the color stops
    std::for_each(stops.begin(), stops.end(), [=](auto &n) {
      if (n.bRGBA)
        cairo_pattern_add_color_stop_rgba(ptr_cp, n.offset, n.r, n.g, n.b, n.a);
      else
        cairo_pattern_add_color_stop_rgb(ptr_cp, n.offset, n.r, n.g, n.b);
    });

    cairo_pattern_set_extend(ptr_cp, CAIRO_EXTEND_REPEAT);
    data_storage->is_loaded = true;
  }

  return data_storage->is_loaded;
}

/**
\brief The routine is called by area, text or other rendering attribute
areas when the color style is needed for painting. The cairo context is
passed. The paint is loaded if need be. File processing or simply parsing
the color name string.

*/
void uxdevice::painter_brush_t::emit(cairo_t *cr) {
  if (!data_storage->is_loaded)
    create();

  if (data_storage->is_loaded) {
    data_storage->emit(cr);
  }
}

void uxdevice::painter_brush_t::emit(cairo_t *cr, const coordinate_t &a) {
  if (!data_storage->is_loaded) {
    create();

    // adjust to user space
    if (data_storage->class_type == paint_definition_class_t::linear_gradient ||
        data_storage->class_type == paint_definition_class_t::radial_gradient ||
        data_storage->class_type ==
            paint_definition_class_t::image_block_pattern)
      translate(-a.x, -a.y);
  }

  if (data_storage->is_loaded) {
    data_storage->emit(cr, a.x, a.y, a.w, a.h);
  }
}
