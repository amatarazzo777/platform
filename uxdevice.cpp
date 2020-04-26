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

  for (auto &n : DL) {
    if (n->index() == IDX(CLEAROPTION)) {
      context.clear(dynamic_cast<CLEAROPTION *>(n.get())->option);
      continue;
    }
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

  cairo_surface_flush(context.xcbSurface);
  xcb_flush(context.connection);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;

  lastTime = std::chrono::high_resolution_clock::now();
  cout << diff.count() << endl << flush;
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
  context.preclear = true;
  render(event{eventType::paint, 0, 0});
  cairo_surface_flush(context.xcbSurface);

  /* Map the window on the screen and flush*/
  xcb_map_window(context.connection, context.window);
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

void uxdevice::platform::antiAlias(antialias_t antialias) {
  DL.push_back(make_unique<ANTIALIAS>(antialias));
}

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
void uxdevice::platform::textOutline(u_int32_t c, double dWidth) {
  DL.push_back(make_unique<TEXTOUTLINE>(c, dWidth));
}
/**
\brief
*/
void uxdevice::platform::textOutline(const string &c, double dWidth) {
  DL.push_back(make_unique<TEXTOUTLINE>(c, dWidth));
}
/**
\brief
*/
void uxdevice::platform::textOutline(double _r, double _g, double _b,
                                     double dWidth) {
  DL.push_back(make_unique<TEXTOUTLINE>(_r, _g, _b, dWidth));
}
/**
\brief
*/
void uxdevice::platform::textOutline(double _r, double _g, double _b, double _a,
                                     double dWidth) {
  DL.push_back(make_unique<TEXTOUTLINE>(_r, _g, _b, _a, dWidth));
}
/**
\brief clears the current text outline from the context.
*/
void uxdevice::platform::textOutlineNone(void) {
  DL.push_back(make_unique<CLEAROPTION>(IDX(TEXTOUTLINE)));
}

void uxdevice::platform::textFill(const string &s) {
  DL.push_back(make_unique<TEXTFILL>(s));
}

void uxdevice::platform::textFill(const ColorStops &s) {
  DL.push_back(make_unique<TEXTFILL>(s));
}
/**
\brief clears the current text fill from the context.
*/
void uxdevice::platform::textFillNone(void) {
  DL.push_back(make_unique<CLEAROPTION>(IDX(TEXTFILL)));
}
/**
\brief
*/
void uxdevice::platform::textShadow(u_int32_t c, int r, double xOffset,
                                    double yOffset) {
  DL.push_back(make_unique<TEXTSHADOW>(c, r, xOffset, yOffset));
}
/**
\brief
*/
void uxdevice::platform::textShadow(const string &c, int r, double xOffset,
                                    double yOffset) {
  DL.push_back(make_unique<TEXTSHADOW>(c, r, xOffset, yOffset));
}
/**
\brief
*/
void uxdevice::platform::textShadow(double _r, double _g, double _b, int r,
                                    double xOffset, double yOffset) {
  DL.push_back(make_unique<TEXTSHADOW>(_r, _g, _b, r, xOffset, yOffset));
}
/**
\brief
*/
void uxdevice::platform::textShadow(double _r, double _g, double _b, double _a,
                                    int r, double xOffset, double yOffset) {
  DL.push_back(make_unique<TEXTSHADOW>(_r, _g, _b, _a, r, xOffset, yOffset));
}
/**
\brief clears the current text fill from the context.
*/
void uxdevice::platform::textShadowNone(void) {
  DL.push_back(make_unique<CLEAROPTION>(IDX(TEXTSHADOW)));
}

/**
\brief
*/
void uxdevice::platform::font(const std::string &s) {
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

/***************************************************************************/
void uxdevice::platform::CLEAROPTION::invoke(
    const DisplayUnitContext &context) {}

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
  fontDescription = pango_font_description_from_string(description.data());
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

void uxdevice::platform::ANTIALIAS::invoke(const DisplayUnitContext &context) {}

void uxdevice::platform::TEXTOUTLINE::invoke(
    const DisplayUnitContext &context) {}

void uxdevice::platform::TEXTFILL::invoke(const DisplayUnitContext &context) {
  // load texture if does not exist
  if (bImage && !bLoaded) {
    image = cairo_image_surface_create_from_png(fileName.data());
    paint = cairo_pattern_create_for_surface(image);
    cairo_pattern_set_extend(paint, CAIRO_EXTEND_REFLECT);
    cairo_pattern_set_filter(paint, CAIRO_FILTER_BILINEAR);
    bLoaded = true;

  } else if (bGradient && !paint) {
    paint = cairo_pattern_create_linear(0, 0, 0, 15);
    cairo_pattern_set_extend(paint, CAIRO_EXTEND_REPEAT);
    for (auto &n : stops) {
      cairo_pattern_add_color_stop_rgba(paint, n.offset, n.r, n.g, n.g, n.a);
    }
  }
}

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

  // create offscreen image of the text
  if (!textImage) {
    cairo_format_t format = CAIRO_FORMAT_ARGB32;

    textImage = cairo_image_surface_create(format, context.AREA()->w,
                                           context.AREA()->h);
    cairo_t *textCr = cairo_create(textImage);

    if (context.validateANTIALIAS()) {
      cairo_set_antialias(textCr, context.ANTIALIAS()->setting);
    } else {
      cairo_set_antialias(textCr, cairo_antialias_t::CAIRO_ANTIALIAS_DEFAULT);
    }

    // clear the image with transparency
    cairo_set_source_rgba(textCr, 0, 0, 0, 0);
    cairo_paint(textCr);

    layout = pango_cairo_create_layout(textCr);
    pango_layout_set_text(layout, context.STRING()->data.data(), -1);
    pango_layout_set_font_description(layout, context.FONT()->fontDescription);

    // pangoColor
    pango_layout_set_width(layout, context.AREA()->w * PANGO_SCALE);
    pango_layout_set_height(layout, context.AREA()->h * PANGO_SCALE);

    pango_cairo_update_layout(textCr, layout);

    // depending on the parameters set for the textual character rendering,
    // different pango apis may be used in conjunction with cairo.
    // essentially this optimizes the textual rendering
    // wuch that less calls are made and more automatic filling
    // occurss within the rendering.
    bool bUsePathLayout = false;
    bool bOutline = false;
    bool bFilled = false;

    // if the text is drawn with an outline
    if (context.validateTEXTOUTLINE()) {
      bUsePathLayout = true;
      bOutline = true;
    }
    // if the text is filled with a texture or gradient
    if (context.validateTEXTFILL()) {
      bFilled = true;
      bUsePathLayout = true;
    }
    // path layout is used for outline and filled options.
    if (bUsePathLayout) {

      // draw the glyph shapes
      pango_cairo_layout_path(textCr, layout);

      // text is filled and outlined.
      if (bFilled && bOutline) {
        cairo_set_source(textCr, context.TEXTFILL()->paint);
        cairo_fill_preserve(textCr);

        cairo_set_source_rgba(
            textCr, context.TEXTOUTLINE()->r, context.TEXTOUTLINE()->g,
            context.TEXTOUTLINE()->b, context.TEXTOUTLINE()->a);
        cairo_set_line_width(textCr, context.TEXTOUTLINE()->lineWidth);
        cairo_stroke(textCr);

        // text is only filled.
      } else if (bFilled) {
        cairo_set_source(textCr, context.TEXTFILL()->paint);
        cairo_fill(textCr);

        // text is only outlined.
      } else if (bOutline) {
        cairo_set_source_rgba(
            textCr, context.TEXTOUTLINE()->r, context.TEXTOUTLINE()->g,
            context.TEXTOUTLINE()->b, context.TEXTOUTLINE()->a);
        cairo_set_line_width(textCr, context.TEXTOUTLINE()->lineWidth);
        cairo_stroke(textCr);
      }

    } else {
      // no outline or fill defined, therefore the pen is used.
      cairo_set_source_rgba(textCr, context.PEN()->r, context.PEN()->g,
                            context.PEN()->b, context.PEN()->a);
      pango_cairo_show_layout(textCr, layout);
    }

    // free drawing context
    cairo_destroy(textCr);
    g_object_unref(layout);
    layout = nullptr;
  }

  // draw shadow if one exists
  if (context.validateTEXTSHADOW()) {
    if (!shadowImage) {

      cairo_format_t format = CAIRO_FORMAT_ARGB32;

      shadowImage = cairo_image_surface_create(format, context.AREA()->w,
                                               context.AREA()->h);
      cairo_t *shadowCr = cairo_create(shadowImage);
      cairo_set_source_rgba(shadowCr, context.TEXTSHADOW()->color.r,
                            context.TEXTSHADOW()->color.g,
                            context.TEXTSHADOW()->color.b, 0);
      cairo_paint(shadowCr);

      if (context.validateANTIALIAS()) {
        cairo_set_antialias(shadowCr, context.ANTIALIAS()->setting);
      } else {
        cairo_set_antialias(shadowCr,
                            cairo_antialias_t::CAIRO_ANTIALIAS_DEFAULT);
      }

      // create an image for the text
      PangoLayout *layout = nullptr;
      layout = pango_cairo_create_layout(shadowCr);
      pango_layout_set_text(layout, context.STRING()->data.data(), -1);
      pango_layout_set_font_description(layout,
                                        context.FONT()->fontDescription);

      // pangoColor
      cairo_set_source_rgba(shadowCr, context.TEXTSHADOW()->color.r,
                            context.TEXTSHADOW()->color.g,
                            context.TEXTSHADOW()->color.b, 1);

      pango_layout_set_width(layout, context.AREA()->w * PANGO_SCALE);
      pango_layout_set_height(layout, context.AREA()->h * PANGO_SCALE);

      pango_cairo_update_layout(shadowCr, layout);
      pango_cairo_show_layout(shadowCr, layout);

      blurImage(shadowImage, context.TEXTSHADOW()->radius);
      cairo_destroy(shadowCr);
      g_object_unref(layout);
      layout = nullptr;
    }
    cairo_save(context.cr);

    cairo_set_source_rgba(context.cr, context.TEXTSHADOW()->color.r,
                          context.TEXTSHADOW()->color.g,
                          context.TEXTSHADOW()->color.b, 1);
    cairo_mask_surface(context.cr, shadowImage,
                       context.AREA()->x + context.TEXTSHADOW()->x,
                       context.AREA()->y + context.TEXTSHADOW()->y);
    cairo_restore(context.cr);
  }

  // move the text rendering to the display.
  cairo_set_source_surface(context.cr, textImage, context.AREA()->x,
                           context.AREA()->y);
  cairo_rectangle(context.cr, context.AREA()->x, context.AREA()->y,
                  context.AREA()->w, context.AREA()->h);
  cairo_fill(context.cr);
}

/// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
/// Stackblur algorithm by Mario Klingemann
/// Details here:
/// http://www.quasimondo.com/StackBlurForCanvas/StackBlurDemo.html
/// C++ implemenation base from:
/// https://gist.github.com/benjamin9999/3809142
/// http://www.antigrain.com/__code/include/agg_blur.h.html
/// This version works only with RGBA color
void uxdevice::platform::blurImage(cairo_surface_t *img, int radius) {
  static unsigned short const stackblur_mul[255] = {
      512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335,
      292, 512, 454, 405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335,
      312, 292, 273, 512, 482, 454, 428, 405, 383, 364, 345, 328, 312, 298,
      284, 271, 259, 496, 475, 456, 437, 420, 404, 388, 374, 360, 347, 335,
      323, 312, 302, 292, 282, 273, 265, 512, 497, 482, 468, 454, 441, 428,
      417, 405, 394, 383, 373, 364, 354, 345, 337, 328, 320, 312, 305, 298,
      291, 284, 278, 271, 265, 259, 507, 496, 485, 475, 465, 456, 446, 437,
      428, 420, 412, 404, 396, 388, 381, 374, 367, 360, 354, 347, 341, 335,
      329, 323, 318, 312, 307, 302, 297, 292, 287, 282, 278, 273, 269, 265,
      261, 512, 505, 497, 489, 482, 475, 468, 461, 454, 447, 441, 435, 428,
      422, 417, 411, 405, 399, 394, 389, 383, 378, 373, 368, 364, 359, 354,
      350, 345, 341, 337, 332, 328, 324, 320, 316, 312, 309, 305, 301, 298,
      294, 291, 287, 284, 281, 278, 274, 271, 268, 265, 262, 259, 257, 507,
      501, 496, 491, 485, 480, 475, 470, 465, 460, 456, 451, 446, 442, 437,
      433, 428, 424, 420, 416, 412, 408, 404, 400, 396, 392, 388, 385, 381,
      377, 374, 370, 367, 363, 360, 357, 354, 350, 347, 344, 341, 338, 335,
      332, 329, 326, 323, 320, 318, 315, 312, 310, 307, 304, 302, 299, 297,
      294, 292, 289, 287, 285, 282, 280, 278, 275, 273, 271, 269, 267, 265,
      263, 261, 259};

  static unsigned char const stackblur_shr[255] = {
      9,  11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17,
      17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19,
      19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21,
      21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
      21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
      22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
      22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23,
      23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
      23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
      23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24};

  if (radius > 254)
    return;
  if (radius < 2)
    return;

  cairo_surface_flush(img);

  unsigned char *src =
      reinterpret_cast<unsigned char *>(cairo_image_surface_get_data(img));
  unsigned int w = cairo_image_surface_get_width(img);
  unsigned int h = cairo_image_surface_get_height(img);
  unsigned int x, y, xp, yp, i;
  unsigned int sp;
  unsigned int stack_start;
  unsigned char *stack_ptr;

  unsigned char *src_ptr;
  unsigned char *dst_ptr;

  unsigned long sum_r;
  unsigned long sum_g;
  unsigned long sum_b;
  unsigned long sum_a;
  unsigned long sum_in_r;
  unsigned long sum_in_g;
  unsigned long sum_in_b;
  unsigned long sum_in_a;
  unsigned long sum_out_r;
  unsigned long sum_out_g;
  unsigned long sum_out_b;
  unsigned long sum_out_a;

  unsigned int wm = w - 1;
  unsigned int hm = h - 1;
  unsigned int w4 = cairo_image_surface_get_stride(img);
  unsigned int mul_sum = stackblur_mul[radius];
  unsigned char shr_sum = stackblur_shr[radius];

  unsigned int div = (radius * 2) + 1;
  unsigned char *stack = new unsigned char[div * 4];

  int minY = 0;
  int maxY = h;

  for (y = minY; y < maxY; y++) {
    sum_r = sum_g = sum_b = sum_a = sum_in_r = sum_in_g = sum_in_b = sum_in_a =
        sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

    src_ptr = src + w4 * y; // start of line (0,y)

    for (i = 0; i <= radius; i++) {
      stack_ptr = &stack[4 * i];
      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];
      sum_r += src_ptr[0] * (i + 1);
      sum_g += src_ptr[1] * (i + 1);
      sum_b += src_ptr[2] * (i + 1);
      sum_a += src_ptr[3] * (i + 1);
      sum_out_r += src_ptr[0];
      sum_out_g += src_ptr[1];
      sum_out_b += src_ptr[2];
      sum_out_a += src_ptr[3];
    }

    for (i = 1; i <= radius; i++) {
      if (i <= wm)
        src_ptr += 4;
      stack_ptr = &stack[4 * (i + radius)];
      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];
      sum_r += src_ptr[0] * (radius + 1 - i);
      sum_g += src_ptr[1] * (radius + 1 - i);
      sum_b += src_ptr[2] * (radius + 1 - i);
      sum_a += src_ptr[3] * (radius + 1 - i);
      sum_in_r += src_ptr[0];
      sum_in_g += src_ptr[1];
      sum_in_b += src_ptr[2];
      sum_in_a += src_ptr[3];
    }

    sp = radius;
    xp = radius;
    if (xp > wm)
      xp = wm;
    src_ptr = src + 4 * (xp + y * w); //   img.pix_ptr(xp, y);
    dst_ptr = src + y * w4;           // img.pix_ptr(0, y);
    for (x = 0; x < w; x++) {
      dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
      dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
      dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
      dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
      dst_ptr += 4;

      sum_r -= sum_out_r;
      sum_g -= sum_out_g;
      sum_b -= sum_out_b;
      sum_a -= sum_out_a;

      stack_start = sp + div - radius;
      if (stack_start >= div)
        stack_start -= div;
      stack_ptr = &stack[4 * stack_start];

      sum_out_r -= stack_ptr[0];
      sum_out_g -= stack_ptr[1];
      sum_out_b -= stack_ptr[2];
      sum_out_a -= stack_ptr[3];

      if (xp < wm) {
        src_ptr += 4;
        ++xp;
      }

      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];

      sum_in_r += src_ptr[0];
      sum_in_g += src_ptr[1];
      sum_in_b += src_ptr[2];
      sum_in_a += src_ptr[3];
      sum_r += sum_in_r;
      sum_g += sum_in_g;
      sum_b += sum_in_b;
      sum_a += sum_in_a;

      ++sp;
      if (sp >= div)
        sp = 0;
      stack_ptr = &stack[sp * 4];

      sum_out_r += stack_ptr[0];
      sum_out_g += stack_ptr[1];
      sum_out_b += stack_ptr[2];
      sum_out_a += stack_ptr[3];
      sum_in_r -= stack_ptr[0];
      sum_in_g -= stack_ptr[1];
      sum_in_b -= stack_ptr[2];
      sum_in_a -= stack_ptr[3];
    }
  }

  int minX = 0;
  int maxX = w;

  for (x = minX; x < maxX; x++) {
    sum_r = sum_g = sum_b = sum_a = sum_in_r = sum_in_g = sum_in_b = sum_in_a =
        sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

    src_ptr = src + 4 * x; // x,0
    for (i = 0; i <= radius; i++) {
      stack_ptr = &stack[i * 4];
      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];
      sum_r += src_ptr[0] * (i + 1);
      sum_g += src_ptr[1] * (i + 1);
      sum_b += src_ptr[2] * (i + 1);
      sum_a += src_ptr[3] * (i + 1);
      sum_out_r += src_ptr[0];
      sum_out_g += src_ptr[1];
      sum_out_b += src_ptr[2];
      sum_out_a += src_ptr[3];
    }
    for (i = 1; i <= radius; i++) {
      if (i <= hm)
        src_ptr += w4; // +stride

      stack_ptr = &stack[4 * (i + radius)];
      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];
      sum_r += src_ptr[0] * (radius + 1 - i);
      sum_g += src_ptr[1] * (radius + 1 - i);
      sum_b += src_ptr[2] * (radius + 1 - i);
      sum_a += src_ptr[3] * (radius + 1 - i);
      sum_in_r += src_ptr[0];
      sum_in_g += src_ptr[1];
      sum_in_b += src_ptr[2];
      sum_in_a += src_ptr[3];
    }

    sp = radius;
    yp = radius;
    if (yp > hm)
      yp = hm;
    src_ptr = src + 4 * (x + yp * w); // img.pix_ptr(x, yp);
    dst_ptr = src + 4 * x;            // img.pix_ptr(x, 0);
    for (y = 0; y < h; y++) {
      dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
      dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
      dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
      dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
      dst_ptr += w4;

      sum_r -= sum_out_r;
      sum_g -= sum_out_g;
      sum_b -= sum_out_b;
      sum_a -= sum_out_a;

      stack_start = sp + div - radius;
      if (stack_start >= div)
        stack_start -= div;
      stack_ptr = &stack[4 * stack_start];

      sum_out_r -= stack_ptr[0];
      sum_out_g -= stack_ptr[1];
      sum_out_b -= stack_ptr[2];
      sum_out_a -= stack_ptr[3];

      if (yp < hm) {
        src_ptr += w4; // stride
        ++yp;
      }

      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];

      sum_in_r += src_ptr[0];
      sum_in_g += src_ptr[1];
      sum_in_b += src_ptr[2];
      sum_in_a += src_ptr[3];
      sum_r += sum_in_r;
      sum_g += sum_in_g;
      sum_b += sum_in_b;
      sum_a += sum_in_a;

      ++sp;
      if (sp >= div)
        sp = 0;
      stack_ptr = &stack[sp * 4];

      sum_out_r += stack_ptr[0];
      sum_out_g += stack_ptr[1];
      sum_out_b += stack_ptr[2];
      sum_out_a += stack_ptr[3];
      sum_in_r -= stack_ptr[0];
      sum_in_g -= stack_ptr[1];
      sum_in_b -= stack_ptr[2];
      sum_in_a -= stack_ptr[3];
    }
  }

  cairo_surface_mark_dirty(img);
  delete[] stack;
}

void uxdevice::platform::TEXTSHADOW::invoke(const DisplayUnitContext &context) {

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

  cairo_set_source_surface(context.cr, context.IMAGE()->cairo,
                           context.AREA()->x, context.AREA()->y);
  cairo_mask_surface(context.cr, context.IMAGE()->cairo, context.AREA()->x,
                     context.AREA()->y);
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
  cairo = cairo_image_surface_create_from_png(context.IMAGE()->fileName.data());
  bLoaded = true;
}

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
