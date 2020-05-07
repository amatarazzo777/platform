/**
\file uxdevice.cpp

\author Anthony Matarazzo

\date 3/26/20
\version 1.0


*/

/**
\brief rendering and platform services.

*/
#include "uxdevice.hpp"

using namespace std;
using namespace uxdevice;

/**
\internal
\brief The routine is the main rendering thread. The thread runs
at specific intervals. Locks are placed on the surface and
rectangle list. The surface may change due to user resizing.

*/
void uxdevice::platform::renderLoop(void) {
  while (bProcessing) {

    // measure processing time
    static std::chrono::system_clock::time_point lastTime =
        std::chrono::high_resolution_clock::now();
    std::chrono::system_clock::time_point start =
        std::chrono::high_resolution_clock::now();

    {
      std::lock_guard<std::mutex> lockSurface(context.surface_mutex);
      if (context.xcbSurface) {

        std::lock_guard<std::mutex> lock(dirtyRectangles_mutex);

        // the process rendering creates the graphic output buffer off screen.
        // only items that are on screen are processed.
        processRendering();

        // update the areas needed from the off screen buffer
        cairo_t *cr = cairo_create(context.xcbSurface);
        cairo_push_group(cr);

        for (auto n : dirtyRectangles)
          copyRectangle(cr, n);

        cairo_pop_group_to_source(cr);
        cairo_paint(cr);
        cairo_destroy(cr);

        xcb_flush(context.connection);

        // all work has been accomplished, clear the dirty list.
        dirtyRectangles.clear();
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff = end - start;
    cout << diff.count() << std::endl << std::flush;

    lastTime = std::chrono::high_resolution_clock::now();
    size_t sleepAmount = 1000 / framesPerSecond;

    std::this_thread::sleep_for(std::chrono::milliseconds(sleepAmount));
  }
}

/**
\internal
\brief The routine checks the display list for work.
If any items are on screen, it is rendered to an offscreen image.
*/
void uxdevice::platform::processRendering(void) {
  std::lock_guard<std::mutex> lock(DL_mutex);

  // need to add clip logic using the cairo
  if (dirtyRectangles.size() == 0 && !bRenderWork)
    return;

  if (context.preclear) {
    cairo_set_source_rgb(context.cr, 1.0, 1.0, 1.0);
    cairo_paint(context.cr);
    context.preclear = false;
    dirtyRectangles.push_back(
        {0, 0, (double)context.windowWidth, (double)context.windowHeight});
  }

  for (auto &n : DL) {
    if (n->index() == IDX(CLEAROPTION)) {
      context.clear(dynamic_cast<CLEAROPTION *>(n.get())->option);
      continue;
    }
    context.set(n.get());
    n->invoke(context);

    cairo_status_t status = cairo_status(context.cr);
    if (status) {
      std::stringstream sError;
      sError << "ERR_CAIRO render loop "
             << "  " << __FILE__ << " " << __func__ << " "
             << cairo_status_to_string(status);
      throw std::runtime_error(sError.str());
    }
    if (n->renderWork())
      postDirty_RenderInternal(context.AREA().x, context.AREA().y,
                                 context.AREA().w, context.AREA().h);
  }
  bRenderWork = false;
}

/**
\internal
\brief The routine copys a rectangle from offscreen to
surface.
*/
void uxdevice::platform::copyRectangle(cairo_t *cr, const bounds &_area) {

  cairo_rectangle(cr, _area.x, _area.y, _area.w, _area.h);
  cairo_set_source_surface(cr, context.offScreen, 0, 0);
  cairo_fill(cr);
}

/**
\internal
\brief The routine posts a direct rectangle to the vector
*/
void uxdevice::platform::postDirty(double x, double y, double w, double h) {
  std::lock_guard<std::mutex> lock(dirtyRectangles_mutex);
  postDirty_RenderInternal(x,y,w,h);
}

void uxdevice::platform::postDirty_RenderInternal(double x, double y, double w, double h) {
  dirtyRectangles.push_back(bounds{x, y, w, h});
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
    postDirty(evt.x, evt.y, evt.w, evt.h);
  } break;
  case eventType::resize:
    resize(evt.w, evt.h);
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
                             const errorHandler &fn) {
  fnEvents = evtDispatcher;
  fnError = fn;
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
  std::lock_guard<std::mutex> lockSurface(context.surface_mutex);
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
                                    const unsigned short height) {
  std::lock_guard<std::mutex> lockSurface(context.surface_mutex);
  context.windowWidth = width;
  context.windowHeight = height;

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

  // create the off screen image buffer
  cairo_format_t format = CAIRO_FORMAT_ARGB32;
  context.offScreen = cairo_image_surface_create(format, context.windowWidth,
                                                 context.windowHeight);
  if (!context.offScreen) {
    closeWindow();
    std::stringstream sError;
    sError << "ERR_CAIRO "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }
  // create cairo context
  context.cr = cairo_create(context.offScreen);
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

  context.preclear = true;
  postDirty(0, 0, context.windowWidth, context.windowHeight);

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
  short int newWidth;
  short int newHeight;
  bool bRequestResize = false;

  // is window open?
  while (bProcessing && !context.connection)
      std::this_thread::sleep_for(std::chrono::milliseconds(60));
  if(!context.connection)
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
  while (bProcessing && (xcbEvent = xcb_wait_for_event(context.connection))) {
    switch (xcbEvent->response_type & ~0x80) {
    case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)xcbEvent;
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
      if (br->detail != XCB_BUTTON_INDEX_4 && br->detail != XCB_BUTTON_INDEX_5)
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

        dispatchEvent(event{eventType::paint, (short)eev->x, (short)eev->y,
                            (short)eev->width, (short)eev->height});
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
    case XCB_CLIENT_MESSAGE: {
      if ((*(xcb_client_message_event_t *)xcbEvent).data.data32[0] ==
          (*reply2).atom) {
        bProcessing = false;
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
void uxdevice::platform::clear(void) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.clear();
  context.currentUnit = DisplayUnitContext::contextArray();
  context.preclear = true;
}

void uxdevice::platform::antiAlias(antialias antialias) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<ANTIALIAS>(antialias));
}

/**
\brief sets the text
*/
void uxdevice::platform::text(const std::string &s) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<STRING>(s));
}
/**
\brief
*/
void uxdevice::platform::text(const std::stringstream &s) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<STRING>(s.str()));
}
/**
\brief
*/
void uxdevice::platform::image(const std::string &s) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<IMAGE>(s));
}
/**
\brief
*/
void uxdevice::platform::pen(const Paint &p) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<PEN>(p));
}
void uxdevice::platform::pen(u_int32_t c) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<PEN>(c));
}

void uxdevice::platform::pen(const string &c) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<PEN>(c));
}
void uxdevice::platform::pen(const std::string &c, double w, double h) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<PEN>(c, w, h));
}

void uxdevice::platform::pen(double _r, double _g, double _b) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<PEN>(_r, _g, _b));
}
void uxdevice::platform::pen(double _r, double _g, double _b, double _a) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<PEN>(_r, _g, _b, _a));
}
void uxdevice::platform::pen(double x0, double y0, double x1, double y1,
                             const ColorStops &cs) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<PEN>(x0, y0, x1, y1, cs));
}
void uxdevice::platform::pen(double cx0, double cy0, double radius0, double cx1,
                             double cy1, double radius1, const ColorStops &cs) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<PEN>(cx0, cy0, radius0, cx1, cy1, radius1, cs));
}

/**
\brief
*/
void uxdevice::platform::background(const Paint &p) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<BACKGROUND>(p));
}
void uxdevice::platform::background(u_int32_t c) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<BACKGROUND>(c));
}
void uxdevice::platform::background(const string &c) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<BACKGROUND>(c));
}
void uxdevice::platform::background(const std::string &c, double w, double h) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<BACKGROUND>(c, w, h));
}

void uxdevice::platform::background(double _r, double _g, double _b) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<BACKGROUND>(_r, _g, _b));
}
void uxdevice::platform::background(double _r, double _g, double _b,
                                    double _a) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<BACKGROUND>(_r, _g, _b, _a));
}
void uxdevice::platform::background(double x0, double y0, double x1, double y1,
                                    const ColorStops &cs) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<BACKGROUND>(x0, y0, x1, y1, cs));
}
void uxdevice::platform::background(double cx0, double cy0, double radius0,
                                    double cx1, double cy1, double radius1,
                                    const ColorStops &cs) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(
      make_unique<BACKGROUND>(cx0, cy0, radius0, cx1, cy1, radius1, cs));
}
/**
\brief
*/
void uxdevice::platform::textAlignment(alignment aln) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<ALIGN>(aln));
}

/**
\brief
*/
void uxdevice::platform::textOutline(const Paint &p, double dWidth) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTOUTLINE>(p, dWidth));
}
void uxdevice::platform::textOutline(u_int32_t c, double dWidth) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTOUTLINE>(c, dWidth));
}
/**
\brief
*/
void uxdevice::platform::textOutline(const string &c, double dWidth) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTOUTLINE>(c, dWidth));
}
void uxdevice::platform::textOutline(const std::string &c, double w, double h,
                                     double dWidth) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTOUTLINE>(c, w, h, dWidth));
}
/**
\brief
*/
void uxdevice::platform::textOutline(double _r, double _g, double _b,
                                     double dWidth) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTOUTLINE>(_r, _g, _b, dWidth));
}
/**
\brief
*/
void uxdevice::platform::textOutline(double _r, double _g, double _b, double _a,
                                     double dWidth) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTOUTLINE>(_r, _g, _b, _a, dWidth));
}
void uxdevice::platform::textOutline(double x0, double y0, double x1, double y1,
                                     const ColorStops &cs, double dWidth) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTOUTLINE>(x0, y0, x1, y1, cs, dWidth));
}

void uxdevice::platform::textOutline(double cx0, double cy0, double radius0,
                                     double cx1, double cy1, double radius1,
                                     const ColorStops &cs, double dWidth) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTOUTLINE>(cx0, cy0, radius0, cx1, cy1, radius1,
                                        cs, dWidth));
}

/**
\brief clears the current text outline from the context.
*/
void uxdevice::platform::textOutlineNone(void) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<CLEAROPTION>(IDX(TEXTOUTLINE)));
}

void uxdevice::platform::textFill(const Paint &p) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTFILL>(p));
}
void uxdevice::platform::textFill(u_int32_t c) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTFILL>(c));
}
void uxdevice::platform::textFill(const string &c) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTFILL>(c));
}
void uxdevice::platform::textFill(const string &c, double w, double h) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTFILL>(c, w, h));
}
void uxdevice::platform::textFill(double _r, double _g, double _b) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTFILL>(_r, _g, _b));
}
void uxdevice::platform::textFill(double _r, double _g, double _b, double _a) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTFILL>(_r, _g, _b, _a));
}
void uxdevice::platform::textFill(double x0, double y0, double x1, double y1,
                                  const ColorStops &cs) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTFILL>(x0, y0, x1, y1, cs));
}
void uxdevice::platform::textFill(double cx0, double cy0, double radius0,
                                  double cx1, double cy1, double radius1,
                                  const ColorStops &cs) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTFILL>(cx0, cy0, radius0, cx1, cy1, radius1, cs));
}

/**
\brief clears the current text fill from the context.
*/
void uxdevice::platform::textFillNone(void) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<CLEAROPTION>(IDX(TEXTFILL)));
}
/**
\brief
*/
void uxdevice::platform::textShadow(const Paint &p, int r, double xOffset,
                                    double yOffset) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTSHADOW>(p, r, xOffset, yOffset));
}
void uxdevice::platform::textShadow(u_int32_t c, int r, double xOffset,
                                    double yOffset) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTSHADOW>(c, r, xOffset, yOffset));
}
/**
\brief
*/
void uxdevice::platform::textShadow(const string &c, int r, double xOffset,
                                    double yOffset) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTSHADOW>(c, r, xOffset, yOffset));
}
void uxdevice::platform::textShadow(const std::string &c, double w, double h,
                                    int r, double xOffset, double yOffset) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTSHADOW>(c, w, h, r, xOffset, yOffset));
}

/**
\brief
*/
void uxdevice::platform::textShadow(double _r, double _g, double _b, int r,
                                    double xOffset, double yOffset) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTSHADOW>(_r, _g, _b, r, xOffset, yOffset));
}
/**
\brief
*/
void uxdevice::platform::textShadow(double _r, double _g, double _b, double _a,
                                    int r, double xOffset, double yOffset) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTSHADOW>(_r, _g, _b, _a, r, xOffset, yOffset));
}

void uxdevice::platform::textShadow(double x0, double y0, double x1, double y1,
                                    const ColorStops &cs, int r, double xOffset,
                                    double yOffset) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(
      make_unique<TEXTSHADOW>(x0, y0, x1, y1, cs, r, xOffset, yOffset));
}

void uxdevice::platform::textShadow(double cx0, double cy0, double radius0,
                                    double cx1, double cy1, double radius1,
                                    const ColorStops &cs, int r, double xOffset,
                                    double yOffset) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<TEXTSHADOW>(cx0, cy0, radius0, cx1, cy1, radius1, cs,
                                       r, xOffset, yOffset));
}

/**
\brief clears the current text fill from the context.
*/
void uxdevice::platform::textShadowNone(void) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<CLEAROPTION>(IDX(TEXTSHADOW)));
}

/**
\brief
*/
void uxdevice::platform::font(const std::string &s) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<FONT>(s));
}
/**
\brief
*/
void uxdevice::platform::area(double x, double y, double w, double h) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<AREA>(areaType::rectangle, x, y, w, h));
}
void uxdevice::platform::area(double x, double y, double w, double h, double rx,
                              double ry) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<AREA>(x, y, w, h, rx, ry));
}
void uxdevice::platform::areaCircle(double x, double y, double d) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<AREA>(x, y, d / 2));
}
void uxdevice::platform::areaEllipse(double cx, double cy, double rx,
                                     double ry) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<AREA>(areaType::ellipse, cx, cy, rx, ry));
}

/**
\brief
*/
void uxdevice::platform::drawText(void) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<DRAWTEXT>());
  bRenderWork = true;
#if 0
  std::lock_guard<std::mutex> lk(RenderWork_mutex);
  bRenderWork_condition.notify_all();
#endif // 0
}
/**
\brief
*/
void uxdevice::platform::drawImage(void) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<DRAWIMAGE>());
  bRenderWork = true;
#if 0
  std::lock_guard<std::mutex> lk(RenderWork_mutex);
  bRenderWork_condition.notify_all();
#endif // 0
}
/**
\brief
*/
void uxdevice::platform::drawArea(void) {
  std::lock_guard<std::mutex> lock(DL_mutex);
  DL.push_back(make_unique<DRAWAREA>());
  bRenderWork = true;
#if 0
  std::lock_guard<std::mutex> lk(RenderWork_mutex);
  bRenderWork_condition.notify_all();
#endif
}

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
Patterns can be a image file, a description of a linear, actual parameters of
linear, a description of a radial, the actual radial parameters stored.
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
  _image = platform::readImage(_description, _width, _height);

  if (_image) {
    _width = cairo_image_surface_get_width(_image);
    _height = cairo_image_surface_get_height(_image);
    _pattern = cairo_pattern_create_for_surface(_image);
    cairo_pattern_set_extend(_pattern, CAIRO_EXTEND_REPEAT);
    cairo_pattern_set_filter(_pattern, CAIRO_FILTER_FAST);
    _type = paintType::pattern;
    _bLoaded = true;

    // determine if the description is another form such as a gradient or color.
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

radial-gradient(ellipse at center, #1e5799 0%,#2989d8 50%,#207cca 51%,#2989d8
51%,#7db9e8 100%);

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

/**
\brief
*/
void uxdevice::platform::save(void) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_save, _1);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::restore(void) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_restore, _1);
  DL.push_back(make_unique<FUNCTION>(func));
}

void uxdevice::platform::push(content c) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func;
  if (c == content::all) {
    func = std::bind(cairo_push_group, _1);
  } else {
    func = std::bind(cairo_push_group_with_content, _1,
                     static_cast<cairo_content_t>(c));
  }
  DL.push_back(make_unique<FUNCTION>(func));
}

void uxdevice::platform::pop(bool bToSource) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func;
  if (bToSource) {
    func = std::bind(cairo_pop_group_to_source, _1);
  } else {
    func = std::bind(cairo_pop_group, _1);
  }
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::translate(double x, double y) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_translate, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::rotate(double angle) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_rotate, _1, angle);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::scale(double x, double y) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_scale, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::transform(const Matrix &m) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_transform, _1, &m._matrix);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::matrix(const Matrix &m) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_set_matrix, _1, &m._matrix);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::identity(void) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_identity_matrix, _1);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::device(double &x, double &y) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x;
    double _y = y;
    cairo_user_to_device(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  CAIRO_FUNCTION func = std::bind(fn, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::deviceDistance(double &x, double &y) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x;
    double _y = y;
    cairo_user_to_device_distance(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  CAIRO_FUNCTION func = std::bind(fn, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}
/**
\brief
*/
void uxdevice::platform::user(double &x, double &y) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x, _y = y;
    cairo_device_to_user(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  CAIRO_FUNCTION func = std::bind(fn, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::userDistance(double &x, double &y) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  auto fn = [](cairo_t *cr, double &x, double &y) {
    double _x = x, _y = y;
    cairo_device_to_user_distance(cr, &_x, &_y);
    x = _x;
    y = _y;
  };

  CAIRO_FUNCTION func = std::bind(fn, _1, x, y);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::cap(lineCap c) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func =
      std::bind(cairo_set_line_cap, _1, static_cast<cairo_line_cap_t>(c));
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::join(lineJoin j) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func =
      std::bind(cairo_set_line_join, _1, static_cast<cairo_line_join_t>(j));
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::lineWidth(double dWidth) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_set_line_width, _1, dWidth);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::miterLimit(double dLimit) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_set_miter_limit, _1, dLimit);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::dashes(const std::vector<double> &dashes,
                                double offset) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func =
      std::bind(cairo_set_dash, _1, dashes.data(), dashes.size(), offset);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::tollerance(double _t) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_set_tolerance, _1, _t);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::op(op_t _op) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func =
      std::bind(cairo_set_operator, _1, static_cast<cairo_operator_t>(_op));
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::source(Paint &p) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  auto fn = [](cairo_t *cr, Paint &p) { p.emit(cr); };
  CAIRO_FUNCTION func = std::bind(fn, _1, p);
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::arc(double xc, double yc, double radius, double angle1,
                             double angle2, bool bNegative) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func;
  if (bNegative) {
    func = std::bind(cairo_arc_negative, _1, xc, yc, radius, angle1, angle2);
  } else {
    func = std::bind(cairo_arc, _1, xc, yc, radius, angle1, angle2);
  }
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::curve(double x1, double y1, double x2, double y2,
                               double x3, double y3, bool bRelative) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func;
  if (bRelative) {
    func = std::bind(cairo_rel_curve_to, _1, x1, y1, x2, y2, x3, y3);
  } else {
    func = std::bind(cairo_curve_to, _1, x1, y1, x2, y2, x3, y3);
  }
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::line(double x, double y, bool bRelative) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func;
  if (bRelative) {
    func = std::bind(cairo_rel_line_to, _1, x, y);
  } else {
    func = std::bind(cairo_line_to, _1, x, y);
  }
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::stroke(bool bPreserve) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func;
  if (bPreserve) {
    func = std::bind(cairo_stroke_preserve, _1);
  } else {
    func = std::bind(cairo_stroke, _1);
  }
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::move(double x, double y, bool bRelative) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func;
  if (bRelative) {
    func = std::bind(cairo_rel_move_to, _1, x, y);
  } else {
    func = std::bind(cairo_move_to, _1, x, y);
  }
  DL.push_back(make_unique<FUNCTION>(func));
}

/**
\brief
*/
void uxdevice::platform::rectangle(double x, double y, double width,
                                   double height) {
  using namespace std::placeholders;
  std::lock_guard<std::mutex> lock(DL_mutex);
  CAIRO_FUNCTION func = std::bind(cairo_rectangle, _1, x, y, width, height);
  DL.push_back(make_unique<FUNCTION>(func));
}

/***************************************************************************/

/**
\internal
\brief The drawText function provides textual character rendering.
*/
void uxdevice::platform::AREA::invoke(DisplayUnitContext &context) {
  cairo_move_to(context.cr, x, y);
}
void uxdevice::platform::AREA::shrink(double a) {
  switch (type) {
  case areaType::none:
    break;
  case areaType::circle:
    x += a;
    y += a;
    rx -= a;
    break;
  case areaType::ellipse:
    x += a;
    y += a;
    rx -= a;
    ry -= a;
    break;
  case areaType::rectangle:
  case areaType::roundedRectangle:
    x += a;
    y += a;
    w -= a * 2;
    h -= a * 2;
    break;
  }
}

/**
\internal
\brief The FONT object provides contextual font object for library.
*/
void uxdevice::platform::FONT::invoke(DisplayUnitContext &context) {
  if (!fontDescription)
    fontDescription = pango_font_description_from_string(description.data());
}

/**
\internal
\brief Maps the last event defined.
*/

void uxdevice::platform::ALIGN::emit(PangoLayout *layout) {
  if (setting == alignment::justified) {
    pango_layout_set_justify(layout, true);
  } else {
    pango_layout_set_justify(layout, false);
    pango_layout_set_alignment(layout, static_cast<PangoAlignment>(setting));
  }
}

/**
\internal
\brief The drawText function provides textual character rendering.
*/
void uxdevice::platform::DRAWTEXT::invoke(DisplayUnitContext &context) {
  // check the context before operating
  bool bColor = context.validatePEN() ||
                (context.validateTEXTOUTLINE() || context.validateTEXTFILL());

  if (!(bColor && (context.validateAREA() && context.validateSTRING() &&
                   context.validateFONT()))) {
    std::stringstream sError;
    sError << "ERR DRAWTEXT AREA, STRING, FONT, "
           << "PEN, TEXTOUTLINE or TEXTFILL not set."
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  AREA &area = context.AREA();
  // create offscreen image of the text
  if (!textImage) {
    cairo_format_t format = CAIRO_FORMAT_ARGB32;

    textImage = cairo_image_surface_create(format, area.w, area.h);
    cairo_t *textCr = cairo_create(textImage);

    if (context.validateANTIALIAS()) {
      context.ANTIALIAS().emit(textCr);
    }

    // clear the image with transparency
    cairo_set_source_rgba(textCr, 0, 0, 0, 0);
    cairo_paint(textCr);

    layout = pango_cairo_create_layout(textCr);
    pango_layout_set_text(layout, context.STRING().data.data(), -1);
    pango_layout_set_font_description(layout, context.FONT().fontDescription);

    if (context.validateALIGN()) {
      context.ALIGN().emit(layout);
    }

    // set the width and height of the layout.
    pango_layout_set_width(layout, area.w * PANGO_SCALE);
    pango_layout_set_height(layout, area.h * PANGO_SCALE);

    pango_cairo_update_layout(textCr, layout);

    // depending on the parameters set for the textual character rendering,
    // different pango apis may be used in conjunction with cairo.
    // essentially this optimizes the textual rendering
    // such that less calls are made and more automatic filling
    // occurs within the rendering.
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
        context.TEXTFILL().emit(textCr);
        cairo_fill_preserve(textCr);
        context.TEXTOUTLINE().emit(textCr);
        cairo_set_line_width(textCr, context.TEXTOUTLINE().lineWidth);
        cairo_stroke(textCr);

        // text is only filled.
      } else if (bFilled) {
        context.TEXTFILL().emit(textCr);
        cairo_fill(textCr);

        // text is only outlined.
      } else if (bOutline) {
        context.TEXTOUTLINE().emit(textCr);
        cairo_set_line_width(textCr, context.TEXTOUTLINE().lineWidth);
        cairo_stroke(textCr);
      }

    } else {
      // no outline or fill defined, therefore the pen is used.
      context.PEN().emit(textCr);
      pango_cairo_show_layout(textCr, layout);
    }

    // free drawing context
    cairo_destroy(textCr);
  }

  // draw shadow if one exists
  if (context.validateTEXTSHADOW()) {
    if (!shadowImage) {

      cairo_format_t format = CAIRO_FORMAT_ARGB32;

      shadowImage = cairo_image_surface_create(format, area.w, area.h);
      cairo_t *shadowCr = cairo_create(shadowImage);
      cairo_set_source_rgba(shadowCr, 0, 0, 0, 0);
      cairo_paint(shadowCr);

      if (context.validateANTIALIAS()) {
        context.ANTIALIAS().emit(shadowCr);
      }

      context.TEXTSHADOW().emit(shadowCr);

      pango_layout_set_width(layout, area.w * PANGO_SCALE);
      pango_layout_set_height(layout, area.h * PANGO_SCALE);

      pango_cairo_update_layout(shadowCr, layout);
      pango_cairo_show_layout(shadowCr, layout);

      blurImage(shadowImage, context.TEXTSHADOW().radius);
      cairo_destroy(shadowCr);
    }
    cairo_save(context.cr);
    context.TEXTSHADOW().emit(context.cr);
    cairo_mask_surface(context.cr, shadowImage, area.x + context.TEXTSHADOW().x,
                       area.y + context.TEXTSHADOW().y);
    cairo_restore(context.cr);
  }

  // move the text rendering to the display.
  cairo_set_source_surface(context.cr, textImage, area.x, area.y);
  cairo_rectangle(context.cr, area.x, area.y, area.w, area.h);
  cairo_fill(context.cr);

  if (layout) {
    g_object_unref(layout);
    layout = nullptr;
  }
}

/// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
/// Stackblur algorithm by Mario Klingemann
/// Details here:
/// http://www.quasimondo.com/StackBlurForCanvas/StackBlurDemo.html
/// C++ implemenation base from:
/// https://gist.github.com/benjamin9999/3809142
/// http://www.antigrain.com/__code/include/agg_blur.h.html
/// This version works only with RGBA color
void uxdevice::platform::blurImage(cairo_surface_t *img, unsigned int radius) {
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

  unsigned int minY = 0;
  unsigned int maxY = h;

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

  unsigned int minX = 0;
  unsigned int maxX = w;

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

/**
\internal
\brief The function draws the image
*/
void uxdevice::platform::DRAWIMAGE::invoke(DisplayUnitContext &context) {
  // check the context before operating
  if (!context.validateAREA() || !context.validateIMAGE()) {
    std::stringstream sError;
    sError << "ERR_DRAWIMAGE AREA or IMAGE not set. "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  AREA &area = context.AREA();
  IMAGE &image = context.IMAGE();

  if (!image.bLoaded && image.bIsSVG) {
    image._image = platform::imageSurfaceSVG(true, image._data, area.w, area.h);
  }

  if (image._image) {
    cairo_set_source_surface(context.cr, image._image, area.x, area.y);
    cairo_mask_surface(context.cr, image._image, area.x, area.y);
  }
}

/**
\internal
\brief The function draws an area
*/
void uxdevice::platform::DRAWAREA::invoke(DisplayUnitContext &context) {
  // check the context before operating
  if (!(context.validateAREA() &&
        (context.validateBACKGROUND() || !context.validatePEN()))) {
    std::stringstream sError;
    sError << "ERR_DRAWBOX AREA or IMAGE not set. "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }
  // include line width for the size of the object
  // if the area will be stroked.
  AREA area = context.AREA();
  if (context.validatePEN()) {
    double dWidth = cairo_get_line_width(context.cr) / 2;
    area.shrink(dWidth);
  }

  switch (area.type) {
  case areaType::none:
    break;
  case areaType::rectangle:
    cairo_rectangle(context.cr, area.x, area.y, area.w, area.h);
    break;
  case areaType::roundedRectangle:
    // derived  from svgren by Ivan Gagis <igagis@gmail.com>
    cairo_move_to(context.cr, area.x + area.rx, area.y);
    cairo_line_to(context.cr, area.x + area.w - area.rx, area.y);

    cairo_save(context.cr);
    cairo_translate(context.cr, area.x + area.w - area.rx, area.y + area.ry);
    cairo_scale(context.cr, area.rx, area.ry);
    cairo_arc(context.cr, 0, 0, 1, -PI / 2, 0);
    cairo_restore(context.cr);

    cairo_line_to(context.cr, area.x + area.w, area.y + area.h - area.ry);

    cairo_save(context.cr);
    cairo_translate(context.cr, area.x + area.w - area.rx,
                    area.y + area.h - area.ry);
    cairo_scale(context.cr, area.rx, area.ry);
    cairo_arc(context.cr, 0, 0, 1, 0, PI / 2);
    cairo_restore(context.cr);

    cairo_line_to(context.cr, area.x + area.rx, area.y + area.h);

    cairo_save(context.cr);
    cairo_translate(context.cr, area.x + area.rx, area.y + area.h - area.ry);
    cairo_scale(context.cr, area.rx, area.ry);
    cairo_arc(context.cr, 0, 0, 1, PI / 2, PI);
    cairo_restore(context.cr);

    cairo_line_to(context.cr, area.x, area.y + area.ry);

    cairo_save(context.cr);
    cairo_translate(context.cr, area.x + area.rx, area.y + area.ry);
    cairo_scale(context.cr, area.rx, area.ry);
    cairo_arc(context.cr, 0, 0, 1, PI, PI * 3 / 2);
    cairo_restore(context.cr);

    cairo_close_path(context.cr);
    break;
  case areaType::circle:
    cairo_new_sub_path(context.cr);
    cairo_arc(context.cr, area.x + area.rx, area.y + area.rx, area.rx, 0.,
              2 * PI);
    cairo_close_path(context.cr);
    break;
  case areaType::ellipse:
    cairo_save(context.cr);
    cairo_translate(context.cr, area.x + area.rx / 2., area.y + area.ry / 2.);
    cairo_scale(context.cr, area.rx / 2., area.ry / 2.);
    cairo_new_sub_path(context.cr);
    cairo_arc(context.cr, 0., 0., 1., 0., 2 * PI);
    cairo_close_path(context.cr);
    cairo_restore(context.cr);
    break;
  }
  // fill using the adjusted area
  if (context.validateBACKGROUND()) {
    context.BACKGROUND().emit(context.cr, area.x, area.y, area.w, area.h);
    cairo_fill_preserve(context.cr);
  }

  if (context.validatePEN()) {
    // use original non adjusted area.
    AREA &area = context.AREA();
    context.PEN().emit(context.cr, area.x, area.y, area.w, area.h);
    cairo_stroke(context.cr);
  }
}

/**
\internal
\brief call previously bound function with the cairo context.
*/
void uxdevice::platform::FUNCTION::invoke(DisplayUnitContext &context) {
  func(context.cr);
}

/**
\internal
\brief uses gio file streaming.
*/
cairo_status_t uxdevice::platform::read_contents(const gchar *file_name,
                                                 guint8 **contents,
                                                 gsize *length) {
  GFile *file;
  GFileInputStream *input_stream;
  gboolean success = FALSE;
  cairo_status_t status = CAIRO_STATUS_SUCCESS;

  file = g_file_new_for_commandline_arg(file_name);
  input_stream = g_file_read(file, NULL, NULL);
  if (input_stream) {
    GFileInfo *file_info;

    file_info = g_file_input_stream_query_info(
        input_stream, G_FILE_ATTRIBUTE_STANDARD_SIZE, NULL, NULL);
    if (file_info) {
      gsize bytes_read;

      *length = g_file_info_get_size(file_info);
      *contents = g_new(guint8, *length);
      success = g_input_stream_read_all(G_INPUT_STREAM(input_stream), *contents,
                                        *length, &bytes_read, NULL, NULL);
      if (!success)
        status = CAIRO_STATUS_READ_ERROR;

      g_object_unref(file_info);
    } else {
      status = CAIRO_STATUS_READ_ERROR;
    }
    g_object_unref(input_stream);
  } else {
    status = CAIRO_STATUS_FILE_NOT_FOUND;
  }

  g_object_unref(file);

  return status;
}

/**
\internal
\brief creates an image surface from an svg.
 In that the null objects are returned. The routine does not maintain
the SVG data.
*/
cairo_surface_t *uxdevice::platform::imageSurfaceSVG(bool bDataPassed,
                                                     std::string &info,
                                                     double width,
                                                     double height) {

  guint8 *contents = nullptr;
  gsize length = 0;
  RsvgHandle *handle = nullptr;
  RsvgDimensionData dimensions;
  cairo_t *cr = nullptr;
  cairo_surface_t *img = nullptr;
  cairo_status_t status = CAIRO_STATUS_SUCCESS;
  double dWidth = 0;
  double dHeight = 0;

  if (bDataPassed) {
    contents = reinterpret_cast<guint8 *>(info.data());
    length = info.size();
  } else {
    // read the file.
    status = read_contents(info.data(), &contents, &length);
    if (status != CAIRO_STATUS_SUCCESS) {
      goto error_exit;
    }
  }
  // create a rsvg handle
  handle = rsvg_handle_new_from_data(contents, length, NULL);
  if (!handle) {
    g_free(contents);
    status = CAIRO_STATUS_READ_ERROR;
    goto error_exit;
  }

  // scale to the image requested.
  dWidth = width;
  dHeight = height;
  rsvg_handle_get_dimensions(handle, &dimensions);

  if (dWidth < 1) {
    dWidth = dimensions.width;
  } else {
    dWidth /= dimensions.width;
  }

  if (dHeight < 1) {
    dHeight = dimensions.height;
  } else {
    dHeight /= dimensions.height;
  }

  // render the image to surface
  img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  status = cairo_surface_status(img);
  if (status != CAIRO_STATUS_SUCCESS) {
    goto error_exit;
  }

  cr = cairo_create(img);
  status = cairo_status(cr);
  if (status != CAIRO_STATUS_SUCCESS) {
    goto error_exit;
  }

  cairo_scale(cr, dWidth, dHeight);
  status = cairo_status(cr);
  if (status != CAIRO_STATUS_SUCCESS) {
    goto error_exit;
  }

  if (!rsvg_handle_render_cairo(handle, cr)) {
    status = CAIRO_STATUS_READ_ERROR;
    goto error_exit;
  }

  // clean up
  cairo_destroy(cr);
  if (!bDataPassed)
    g_free(contents);

  g_object_unref(handle);

  return img;

error_exit:
  if (cr)
    cairo_destroy(cr);
  if (img)
    cairo_surface_destroy(img);
  if (!bDataPassed && contents)
    g_free(contents);
  if (handle)
    g_object_unref(handle);

  return nullptr;
}

/**
\internal
\brief reads the image and creates a cairo surface image.
*/
cairo_surface_t *uxdevice::platform::readImage(std::string &data, double w,
                                               double h) {
  const string dataPNG = "data:image/png;base64,";
  const string dataSVG = "<?xml";
  cairo_surface_t *image = nullptr;

  if (data.size() == 0)
    return nullptr;

  // data is passed as base 64 PNG?
  if (data.compare(0, dataPNG.size(), dataPNG) == 0) {

    typedef struct _readInfo {
      unsigned char *data = nullptr;
      size_t dataLen = 0;
      int val = 0;
      int valB = -8;
      size_t decodePos = 0;
    } readInfo;

    readInfo pngData;

    // from base64 decode snippet in c++
    //  stackoverflow.com/base64 decode snippet in c++ - Stack Overflow.html
    pngData.dataLen = data.size();
    pngData.data = reinterpret_cast<unsigned char *>(data.data());
    pngData.val = 0;
    pngData.valB = -8;
    pngData.decodePos = dataPNG.size();

    cairo_read_func_t fn = [](void *closure, unsigned char *data,
                              unsigned int length) -> cairo_status_t {
      static const uint8_t lookup[] = {
          62,  255, 62,  255, 63,  52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
          255, 0,   255, 255, 255, 255, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
          10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
          255, 255, 255, 255, 63,  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
          36,  37,  38,  39,  40,  41,  42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
      static_assert(sizeof(lookup) == 'z' - '+' + 1);
      readInfo *p = reinterpret_cast<readInfo *>(closure);

      size_t bytesDecoded = 0;
      while (bytesDecoded < length) {

        // requesting more than the size?
        if (p->decodePos > p->dataLen)
          return CAIRO_STATUS_READ_ERROR;

        uint8_t c = p->data[p->decodePos];

        if (c < '+' || c > 'z')
          return CAIRO_STATUS_READ_ERROR;

        c -= '+';
        if (lookup[c] >= 64)
          return CAIRO_STATUS_READ_ERROR;

        p->val = (p->val << 6) + lookup[c];
        p->valB += 6;
        if (p->valB >= 0) {
          *data = static_cast<unsigned char>((p->val >> p->valB) & 0xFF);
          data++;
          bytesDecoded++;
          p->valB -= 8;
        }

        p->decodePos++;
      }

      return CAIRO_STATUS_SUCCESS;
    };

    image = cairo_image_surface_create_from_png_stream(fn, &pngData);

    // if not successful read, set the contents to a null pointer.
    if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS)
      image = nullptr;

    // data in passed as a SVG text?
    // defer until draw to know the size
  } else if (data.compare(0, dataSVG.size(), dataSVG) == 0) {
    image = imageSurfaceSVG(true, data, w, h);

    // file name?
  } else if (data.find(".png") != std::string::npos) {
    image = cairo_image_surface_create_from_png(data.data());

    // if not successful read, set the contents to a null pointer.
    if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS)
      image = nullptr;

  } else if (data.find(".svg") != std::string::npos) {
    image = imageSurfaceSVG(false, data, w, h);
  }

  return image;
}

/**
\internal
\brief reads the image and creates a cairo surface image.
*/
void uxdevice::platform::IMAGE::invoke(DisplayUnitContext &context) {
  if (bLoaded)
    return;

  if (!context.validateAREA()) {
    std::stringstream sError;
    sError << "ERR IMAGE AREA not set. "
           << "  " << __FILE__ << " " << __func__;
    throw std::runtime_error(sError.str());
  }

  _image = readImage(_data, context.AREA().w, context.AREA().h);

  if (_image)
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

#if defined(__linux__)
  if (context.xcbSurface) {
    if (context.windowWidth == w && context.windowHeight == h)
      return;
    std::lock_guard<std::mutex> lockSurface(context.surface_mutex);

    // copy old surface to new one.
    cairo_format_t format = CAIRO_FORMAT_ARGB32;
    cairo_surface_t *newSurface = cairo_image_surface_create(format, w, h);

    cairo_t *cr = cairo_create(newSurface);
    cairo_rectangle(cr, 0, 0, context.windowWidth, context.windowHeight);
    cairo_set_source_surface(cr, context.offScreen, 0, 0);
    cairo_fill(cr);
    cairo_surface_destroy(context.offScreen);

    context.offScreen = newSurface;
    // create cairo context
    cairo_destroy(context.cr);
    context.cr = cairo_create(context.offScreen);

    cairo_xcb_surface_set_size(context.xcbSurface, w, h);
    context.preclear = true;
    bRenderWork = true;
    // postDirty(0, 0, w, h);
  }

  context.windowWidth = w;
  context.windowHeight = h;

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
