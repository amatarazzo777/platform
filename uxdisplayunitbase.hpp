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
#pragma once
/**
\author Anthony Matarazzo
\file uxdisplayunitbase.hpp
\date 5/12/20
\version 1.0
\details

*/
/**
\internal
\typedef indirect_index_display_unit_t as a base member, the key data member
uses this type. All classes can provide an indirect numerical or string key.
The [] operator exposes searching for objects.
 */
namespace uxdevice {
typedef std::variant<std::monostate, std::string, std::size_t>
    indirect_index_display_unit_t;
}

/**
\internal
\template specializes the std::hash<uxdevice::indirect_index_display_unit_t>
 * std structure for () operator hashing.
 */
template <> struct std::hash<uxdevice::indirect_index_display_unit_t> {
  std::size_t
  operator()(uxdevice::indirect_index_display_unit_t const &o) const noexcept {
    size_t value = o.index();
    uxdevice::hash_combine(value, std::type_index(typeid(o)).hash_code());
    if (auto pval = std::get_if<std::string>(&o))
      uxdevice::hash_combine(value, *pval);
    else if (auto pval = std::get_if<std::size_t>(&o))
      uxdevice::hash_combine(value, *pval);
    return value;
  }
};

/**
\internal

\class index_by_t

\brief A class which is inherited which manages the index key data.
The operators, copy, move constructors are invoked by the class deriving:
display_unit_t.

 */
namespace uxdevice {
class index_by_t {
public:
  /// @brief default constructor
  index_by_t() {}

  /// @brief constructor storing a string as key
  index_by_t(std::string _k) : key(_k) {}

  /// @brief constructor storing a size_t as key
  index_by_t(std::size_t _k) : key(_k) {}

  /// @brief copy assignment operator
  index_by_t &operator=(const index_by_t &other) {
    key = other.key;
    return *this;
  }
  /// @brief move assignment
  index_by_t &operator=(index_by_t &&other) noexcept {
    key = std::move(other.key);
    return *this;
  }
  /// @brief move constructor
  index_by_t(index_by_t &&other) noexcept { key = std::move(other.key); }

  /// @brief copy constructor
  index_by_t(const index_by_t &other) { *this = other; }

  // @brief virtual destructor, all objects must have a destructor
  virtual ~index_by_t() {}

  indirect_index_display_unit_t key = {};
};
} // namespace uxdevice

/// \typedef cairo_function_t
/// @brief holds a call to a cairo function with parameters bound.
/// The cairo context - cr is provided later.
namespace uxdevice {
typedef std::function<void(cairo_t *cr)> cairo_function_t;
using cairo_function_t = cairo_function_t;

} // namespace uxdevice
template <> struct std::hash<uxdevice::cairo_function_t> {
  std::size_t operator()(uxdevice::cairo_function_t const &o) const noexcept {
    return reinterpret_cast<std::size_t>(std::addressof(o));
  }
};

namespace uxdevice {
using cairo_option_function_t = class cairo_option_function_t {
public:
  cairo_option_function_t() {}
  cairo_option_function_t &operator=(const cairo_option_function_t &other) {
    value = other.value;
    return *this;
  }
  /// @brief move assignment
  cairo_option_function_t &operator=(cairo_option_function_t &&other) noexcept {
    value = std::move(other.value);
    return *this;
  }

  cairo_option_function_t(cairo_option_function_t &other) noexcept
      : value(other.value) {}

  cairo_option_function_t(cairo_option_function_t &&other) noexcept
      : value(std::move(other.value)) {}
  virtual ~cairo_option_function_t() {}
  virtual void invoke(display_context_t &context);
  virtual void emit(cairo_t *cr);
  UX_DECLARE_HASH_MEMBERS_INTERFACE

  std::list<option_function_object_t> value = {};
};
} // namespace uxdevice
  // namespace uxdevice

/**
\internal
\class display_unit_classification_t
\brief

\brief classifies the type of display unit. This can be used
to determine the functional aspects of how the rendering engine uses the
objects. */
namespace uxdevice {
enum class display_unit_classification_t {
  none,
  painter_brush,
  marker,
  storage_emitter,
  class_storage_emitter,
  storage_drawing_function,
  class_storage_drawing_function
};
}

/**
\internal
\class polymorphic_overloads_t

\brief base class for all display units. d

*/

namespace uxdevice {
class polymorphic_overloads_t {
public:
  virtual void invoke(display_context_t &context) {}
  virtual void emit(cairo_t *cr) {}
  virtual void emit(cairo_t *cr, const coordinate_t &a) {}
  virtual void emit(display_context_t &context) {}
  virtual void emit(cairo_t *cr, double x, double y, double w, double h) {}
  virtual void emit(PangoLayout *layout) {}
  virtual ~polymorphic_overloads_t() {}
};
} // namespace uxdevice

/**
\internal
\class display_unit_t
\brief

\brief base class for all display units. defaulted
is the is_output function. Drawing object should override
this and return true. This enables the checking of the surface
for errors after invocation.

*/
namespace uxdevice {
class display_unit_t : public index_by_t,
                       public polymorphic_overloads_t,
                       public std::enable_shared_from_this<display_unit_t> {
public:
  display_unit_t() {}

  /// @brief copy assignment operator
  display_unit_t &operator=(const display_unit_t &other) {
    index_by_t::operator=(other);
    is_processed = other.is_processed;
    viewport_inked = other.viewport_inked;
    error_description = other.error_description;
    return *this;
  }
  /// @brief move assignment
  display_unit_t &operator=(display_unit_t &&other) noexcept {
    index_by_t::operator=(other);
    is_processed = std::move(other.is_processed);
    viewport_inked = std::move(other.viewport_inked);
    error_description = std::move(other.error_description);
    return *this;
  }

  /// @brief move constructor
  display_unit_t(display_unit_t &&other) noexcept
      : index_by_t(other), is_processed(std::move(other.is_processed)),
        viewport_inked(std::move(other.viewport_inked)),
        error_description(std::move(other.error_description)) {}

  /// @brief copy constructor
  display_unit_t(const display_unit_t &other)
      : index_by_t(other), is_processed(other.is_processed),
        viewport_inked(other.viewport_inked),
        error_description(other.error_description) {}

  virtual ~display_unit_t() {}

  virtual bool is_output(void) { return false; }
  void error(const char *s) { error_description = s; }
  bool valid(void) { return error_description == nullptr; }
  void changed(void) { bchanged = true; }
  bool has_changed(void) { return bchanged; }

  bool is_processed = false;
  bool viewport_inked = false;
  bool bchanged = false;
  const char *error_description = nullptr;
};
} // namespace uxdevice

/**
\internal
\class drawing_output_t
\brief base class for objects that produce image_block_t drawing commands
The is_output is overridden to return true. As well the object uses
the render work list to determine if a particular image_block_t is on screen.

\details


 */
namespace uxdevice {
class drawing_output_t : public display_unit_t {
public:
  typedef display_context_t::context_cairo_region_t context_cairo_region_t;

  /// @brief default constructor
  drawing_output_t() : display_unit_t() {}

  /// @brief move assignment
  drawing_output_t &operator=(drawing_output_t &&other) noexcept {
    display_unit_t::operator=(other);

    internal_buffer = std::move(other.internal_buffer);

    fn_draw = std::move(other.fn_draw);
    fn_draw_clipped = std::move(other.fn_draw_clipped);
    fn_cache_surface = std::move(other.fn_cache_surface);
    fn_base_surface = std::move(other.fn_base_surface);

    options = std::move(other.options);
    ink_rectangle = std::move(other.ink_rectangle);
    intersection_int = std::move(other.intersection_int);
    intersection_double = std::move(other.intersection_double);
    return *this;
  }

  /// @brief copy assignment operator
  drawing_output_t &operator=(const drawing_output_t &other) {
    display_unit_t::operator=(other);

    if (other.internal_buffer.rendered)
      internal_buffer.rendered =
          cairo_surface_reference(other.internal_buffer.rendered);

    if (other.internal_buffer.cr)
      internal_buffer.cr = cairo_reference(other.internal_buffer.cr);

    fn_draw = other.fn_draw;
    fn_draw_clipped = other.fn_draw_clipped;
    fn_cache_surface = other.fn_cache_surface;
    fn_base_surface = other.fn_base_surface;

    std::copy(other.options.value.begin(), other.options.value.end(),
              std::back_inserter(options.value));

    ink_rectangle = other.ink_rectangle;
    intersection_int = other.intersection_int;
    intersection_double = other.intersection_double;

    return *this;
  }

  /// @brief move constructor
  drawing_output_t(drawing_output_t &&other) noexcept
      : display_unit_t(other),
        internal_buffer(std::move(other.internal_buffer)),

        fn_cache_surface(std::move(other.fn_cache_surface)),
        fn_base_surface(std::move(other.fn_base_surface)),
        fn_draw(std::move(other.fn_draw)),
        fn_draw_clipped(std::move(other.fn_draw_clipped)),

        options(std::move(other.options)),
        ink_rectangle(std::move(other.ink_rectangle)),
        intersection_int(std::move(other.intersection_int)),
        intersection_double(std::move(other.intersection_double)) {}

  /// @brief copy constructor
  drawing_output_t(const drawing_output_t &other) {
    // invoke copy operator
    *this = other;
  }

  virtual ~drawing_output_t() {
    display_context_t::destroy_buffer(internal_buffer);
  }

  void intersect(cairo_rectangle_t &r);
  void intersect(context_cairo_region_t &r);

  bool is_output(void) { return true; }

  // These functions switch the rendering apparatus from off
  // screen threaded to on screen. all rendering is serialize to the main
  // surface
  //
  std::atomic_flag lockFunctors = ATOMIC_FLAG_INIT;
#define LOCK_FUNCTORS_SPIN                                                     \
  while (lockFunctors.test_and_set(std::memory_order_acquire))

#define LOCK_FUNCTORS_CLEAR lockFunctors.clear(std::memory_order_release)

  void functors_lock(bool b) {
    if (b)
      LOCK_FUNCTORS_SPIN;
    else
      LOCK_FUNCTORS_CLEAR;
  }

  bool has_ink_extents = false;
  cairo_rectangle_int_t c = cairo_rectangle_int_t();
  cairo_region_overlap_t overlap = CAIRO_REGION_OVERLAP_OUT;

  std::atomic<bool> bRenderBufferCached = false;
  draw_buffer_t internal_buffer = {};

  draw_logic_t fn_cache_surface = draw_logic_t();
  draw_logic_t fn_base_surface = draw_logic_t();
  draw_logic_t fn_draw = draw_logic_t();
  draw_logic_t fn_draw_clipped = draw_logic_t();

  // measure processing time
  std::chrono::system_clock::time_point last_render_time = {};
  void evaluate_cache(display_context_t &context);
  bool first_time_rendered = true;
  cairo_option_function_t options = {};
  cairo_rectangle_int_t ink_rectangle = cairo_rectangle_int_t();
  cairo_rectangle_int_t intersection_int = cairo_rectangle_int_t();
  cairo_rectangle_t intersection_double = cairo_rectangle_t();
};
} // namespace uxdevice

/**
\internal
\def TYPED_INDEX_INTERFACE(CLASS_NAME)
\brief adds an index() method while returning
the reference to *this in continuation syntax. This provides
the capability to index in the same expression.
*/
#define TYPED_INDEX_IMPLEMENTATION(CLASS_NAME)                                 \
  CLASS_NAME &index(const std::string &_k) {                                   \
    key = _k;                                                                  \
    return *this;                                                              \
  }                                                                            \
  CLASS_NAME &index(const std::size_t &_k) {                                   \
    key = _k;                                                                  \
    return *this;                                                              \
  }

/**
\internal
\def UX_DECLARE_PAINTER_BRUSH(CLASS_NAME)
\brief creates a painter brush object that is also a display unit.
class inherits publicly display_unit_t and painter_brush_t
*/
#define UX_DECLARE_PAINTER_BRUSH(CLASS_NAME)                                   \
  namespace uxdevice {                                                         \
  using CLASS_NAME = class CLASS_NAME : public display_unit_t,                 \
        public painter_brush_t {                                               \
  public:                                                                      \
    using painter_brush_t::painter_brush_t;                                    \
    CLASS_NAME &operator=(const CLASS_NAME &other) {                           \
      painter_brush_t::operator=(other);                                       \
      display_unit_t::operator=(other);                                        \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME(const CLASS_NAME &other)                                        \
        : display_unit_t(other), painter_brush_t(other) {}                     \
    CLASS_NAME(CLASS_NAME &&other)                                             \
        : display_unit_t(other), painter_brush_t(other) {}                     \
    virtual void invoke(display_context_t &context);                           \
    virtual void emit(cairo_t *cr) {                                           \
      painter_brush_t::emit(cr);                                               \
      cairo_set_line_width(cr, lineWidth);                                     \
    }                                                                          \
                                                                               \
    virtual void emit(cairo_t *cr, const coordinate_t &a) {                    \
      painter_brush_t::emit(cr);                                               \
      cairo_set_line_width(cr, lineWidth);                                     \
    }                                                                          \
    UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, painter_brush_t::hash_code(), \
                           lineWidth, radius, x, y)                            \
    double lineWidth = 1;                                                      \
    unsigned short radius = 3;                                                 \
    double x = 1, y = 1;                                                       \
    const static display_unit_classification_t classification =                \
        display_unit_classification_t::painter_brush;                          \
    TYPED_INDEX_IMPLEMENTATION(CLASS_NAME)                                     \
  };                                                                           \
  }                                                                            \
  UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::CLASS_NAME);

/**
\internal
\def UX_DECLARE_MARKER

\param CLASS_NAME - the name the display unit should assume.

\param PUBLIC_OVERRIDES - this applicable invoke and emit functions. The base
class display_unit_t has base empty implementation versions of these virtual
functions. Depending on the particular display unit type, pango or cairo
emit functions may be useful.

\brief declares a class that marks a unit but does not store a value.
This is useful for switch and state logic. When the item is present, the
invoke method is called. class inherits publicly display_unit_t
*/
#define UX_DECLARE_MARKER(CLASS_NAME, PUBLIC_OVERRIDES)                        \
  namespace uxdevice {                                                         \
  using CLASS_NAME = class CLASS_NAME : public display_unit_t {                \
  public:                                                                      \
    CLASS_NAME() {}                                                            \
                                                                               \
    CLASS_NAME &operator=(CLASS_NAME &&other) noexcept {                       \
      display_unit_t::operator=(other);                                        \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    CLASS_NAME &operator=(const CLASS_NAME &other) {                           \
      display_unit_t::operator=(other);                                        \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    CLASS_NAME(CLASS_NAME &&other) noexcept : display_unit_t(other) {}         \
    CLASS_NAME(const CLASS_NAME &other) : display_unit_t(other) {}             \
                                                                               \
    virtual ~CLASS_NAME() {}                                                   \
                                                                               \
    PUBLIC_OVERRIDES                                                           \
                                                                               \
    TYPED_INDEX_IMPLEMENTATION(CLASS_NAME)                                     \
    UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS)                               \
    const static display_unit_classification_t classification =                \
        display_unit_classification_t::marker;                                 \
  };                                                                           \
  }                                                                            \
  UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::CLASS_NAME);

/**
\internal

\def UX_DECLARE_STORAGE_EMITTER

\param CLASS_NAME - the name the display unit should assume.

\param STORAGE_TYPE - the storage class or trivial type.

\param PUBLIC_OVERRIDES - this applicable invoke and emit functions. The base
class display_unit_t has base empty implementation versions of these virtual
functions. Depending on the particular display unit type, pango or cairo
emit functions may be useful.

\brief provides the flexibility to store associated data. the invoke
public method is called. The class has a public member named "value" of the
given type within the second macro parameter.

*/

#define UX_DECLARE_STORAGE_EMITTER(CLASS_NAME, STORAGE_TYPE, PUBLIC_OVERRIDES) \
  namespace uxdevice {                                                         \
  using CLASS_NAME = class CLASS_NAME : public display_unit_t {                \
  public:                                                                      \
    CLASS_NAME() : value(STORAGE_TYPE{}) {}                                    \
    CLASS_NAME(const STORAGE_TYPE &o) : value(o) {}                            \
    CLASS_NAME &operator=(const CLASS_NAME &other) {                           \
      display_unit_t::operator=(other);                                        \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME &operator=(CLASS_NAME &&other) noexcept {                       \
      index_by_t::operator=(other);                                            \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME(const CLASS_NAME &other)                                        \
        : display_unit_t(other), value(other.value) {}                         \
    CLASS_NAME(CLASS_NAME &&other)                                             \
    noexcept : display_unit_t(other), value(other.value) {}                    \
                                                                               \
    virtual ~CLASS_NAME() {}                                                   \
                                                                               \
    PUBLIC_OVERRIDES                                                           \
                                                                               \
    TYPED_INDEX_IMPLEMENTATION(CLASS_NAME)                                     \
    UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, value)                        \
                                                                               \
    STORAGE_TYPE value;                                                        \
    const static display_unit_classification_t classification =                \
        display_unit_classification_t::storage_emitter;                        \
  };                                                                           \
  }

/**
\internal
\def UX_DECLARE_CLASS_STORAGE_EMITTER

\param CLASS_NAME - the name the display unit should assume.

\param STORAGE_TYPE - the storage class type. The storage type is publicly
inherited may public

\param CONSTRUCTOR_INHEIRTANCE - inherits the data class interface as public.
  There constructors are inherited as well as the public methods of the class.

\brief provides the flexibility to store associated data. the invoke
public method is called. The class has a public member named "value" of the
given type within the second macro parameter.


The PUBLIC_OVERRIDES parameter is left off as the data storage class
should implement the necessary invoke() and emit() public members. Of particular
interest is that the class storage emitters and class drawing emitters are
befriended using the friend class. The base class and derived class data members
are accessible from the data storage class. Importantly this allows the
drawing_output_t data members to be manipulated for the objects drawing
capability.

The drawing_output_t object has std::function member variables that should be
populated as part of the data classes invocation. These functions provide
rendering functionality as called by the rendering loop.

*/
#define UX_DECLARE_CLASS_STORAGE_EMITTER(CLASS_NAME, STORAGE_TYPE,             \
                                         CONSTRUCTOR_INHEIRTANCE)              \
  namespace uxdevice {                                                         \
  using CLASS_NAME = class CLASS_NAME : public display_unit_t,                 \
        public STORAGE_TYPE {                                                  \
  public:                                                                      \
    CONSTRUCTOR_INHEIRTANCE                                                    \
    CLASS_NAME() : STORAGE_TYPE() {}                                           \
    CLASS_NAME &operator=(const CLASS_NAME &other) {                           \
      display_unit_t::operator=(other);                                        \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME &operator=(CLASS_NAME &&other) noexcept {                       \
      index_by_t::operator=(other);                                            \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME(const CLASS_NAME &other) : display_unit_t(other) {}             \
    CLASS_NAME(CLASS_NAME &&other) noexcept : display_unit_t(other) {}         \
                                                                               \
    virtual ~CLASS_NAME() {}                                                   \
                                                                               \
    TYPED_INDEX_IMPLEMENTATION(CLASS_NAME)                                     \
    const static display_unit_classification_t classification =                \
        display_unit_classification_t::class_storage_emitter;                  \
  };                                                                           \
  }                                                                            \
  UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::CLASS_NAME);

/**
\internal
\def UX_DECLARE_STORAGE_DRAWING_FUNCTION

\param CLASS_NAME - the name the display unit should assume.
\param STORAGE_TYPE - the storage class or trivial type.
\param PUBLIC_OVERRIDES - this applicable invoke and emit functions. The base
class display_unit_t has base empty implementation versions of these virtual
functions. Depending on the particular display unit type, pango or cairo
emit functions may be useful.

\brief provides the flexibility to store associated data from a class. Classes
should be created for complex items that require a storage_type
The invoke
public method is called when on screen. The class has a public member named
"value" of the given type within the second macro parameter.

*/
#define UX_DECLARE_STORAGE_DRAWING_FUNCTION(CLASS_NAME, STORAGE_TYPE,          \
                                            PUBLIC_OVERRIDES)                  \
  namespace uxdevice {                                                         \
  using CLASS_NAME = class CLASS_NAME : public drawing_output_t {              \
  public:                                                                      \
    CLASS_NAME() : value(STORAGE_TYPE{}) {}                                    \
    CLASS_NAME(const STORAGE_TYPE &o) : value(o) {}                            \
    CLASS_NAME &operator=(const CLASS_NAME &other) {                           \
      drawing_output_t::operator=(other);                                      \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME &operator=(CLASS_NAME &&other) noexcept {                       \
      index_by_t::operator=(other);                                            \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME(const CLASS_NAME &other)                                        \
        : drawing_output_t(other), value(other.value) {}                       \
    CLASS_NAME(CLASS_NAME &&other)                                             \
    noexcept : drawing_output_t(other), value(other.value) {}                  \
                                                                               \
    virtual ~CLASS_NAME() {}                                                   \
                                                                               \
    PUBLIC_OVERRIDES                                                           \
    TYPED_INDEX_IMPLEMENTATION(CLASS_NAME)                                     \
    UX_HASH_OBJECT_MEMBERS(UX_HASH_TYPE_ID_THIS, value)                        \
                                                                               \
    STORAGE_TYPE value;                                                        \
    const static display_unit_classification_t classification =                \
        display_unit_classification_t::storage_drawing_function;               \
  };                                                                           \
  }

/**
\internal
\def UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION

\param CLASS_NAME - the name the display unit should assume.
\param STORAGE_TYPE - the storage class or trivial type.
\param CONSTRUCTOR_INHEIRTANCE - the data interface is inherited and exposed as
well as the constructors.


\brief Provides the ability to store an object accepting a specific parameter
api. These objects are called to render functions and also have
a dimension. They occupy a bounds as defined by inkRectangle. They can be
established as on or off view_port by intersection test with the
view_port rectangle.


*/
#define UX_DECLARE_CLASS_STORAGE_DRAWING_FUNCTION(CLASS_NAME, STORAGE_TYPE,    \
                                                  CONSTRUCTOR_INHEIRTANCE)     \
  namespace uxdevice {                                                         \
  using CLASS_NAME = class CLASS_NAME : public drawing_output_t,               \
        public STORAGE_TYPE {                                                  \
  public:                                                                      \
    CONSTRUCTOR_INHEIRTANCE                                                    \
    CLASS_NAME &operator=(const CLASS_NAME &other) {                           \
      drawing_output_t::operator=(other);                                      \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME &operator=(CLASS_NAME &&other) noexcept {                       \
      index_by_t::operator=(other);                                            \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME(const CLASS_NAME &other) : drawing_output_t(other) {}           \
    CLASS_NAME(CLASS_NAME &&other) noexcept : drawing_output_t(other) {}       \
                                                                               \
    virtual ~CLASS_NAME() {}                                                   \
                                                                               \
    TYPED_INDEX_IMPLEMENTATION(CLASS_NAME)                                     \
    const static display_unit_classification_t classification =                \
        display_unit_classification_t::class_storage_drawing_function;         \
  };                                                                           \
  }                                                                            \
  UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::CLASS_NAME);

/**
\internal
\def UX_DECLARE_EVENT_LISTENER(CLASS_NAME)
\brief the macro creates a named listener which inherits
the listener_t interface publicly. The type_info is stored within
the based class.
*/
#define UX_DECLARE_EVENT_LISTENER(CLASS_NAME)                                  \
  namespace uxdevice {                                                         \
  using CLASS_NAME = class CLASS_NAME : public listener_t {                    \
  public:                                                                      \
    CLASS_NAME(const event_handler_t &_dispatch_event)                         \
        : listener_t(std::type_index(typeid(this)), _dispatch_event) {}        \
    CLASS_NAME &operator=(const CLASS_NAME &other) {                           \
      listener_t::operator=(other);                                            \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME &operator=(CLASS_NAME &&other) noexcept {                       \
      listener_t::operator=(other);                                            \
      return *this;                                                            \
    }                                                                          \
    CLASS_NAME(const CLASS_NAME &other) : listener_t(other) {}                 \
    CLASS_NAME(CLASS_NAME &&other) noexcept : listener_t(other) {}             \
                                                                               \
    virtual ~CLASS_NAME() {}                                                   \
                                                                               \
    UX_HASH_OBJECT_MEMBERS(listener_t::hash_code(), UX_HASH_TYPE_ID_THIS)      \
    TYPED_INDEX_IMPLEMENTATION(CLASS_NAME)                                     \
  };                                                                           \
  }                                                                            \
  UX_REGISTER_STD_HASH_SPECIALIZATION(uxdevice::CLASS_NAME);
