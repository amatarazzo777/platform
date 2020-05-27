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
\brief The routine is the main rendering thread. The thread runs
at specific intervals. Locks are placed on the surface and
rectangle list. The surface may change due to user resizing the gui
window so a spin flag is used to accommodate the functionality. That is
drawing cannot occur on the graphical while the surface is being resized.
*/
void uxdevice::platform::renderLoop(void) {
  while (bProcessing) {

    // measure processing time
    std::chrono::system_clock::time_point start =
        std::chrono::high_resolution_clock::now();

    // surfacePrime checks to see if the surface exists.
    // if so, the two possible work flows are painting
    // background rectangles that are cause by the user resizing the
    // window to a greater value. These values are inserted as part
    // of the paint event. As well, the underlying surface may
    // need to be resized. This function acquires locks on these
    // small lists for the multi-threaded necessity.
    // searches for unready and syncs display context
    if (context.surfacePrime()) {
      context.render();

      // blits the surface and xcb connection.
      context.flush();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff = end - start;

    size_t sleepAmount = 1000 / framesPerSecond - diff.count();

    std::this_thread::sleep_for(std::chrono::milliseconds(sleepAmount));
  }
}

/*
\brief the dispatch routine is invoked by the messageLoop.
If default
 * handling is to be supplied, the method invokes the
necessary operation.

*/
void uxdevice::platform::dispatchEvent(const event &evt) {

  switch (evt.type) {
  case eventType::none:
    break;
  case eventType::paint: {
    context.state(true, evt.x, evt.y, evt.w, evt.h);
  } break;
  case eventType::resize:
    context.resizeSurface(evt.w, evt.h);
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

  case eventType::focus:
    break;
  case eventType::blur:
    break;
  case eventType::mouseenter:
    break;
  case eventType::click:
    break;
  case eventType::dblclick:
    break;
  case eventType::contextmenu:
    break;
  case eventType::mouseleave:
    break;
  }
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
void uxdevice::platform::startProcessing(int _fps) {
  // setup the event dispatcher
  eventHandler ev = std::bind(&uxdevice::platform::dispatchEvent, this,
                              std::placeholders::_1);
  framesPerSecond = _fps;

  std::thread thrRenderer([=]() {
    bProcessing = true;
    renderLoop();
  });

  std::thread thrMessageQueue([=]() {
    bProcessing = true;
    messageLoop();
  });

  thrRenderer.detach();
  thrMessageQueue.detach();
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

\brief The function is invoked when an event occurs. Normally this occurs
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
                             const errorHandler &fn)
    : fnError(fn), fnEvents(evtDispatcher) {

// initialize private members
#if defined(__linux__)

#elif defined(_WIN64)

  CoInitialize(NULL);

#endif
}
/**
  \internal
  \brief terminates the xserver connection
  and frees resources.
*/
uxdevice::platform::~platform() {
  closeWindow();

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
                                    const unsigned short height,
                                    Paint background) {

  context.windowWidth = width;
  context.windowHeight = height;
  context.brush = background;

#if defined(__linux__)
  // this open provides interoperability between xcb and xwindows
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

  // create xcb surface
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

  // create cairo context
  context.cr = cairo_create(context.xcbSurface);
  if (!context.cr) {
    closeWindow();
    std::stringstream sError;
    sError << "ERR_CAIRO "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  /* Map the window on the screen and flush*/
  xcb_map_window(context.connection, context.window);
  xcb_flush(context.connection);
  context.windowOpen = true;

  cairo_surface_flush(context.xcbSurface);

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
  \brief Initialize the directx video system.

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
  while (bProcessing && (xcbEvent = xcb_wait_for_event(context.connection))) {
    xcbEvents.push_back(xcbEvent);

    // qt5 does this, it queues all of the input messages at once.
    // this makes the processing of painting and reading input faster.
    while (bProcessing &&
           (xcbEvent = xcb_poll_for_queued_event(context.connection)))
      xcbEvents.push_back(xcbEvent);

    while (!xcbEvents.empty()) {
      xcbEvent = xcbEvents.front();
      switch (xcbEvent->response_type & ~0x80) {
      case XCB_MOTION_NOTIFY: {
        xcb_motion_notify_event_t *motion =
            (xcb_motion_notify_event_t *)xcbEvent;
        dispatchEvent(event{
            eventType::mousemove,
            (short)motion->event_x,
            (short)motion->event_y,
        });
      } break;
      case XCB_BUTTON_PRESS: {
        xcb_button_press_event_t *bp = (xcb_button_press_event_t *)xcbEvent;
        if (bp->detail == XCB_BUTTON_INDEX_4 ||
            bp->detail == XCB_BUTTON_INDEX_5) {
          dispatchEvent(
              event{eventType::wheel, (short)bp->event_x, (short)bp->event_y,
                    (short)(bp->detail == XCB_BUTTON_INDEX_4 ? 1 : -1)});

        } else {
          dispatchEvent(event{eventType::mousedown, (short)bp->event_x,
                              (short)bp->event_y, (short)bp->detail});
        }
      } break;
      case XCB_BUTTON_RELEASE: {
        xcb_button_release_event_t *br = (xcb_button_release_event_t *)xcbEvent;
        // ignore button 4 and 5 which are wheel events.
        if (br->detail != XCB_BUTTON_INDEX_4 &&
            br->detail != XCB_BUTTON_INDEX_5)
          dispatchEvent(event{eventType::mouseup, (short)br->event_x,
                              (short)br->event_y, (short)br->detail});
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
          if (XLookupString(&keyEvent, buf.data(), buf.size(), nullptr,
                            nullptr))
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

        dispatchEvent(event{eventType::paint, (short)eev->x, (short)eev->y,
                            (short)eev->width, (short)eev->height});

      } break;
      case XCB_CONFIGURE_NOTIFY: {
        const xcb_configure_notify_event_t *cfgEvent =
            (const xcb_configure_notify_event_t *)xcbEvent;

        if (cfgEvent->window == context.window) {
          dispatchEvent(event{eventType::resize, (short)cfgEvent->width,
                              (short)cfgEvent->height});
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
\brief API interface, just data is passed to objects. Objects are
dynamically allocated as classes derived from a unit base. Mutex is used one
display list to not get in the way of the rendering loop,

*/

/**
\brief clears the display list
*/

void uxdevice::platform::clear(void) {
  DL_SPIN;
  context.clear();
  DL.clear();
  DL_CLEAR;
}

void uxdevice::platform::antiAlias(antialias antialias) {
  DL_SPIN;
  DL.push_back(make_shared<ANTIALIAS>(antialias));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<ANTIALIAS>(DL.back()));
  DL_CLEAR;
}

/**
\brief sets the text
*/
void uxdevice::platform::text(const std::string &s) {
  DL_SPIN;
  DL.push_back(make_shared<STRING>(s));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<STRING>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::text(const std::stringstream &s) {
  DL_SPIN;
  DL.push_back(make_shared<STRING>(s.str()));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<STRING>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::image(const std::string &s) {
  DL_SPIN;
  DL.push_back(make_shared<IMAGE>(s));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<IMAGE>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::pen(const Paint &p) {
  DL_SPIN;
  DL.push_back(make_shared<PEN>(p));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<PEN>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::pen(u_int32_t c) {
  DL_SPIN;
  DL.push_back(make_shared<PEN>(c));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<PEN>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::pen(const string &c) {
  DL_SPIN;
  DL.push_back(make_shared<PEN>(c));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<PEN>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::pen(const std::string &c, double w, double h) {
  DL_SPIN;
  DL.push_back(make_shared<PEN>(c, w, h));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<PEN>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::pen(double _r, double _g, double _b) {
  DL_SPIN;
  DL.push_back(make_shared<PEN>(_r, _g, _b));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<PEN>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::pen(double _r, double _g, double _b, double _a) {
  DL_SPIN;
  DL.push_back(make_shared<PEN>(_r, _g, _b, _a));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<PEN>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::pen(double x0, double y0, double x1, double y1,
                             const ColorStops &cs) {
  DL_SPIN;
  DL.push_back(make_shared<PEN>(x0, y0, x1, y1, cs));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<PEN>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::pen(double cx0, double cy0, double radius0, double cx1,
                             double cy1, double radius1, const ColorStops &cs) {
  DL_SPIN;
  DL.push_back(make_shared<PEN>(cx0, cy0, radius0, cx1, cy1, radius1, cs));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<PEN>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::surfaceBrush(Paint &b) { context.surfaceBrush(b); }
/**
\brief
*/
void uxdevice::platform::background(const Paint &p) {
  DL_SPIN;
  DL.push_back(make_shared<BACKGROUND>(p));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<BACKGROUND>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::background(u_int32_t c) {
  DL_SPIN;
  DL.push_back(make_shared<BACKGROUND>(c));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<BACKGROUND>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::background(const string &c) {
  DL_SPIN;
  DL.push_back(make_shared<BACKGROUND>(c));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<BACKGROUND>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::background(const std::string &c, double w, double h) {
  DL_SPIN;
  DL.push_back(make_shared<BACKGROUND>(c, w, h));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<BACKGROUND>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::background(double _r, double _g, double _b) {
  DL_SPIN;
  DL.push_back(make_shared<BACKGROUND>(_r, _g, _b));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<BACKGROUND>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::background(double _r, double _g, double _b,
                                    double _a) {
  DL_SPIN;
  DL.push_back(make_shared<BACKGROUND>(_r, _g, _b, _a));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<BACKGROUND>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::background(double x0, double y0, double x1, double y1,
                                    const ColorStops &cs) {
  DL_SPIN;
  DL.push_back(make_shared<BACKGROUND>(x0, y0, x1, y1, cs));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<BACKGROUND>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::background(double cx0, double cy0, double radius0,
                                    double cx1, double cy1, double radius1,
                                    const ColorStops &cs) {
  DL_SPIN;
  DL.push_back(
      make_shared<BACKGROUND>(cx0, cy0, radius0, cx1, cy1, radius1, cs));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<BACKGROUND>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::textAlignment(alignment aln) {
  DL_SPIN;
  DL.push_back(make_shared<ALIGN>(aln));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<ALIGN>(DL.back()));
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::textOutline(const Paint &p, double dWidth) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTOUTLINE>(p, dWidth));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTOUTLINE>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textOutline(u_int32_t c, double dWidth) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTOUTLINE>(c, dWidth));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTOUTLINE>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::textOutline(const string &c, double dWidth) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTOUTLINE>(c, dWidth));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTOUTLINE>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textOutline(const std::string &c, double w, double h,
                                     double dWidth) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTOUTLINE>(c, w, h, dWidth));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTOUTLINE>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::textOutline(double _r, double _g, double _b,
                                     double dWidth) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTOUTLINE>(_r, _g, _b, dWidth));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTOUTLINE>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::textOutline(double _r, double _g, double _b, double _a,
                                     double dWidth) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTOUTLINE>(_r, _g, _b, _a, dWidth));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTOUTLINE>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textOutline(double x0, double y0, double x1, double y1,
                                     const ColorStops &cs, double dWidth) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTOUTLINE>(x0, y0, x1, y1, cs, dWidth));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTOUTLINE>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::textOutline(double cx0, double cy0, double radius0,
                                     double cx1, double cy1, double radius1,
                                     const ColorStops &cs, double dWidth) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTOUTLINE>(cx0, cy0, radius0, cx1, cy1, radius1,
                                        cs, dWidth));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTOUTLINE>(DL.back()));
  DL_CLEAR;
}

/**
\brief clears the current text outline from the context.
*/
void uxdevice::platform::textOutlineNone(void) {
  DL_SPIN;
  DL.push_back(
      make_shared<CLEARUNIT>((void **)&context.currentUnits.textoutline));
  DL.back()->invoke(context);
  DL_CLEAR;
}

void uxdevice::platform::textFill(const Paint &p) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTFILL>(p));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTFILL>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textFill(u_int32_t c) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTFILL>(c));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTFILL>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textFill(const string &c) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTFILL>(c));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTFILL>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textFill(const string &c, double w, double h) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTFILL>(c, w, h));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTFILL>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textFill(double _r, double _g, double _b) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTFILL>(_r, _g, _b));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTFILL>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textFill(double _r, double _g, double _b, double _a) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTFILL>(_r, _g, _b, _a));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTFILL>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textFill(double x0, double y0, double x1, double y1,
                                  const ColorStops &cs) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTFILL>(x0, y0, x1, y1, cs));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTFILL>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textFill(double cx0, double cy0, double radius0,
                                  double cx1, double cy1, double radius1,
                                  const ColorStops &cs) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTFILL>(cx0, cy0, radius0, cx1, cy1, radius1, cs));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTFILL>(DL.back()));
  DL_CLEAR;
}

/**
\brief clears the current text fill from the context.
*/
void uxdevice::platform::textFillNone(void) {
  DL_SPIN;
  DL.push_back(make_shared<CLEARUNIT>((void **)&context.currentUnits.textfill));
  DL.back()->invoke(context);
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::textShadow(const Paint &p, int r, double xOffset,
                                    double yOffset) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTSHADOW>(p, r, xOffset, yOffset));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTSHADOW>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textShadow(u_int32_t c, int r, double xOffset,
                                    double yOffset) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTSHADOW>(c, r, xOffset, yOffset));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTSHADOW>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::textShadow(const string &c, int r, double xOffset,
                                    double yOffset) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTSHADOW>(c, r, xOffset, yOffset));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTSHADOW>(DL.back()));
  DL_CLEAR;
}
void uxdevice::platform::textShadow(const std::string &c, double w, double h,
                                    int r, double xOffset, double yOffset) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTSHADOW>(c, w, h, r, xOffset, yOffset));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTSHADOW>(DL.back()));
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::textShadow(double _r, double _g, double _b, int r,
                                    double xOffset, double yOffset) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTSHADOW>(_r, _g, _b, r, xOffset, yOffset));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTSHADOW>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::textShadow(double _r, double _g, double _b, double _a,
                                    int r, double xOffset, double yOffset) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTSHADOW>(_r, _g, _b, _a, r, xOffset, yOffset));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTSHADOW>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::textShadow(double x0, double y0, double x1, double y1,
                                    const ColorStops &cs, int r, double xOffset,
                                    double yOffset) {
  DL_SPIN;
  DL.push_back(
      make_shared<TEXTSHADOW>(x0, y0, x1, y1, cs, r, xOffset, yOffset));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTSHADOW>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::textShadow(double cx0, double cy0, double radius0,
                                    double cx1, double cy1, double radius1,
                                    const ColorStops &cs, int r, double xOffset,
                                    double yOffset) {
  DL_SPIN;
  DL.push_back(make_shared<TEXTSHADOW>(cx0, cy0, radius0, cx1, cy1, radius1, cs,
                                       r, xOffset, yOffset));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<TEXTSHADOW>(DL.back()));
  DL_CLEAR;
}

/**
\brief clears the current text fill from the context.
*/
void uxdevice::platform::textShadowNone(void) {
  DL_SPIN;
  DL.push_back(
      make_shared<CLEARUNIT>((void **)&context.currentUnits.textshadow));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::font(const std::string &s) {
  DL_SPIN;
  DL.push_back(make_shared<FONT>(s));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<FONT>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::area(double x, double y, double w, double h) {
  DL_SPIN;
  DL.push_back(make_shared<AREA>(areaType::rectangle, x, y, w, h));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<AREA>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::area(double x, double y, double w, double h, double rx,
                              double ry) {
  DL_SPIN;
  DL.push_back(make_shared<AREA>(x, y, w, h, rx, ry));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<AREA>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::areaCircle(double x, double y, double d) {
  DL_SPIN;
  DL.push_back(make_shared<AREA>(x, y, d / 2));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<AREA>(DL.back()));
  DL_CLEAR;
}

void uxdevice::platform::areaEllipse(double cx, double cy, double rx,
                                     double ry) {
  DL_SPIN;
  DL.push_back(make_shared<AREA>(areaType::ellipse, cx, cy, rx, ry));
  DL.back()->invoke(context);
  context.setUnit(std::dynamic_pointer_cast<AREA>(DL.back()));

  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::drawText(void) {
  DL_SPIN;
  DL.push_back(make_shared<DRAWTEXT>());
  DL.back()->invoke(context);
  context.addDrawable(std::dynamic_pointer_cast<DrawingOutput>(DL.back()));
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::drawImage(void) {
  DL_SPIN;

  DL.push_back(make_shared<DRAWIMAGE>());
  DL.back()->invoke(context);
  context.addDrawable(std::dynamic_pointer_cast<DrawingOutput>(DL.back()));
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::drawArea(void) {
  DL_SPIN;

  DL.push_back(std::make_shared<DRAWAREA>());
  DL.back()->invoke(context);
  context.addDrawable(std::dynamic_pointer_cast<DrawingOutput>(DL.back()));
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::save(void) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_save, _1);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::restore(void) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_restore, _1);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

void uxdevice::platform::push(content c) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func;
  if (c == content::all) {
    func = std::bind(cairo_push_group, _1);
  } else {
    func = std::bind(cairo_push_group_with_content, _1,
                     static_cast<cairo_content_t>(c));
  }
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

void uxdevice::platform::pop(bool bToSource) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func;
  if (bToSource) {
    func = std::bind(cairo_pop_group_to_source, _1);
  } else {
    func = std::bind(cairo_pop_group, _1);
  }
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::translate(double x, double y) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_translate, _1, x, y);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::rotate(double angle) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_rotate, _1, angle);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::scale(double x, double y) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_scale, _1, x, y);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::transform(const Matrix &m) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_transform, _1, &m._matrix);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::matrix(const Matrix &m) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_set_matrix, _1, &m._matrix);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::identity(void) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_identity_matrix, _1);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::device(double &x, double &y) {
  using namespace std::placeholders;
  DL_SPIN;
  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x;
    double _y = y;
    cairo_user_to_device(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  CAIRO_FUNCTION func = std::bind(fn, _1, x, y);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::deviceDistance(double &x, double &y) {
  using namespace std::placeholders;
  DL_SPIN;
  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x;
    double _y = y;
    cairo_user_to_device_distance(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  CAIRO_FUNCTION func = std::bind(fn, _1, x, y);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}
/**
\brief
*/
void uxdevice::platform::user(double &x, double &y) {
  using namespace std::placeholders;
  DL_SPIN;
  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x, _y = y;
    cairo_device_to_user(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  CAIRO_FUNCTION func = std::bind(fn, _1, x, y);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::userDistance(double &x, double &y) {
  using namespace std::placeholders;
  DL_SPIN;
  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x, _y = y;
    cairo_device_to_user_distance(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  CAIRO_FUNCTION func = std::bind(fn, _1, x, y);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::cap(lineCap c) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_OPTION func =
      std::bind(cairo_set_line_cap, _1, static_cast<cairo_line_cap_t>(c));
  DL.push_back(make_shared<OPTION_FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::join(lineJoin j) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func =
      std::bind(cairo_set_line_join, _1, static_cast<cairo_line_join_t>(j));
  DL.push_back(make_shared<OPTION_FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::lineWidth(double dWidth) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_set_line_width, _1, dWidth);
  DL.push_back(make_shared<OPTION_FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::miterLimit(double dLimit) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_set_miter_limit, _1, dLimit);
  DL.push_back(make_shared<OPTION_FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::dashes(const std::vector<double> &dashes,
                                double offset) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func =
      std::bind(cairo_set_dash, _1, dashes.data(), dashes.size(), offset);
  DL.push_back(make_shared<OPTION_FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::tollerance(double _t) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_set_tolerance, _1, _t);
  DL.push_back(make_shared<OPTION_FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::op(op_t _op) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func =
      std::bind(cairo_set_operator, _1, static_cast<cairo_operator_t>(_op));
  DL.push_back(make_shared<OPTION_FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::source(Paint &p) {
  using namespace std::placeholders;
  DL_SPIN;
  auto fn = [](cairo_t *cr, Paint &p) { p.emit(cr); };
  CAIRO_FUNCTION func = std::bind(fn, _1, p);
  DL.push_back(make_shared<OPTION_FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::arc(double xc, double yc, double radius, double angle1,
                             double angle2, bool bNegative) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func;
  if (bNegative) {
    func = std::bind(cairo_arc_negative, _1, xc, yc, radius, angle1, angle2);
  } else {
    func = std::bind(cairo_arc, _1, xc, yc, radius, angle1, angle2);
  }
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::curve(double x1, double y1, double x2, double y2,
                               double x3, double y3, bool bRelative) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func;
  if (bRelative) {
    func = std::bind(cairo_rel_curve_to, _1, x1, y1, x2, y2, x3, y3);
  } else {
    func = std::bind(cairo_curve_to, _1, x1, y1, x2, y2, x3, y3);
  }
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::line(double x, double y, bool bRelative) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func;
  if (bRelative) {
    func = std::bind(cairo_rel_line_to, _1, x, y);
  } else {
    func = std::bind(cairo_line_to, _1, x, y);
  }
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::stroke(bool bPreserve) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func;
  if (bPreserve) {
    func = std::bind(cairo_stroke_preserve, _1);
  } else {
    func = std::bind(cairo_stroke, _1);
  }
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::move(double x, double y, bool bRelative) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func;
  if (bRelative) {
    func = std::bind(cairo_rel_move_to, _1, x, y);
  } else {
    func = std::bind(cairo_move_to, _1, x, y);
  }
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/**
\brief
*/
void uxdevice::platform::rectangle(double x, double y, double width,
                                   double height) {
  using namespace std::placeholders;
  DL_SPIN;
  CAIRO_FUNCTION func = std::bind(cairo_rectangle, _1, x, y, width, height);
  DL.push_back(make_shared<FUNCTION>(func));
  DL.back()->invoke(context);
  DL_CLEAR;
}

/***************************************************************************/

/**
  \internal
  \brief the function draws the cursor.
  */
void uxdevice::platform::drawCaret(const int x, const int y, const int h) {}

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
