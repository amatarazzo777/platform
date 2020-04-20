/**
\file uxdevice.cpp

\author Anthony Matarazzo

\date 3/26/20
\version 1.0
*/
#define PI 3.1415926535

/**
\brief rendering and platform services.

*/
#include "uxdevice.hpp"

using namespace std;
using namespace uxdevice;

/**
\internal
\brief The routine iterates the display list moving
parameters to the class member communication areas.
If processing is requested, the function operation
is
invoked.
*/
void uxdevice::platform::render(const event &evt) {
  static size_t count = 0;

  count++;

  static std::chrono::system_clock::time_point lastTime =
      std::chrono::high_resolution_clock::now();
  std::chrono::system_clock::time_point start =
      std::chrono::high_resolution_clock::now();

  double dx = evt.x;
  double dy = evt.y;
  double dw = evt.width;
  double dh = evt.height;

  cairo_push_group(context.cr);
  if (context.preclear) {
    cairo_set_source_rgb(context.cr, 1.0, 1.0, 1.0);
    cairo_paint(context.cr);
    context.preclear = false;
  }
  // cairo_set_operator(context.cr, CAIRO_OPERATOR_SOURCE);
  // cairo_set_antialias(context.cr, CAIRO_ANTIALIAS_SUBPIXEL);

  // cairo_set_operator(context.cr, CAIRO_OPERATOR_OVER);
  // cairo_set_antialias(m_cr, CAIRO_ANTIALIAS_SUBPIXEL);

  for (auto &n : DL) {
    context.set(n.get());
    n->invoke(context);

    if (cairo_status(context.cr)) {
      std::stringstream sError;
      sError << "ERR_CAIRO render loop "
             << "  " << __FILE__ << " " << __func__;
      throw std::runtime_error(sError.str());
    }
  }

  dx = 0;
  dy = 0;
  dw = context.windowWidth;

  dh = context.windowHeight;

  cairo_pop_group_to_source(context.cr);
  cairo_paint(context.cr);

#if 0
  cairo_fill(context.cr);
  cairo_rectangle(context.cr, 100,100,200,200);
  cairo_stroke(context.cr);

#endif

  cairo_surface_flush(context.xcbSurface);
  // cairo_surface_write_to_png (context.xcbSurface, "test.png");
  xcb_flush(context.connection);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;

  lastTime = std::chrono::high_resolution_clock::now();
}

/**
\internal
\brief a simple test of the pointer and shared memory .
*/
// change data on mouse move
void uxdevice::platform::test(int x, int y) {}

/*
\brief the dispatch routine is invoked by the messageLoop.
If default
 * handling is to be supplied, the method invokes the
necessary operation.

*/
void uxdevice::platform::dispatchEvent(const event &evt) {

  switch (evt.evtType) {
  case eventType::paint: {
    render(evt);
  } break;
  case eventType::resize:
    resize(evt.width, evt.height);
    break;
  case eventType::keydown: {

  } break;
  case eventType::keyup: {

  } break;
  case eventType::keypress: {

  } break;
  case eventType::mousemove:
    break;
  case eventType::mousedown:

    break;
  case eventType::mouseup:
    break;
  case eventType::wheel:
    break;
  }
/* these events do not come from the platform. However,
they are spawned from conditions based upon the platform events.
*/
#if 0
eventType::focus
eventType::blur
eventType::mouseenter
eventType::click
eventType::dblclick
eventType::contextmenu
eventType::mouseleave
#endif
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
void uxdevice::platform::processEvents(void) {
  // setup the event dispatcher
  eventHandler ev = std::bind(&uxdevice::platform::dispatchEvent, this,
                              std::placeholders::_1);

  messageLoop();
}

/**
\internal

\brief The function maps the event id to the appropriate vector.
This is kept statically here for resource management.

\param eventType evtType
*/
vector<eventHandler> &uxdevice::platform::getEventVector(eventType evtType) {
  static unordered_map<eventType, vector<eventHandler> &> eventTypeMap = {
      {eventType::focus, onfocus},
      {eventType::blur, onblur},
      {eventType::resize, onresize},
      {eventType::keydown, onkeydown},
      {eventType::keyup, onkeyup},
      {eventType::keypress, onkeypress},
      {eventType::mouseenter, onmouseenter},
      {eventType::mouseleave, onmouseleave},
      {eventType::mousemove, onmousemove},
      {eventType::mousedown, onmousedown},
      {eventType::mouseup, onmouseup},
      {eventType::click, onclick},
      {eventType::dblclick, ondblclick},
      {eventType::contextmenu, oncontextmenu},
      {eventType::wheel, onwheel}};
  auto it = eventTypeMap.find(evtType);
  return it->second;
}
/**
\internal
\brief
The function will return the address of a std::function for the purposes
of equality testing. Function from
https://stackoverflow.com/questions/20833453/comparing-stdfunctions-for-equality

*/
template <typename T, typename... U>
size_t getAddress(std::function<T(U...)> f) {
  typedef T(fnType)(U...);
  fnType **fnPointer = f.template target<fnType *>();
  return (size_t)*fnPointer;
}

#if 0
/**

\brief The function is invoked when an event occurrs. Normally this occurs
from the platform device. However, this may be invoked by the soft
generation of events.

*/
void uxdevice::platform::dispatch(const event &e) {
  auto &v = getEventVector(e.evtType);
  for (auto &fn : v)
    fn(e);
}
#endif

/**
  \internal
  \brief constructor for the platform object. The platform object is coded
  such that each of the operating systems supported is encapsulated within
  preprocessor blocks.

  \param eventHandler evtDispatcher the dispatcher routine which connects
  the platform to the object model system. \param unsigned short width -
  window size. \param unsigned short height - window size.
*/
uxdevice::platform::platform(const eventHandler &evtDispatcher,
                             const errorHandler &fn) {
  fnEvents = evtDispatcher;
  fnError = fn;
// initialize private members
#if defined(__linux__)

#elif defined(_WIN64)

  CoInitialize(NULL);

#endif

#ifdef USE_IMAGE_MAGICK
  Magick::InitializeMagick("");

#endif
}
/**
  \internal
  \brief terminates the xserver connection
  and frees resources.
*/
uxdevice::platform::~platform() {

#if defined(__linux__)

#elif defined(_WIN64)
  CoUninitialize();

#endif
}
/**
  \internal
  \brief opens a window on the target OS

*/
void uxdevice::platform::openWindow(const std::string &sWindowTitle,
                                    const unsigned short width,
                                    const unsigned short height) {
  context.windowWidth = width;
  context.windowHeight = height;

#if defined(__linux__)
  // this open provide interoperability between xcb and xwindows
  // this is used here because of the necessity of key mapping.
  context.xdisplay = XOpenDisplay(nullptr);
  if (!context.xdisplay) {
    closeWindow();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  /* get the connection to the X server */
  context.connection = XGetXCBConnection(context.xdisplay);
  if (!context.xdisplay) {
    closeWindow();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  /* Get the first screen */
  context.screen =
      xcb_setup_roots_iterator(xcb_get_setup(context.connection)).data;
  if (!context.screen) {
    closeWindow();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  context.syms = xcb_key_symbols_alloc(context.connection);
  if (!context.syms) {
    closeWindow();
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
    closeWindow();
    std::stringstream sError;
    sError << "ERR_XWIN "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }
  // resize to windowWidth,windowHeight
  resize(context.windowWidth, context.windowHeight);

  /* Create a window */
  context.window = xcb_generate_id(context.connection);
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  mask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
  mask = XCB_CW_BACK_PIXMAP | XCB_CW_BORDER_PIXEL | XCB_CW_BIT_GRAVITY |
         XCB_CW_OVERRIDE_REDIRECT | XCB_CW_SAVE_UNDER | XCB_CW_EVENT_MASK;

  uint32_t vals[] = {
      XCB_BACK_PIXMAP_NONE,
      // XCB_BACK_PIXMAP_PARENT_RELATIVE,
      context.screen->black_pixel, XCB_GRAVITY_NORTH_WEST, 0, 1,
      XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS |
          XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
          XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_BUTTON_PRESS |
          XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_STRUCTURE_NOTIFY};

  xcb_create_window(context.connection, XCB_COPY_FROM_PARENT, context.window,
                    context.screen->root, 0, 0,
                    static_cast<unsigned short>(context.windowWidth),
                    static_cast<unsigned short>(context.windowHeight), 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, context.screen->root_visual,
                    mask, vals);
  // set window title
  xcb_change_property(context.connection, XCB_PROP_MODE_REPLACE, context.window,
                      XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, sWindowTitle.size(),
                      sWindowTitle.data());

  /* Map the window on the screen and flush*/
  xcb_map_window(context.connection, context.window);
  xcb_flush(context.connection);

  if (!context.window) {
    closeWindow();
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

  context.xcbSurface = cairo_xcb_surface_create(
      context.connection, context.window, context.visualType,
      context.windowWidth, context.windowHeight);
  if (!context.xcbSurface) {
    closeWindow();
    std::stringstream sError;
    sError << "ERR_CAIRO "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  context.cr = cairo_create(context.xcbSurface);
  if (!context.cr) {
    closeWindow();
    std::stringstream sError;
    sError << "ERR_CAIRO "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }
  cairo_set_source_rgb(context.cr, 0, 1.0, 1.0);
  cairo_paint(context.cr);

  cairo_surface_flush(context.xcbSurface);
  xcb_flush(context.connection);
  context.windowOpen = true;

  return;

#elif defined(_WIN64)

  // Register the window class.
  WNDCLASSEX wcex = {sizeof(WNDCLASSEX)};
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = &uxdevice::platform::WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = sizeof(LONG_PTR);
  wcex.hInstance = HINST_THISCOMPONENT;
  wcex.hbrBackground = NULL;
  wcex.lpszMenuName = NULL;
  wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
  wcex.lpszClassName = "viewManagerApp";
  RegisterClassEx(&wcex);
  // Create the window.
  context.hwnd = CreateWindow("viewManagerApp", sWindowTitle.data(),
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                              static_cast<UINT>(context.windowWidth),
                              static_cast<UINT>(context.windowHeighth), NULL,
                              NULL, HINST_THISCOMPONENT, 0L);

  SetWindowLongPtr(context.hwnd, GWLP_USERDATA, (long long)this);

  if (!initializeVideo())
    throw std::runtime_error("Could not initialize direct x video subsystem.");

  // create offscreen bitmap
  resize(context.windowWidth, context.windowHeight);

  ShowWindow(context.hwnd, SW_SHOWNORMAL);
  UpdateWindow(context.hwnd);
  context.windowOpen = true;

#endif
}

/**
  \internal
  \brief Initalize the direct 3 video system.

  Orginal code from
*/
#if defined(_WIN64)
bool uxdevice::platform::initializeVideo() {
  HRESULT hr;

  // Create a Direct2D factory.
  hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                         &context.pD2DFactory);

  RECT rc;
  GetClientRect(context.hwnd, &rc);

  D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

  // Create a Direct2D render target.
  hr = context.pD2DFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(context.hwnd, size),
      &context.pRenderTarget);
  return true;
}

/**
  \brief terminateVideo
  \description the routine frees the resources of directx.
*/
void uxdevice::platform::terminateVideo(void) {
  context.pD2DFactory->Release();
  context.pRenderTarget->Release();
}

#endif

/**
  \internal
  \brief closes a window on the target OS


*/
void uxdevice::platform::closeWindow(void) {

#if defined(__linux__)

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
  if (context.connection) {
    xcb_disconnect(context.connection);
    context.connection = 0;
  }
  if (context.xdisplay) {
    XCloseDisplay(context.xdisplay);
    context.xdisplay = nullptr;
  }
  context.windowOpen = false;

#elif defined(_WIN64)

#endif
}

#if defined(_WIN64)

/**
\internal
\brief The default window message processor for the application.
This is the version of the Microsoft Windows operating system.

*/
LRESULT CALLBACK uxdevice::platform::WndProc(HWND hwnd, UINT message,
                                             WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;
  bool handled = false;
  /** get the platform objext which is stored within the user data of the
   window. this is necessary as the wndproc for the windows operating system
   is called from an external library. The routine needs to be a static
   implementation which is not directly locate within the class.
  */
  LONG_PTR lpUserData = GetWindowLongPtr(hwnd, GWLP_USERDATA);
  platform *platformInstance = (platform *)lpUserData;
  switch (message) {
  case WM_SIZE:
    platformInstance->dispatchEvent(event{eventType::resize,
                                          static_cast<short>(LOWORD(lParam)),
                                          static_cast<short>(HIWORD(lParam))});
    result = 0;
    handled = true;
    break;
  case WM_KEYDOWN: {
    UINT scandCode = (lParam >> 8) & 0xFFFFFF00;
    platformInstance->dispatchEvent(
        event{eventType::keydown, (unsigned int)wParam});
    handled = true;
  } break;
  case WM_KEYUP: {
    UINT scandCode = (lParam >> 8) & 0xFFFFFF00;
    platformInstance->dispatchEvent(
        event{eventType::keyup, (unsigned int)wParam});
    handled = true;
  } break;
  case WM_CHAR: {
    // filter out some of the control keys that
    // slip through such as the back and tab keys
    if (wParam > 27) {
      WCHAR tmp[2];
      tmp[0] = wParam;
      tmp[1] = 0x00;
      char ch = wParam;
      platformInstance->dispatchEvent(event{eventType::keypress, ch});
      handled = true;
    }
  } break;
  case WM_LBUTTONDOWN:
    platformInstance->dispatchEvent(
        event{eventType::mousedown, static_cast<short>(LOWORD(lParam)),
              static_cast<short>(HIWORD(lParam)), 1});
    handled = true;
    break;
  case WM_LBUTTONUP:
    platformInstance->dispatchEvent(
        event{eventType::mouseup, static_cast<short>(LOWORD(lParam)),
              static_cast<short>(HIWORD(lParam)), 1});
    handled = true;
    break;
  case WM_MBUTTONDOWN:
    platformInstance->dispatchEvent(
        event{eventType::mousedown, static_cast<short>(LOWORD(lParam)),
              static_cast<short>(HIWORD(lParam)), 2});
    handled = true;
    break;
  case WM_MBUTTONUP:
    platformInstance->dispatchEvent(
        event{eventType::mouseup, static_cast<short>(LOWORD(lParam)),
              static_cast<short>(HIWORD(lParam)), 2});
    handled = true;
    break;
  case WM_RBUTTONDOWN:
    platformInstance->dispatchEvent(
        event{eventType::mousedown, static_cast<short>(LOWORD(lParam)),
              static_cast<short>(HIWORD(lParam)), 3});
    handled = true;
    break;
  case WM_RBUTTONUP:
    platformInstance->dispatchEvent(
        event{eventType::mouseup, static_cast<short>(LOWORD(lParam)),
              static_cast<short>(HIWORD(lParam)), 3});
    handled = true;
    break;
  case WM_MOUSEMOVE:
    platformInstance->dispatchEvent(event{eventType::mousemove,
                                          static_cast<short>(LOWORD(lParam)),
                                          static_cast<short>(HIWORD(lParam))});
    result = 0;
    handled = true;
    break;
  case WM_MOUSEWHEEL: {
    platformInstance->dispatchEvent(event{
        eventType::wheel, static_cast<short>(LOWORD(lParam)),
        static_cast<short>(HIWORD(lParam)), GET_WHEEL_DELTA_WPARAM(wParam)});
    handled = true;
  } break;
  case WM_DISPLAYCHANGE:
    InvalidateRect(hwnd, NULL, FALSE);
    result = 0;
    handled = true;
    break;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    platformInstance->dispatchEvent(event{eventType::paint});
    EndPaint(hwnd, &ps);
    ValidateRect(hwnd, NULL);
    result = 0;
    handled = true;
  } break;
  case WM_DESTROY:
    PostQuitMessage(0);
    result = 1;
    handled = true;
    break;
  }
  if (!handled)
    result = DefWindowProc(hwnd, message, wParam, lParam);
  return result;
}
#endif

/**
\internal
\brief the routine handles the message processing for the specific
operating system. The function is called from processEvents.

*/
void uxdevice::platform::messageLoop(void) {
#if defined(__linux__)
  xcb_generic_event_t *xcbEvent;
  short int newWidth;
  short int newHeight;
  bool bRequestResize = false;

  while ((xcbEvent = xcb_wait_for_event(context.connection))) {
    switch (xcbEvent->response_type & ~0x80) {
    case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)xcbEvent;
      dispatchEvent(event{
          eventType::mousemove,
          motion->event_x,
          motion->event_y,
      });
    } break;
    case XCB_BUTTON_PRESS: {
      xcb_button_press_event_t *bp = (xcb_button_press_event_t *)xcbEvent;
      if (bp->detail == XCB_BUTTON_INDEX_4 ||
          bp->detail == XCB_BUTTON_INDEX_5) {
        dispatchEvent(event{eventType::wheel, bp->event_x, bp->event_y,
                            bp->detail == XCB_BUTTON_INDEX_4 ? 1 : -1});

      } else {
        dispatchEvent(
            event{eventType::mousedown, bp->event_x, bp->event_y, bp->detail});
      }
    } break;
    case XCB_BUTTON_RELEASE: {
      xcb_button_release_event_t *br = (xcb_button_release_event_t *)xcbEvent;
      // ignore button 4 and 5 which are wheel events.
      if (br->detail != XCB_BUTTON_INDEX_4 && br->detail != XCB_BUTTON_INDEX_5)
        dispatchEvent(
            event{eventType::mouseup, br->event_x, br->event_y, br->detail});
    } break;
    case XCB_KEY_PRESS: {
      xcb_key_press_event_t *kp = (xcb_key_press_event_t *)xcbEvent;
      xcb_keysym_t sym = xcb_key_press_lookup_keysym(context.syms, kp, 0);
      if (sym < 0x99) {
        XKeyEvent keyEvent;
        keyEvent.display = context.xdisplay;
        keyEvent.keycode = kp->detail;
        keyEvent.state = kp->state;
        std::array<char, 16> buf{};
        if (XLookupString(&keyEvent, buf.data(), buf.size(), nullptr, nullptr))
          dispatchEvent(event{eventType::keypress, (char)buf[0]});
      } else {
        dispatchEvent(event{eventType::keydown, sym});
      }
    } break;
    case XCB_KEY_RELEASE: {
      xcb_key_release_event_t *kr = (xcb_key_release_event_t *)xcbEvent;
      xcb_keysym_t sym = xcb_key_press_lookup_keysym(context.syms, kr, 0);
      dispatchEvent(event{eventType::keyup, sym});
    } break;
    case XCB_EXPOSE: {
      xcb_expose_event_t *eev = (xcb_expose_event_t *)xcbEvent;

      if (eev->count == 0) {
        if (bRequestResize) {
          bRequestResize = false;
          dispatchEvent(event{eventType::resize, newWidth, newHeight});
        }

        dispatchEvent(
            event{eventType::paint, eev->x, eev->y, eev->width, eev->height});
      }
    } break;
    case XCB_CONFIGURE_NOTIFY: {
      const xcb_configure_notify_event_t *cfgEvent =
          (const xcb_configure_notify_event_t *)xcbEvent;

      if (cfgEvent->window == context.window) {
        newWidth = cfgEvent->width;
        newHeight = cfgEvent->height;
        if ((newWidth != context.windowWidth ||
             newHeight != context.windowWidth) &&
            (newWidth > 0) && (newHeight > 0)) {
          bRequestResize = true;
        }
      }
    }
    }
    free(xcbEvent);
  }
#elif defined(_WIN64)
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
#endif
}

/**
\brief API interface, just data is passed.

*/

/**
\brief clears the display list
*/
void uxdevice::platform::clear(void) { DL.clear(); }
/**
\brief sets the text
*/
void uxdevice::platform::text(const std::string &s) {
  DL.push_back(make_unique<STRING>(s));
}
/**
\brief
*/
void uxdevice::platform::text(const std::stringstream &s) {
  DL.push_back(make_unique<STRING>(s.str()));
}
/**
\brief
*/
void uxdevice::platform::image(const std::string &s) {
  DL.push_back(make_unique<IMAGE>(s));
}
/**
\brief
*/
void uxdevice::platform::pen(u_int32_t c) { DL.push_back(make_unique<PEN>(c)); }
/**
\brief
*/
void uxdevice::platform::pen(const string &c) {
  DL.push_back(make_unique<PEN>(c));
}
/**
\brief
*/
void uxdevice::platform::pen(double _r, double _g, double _b) {
  DL.push_back(make_unique<PEN>(_r, _g, _b));
}
/**
\brief
*/
void uxdevice::platform::pen(double _r, double _g, double _b, double _a) {
  DL.push_back(make_unique<PEN>(_r, _g, _b, _a));
}
/**
\brief
*/
void uxdevice::platform::background(u_int32_t c) {
  DL.push_back(make_unique<BACKGROUND>(c));
}
/**
\brief
*/
void uxdevice::platform::background(const string &c) {
  DL.push_back(make_unique<BACKGROUND>(c));
}
/**
\brief
*/
void uxdevice::platform::background(double _r, double _g, double _b) {
  DL.push_back(make_unique<BACKGROUND>(_r, _g, _b));
}
/**
\brief
*/
void uxdevice::platform::background(double _r, double _g, double _b,
                                    double _a) {
  DL.push_back(make_unique<BACKGROUND>(_r, _g, _b, _a));
}
/**
\brief
*/
void uxdevice::platform::fontDescription(const std::string &s) {
  DL.push_back(make_unique<FONT>(s));
}
/**
\brief
*/
void uxdevice::platform::area(double x, double y, double w, double h) {
  DL.push_back(make_unique<AREA>(x, y, w, h));
}
/**
\brief
*/
void uxdevice::platform::drawText(void) {
  DL.push_back(make_unique<DRAWTEXT>());
}
/**
\brief
*/
void uxdevice::platform::drawImage(void) {
  DL.push_back(make_unique<DRAWIMAGE>());
}
/**
\brief
*/
void uxdevice::platform::drawBox(void) { DL.push_back(make_unique<DRAWBOX>()); }

/**
\brief
*/
void uxdevice::platform::translate(double x, double y) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func = std::bind(cairo_translate, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::rotate(double angle) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func = std::bind(cairo_rotate, _1, angle);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::scale(double x, double y) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func = std::bind(cairo_scale, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/

void uxdevice::platform::arc(double xc, double yc, double radius, double angle1,
                             double angle2) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func =
      std::bind(cairo_arc, _1, xc, yc, radius, angle1, angle2);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::arc_negative(double xc, double yc, double radius,
                                      double angle1, double angle2) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func =
      std::bind(cairo_arc_negative, _1, xc, yc, radius, angle1, angle2);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::curve_to(double x1, double y1, double x2, double y2,
                                  double x3, double y3) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func = std::bind(cairo_curve_to, _1, x1, y1, x2, y2, x3, y3);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::line_to(double x, double y) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func = std::bind(cairo_line_to, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::stroke(void) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func = std::bind(cairo_stroke, _1);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::move_to(double x, double y) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func = std::bind(cairo_move_to, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::rectangle(double x, double y, double width,
                                   double height) {
  using namespace std::placeholders;
  CAIRO_FUNCTION func = std::bind(cairo_rectangle, _1, x, y, width, height);
  DL.push_back(make_unique<FUNCTION>(func));
}

#if defined(USE_IMAGE_MAGICK)
// these functions store a pointer to the image magick member function
// later, the actual image is placed within the call before invocation.
void uxdevice::platform::addNoise(Magick::NoiseType noiseType_) {
  using namespace std::placeholders;
  const double attenuate_ = 1.0;
  ImageFunction func =
      std::bind(&Magick::Image::addNoise, _1, noiseType_, attenuate_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::addNoiseChannel(const Magick::ChannelType channel_,
                                         const Magick::NoiseType noiseType_) {
  using namespace std::placeholders;
  const double attenuate_ = 1.0;
  ImageFunction func = std::bind(&Magick::Image::addNoiseChannel, _1, channel_,
                                 noiseType_, attenuate_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::blur(const double radius_, const double sigma_) {
  using namespace std::placeholders;

  ImageFunction func = std::bind(&Magick::Image::blur, _1, radius_, sigma_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::blurChannel(const Magick::ChannelType channel_,
                                     const double radius_,
                                     const double sigma_) {
  using namespace std::placeholders;
  ImageFunction func =
      std::bind(&Magick::Image::blurChannel, _1, channel_, radius_, sigma_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::charcoal(const double radius_, const double sigma_) {
  using namespace std::placeholders;

  // NOTE extract alpha channel and use it as a mask after image processing to
  // maintain alpha transparency

  ImageFunction func = std::bind(&Magick::Image::charcoal, _1, radius_, sigma_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

#if 0
/******** forward ?? the reference cannot be sent using the bind method due to compilation issue.
*/
void uxdevice::platform::colorize(const unsigned int alpha_,const Magick::Color &penColor_) {
  using namespace std::placeholders;
  const Magick::Color c(penColor_);
  ImageFunction func = std::bind(&Magick::Image::colorize, _1, alpha_, &c));
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::colorize(const unsigned int alphaRed_,const unsigned int alphaGreen_,
   const unsigned int alphaBlue_,const Magick::Color penColor_) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::colorize, _1, alphaRed_,
                                 alphaGreen_, alphaBlue_, penColor_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

#endif // 0
void uxdevice::platform::colorize(const unsigned int alpha_,const Magick::Color &penColor_) {
}

void uxdevice::platform::colorize(const unsigned int alphaRed_,const unsigned int alphaGreen_,
   const unsigned int alphaBlue_,const Magick::Color &penColor_) {

}

void uxdevice::platform::contrast(bool sharpen_) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::contrast, _1, sharpen_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::cycleColormap(int amount_) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::cycleColormap, _1, amount_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::despeckle(void) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::despeckle, _1);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::distort(const Magick::DistortMethod method,
                                 const std::vector<double> &args, const bool bestfit) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::distort, _1, method,
                                 args.size(), args.data(), bestfit);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::equalize(void) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::equalize, _1);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::enhance(void) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::enhance, _1);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::gaussianBlur(const double width_,
                                      const double sigma_) {
  using namespace std::placeholders;
  ImageFunction func =
      std::bind(&Magick::Image::gaussianBlur, _1, width_, sigma_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::gaussianBlurChannel(const Magick::ChannelType channel_,
                                             const double radius_,
                                             const double sigma_) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::gaussianBlurChannel, _1,
                                 channel_, radius_,sigma_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::implode(const double factor_) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::implode, _1, factor_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::medianFilter(const double radius_) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::medianFilter, _1, radius_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::modulate(double brightness_, double saturation_,
                                  double hue_) {
  using namespace std::placeholders;
  ImageFunction func =
      std::bind(&Magick::Image::modulate, _1, brightness_, saturation_, hue_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::motionBlur(const double radius_, const double sigma_,
                                    const double angle_) {
  using namespace std::placeholders;
  ImageFunction func =
      std::bind(&Magick::Image::motionBlur, _1, radius_, sigma_, angle_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::negate(bool grayscale_) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::negate, _1, grayscale_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::normalize(void) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::normalize, _1);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::oilPaint(const double radius_, const double sigma_) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::oilPaint, _1, radius_,sigma_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::raise(const Magick::Geometry &geometry_,
                               bool raisedFlag_) {
  using namespace std::placeholders;
  ImageFunction func =
      std::bind(&Magick::Image::raise, _1, geometry_, raisedFlag_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::shade(double azimuth_, double elevation_,
                               bool colorShading_) {
  using namespace std::placeholders;
  ImageFunction func =
      std::bind(&Magick::Image::shade, _1, azimuth_, elevation_, colorShading_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}

void uxdevice::platform::shadow(const double percent_opacity,
                                const double sigma_, const ssize_t x_,
                                const ssize_t y_) {
  using namespace std::placeholders;
  ImageFunction func =
      std::bind(&Magick::Image::shadow, _1, percent_opacity, sigma_, x_, y_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}
void uxdevice::platform::sharpen(const double radius_, const double sigma_) {
  using namespace std::placeholders;
  ImageFunction func = std::bind(&Magick::Image::sharpen, _1, radius_, sigma_);
  DL.push_back(make_unique<IMAGEPROCESS>(func));
}
#endif // defined

/***************************************************************************/

/**
\internal
\brief The drawText function provides textual character rendering.
*/
void uxdevice::platform::AREA::invoke(const DisplayUnitContext &context) {
  cairo_move_to(context.cr, x, y);
}

/**
\internal
\brief The drawText function provides textual character rendering.
*/
void uxdevice::platform::STRING::invoke(const DisplayUnitContext &context) {}

/**
\internal
\brief The FONT object provides contextual font object for library.
*/
void uxdevice::platform::FONT::invoke(const DisplayUnitContext &context) {
#if defined(USE_PANGO)
  fontDescription = pango_font_description_from_string(description.data());

#else
  cairo_select_font_face(context.cr, description.data(),
                         CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(context.cr, pointSize);

#endif
}

/**
\internal
\brief Sets the alignment on the textual layout. PANGO supported,

*/
void uxdevice::platform::ALIGN::invoke(const DisplayUnitContext &context) {}

/**
\internal
\brief Maps the last event defined.
*/
void uxdevice::platform::EVENT::invoke(const DisplayUnitContext &context) {}

/**
\internal
\brief The drawText function provides textual character rendering.
*/
void uxdevice::platform::DRAWTEXT::invoke(const DisplayUnitContext &context) {
  // check the context before operating
  if (!context.validateAREA() || !context.validateSTRING()) {
    std::stringstream sError;
    sError << "ERR_DRAWTEXT AREA or STRING not set. "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

#if defined USE_PANGO

  layout = pango_cairo_create_layout(context.cr);
  pango_layout_set_text(layout, context.STRING()->data.data(), -1);
  pango_layout_set_font_description(layout, context.FONT()->fontDescription);

  // pangoColor
  pango_layout_set_width(layout, context.AREA()->w * PANGO_SCALE);
  pango_layout_set_height(layout, context.AREA()->h * PANGO_SCALE);

  pango_cairo_update_layout(context.cr, layout);

  pango_cairo_show_layout(context.cr, layout);

  g_object_unref(layout);
  layout = nullptr;

#else
  cairo_show_text(context.cr, context.STRING()->data.data());
#endif
}

/**
\internal
\brief The function draws the image
*/
void uxdevice::platform::DRAWIMAGE::invoke(const DisplayUnitContext &context) {
  // check the context before operating
  if (!context.validateAREA() || !context.validateIMAGE()) {
    std::stringstream sError;
    sError << "ERR_DRAWIMAGE AREA or IMAGE not set. "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  if (context.IMAGE()->type != IMAGE::ImageSystemType::cairo_type)
    context.IMAGE()->toCairo();

  cairo_set_source_surface(context.cr, context.IMAGE()->cairo,
                           context.AREA()->x, context.AREA()->y);
  cairo_mask_surface(context.cr, context.IMAGE()->cairo, context.AREA()->x,
                     context.AREA()->y);
#if 0
  cairo_rectangle(context.cr, context.AREA()->x, context.AREA()->y,
                  context.AREA()->w, context.AREA()->h);
  cairo_set_source_surface(context.cr, context.IMAGE()->surface,
                           context.AREA()->x, context.AREA()->y);
  cairo_fill(context.cr);
#endif
}

/**
\internal
\brief The function draws the image
*/
void uxdevice::platform::DRAWBOX::invoke(const DisplayUnitContext &context) {
  // check the context before operating
  if (!context.validateAREA() &&
      (!context.validateBACKGROUND() || !context.validatePEN())) {
    std::stringstream sError;
    sError << "ERR_DRAWBOX AREA or IMAGE not set. "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  cairo_rectangle(context.cr, context.AREA()->x, context.AREA()->y,
                  context.AREA()->w, context.AREA()->h);
  if (context.validateBACKGROUND()) {
    cairo_set_source_rgba(context.cr, context.BACKGROUND()->r,
                          context.BACKGROUND()->g, context.BACKGROUND()->b,
                          context.BACKGROUND()->a);
    cairo_fill_preserve(context.cr);
  }
  if (context.validatePEN()) {
    cairo_set_source_rgba(context.cr, context.PEN()->r, context.PEN()->g,
                          context.PEN()->b, context.PEN()->a);
    cairo_stroke(context.cr);
  }

#if 0
  cairo_rectangle(context.cr, context.AREA()->x, context.AREA()->y,
                  context.AREA()->w, context.AREA()->h);
  cairo_set_source_surface(context.cr, context.IMAGE()->surface,
                           context.AREA()->x, context.AREA()->y);
  cairo_fill(context.cr);
#endif
}

/**
\internal
\brief The function invokes the color operation
*/
void uxdevice::platform::PEN::invoke(const DisplayUnitContext &context) {
  cairo_set_source_rgba(context.cr, r, g, b, a);
}

/**
\internal
\brief The function invokes the color operation
*/
void uxdevice::platform::BACKGROUND::invoke(const DisplayUnitContext &context) {
  cairo_set_source_rgba(context.cr, r, g, b, a);
}

void uxdevice::platform::FUNCTION::invoke(const DisplayUnitContext &context) {
  func(context.cr);
}

/**
\internal
\brief The drawText function provides textual character rendering.
*/
void uxdevice::platform::IMAGE::invoke(const DisplayUnitContext &context) {
  if (bLoaded)
    return;

#if defined(USE_IMAGE_MAGICK)

  magick = Magick::Image();
  magick.type(Magick::TrueColorType);
  magick.backgroundColor("None");
  magick.read(context.IMAGE()->fileName);
  Magick::Color bg_color = magick.pixelColor(0, 0);
  magick.transparent(bg_color);
  type = ImageSystemType::magick_type;
  cairo = nullptr;

#else
  cairo = cairo_image_surface_create_from_png(context.IMAGE()->fileName.data());
  type = ImageSystemType::cairo_type;
  magick = nullptr;

#endif // USE_IMAGE_MAGICK
  bLoaded = true;
}

void uxdevice::platform::IMAGE::toCairo(void) {
  if (type != ImageSystemType::magick_type) {
    std::stringstream sError;
    sError << "ERR_ IMAGECHAIN no magick current image. "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  cairo_format_t format = CAIRO_FORMAT_ARGB32;
  int stride = cairo_format_stride_for_width(format, magick.columns());
  unsigned char *surfacedata =
      reinterpret_cast<unsigned char *>(malloc(stride * magick.rows()));
  magick.type(Magick::TrueColorAlphaType);

  Magick::Pixels view(magick);

  const Magick::Quantum *pixels =
      view.getConst(0, 0, magick.columns(), magick.rows());

  for (std::size_t y = 0; y < magick.rows(); y++) {
    unsigned int *pBuffer =
        reinterpret_cast<unsigned int *>(surfacedata + y * stride);

    for (std::size_t x = 0; x < magick.columns(); x++) {
      unsigned char R, G, B, A;
      R = static_cast<unsigned char>(*pixels / QuantumRange * 255.0);
      pixels++;
      G = static_cast<unsigned char>(*pixels / QuantumRange * 255.0);
      pixels++;
      B = static_cast<unsigned char>(*pixels / QuantumRange * 255.0);
      pixels++;
      A = static_cast<unsigned char>(*pixels / QuantumRange * 255.0);
      pixels++;
      *pBuffer = A << 24 | R << 16 | G << 8 | B;
      pBuffer++;
    }
  }

  cairo = cairo_image_surface_create_for_data(
      surfacedata, format, magick.columns(), magick.rows(), stride);
  type = ImageSystemType::cairo_type;
  magick = nullptr;
}

void uxdevice::platform::IMAGE::toMagick(void) {
  if (cairo == nullptr) {
    std::stringstream sError;
    sError << "ERR_ IMAGECHAIN no cairo current image. "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  string sImageMap;
  Magick::StorageType store;

  switch (cairo_image_surface_get_format(cairo)) {
  case CAIRO_FORMAT_ARGB32:
    sImageMap = "ARGB";
    store = Magick::StorageType::LongPixel;
    break;
  }

  magick = Magick::Image(cairo_image_surface_get_width(cairo),
                         cairo_image_surface_get_height(cairo), sImageMap,
                         store, cairo_image_surface_get_data(cairo));

  type = ImageSystemType::magick_type;
  cairo_surface_destroy(cairo);
  cairo = nullptr;
}

#if defined(USE_IMAGE_MAGICK)
void uxdevice::platform::IMAGEPROCESS::invoke(
    const DisplayUnitContext &context) {
  if (bCompleted)
    return;

  if (context.IMAGE()->type != IMAGE::ImageSystemType::magick_type)
    context.IMAGE()->toMagick();

  func(&context.IMAGE()->magick);

  bCompleted = true;
}
#endif // defined

/**
  \internal
  \brief the function draws the cursor.
  */
void uxdevice::platform::drawCaret(const int x, const int y, const int h) {}

/**
\brief The function resizes the surface, sets the preclear flag (used to
   paint the entire surface a solid color before laying information on top,
   and updates the context window sizes.

*/
void uxdevice::platform::resize(const int w, const int h) {

  context.windowWidth = w;
  context.windowHeight = h;

#if defined(__linux__)
  if (context.xcbSurface) {
    cairo_xcb_surface_set_size(context.xcbSurface, w, h);
    context.preclear = true;
  }

#elif defined(_WIN64)

  // get the size ofthe window
  RECT rc;
  GetClientRect(context.hwnd, &rc);

  // resize the pixel memory
  context.windowWidth = rc.right - rc.left;
  context.windowHeight = rc.bottom - rc.top;

  int _bufferSize = context.windowWidth * context.windowHeight * 4;

  // clear to white
  clear();

  // free existing resources
  if (context.pRenderTarget) {
    context.pRenderTarget->Resize(D2D1::SizeU(_w, _h));

  } else {

    // Create a Direct2D render target
    HRESULT hr = context.pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(context.hwnd, D2D1::SizeU(_w, _h)),
        &context.pRenderTarget);
    if (FAILED(hr))
      return;
  }
#endif
}

/**
\brief The function copies the pixel buffer to the screen

*/
void uxdevice::platform::flip() {
#if defined(__linux__)

#if defined(USE_IMAGE_MAGICK)

#endif

#elif defined(_WIN64)
  if (!context.pRenderTarget)
    return;

  context.pRenderTarget->BeginDraw();

  // create offscreen bitmap for pixel rendering
  D2D1_PIXEL_FORMAT desc2D = D2D1::PixelFormat();
  desc2D.format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc2D.alphaMode = D2D1_ALPHA_MODE_IGNORE;

  D2D1_BITMAP_PROPERTIES bmpProperties = D2D1::BitmapProperties();
  context.pRenderTarget->GetDpi(&bmpProperties.dpiX, &bmpProperties.dpiY);
  bmpProperties.pixelFormat = desc2D;

  RECT rc;
  GetClientRect(context.hwnd, &rc);

  D2D1_SIZE_U size = D2D1::SizeU(_w, _h);
  HRESULT hr = m_pRenderTarget->CreateBitmap(
      size, context.offscreenBuffer.data(), context.windowWidth * 4,
      &bmpProperties, &context.pBitmap);

  // render bitmap to screen
  D2D1_RECT_F rectf;
  rectf.left = 0;
  rectf.top = 0;
  rectf.bottom = _h;
  rectf.right = _w;

  context.pRenderTarget->DrawBitmap(
      context.pBitmap, rectf, 1.0f,
      D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

  context.pRenderTarget->EndDraw();
  context.pBitmap->Release();

#endif
}

std::string _errorReport(std::string sourceFile, int ln, std::string sfunc,
                         std::string cond, std::string ecode) {
  std::stringstream ss;
  ss << sourceFile << "(" << ln << ") " << sfunc << "  " << cond << ecode;
  return ss.str();
}
