/*
 * This file is part of the PLATFORM_OBJ distribution
 *
 * (https://github.com/amatarazzo777/platform_obj)
 *
 * Copyright (c) 2020 Anthony Matarazzo.
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
\brief The file encapsulates the objects that may be created,
controlled, and rendered by the system. Attributes. externalized data such
as std::shared pointers may also be stored. Each object has a hash
function which is also queried to detect changes. Only on screen related
objects are included within the set. The intersection test is also included
as part of a base hierarchy of classes for objects that are declared as such
(_DRAWING_FUNCTION). display_unit_t is the logical base class.

*/

#pragma once

/**

\class coordinate_storage_t
\brief storage class used by the coordinate_t object.

\details The constructor interface is inherited by objects
that are display units.


 */
namespace uxdevice {
class coordinate_storage_t
    : public polymorphic_overloads_t,
      public std::enable_shared_from_this<coordinate_storage_t> {
public:
  coordinate_storage_t() {}
  coordinate_storage_t(double _x, double _y, double _w, double _h)
      : x(_x), y(_y), w(_w), h(_h) {}
  coordinate_storage_t(double _x, double _y) : x(_x), y(_y) {}
  virtual ~coordinate_storage_t() {}

  void invoke(display_context_t &context);
  void emit(display_context_t &context) { emit(context.cr); }
  void emit(cairo_t *cr);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, x, y, w, h)

  double x = {};
  double y = {};
  double w = {};
  double h = {};

  friend class coordinate_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::coordinate_storage_t);

/**
\internal
\class arc_storage_t
\brief

\details


 */
namespace uxdevice {
class arc_storage_t : public polymorphic_overloads_t,
                      public std::enable_shared_from_this<arc_storage_t> {
public:
  arc_storage_t() {}
  arc_storage_t(double _xc, double _yc, double _radius, double _angle1,
                double _angle2)
      : xc(_xc), yc(_yc), radius(_radius), angle1(_angle1), angle2(_angle2) {}
  virtual ~arc_storage_t() {}

  void invoke(display_context_t &context);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, xc, yc, radius, angle1, angle2)

  double xc = {};
  double yc = {};
  double radius = {};
  double angle1 = {};
  double angle2 = {};
  friend class arc_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::arc_storage_t);

/**
\internal
\class negative_arc_storage_t
\brief

\details


 */
namespace uxdevice {
struct negative_arc_storage_t
    : public polymorphic_overloads_t,
      public std::enable_shared_from_this<arc_storage_t> {
public:
  negative_arc_storage_t() {}
  negative_arc_storage_t(double _xc, double _yc, double _radius, double _angle1,
                         double _angle2)
      : xc(_xc), yc(_yc), radius(_radius), angle1(_angle1), angle2(_angle2) {}
  virtual ~negative_arc_storage_t() {}

  void invoke(display_context_t &context);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, xc, yc, radius, angle1, angle2)

  double xc = {};
  double yc = {};
  double radius = {};
  double angle1 = {};
  double angle2 = {};

  friend class arc_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::negative_arc_storage_t);

/**
\internal
\class rectangle_storage_t
\brief

\details


 */
namespace uxdevice {
class rectangle_storage_t : public polymorphic_overloads_t,
                            public std::enable_shared_from_this<arc_storage_t> {
public:
  rectangle_storage_t() {}
  rectangle_storage_t(double _x, double _y, double _width, double _height)
      : x(_x), y(_y), width(_width), height(_height) {}
  virtual ~rectangle_storage_t() {}

  void invoke(display_context_t &context);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, x, y, width, height)

  double x = {};
  double y = {};
  double width = {};
  double height = {};
  friend class rectangle_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::rectangle_storage_t);

/**
\internal
\class curve_storage_t
\brief

\details


 */
namespace uxdevice {
class curve_storage_t : public polymorphic_overloads_t,
                        public std::enable_shared_from_this<arc_storage_t> {
public:
  curve_storage_t() {}
  curve_storage_t(double _x1, double _y1, double _x2, double _y2, double _x3,
                  double _y3)
      : x1(_x1), y1(_y1), x2(_x2), y2(_y2), x3(_x3), y3(_y3) {}
  virtual ~curve_storage_t() {}

  void invoke(display_context_t &context);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, x1, y1, x2, y2, x3, y3)

  double x1 = {};
  double y1 = {};
  double x2 = {};
  double y2 = {};
  double x3 = {};
  double y3 = {};
  friend class curve_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::curve_storage_t);

/**
\internal
\class line_storage_t
\brief

\details


 */
namespace uxdevice {
class line_storage_t : public polymorphic_overloads_t,
                       public std::enable_shared_from_this<arc_storage_t> {
public:
  line_storage_t() {}
  line_storage_t(double _x, double _y) : x(_x), y(_y) {}
  virtual ~line_storage_t() {}

  void invoke(display_context_t &context);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, x, y)

  double x = {};
  double y = {};
  friend class line_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::line_storage_t);

/**
\internal
\class stroke_fill_path_storage_t
\brief

\details


 */
namespace uxdevice {
struct stroke_fill_path_storage_t
    : public polymorphic_overloads_t,
      public std::enable_shared_from_this<arc_storage_t> {
  painter_brush_t fill_brush = {};
  painter_brush_t stroke_brush = {};

  stroke_fill_path_storage_t() {}

  stroke_fill_path_storage_t(const painter_brush_t &f, const painter_brush_t &s)
      : fill_brush(f), stroke_brush(s) {}
  virtual ~stroke_fill_path_storage_t() {}

  void invoke(display_context_t &context);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, fill_brush.hash_code(),
                         stroke_brush.hash_code())
  friend class stroke_fill_path_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::stroke_fill_path_storage_t);

/**

\typedef line_dash_storage_t
\brief storage alias for the line dashes array. needed for registering the
hashing function.

\details


 */
namespace uxdevice {
class line_dash_storage_t : public polymorphic_overloads_t,
                            public std::enable_shared_from_this<arc_storage_t> {
public:
  line_dash_storage_t() {}
  line_dash_storage_t(const std::vector<double> &_value, const double &_offset)
      : value(_value), offset(_offset) {}

  virtual ~line_dash_storage_t() {}

  void invoke(display_context_t &context);
  void emit(display_context_t &context) { emit(context.cr); }
  void emit(cairo_t *cr);

  std::vector<double> value = {};
  double offset = {};

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, UX_HASH_VECTOR_OBJECTS(value),
                         offset)
  friend class line_dash_t;
};
} // namespace uxdevice

UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::line_dash_storage_t);

/**

\class image_block_storage_t
\brief storage class used by the image_block_t object. The oject is
responsible for encapsulating and dynamically allocating, and releasing
memory.

\details


 */
namespace uxdevice {
class image_block_storage_t
    : public polymorphic_overloads_t,
      public std::enable_shared_from_this<arc_storage_t> {
public:
  /// @brief default constructor
  image_block_storage_t()
      : description{}, image_block_ptr{}, is_SVG{}, is_loaded{}, coordinate{} {}

  image_block_storage_t(const std::string &image_description)
      : description(image_description) {}

  /// @brief move assignment
  image_block_storage_t &operator=(image_block_storage_t &&other) noexcept {
    image_block_ptr = std::move(other.image_block_ptr);
    is_SVG = other.is_SVG;
    is_loaded = other.is_loaded;
    coordinate = std::move(other.coordinate);
    return *this;
  }

  /// @brief copy assignment operator
  image_block_storage_t &operator=(const image_block_storage_t &other) {
    image_block_ptr = cairo_surface_reference(other.image_block_ptr);
    is_SVG = other.is_SVG;
    is_loaded = other.is_loaded;
    coordinate = other.coordinate;
    return *this;
  }

  /// @brief move constructor
  image_block_storage_t(image_block_storage_t &&other) noexcept
      : image_block_ptr(std::move(other.image_block_ptr)),
        is_SVG(std::move(other.is_SVG)), is_loaded(std::move(other.is_loaded)),
        coordinate(std::move(other.coordinate)) {}

  /// @brief copy constructor
  image_block_storage_t(const image_block_storage_t &other)
      : image_block_ptr(cairo_surface_reference(other.image_block_ptr)),
        is_SVG(other.is_SVG), is_loaded(other.is_loaded),
        coordinate(other.coordinate) {}

  virtual ~image_block_storage_t() {
    if (image_block_ptr)
      cairo_surface_destroy(image_block_ptr);
  }

  void invoke(display_context_t &context);
  void emit(display_context_t &context) { emit(context.cr); }
  void emit(cairo_t *cr);

  void is_valid(void);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, description, is_SVG, is_loaded,
                         coordinate)

  std::string description = {};
  cairo_surface_t *image_block_ptr = {};
  bool is_SVG = {};
  bool is_loaded = {};
  std::shared_ptr<coordinate_t> coordinate = {};
  friend class image_block_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::image_block_storage_t);

/**
\internal
\class text_font_data_storage
\brief

\details


 */

namespace uxdevice {
class text_font_storage_t : public polymorphic_overloads_t,
                            public std::enable_shared_from_this<arc_storage_t> {
public:
  // these become public members of the base class.
  text_font_storage_t() : description{}, font_ptr(nullptr) {}
  text_font_storage_t(const std::string &_description)
      : description(_description), font_ptr(nullptr) {}

  virtual ~text_font_storage_t() {
    if (font_ptr)
      pango_font_description_free(font_ptr);
  }

  text_font_storage_t &operator=(const text_font_storage_t &&other) {
    description = other.description;
    return *this;
  }
  text_font_storage_t &operator=(const text_font_storage_t &other) {
    description = other.description;
    return *this;
  }

  text_font_storage_t &operator=(const std::string &_desc) {
    description = _desc;
    return *this;
  }

  /// @brief move assignment
  text_font_storage_t &operator=(const std::string &&_desc) noexcept {
    description = _desc;
    return *this;
  }

  void invoke(display_context_t &context);
  void emit(PangoLayout *layout);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, description)

  std::string description = {};
  PangoFontDescription *font_ptr = {};

  friend class text_font_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::text_font_storage_t);

/**

\class textual_render_storage_t
\brief class used to store parameters and options for a textual render. The
object is created as the side effect of inserting text, char *, std string or
a std::shared_ptr<std::string>.
*/
namespace uxdevice {
class textual_render_storage_t
    : public polymorphic_overloads_t,
      public std::enable_shared_from_this<arc_storage_t> {
public:
  typedef std::function<void(cairo_t *cr, coordinate_t &a)>
      internal_cairo_function_t;
  textual_render_storage_t() {}

  virtual ~textual_render_storage_t() {
    if (shadow_image)
      cairo_surface_destroy(shadow_image);

    if (shadow_cr)
      cairo_destroy(shadow_cr);

    if (layout)
      g_object_unref(layout);
  }

  void invoke(display_context_t &context);
  void emit(PangoLayout *layout);

  UX_DECLARE_TYPE_INDEX_MEMORY(rendering_parameter)

  UX_DECLARE_HASH_MEMBERS_INTERFACE

  cairo_surface_t *shadow_image = nullptr;
  cairo_t *shadow_cr = nullptr;
  PangoLayout *layout = nullptr;
  PangoRectangle ink_rect = PangoRectangle();
  PangoRectangle logical_rect = PangoRectangle();
  Matrix matrix = {};
  friend class textual_render_t;

private:
  bool set_layout_options(cairo_t *cr);
  void create_shadow(void);
  internal_cairo_function_t precise_rendering_function(void);
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::textual_render_storage_t);

/**

\class text_tab_stops_storage_t
\brief sets the tab stops on the layout
*/

namespace uxdevice {
class text_tab_stops_storage_t
    : public polymorphic_overloads_t,
      public std::enable_shared_from_this<arc_storage_t> {
public:
  text_tab_stops_storage_t() {}
  text_tab_stops_storage_t(const std::vector<double> &_value) : value(_value) {}
  virtual ~text_tab_stops_storage_t() {}

  void invoke(display_context_t &context);
  void emit(PangoLayout *layout);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, UX_HASH_VECTOR_OBJECTS(value))

  std::vector<double> value = {};
  friend class text_tab_stops_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::text_tab_stops_storage_t);

/**

\class listener_storage_t
\brief class used to store members of a listener event. This is the base class
storage. Listeners inherit from listener_t  as a base.
*/
namespace uxdevice {
class listener_storage_t : public polymorphic_overloads_t,
                           public std::enable_shared_from_this<arc_storage_t> {
public:
  listener_storage_t() {}
  listener_storage_t(const std::type_index &_ti, const event_handler_t &_evt)
      : type(_ti), dispatch_event(_evt) {}

  /// @brief move constructor
  listener_storage_t(listener_storage_t &&other) noexcept
      : type(other.type), dispatch_event(other.dispatch_event) {}

  /// @brief copy constructor
  listener_storage_t(const listener_storage_t &other)
      : type(other.type), dispatch_event(other.dispatch_event) {}
  virtual ~listener_storage_t() {}

  std::type_index type = std::type_index(typeid(this));
  event_handler_t dispatch_event = {};

  // apply overrides
  void invoke(display_context_t &context);

  UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, type, dispatch_event ? 1 : 0)
  friend class listener_t;
};
} // namespace uxdevice
UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::listener_storage_t);

/**
The following is is the exact name that appears within the API. These macros
provide expansion to create objects that have invoke and emit methods. The base
services for these objects to be functional for the display are also provided.
These objects have all of the move and copy operators implemented.
There are several types of objects.

*/
UX_DECLARE_PAINTER_BRUSH(surface_area_brush_t)

UX_DECLARE_CLASS_STORAGE_EMITTER(
    text_font_t, text_font_storage_t,
    using text_font_storage_t::text_font_storage_t;)

UX_DECLARE_STORAGE_EMITTER(surface_area_title_t, std::string,
                           void invoke(display_context_t &context);
                           void emit(display_context_t &context);)

UX_DECLARE_MARKER(text_render_fast_t, void invoke(display_context_t &context);)

UX_DECLARE_MARKER(text_render_path_t, void invoke(display_context_t &context);)

UX_DECLARE_PAINTER_BRUSH(text_color_t)
UX_DECLARE_PAINTER_BRUSH(text_outline_t)
UX_DECLARE_PAINTER_BRUSH(text_fill_t)
UX_DECLARE_PAINTER_BRUSH(text_shadow_t)

UX_DECLARE_STORAGE_EMITTER(text_alignment_t, text_alignment_options_t,
                           void invoke(display_context_t &context);
                           void emit(PangoLayout *layout);)

UX_DECLARE_STORAGE_EMITTER(text_indent_t, double,
                           void invoke(display_context_t &context);
                           void emit(PangoLayout *layout);)

UX_DECLARE_STORAGE_EMITTER(text_ellipsize_t, text_ellipsize_options_t,
                           void invoke(display_context_t &context);
                           void emit(PangoLayout *layout);)

UX_DECLARE_STORAGE_EMITTER(text_line_space_t, double,
                           void invoke(display_context_t &context);
                           void emit(PangoLayout *layout);)

UX_DECLARE_STORAGE_EMITTER(text_tab_stops_t, text_tab_stops_storage_t,
                           void invoke(display_context_t &context);
                           void emit(PangoLayout *layout);)

UX_DECLARE_STORAGE_EMITTER(text_data_t, std::string,
                           void invoke(display_context_t &context);
                           void emit(PangoLayout *layout);)

// image drawing and other block operations
UX_DECLARE_CLASS_STORAGE_EMITTER(
    coordinate_t, coordinate_storage_t,
    using coordinate_storage_t::coordinate_storage_t;)

UX_DECLARE_STORAGE_EMITTER(antialias_t, antialias_options_t,
                           void invoke(display_context_t &context);
                           void emit(cairo_t *cr);)

UX_DECLARE_STORAGE_EMITTER(line_width_t, double,
                           void invoke(display_context_t &context);
                           void emit(cairo_t *cr);)

UX_DECLARE_STORAGE_EMITTER(line_cap_t, line_cap_options_t,
                           void invoke(display_context_t &context);
                           void emit(cairo_t *cr);)

UX_DECLARE_STORAGE_EMITTER(line_join_t, line_join_options_t,
                           void invoke(display_context_t &context);
                           void emit(cairo_t *cr);)

UX_DECLARE_STORAGE_EMITTER(miter_limit_t, double,
                           void invoke(display_context_t &context);
                           void emit(cairo_t *cr);)

UX_DECLARE_STORAGE_EMITTER(line_dashes_t, line_dash_storage_t,
                           void invoke(display_context_t &context);
                           void emit(cairo_t *cr);)

UX_DECLARE_STORAGE_EMITTER(tollerance_t, double,
                           void invoke(display_context_t &context);
                           void emit(cairo_t *cr);)

UX_DECLARE_STORAGE_EMITTER(graphic_operator_t, graphic_operator_options_t,
                           void invoke(display_context_t &context);
                           void emit(cairo_t *cr);)

UX_DECLARE_MARKER(relative_coordinate_t,
                  void invoke(display_context_t &context);)

UX_DECLARE_MARKER(absolute_coordinate_t,
                  void invoke(display_context_t &context);)

UX_DECLARE_STORAGE_EMITTER(function_object_t, cairo_function_t,
                           void invoke(display_context_t &context);)

UX_DECLARE_CLASS_STORAGE_EMITTER(
    option_function_object_t, cairo_option_function_t,
    using cairo_option_function_t::cairo_option_function_t;)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(
    textual_render_t, textual_render_storage_t,
    using textual_render_storage_t::textual_render_storage_t;)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(
    image_block_t, image_block_storage_t,
    using image_block_storage_t::image_block_storage_t;)

UX_DECLARE_STORAGE_DRAWING_FUNCTION(draw_function_object_t, cairo_function_t,
                                    void invoke(display_context_t &context);)

// primitives - drawing functions
UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(arc_t, arc_storage_t,
                                          using arc_storage_t::arc_storage_t;)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(
    negative_arc_t, negative_arc_storage_t,
    using negative_arc_storage_t::negative_arc_storage_t;)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(
    curve_t, curve_storage_t, using curve_storage_t::curve_storage_t;)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(line_t, line_storage_t,
                                          using line_storage_t::line_storage_t;)

UX_DECLARE_STORAGE_DRAWING_FUNCTION(hline_t, double,
                                    void invoke(display_context_t &context);)

UX_DECLARE_STORAGE_DRAWING_FUNCTION(vline_t, double,
                                    void invoke(display_context_t &context);)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(
    rectangle_t, rectangle_storage_t,
    using rectangle_storage_t::rectangle_storage_t;)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(
    stroke_path_t, painter_brush_t, using painter_brush_t::painter_brush_t;)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(
    fill_path_t, painter_brush_t, using painter_brush_t::painter_brush_t;)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(
    stroke_fill_path_t, stroke_fill_path_storage_t,
    using stroke_fill_path_storage_t::stroke_fill_path_storage_t;)

UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(
    mask_t, painter_brush_t, using painter_brush_t::painter_brush_t;)

UX_DECLARE_STORAGE_DRAWING_FUNCTION(paint_t, double,
                                    void invoke(display_context_t &context);)

UX_DECLARE_MARKER(close_path_t, void invoke(display_context_t &context);)

// event listeners
UX_DECLARE_CLASS_STORAGE_EMITTER(listener_t, listener_storage_t,
                                 using listener_storage_t::listener_storage_t;)

UX_DECLARE_EVENT_LISTENER(listen_paint_t)
UX_DECLARE_EVENT_LISTENER(listen_focus_t)
UX_DECLARE_EVENT_LISTENER(listen_blur_t)
UX_DECLARE_EVENT_LISTENER(listen_resize_t)
UX_DECLARE_EVENT_LISTENER(listen_keydown_t)
UX_DECLARE_EVENT_LISTENER(listen_keyup_t)
UX_DECLARE_EVENT_LISTENER(listen_keypress_t)
UX_DECLARE_EVENT_LISTENER(listen_mouseenter_t)
UX_DECLARE_EVENT_LISTENER(listen_mousemove_t)
UX_DECLARE_EVENT_LISTENER(listen_mousedown_t)
UX_DECLARE_EVENT_LISTENER(listen_mouseup_t)
UX_DECLARE_EVENT_LISTENER(listen_click_t)
UX_DECLARE_EVENT_LISTENER(listen_dblclick_t)
UX_DECLARE_EVENT_LISTENER(listen_contextmenu_t)
UX_DECLARE_EVENT_LISTENER(listen_wheel_t)
UX_DECLARE_EVENT_LISTENER(listen_mouseleave_t)
