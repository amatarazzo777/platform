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
\file uxdevice.cpp

\author Anthony Matarazzo

\date 3/26/20
\version 1.0

\brief rendering and platform services.

*/
#include "uxdevice.hpp"

using namespace std;
using namespace uxdevice;

/**
\internal
\fn render_loop(void)
\brief The routine is the main rendering thread. The thread runs
when necessary based upon a condition variable.
Locks are placed on the surface and
rectangle list. The surface may change due to user resizing the gui
window so a spin flag is used to accommodate the functionality. That is
drawing cannot occur on the graphical while the surface is being resized.
*/
void uxdevice::surface_area_t::render_loop(void) {
  while (bProcessing) {

    // surfacePrime checks to see if the surface exists.
    // if so, the two possible work flows are painting
    // background rectangles that are cause by the user resizing the
    // window to a greater value. These values are inserted as part
    // of the paint event. As well, the underlying surface may
    // need to be resized. This function acquires locks on these
    // small lists for the multi-threaded necessity.
    // searc_thes for unready and syncs display context
    // if no work exists  it waits on the cvRenderWork condition variable.
    if (context.surface_prime()) {
      context.render();
    }

    if (context.error_state())
      fnError(context.error_text());
  }
}

/**
\internal
\fn dispatch_event(const event_t &evt)
\brief the dispatch routine is invoked by the messageLoop.
If default
 * handling is to be supplied, the method invokes the
necessary operation.

*/
void uxdevice::surface_area_t::dispatch_event(const event_t &evt) {

  if (evt.type == std::type_index(typeid(listen_paint_t)))
    context.state_surface(evt.x, evt.y, evt.w, evt.h);
  else if (evt.type == std::type_index(typeid(listen_resize_t)))
    context.resize_surface(evt.w, evt.h);

  if (fnEvents)
    fnEvents(evt);
}
/**
\internal
\brief The entry point that processes messages from the operating
system application level services. Typically on Linux this is a
coupling of xcb and keysyms library for keyboard. Previous
incarnations of technology such as this typically used xserver.
However, XCB is the newer form. Primarily looking at the code of such
programs as vlc, the routine simply places pixels into the memory
buffer. while on windows the direct x library is used in combination
with windows message queue processing.
*/
void uxdevice::surface_area_t::start_processing(void) {
  // setup the event dispatcher
  event_handler_t ev = std::bind(&uxdevice::surface_area_t::dispatch_event,
                                 this, std::placeholders::_1);
  context.cache_threshold = 2000;
  std::thread thrRenderer([=]() {
    bProcessing = true;
    render_loop();
  });

  std::thread thrMessageQueue([=]() {
    bProcessing = true;
    message_loop();
  });

  thrRenderer.detach();
  thrMessageQueue.detach();
}

/**
\fn get_event_vector(eventType evtType)
\internal

\brief The function maps the event id to the appropriate vector.
This is kept statically here for retext_color_t management.

\param eventType evtType
*/
std::list<event_handler_t> &
uxdevice::surface_area_t::get_event_vector(std::type_index evtType) {
  static std::unordered_map<std::type_index, std::list<event_handler_t> &>
      eventTypeMap = {
          {std::type_index(typeid(listen_focus_t)), onfocus},
          {std::type_index(typeid(listen_blur_t)), onblur},
          {std::type_index(typeid(listen_resize_t)), onresize},
          {std::type_index(typeid(listen_keydown_t)), onkeydown},
          {std::type_index(typeid(listen_keyup_t)), onkeyup},
          {std::type_index(typeid(listen_keypress_t)), onkeypress},
          {std::type_index(typeid(listen_mouseenter_t)), onmouseenter},
          {std::type_index(typeid(listen_mouseleave_t)), onmouseleave},
          {std::type_index(typeid(listen_mousemove_t)), onmousemove},
          {std::type_index(typeid(listen_mousedown_t)), onmousedown},
          {std::type_index(typeid(listen_mouseup_t)), onmouseup},
          {std::type_index(typeid(listen_click_t)), onclick},
          {std::type_index(typeid(listen_dblclick_t)), ondblclick},
          {std::type_index(typeid(listen_contextmenu_t)), oncontextmenu},
          {std::type_index(typeid(listen_wheel_t)), onwheel}};
  auto it = eventTypeMap.find(evtType);
  return it->second;
}

#if 0
/**

\brief The function is invoked when an event occurs. Normally this occurs
from the platform device. However, this may be invoked by the soft
generation of events.

*/
void uxdevice::surface_area_t::dispatch(const event &e) {
  auto &v = getEventVector(e.evtType);
  for (auto &fn : v)
    fn(e);
}
#endif

/**
\overload
\fn surface_area_t() default window constructor.


\brief Opens a default window approximately 60% of window view area
with the program name according to cpp macros.

*/
uxdevice::surface_area_t::surface_area_t() {
  open_window(coordinate_list_t{}, DEFAULT_WINDOW_TITLE, painter_brush_t{},
              event_handler_t{});
}

/**
\overload
\fn surface_area_t() default window constructor.
\param const std::string &surface_area_title

\brief Opens a default window approximately 60% of window view area
with the title given. Default background, without an external window
event handler.

*/
uxdevice::surface_area_t::surface_area_t(
    const std::string &surface_area_title) {
  open_window(coordinate_list_t{}, surface_area_title, painter_brush_t{},
              event_handler_t{});

  set_surface_defaults();
}

/**
\overload
\fn surface_area_t() default window constructor.
\param const coordinate_list_t &coordinate_t - coordinate and
   dimensions of the window.

\brief Opens a default window approximately 60% of window view area
with the title given. Default background, without an external window
event handler.

*/
uxdevice::surface_area_t::surface_area_t(const coordinate_list_t &coordinate) {
  open_window(coordinate, DEFAULT_WINDOW_TITLE, painter_brush_t{},
              event_handler_t{});
}
/**
\overload
\fn surface_area_t() default window constructor.
\param const std::string &surface_area_title

\brief Opens a default window approximately 60% of window view area
with the title given. Default background, without an external window
event handler.

*/
uxdevice::surface_area_t::surface_area_t(
    const event_handler_t &dispatch_events) {
  open_window(coordinate_list_t{}, DEFAULT_WINDOW_TITLE, painter_brush_t{},
              dispatch_events);
}

/**
\overload
\fn surface_area_t() default window constructor.
\param const std::string &surface_area_title

\brief Opens a default window approximately 60% of window view area
with the title given. Default background, without an external window
event handler.

*/
uxdevice::surface_area_t::surface_area_t(
    const coordinate_list_t &coordinate,
    const std::string &surface_area_title) {
  open_window(coordinate, surface_area_title, painter_brush_t{},
              event_handler_t{});
}

/**
\overload
\fn surface_area_t() default window constructor.

\param const coordinate_list_t &coordinate
\param const std::string &window_title
\param const painter_brush_t &background

\brief Opens a default window approximately 60% of window view area
with the title given. Default background, without an external window
event handler.

*/
uxdevice::surface_area_t::surface_area_t(
    const coordinate_list_t &coordinate, const std::string &surface_area_title,
    const painter_brush_t &surface_background_brush) {
  open_window(coordinate, surface_area_title, surface_background_brush,
              event_handler_t{});
}

/**
\overload
\fn surface_area_t() default window constructor.

\param const coordinate_list_t &coordinate
\param const std::string &window_title
\param const event_handler_t &dispatch_events
\param const painter_brush_t &surface_background_brush

\brief Opens a window the size and position of the coordinate provided.
The given title is used as the window title.  The background parameter serves
as the background surface brush, When the window is cleared, this is the
texture,color or gradient used. The window events such as focus, mouse,
resize and paint events may be listened to by the event dispatcher. The
 event_handler_t type is used as the std::function storage type .

*/
uxdevice::surface_area_t::surface_area_t(
    const coordinate_list_t &coordinate, const std::string &window_title,
    const painter_brush_t &surface_background_brush,
    const event_handler_t &dispatch_events) {
  open_window(coordinate, window_title, surface_background_brush,
              dispatch_events);
}

/**
  \internal
  \brief Destructor, closes a window on the target OS


*/
uxdevice::surface_area_t::~surface_area_t(void) { close_window(); }

/**
  \internal
  \brief sets the defaults for the context. font, colors, etc.
*/
void surface_area_t::set_surface_defaults(void) { SYSTEM_DEFAULTS }

/**
\brief API interface, just data is passed to objects. Objects are
dynamically allocated as classes derived from a unit base. Mutex is used one
display list to not get in the way of the rendering loop,

*/

/**
\fn clear(void)

\brief clears the display list and the context. However the brush
is not cleared.  The display and all objects are released.
*/

void uxdevice::surface_area_t::clear(void) {
  context.clear();
  display_list_clear();
}

/**
\fn notify_complete(void)

\brief An essential and very important function that releases the
wait state within the renderer. Without calling this function,
no paint will occur unless the timeout is met. The timeout
is not added yet. Essentially this increases the through put
capabilities to top computer speed. This can show system problems
and deadlocks much easier and allow analysis of performance issues
or more informative cpu usage since the data perception is changed.

*/
void uxdevice::surface_area_t::notify_complete(void) {
  context.state_notify_complete();
}

/**
\brief called by each of thedisplay unit objects to index the item if a key
exists. A key can be given as a text_data_t or an integer. The [] operator is
used to access the data.
*/
void uxdevice::surface_area_t::maintain_index(
    const std::shared_ptr<display_unit_t> obj) {
  if (!std::holds_alternative<std::monostate>(obj->key))
    mapped_objects[obj->key] = obj;
  return;
}

/**
\internal
\fn stream input
\overload
\param const std::string &s

\brief A stream interface routine that is declared using the
UX_DECLARE_STREAM_INTERFACE macro within the device published development API.
uxdevice.hpp is where is interface is declared.

The routine is specialized because it creates a textual_rendering_t object
that accepts the textual data. Textual data is stored in a separate object.
The textual_rendering_t object encapsulates the pango cairo api functions
which is also added.

*/
surface_area_t &uxdevice::surface_area_t::stream_input(const std::string &s) {
  auto item = display_list<text_data_t>(s);
  auto textrender = display_list<textual_render_t>();
  return *this;
}

/**
\internal
\fn stream input
\overload

\param const std::shared_ptr<std::string> _val

\brief An overloaded stream interface implemetatione that is declared using
the UX_DECLARE_STREAM_INTERFACE macro inside the uxdevice::surface_area_t class.

\details The routine is specialized because it creates
a textual_rendering_t object that accepts the textual data. Textual data is
stored in a separate object. The textual_rendering_t object encapsulates the
pango cairo api functions.

*/
//////////////////////////////////////////logic bugs, where left off
surface_area_t &uxdevice::surface_area_t::stream_input(
    const std::shared_ptr<std::string> _val) {
  auto item = display_list<text_data_t>(*_val);
  item->key = reinterpret_cast<std::size_t>(_val.get());
  auto textrender = display_list<textual_render_t>();
  return *this;
}

/**
\internal
\fn stream input
\overload

\param const std::stringstream &_val

\brief An overloaded stream interface implemetatione that is declared using
the UX_DECLARE_STREAM_INTERFACE macro inside the uxdevice::surface_area_t class.
The macro only declares the interface prototypes. The implementation for the
specific type is below.

\details The routine is specialized because it creates
a textual_rendering_t object that accepts the textual data. Textual data is
stored in a separate object. The textual_rendering_t object encapsulates the
pango cairo api functions.

*/
surface_area_t &
uxdevice::surface_area_t::stream_input(const std::stringstream &_val) {
  auto item = display_list<text_data_t>(_val.str());
  auto textrender = display_list<textual_render_t>();
  return *this;
}

/**

\fn save

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::save(void) {
  using namespace std::placeholders;
  display_list<function_object_t>(std::bind(cairo_save, _1));
  return *this;
}

/**

\fn restore

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::restore(void) {
  using namespace std::placeholders;
  display_list<function_object_t>(std::bind(cairo_restore, _1));
  return *this;
}

/**

\fn push
\param content_options_t c

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::push(content_options_t c) {
  using namespace std::placeholders;

  if (c == content_options_t::all)
    display_list<function_object_t>(std::bind(cairo_push_group, _1));
  else
    display_list<function_object_t>(std::bind(cairo_push_group_with_content, _1,
                                              static_cast<cairo_content_t>(c)));

  return *this;
}

/**

\fn pop
\param bool bToSource

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::pop(bool bToSource) {
  using namespace std::placeholders;

  if (bToSource)
    display_list<function_object_t>(std::bind(cairo_pop_group_to_source, _1));
  else
    display_list<function_object_t>(std::bind(cairo_pop_group, _1));

  return *this;
}

/**

\fn translate
\param painter_brush_t &b
\param double x
\param  double y
\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::translate(double x, double y) {
  using namespace std::placeholders;
  display_list<function_object_t>(std::bind(cairo_translate, _1, x, y));
  return *this;
}

/**

\fn rotate
\param  double angle

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::rotate(double angle) {
  using namespace std::placeholders;
  display_list<function_object_t>(std::bind(cairo_rotate, _1, angle));
  return *this;
}

/**

\fn device_offset
\param  double x
\param  double y
\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::device_offset(double x, double y) {
  context.device_offset(x, y);
  return *this;
}

/**

\fn device_scale
\param  double x
\param  double y
\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::device_scale(double x, double y) {
  context.device_scale(x, y);
  return *this;
}

/**

\fn scale
\param  double x
\param  double y
\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::scale(double x, double y) {
  using namespace std::placeholders;
  display_list<function_object_t>(std::bind(cairo_scale, _1, x, y));
  return *this;
}

/**

\fn transform
\param const Matrix &m

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::transform(const Matrix &m) {
  using namespace std::placeholders;
  display_list<function_object_t>(std::bind(cairo_transform, _1, &m._matrix));
  return *this;
}

/**

\fn matrix
\param const Matrix &m

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::matrix(const Matrix &m) {
  using namespace std::placeholders;
  display_list<function_object_t>(std::bind(cairo_set_matrix, _1, &m._matrix));
  return *this;
}

/**

\fn identity

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::identity(void) {
  using namespace std::placeholders;
  display_list<function_object_t>(std::bind(cairo_identity_matrix, _1));
  return *this;
}

/**

\fn device
\param double &x
\param  double &y

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::device(double &x, double &y) {
  using namespace std::placeholders;

  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x;
    double _y = y;
    cairo_user_to_device(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  display_list<function_object_t>(std::bind(fn, _1, x, y));

  return *this;
}

/**

\fn device_distance
\param double &x
\param  double &y

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::device_distance(double &x,
                                                          double &y) {
  using namespace std::placeholders;

  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x;
    double _y = y;
    cairo_user_to_device_distance(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  display_list<function_object_t>(std::bind(fn, _1, x, y));

  return *this;
}

/**

\fn user
\param double &x
\param  double &y

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::user(double &x, double &y) {
  using namespace std::placeholders;

  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x, _y = y;
    cairo_device_to_user(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  display_list<function_object_t>(std::bind(fn, _1, x, y));

  return *this;
}

/**

\fn user_distance
\param double &x
\param  double &y

\brief

\details


 */
surface_area_t &uxdevice::surface_area_t::user_distance(double &x, double &y) {
  using namespace std::placeholders;

  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x, _y = y;
    cairo_device_to_user_distance(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  display_list<function_object_t>(std::bind(fn, _1, x, y));

  return *this;
}

/**

\fn draw_caret
\param const int x
\param const int y,
\param  const int h
\param painter_brush_t &b

\brief

\details


 */
void uxdevice::surface_area_t::draw_caret(const int x, const int y,
                                          const int h) {}

std::string _errorReport(std::string text_color_tFile, int ln,
                         std::string sfunc, std::string cond,
                         std::string ecode) {
  std::stringstream ss;
  ss << text_color_tFile << "(" << ln << ") " << sfunc << "  " << cond << ecode;
  return ss.str();
}

/**
  \internal
  \brief opens a window on the target OS. used by all of the constuctors.
  parameters may be nulled or defaulted.


*/
void uxdevice::surface_area_t::open_window(
    const coordinate_list_t &coord, const std::string &sWindowTitle,
    const painter_brush_t &background, const event_handler_t &dispatch_events) {
  surface_area_t ret;
  auto it = coord.begin();

  context.window_width = *it;
  it++;
  context.window_height = *it;
  context.brush = background;

  // this open provides interoperability between xcb and xwindows
  // this is used here because of the necessity of key mapping.
  context.xdisplay = XOpenDisplay(nullptr);
  if (!context.xdisplay) {
    close_window();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  /* get the connection to the X server */
  context.connection = XGetXCBConnection(context.xdisplay);
  if (!context.xdisplay) {
    close_window();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  /* Get the first screen */
  context.screen =
      xcb_setup_roots_iterator(xcb_get_setup(context.connection)).data;
  if (!context.screen) {
    close_window();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  context.syms = xcb_key_symbols_alloc(context.connection);
  if (!context.syms) {
    close_window();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  /* Create black (foreground) graphic context */
  context.window = context.screen->root;
  context.graphics = xcb_generate_id(context.connection);
  uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  uint32_t values[] = {context.screen->black_pixel, 0};
  xcb_create_gc(context.connection, context.graphics, context.window, mask,
                values);

  if (!context.graphics) {
    close_window();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  /* Create a window */
  context.window = xcb_generate_id(context.connection);
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  mask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
  mask = XCB_CW_BORDER_PIXEL | XCB_CW_BIT_GRAVITY | XCB_CW_OVERRIDE_REDIRECT |
         XCB_CW_SAVE_UNDER | XCB_CW_EVENT_MASK;

  uint32_t vals[] = {
      context.screen->black_pixel, XCB_GRAVITY_NORTH_WEST, 0, 1,
      XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS |
          XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
          XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_BUTTON_PRESS |
          XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_STRUCTURE_NOTIFY};

  xcb_create_window(context.connection, XCB_COPY_FROM_PARENT, context.window,
                    context.screen->root, 0, 0,
                    static_cast<unsigned short>(context.window_width),
                    static_cast<unsigned short>(context.window_height), 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, context.screen->root_visual,
                    mask, vals);
  // set window title
  xcb_change_property(context.connection, XCB_PROP_MODE_REPLACE, context.window,
                      XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, sWindowTitle.size(),
                      sWindowTitle.data());

  if (!context.window) {
    close_window();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  /* you init the connection and screen_nbr */
  xcb_depth_iterator_t depth_iter;

  depth_iter = xcb_screen_allowed_depths_iterator(context.screen);
  for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
    xcb_visualtype_iterator_t visual_iter;

    visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
    for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
      if (context.screen->root_visual == visual_iter.data->visual_id) {
        context.visualType = visual_iter.data;
        break;
      }
    }
  }

  // create xcb surface

  context.xcbSurface = cairo_xcb_surface_create(
      context.connection, context.window, context.visualType,
      context.window_width, context.window_height);
  if (!context.xcbSurface) {
    close_window();
    std::stringstream sError;
    sError << "ERR_CAIRO "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  // create cairo context
  context.cr = cairo_create(context.xcbSurface);
  if (!context.cr) {
    close_window();
    std::stringstream sError;
    sError << "ERR_CAIRO "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  /* Map the window on the screen and flush*/
  xcb_map_window(context.connection, context.window);
  xcb_flush(context.connection);
  context.window_open = true;

  cairo_surface_flush(context.xcbSurface);
  start_processing();

  return;
}
/**
  \internal
  \brief closes a window on the target OS


*/
void uxdevice::surface_area_t::close_window(void) {

  if (context.xcbSurface) {
    cairo_surface_destroy(context.xcbSurface);
    context.xcbSurface = nullptr;
  }

  if (context.cr) {
    cairo_destroy(context.cr);
    context.cr = nullptr;
  }

  if (context.graphics) {
    xcb_free_gc(context.connection, context.graphics);
    context.graphics = 0;
  }

  if (context.syms) {
    xcb_key_symbols_free(context.syms);
    context.syms = nullptr;
  }

  if (context.window) {
    xcb_destroy_window(context.connection, context.window);
    context.window = 0;
  }

  if (context.xdisplay) {
    XCloseDisplay(context.xdisplay);
    context.xdisplay = nullptr;
  }

  context.window_open = false;
}

/**
\internal
\brief the routine handles the message processing for the specific
operating system. The function is called from processEvents.

*/
void uxdevice::surface_area_t::message_loop(void) {
  xcb_generic_event_t *xcbEvent;

  // is window open?
  while (bProcessing && !context.connection)
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
  if (!context.connection)
    return;

  // setup close window event
  xcb_intern_atom_cookie_t cookie =
      xcb_intern_atom(context.connection, 1, 12, "WM_PROTOCOLS");
  xcb_intern_atom_reply_t *reply =
      xcb_intern_atom_reply(context.connection, cookie, 0);

  xcb_intern_atom_cookie_t cookie2 =
      xcb_intern_atom(context.connection, 0, 16, "WM_DELETE_WINDOW");
  xcb_intern_atom_reply_t *reply2 =
      xcb_intern_atom_reply(context.connection, cookie2, 0);

  xcb_change_property(context.connection, XCB_PROP_MODE_REPLACE, context.window,
                      (*reply).atom, 4, 32, 1, &(*reply2).atom);

  // process Message queue
  std::list<xcb_generic_event_t *> xcbEvents;
  bool bvideo_output = false;
  while (bProcessing && (xcbEvent = xcb_wait_for_event(context.connection))) {
    xcbEvents.emplace_back(xcbEvent);
    bvideo_output = false;
    // qt5 does this, it queues all of the input messages at once.
    // this makes the processing of painting and reading input faster.
    while (bProcessing &&
           (xcbEvent = xcb_poll_for_queued_event(context.connection)))
      xcbEvents.emplace_back(xcbEvent);

    while (!xcbEvents.empty()) {
      xcbEvent = xcbEvents.front();
      switch (xcbEvent->response_type & ~0x80) {
      case XCB_MOTION_NOTIFY: {
        xcb_motion_notify_event_t *motion =
            (xcb_motion_notify_event_t *)xcbEvent;
        dispatch_event(event_t{
            std::type_index{typeid(listen_mousemove_t)},
            (short)motion->event_x,
            (short)motion->event_y,
        });
      } break;
      case XCB_BUTTON_PRESS: {
        xcb_button_press_event_t *bp = (xcb_button_press_event_t *)xcbEvent;
        if (bp->detail == XCB_BUTTON_INDEX_4 ||
            bp->detail == XCB_BUTTON_INDEX_5) {
          dispatch_event(
              event_t{std::type_index{typeid(listen_wheel_t)},
                      (short)bp->event_x, (short)bp->event_y,
                      (short)(bp->detail == XCB_BUTTON_INDEX_4 ? 1 : -1)});

        } else {

          dispatch_event(event_t{std::type_index{typeid(listen_mousedown_t)},
                                 (short)bp->event_x, (short)bp->event_y,
                                 (short)bp->detail});
        }
      } break;
      case XCB_BUTTON_RELEASE: {
        xcb_button_release_event_t *br = (xcb_button_release_event_t *)xcbEvent;
        // ignore button 4 and 5 which are wheel events.
        if (br->detail != XCB_BUTTON_INDEX_4 &&
            br->detail != XCB_BUTTON_INDEX_5)
          dispatch_event(event_t{std::type_index{typeid(listen_mouseup_t)},
                                 (short)br->event_x, (short)br->event_y,
                                 (short)br->detail});
      } break;
      case XCB_KEY_PRESS: {
        xcb_key_press_event_t *kp = (xcb_key_press_event_t *)xcbEvent;
        xcb_keysym_t sym = xcb_key_press_lookup_keysym(context.syms, kp, 0);
        if (sym < 0x99) {
          XKeyEvent keyEvent;
          keyEvent.display = context.xdisplay;
          keyEvent.keycode = kp->detail;
          keyEvent.state = kp->state;
          keyEvent.root = kp->root;
          keyEvent.time = kp->time;
          keyEvent.window = kp->event;
          keyEvent.serial = kp->sequence;

          std::array<char, 316> buf{};
          if (XLookupString(&keyEvent, buf.data(), buf.size(), nullptr,
                            nullptr))
            dispatch_event(event_t{std::type_index{typeid(listen_keypress_t)},
                                   (char)buf[0]});
        } else {
          dispatch_event(
              event_t{std::type_index{typeid(listen_keydown_t)}, sym});
        }
      } break;
      case XCB_KEY_RELEASE: {
        xcb_key_release_event_t *kr = (xcb_key_release_event_t *)xcbEvent;
        xcb_keysym_t sym = xcb_key_press_lookup_keysym(context.syms, kr, 0);
        dispatch_event(event_t{std::type_index{typeid(listen_keyup_t)}});
      } break;
      case XCB_EXPOSE: {
        xcb_expose_event_t *eev = (xcb_expose_event_t *)xcbEvent;

        dispatch_event(event_t{std::type_index{typeid(listen_paint_t)},
                               (short)eev->x, (short)eev->y, (short)eev->width,
                               (short)eev->height});
        bvideo_output = true;
      } break;
      case XCB_CONFIGURE_NOTIFY: {
        const xcb_configure_notify_event_t *cfgEvent =
            (const xcb_configure_notify_event_t *)xcbEvent;

        if (cfgEvent->window == context.window &&
            ((short)cfgEvent->width != context.window_width ||
             (short)cfgEvent->height != context.window_height)) {
          dispatch_event(event_t{std::type_index{typeid(listen_resize_t)},
                                 (short)cfgEvent->width,
                                 (short)cfgEvent->height});
          bvideo_output = true;
        }
      } break;
      case XCB_CLIENT_MESSAGE: {
        if ((*(xcb_client_message_event_t *)xcbEvent).data.data32[0] ==
            (*reply2).atom) {
          bProcessing = false;
        }
      } break;
      }
      free(xcbEvent);
      xcbEvents.pop_front();
    }

    if (bvideo_output)
      context.state_notify_complete();
  }
}
