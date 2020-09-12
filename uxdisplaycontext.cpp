
#include "uxdevice.hpp"

// error macro for this source
#ifdef ERROR_CHECK
#undef ERROR_CHECK
#endif // ERROR_CHECK

#define ERROR_CHECK(obj)                                                       \
  {                                                                            \
    cairo_status_t stat = errorCheck(obj);                                     \
    if (stat)                                                                  \
      errorState(__func__, __LINE__, __FILE__, stat);                          \
  }
/**
\internal
\brief The routine checks the system for render work which primarily
arrives to the thread via the regions list. However, when no official work exists,
 the condition variable cvRenderWork is place in a wait state. The condition
 may be awoken by calling the routine stateNotifyComplete().
*/
bool uxdevice::DisplayContext::surfacePrime() {
#if defined(__linux__)
  bool bRet = false;

  // no surface allocated yet
  XCB_SPIN;
  bool bExists = xcbSurface != nullptr;
  XCB_CLEAR;

  if (!bExists) {
    return bRet;
  }

  // determine if painting should also occur
  bRet = state();
  if (bRet)
    return bRet;

  // wait for render work if none has already been provided.
  // the state routines which produce region rectangular information
  // supplies the notification.
  if (!bRet) {
    std::unique_lock<std::mutex> lk(mutexRenderWork);
    cvRenderWork.wait(lk);
    lk.unlock();
  }

  return bRet;

#elif defined(_WIN64)

  // get the size of the window
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
\internal
\brief The routine provides the syncronization of the xcb cairo surface
and the video system of xcb.
*/
void uxdevice::DisplayContext::flush() {

  XCB_SPIN;
  if (xcbSurface) {
    cairo_surface_flush(xcbSurface);
    ERROR_CHECK(xcbSurface);
  }
  XCB_CLEAR;

  if (connection)
    xcb_flush(connection);
}
/**
\internal
\brief The routine is called by a client to resize the window surface.
In addition, other work may be applied such as paint, hwoever those go
into a separate list as the operating system provides these message
independently, that is Configure window event and a separate paint
rectangle.
*/
void uxdevice::DisplayContext::resizeSurface(const int w, const int h) {
  SURFACE_REQUESTS_SPIN;
  if (w != windowWidth || h != windowHeight)
    _surfaceRequests.emplace_back(w, h);
  SURFACE_REQUESTS_CLEAR;
}
/**
\internal
\brief The routine checks the system for render work which primarily
arrives to the thread via the regions list. However, when no official work exists,
 the condition variable cvRenderWork is place in a wait state. The condition
 may be awoken by calling the routine stateNotifyComplete().
*/
void uxdevice::DisplayContext::applySurfaceRequests(void) {
  SURFACE_REQUESTS_SPIN;
  // take care of surface requests
  if (!_surfaceRequests.empty()) {
    auto flat = _surfaceRequests.back();
    _surfaceRequests.clear();

    XCB_SPIN;
    cairo_surface_flush(xcbSurface);
    cairo_xcb_surface_set_size(xcbSurface, flat.w, flat.h);
    ERROR_CHECK(xcbSurface);
    XCB_CLEAR;

    windowWidth = flat.w;
    windowHeight = flat.h;
  }
  SURFACE_REQUESTS_CLEAR;
}

void uxdevice::DisplayContext::render(void) {
  bClearFrame = false;

  // rectangle of area needs painting background first.
  // these are subareas perhaps multiples exist because of resize
  // coordinates. The information is generated from the
  // paint dispatch event. When the window is opened
  // render work will contain entire window

  applySurfaceRequests();

  //partitionVisibility();

  REGIONS_SPIN;
  cairo_region_t *current = nullptr;
  while (!_regions.empty()) {
    CairoRegion r = _regions.front();
    _regions.pop_front();
    REGIONS_CLEAR;
    // os surface requests are ideally full screen block coordinates
    // when multiples exist, such as clear, set surface as well as
    // objects that fit within the larger bounds,
    // simply continue as there is no redraw needed
    if (current) {
      cairo_region_overlap_t ovrlp =
          cairo_region_contains_rectangle(current, &r.rect);
      if (ovrlp == CAIRO_REGION_OVERLAP_IN)
        continue;
    } else {
      if (r.bOSsurface)
        current = cairo_region_reference(r._ptr);
    }

    // the xcb spin locks the primary cairo context
    // while drawing operations occur. these blocks
    // are distinct work items
    XCB_SPIN;
    cairo_push_group(cr);
    BRUSH_SPIN;
    brush.emit(cr);
    BRUSH_CLEAR;
    ERROR_CHECK(cr);
    XCB_CLEAR;

    XCB_SPIN;
    cairo_rectangle(cr, r.rect.x, r.rect.y, r.rect.width, r.rect.height);
    cairo_fill(cr);
    ERROR_CHECK(cr);
    XCB_CLEAR;


    plot(r);


    XCB_SPIN;
    cairo_pop_group_to_source(cr);
    cairo_paint(cr);
    ERROR_CHECK(cr);
    XCB_CLEAR;

    flush();

    // processing surface requests
    applySurfaceRequests();
    REGIONS_SPIN;
    if (bClearFrame) {
      bClearFrame = false;
      break;
    }
  }
  REGIONS_CLEAR;
  cairo_region_destroy(current);
}
/**
\internal
\brief The allocates an xcb and cairo image surface.
*/
uxdevice::DRAWBUFFER uxdevice::DisplayContext::allocateBuffer(int width,
                                                              int height) {
#if 0
  XCB_SPIN;
  cairo_surface_t *rendered =
  cairo_surface_create_similar (xcbSurface,
                                CAIRO_CONTENT_COLOR_ALPHA,
                                width, height);
  XCB_CLEAR;
#endif // 0

  cairo_surface_t *rendered =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  ERROR_CHECK(rendered);

  cairo_t *cr = cairo_create(rendered);
  ERROR_CHECK(cr);

  return DRAWBUFFER{cr, rendered};
}
/**
\internal
\brief The routine frees the buffer.
*/
void uxdevice::DisplayContext::destroyBuffer(DRAWBUFFER &_buffer) {
  if (_buffer.cr)
    cairo_destroy(_buffer.cr);
  if (_buffer.rendered)
    cairo_surface_destroy(_buffer.rendered);
  _buffer = {};
}
/**
\internal
\brief The routine adds a drawing output object to the
appropriate list, on or offscreen. If the item is on screen,
a region area paint is requested for the object's area.

*/
void uxdevice::DisplayContext::addDrawable(
    std::shared_ptr<DrawingOutput> _obj) {
    viewportRectangle = {(double)offsetx, (double)offsety,
                       (double)offsetx + (double)windowWidth,
                       (double)offsety + (double)windowHeight};
  _obj->intersect(viewportRectangle);
  if (_obj->overlap == CAIRO_REGION_OVERLAP_OUT) {
    DRAWABLES_OFF_SPIN;
    viewportOff.emplace_back(_obj);
    DRAWABLES_OFF_CLEAR;
  } else {
    DRAWABLES_ON_SPIN;
    viewportOn.emplace_back(_obj);
    DRAWABLES_ON_CLEAR;
    state(_obj);
  }
  _obj->viewportInked = true;
}
/**
\internal
\brief The routine scans the offscreen list to see if any are now visible.
*/
void uxdevice::DisplayContext::partitionVisibility(void) {
  // determine if any off screen elements are visible
  DRAWABLES_OFF_SPIN;

  viewportRectangle = {(double)offsetx, (double)offsety,
                       (double)offsetx + (double)windowWidth,
                       (double)offsety + (double)windowHeight};
  if(viewportOff.empty()) {
    DRAWABLES_OFF_CLEAR;
    return;
  }

  DrawingOutputCollection::iterator obj = viewportOff.begin();
  while (obj != viewportOff.end()) {
    std::shared_ptr<DrawingOutput> n = *obj;
    DRAWABLES_OFF_CLEAR;

    n->intersect(viewportRectangle);
    if (bClearFrame) {
      bClearFrame=false;
      break;
    }

    if (n->overlap != CAIRO_REGION_OVERLAP_OUT) {
      DrawingOutputCollection::iterator next = obj;
      next++;
      DRAWABLES_ON_SPIN;
      viewportOn.emplace_back(n);
      DRAWABLES_ON_CLEAR;

      DRAWABLES_OFF_SPIN;
      if (bClearFrame || viewportOff.empty()) {
        bClearFrame=false;
        DRAWABLES_OFF_CLEAR;
        break;
      }

      viewportOff.erase(obj);
      DRAWABLES_OFF_CLEAR;
      obj = next;
    } else {
      obj++;
    }

    if (bClearFrame) {
      bClearFrame=false;
      break;
    }

    DRAWABLES_OFF_SPIN;
  }
  DRAWABLES_OFF_CLEAR;
}
/**
\internal
\brief The routine clears the display context.
*/
void uxdevice::DisplayContext::clear(void) {
  bClearFrame = true;

  REGIONS_SPIN;
  _regions.remove_if([](auto &n) { return !n.bOSsurface; });

  offsetx = 0;
  offsety = 0;
  currentUnits = {};

  REGIONS_CLEAR;

  DRAWABLES_ON_SPIN;
  viewportOn.clear();
  DRAWABLES_ON_CLEAR;

  DRAWABLES_OFF_SPIN;
  viewportOff.clear();
  DRAWABLES_OFF_CLEAR;

  state(0, 0, windowWidth, windowHeight);
}
/**
\internal
\brief The routine sets the background surface brush.
*/
void uxdevice::DisplayContext::surfaceBrush(Paint &b) {
  BRUSH_SPIN;
  brush = b;
  BRUSH_CLEAR;
  state(0, 0, windowWidth, windowHeight);
}
/**
\internal
\brief The routine accepts a drawing output object and adds the
associated render work with the object's coordinates.
note stateNotifyComplete must be called after this to inform the renderer
there is work.
*/
void uxdevice::DisplayContext::state(std::shared_ptr<DrawingOutput> obj) {
  REGIONS_SPIN;
  std::size_t onum = reinterpret_cast<std::size_t>(obj.get());

  _regions.emplace_back(
      CairoRegion(onum, obj->inkRectangle.x, obj->inkRectangle.y,
                  obj->inkRectangle.width, obj->inkRectangle.height));
  REGIONS_CLEAR;
}
/**
\internal
\brief The routine adds a generalized region area paint for the rendered to
find. note stateNotifyComplete must be called after this to inform the renderer
there is work.
*/
void uxdevice::DisplayContext::state(int x, int y, int w, int h) {
  REGIONS_SPIN;
  _regions.emplace_back(CairoRegion{false, x, y, w, h});
  REGIONS_CLEAR;
}
/**
\internal
\brief The routine adds a surface oriented painting request to the render queue.
the items are inserted first before any other so that painting
of a newly resized window area occurs first.
*/
void uxdevice::DisplayContext::stateSurface(int x, int y, int w, int h) {
  REGIONS_SPIN;
  auto it = std::find_if(_regions.begin(), _regions.end(),
                         [](auto &n) { return !n.bOSsurface; });
  if (it != _regions.end())
    _regions.insert(it, CairoRegion{true, x, y, w, h});
  else
    _regions.emplace_back(CairoRegion{true, x, y, w, h});

  REGIONS_CLEAR;
}
/**
\internal
\brief The routine notifies the condition vraiable that work
has been requested and should immedialy being to render.
Having this as a separate function provides the ability
to add work without rendering occurring. However, message
queue calls this when a resize occurs.
*/
void uxdevice::DisplayContext::stateNotifyComplete(void) {
  cvRenderWork.notify_one();
}
/**
\internal
\brief The routine returns whether work is within the system.
*/
bool uxdevice::DisplayContext::state(void) {

  REGIONS_SPIN;
  bool ret = !_regions.empty();
  REGIONS_CLEAR;

  // surface requests should be performed,
  // the render function sets the surface size
  // and exits if no region work.
  if (!ret) {
    SURFACE_REQUESTS_SPIN;
    ret = !_surfaceRequests.empty();
    SURFACE_REQUESTS_CLEAR;
  }

  return ret;
}

/**
 \details Routine iterates each of objects that draw and tests if
 the rectangle is within the region.

*/
void uxdevice::DisplayContext::plot(CairoRegion &plotArea) {
  // if an object is named as what should be updated.
  // setting the flag informs that the contents
  // has been evaluated and ma be removed
  DRAWABLES_ON_SPIN;
  if (viewportOn.empty()) {
    DRAWABLES_ON_CLEAR;
    return;
  }

  auto itUnit = viewportOn.begin();
  bool bDone = false;
  while (!bDone) {

    std::shared_ptr<DrawingOutput> n =*itUnit;
    DRAWABLES_ON_CLEAR;
    n->intersect(plotArea._rect);

    switch (n->overlap) {
    case CAIRO_REGION_OVERLAP_OUT:
      break;
    case CAIRO_REGION_OVERLAP_IN: {
      n->functorsLock(true);
      XCB_SPIN;
      n->fnDraw(*this);
      XCB_CLEAR;
      n->functorsLock(false);
      ERROR_CHECK(cr);
    } break;
    case CAIRO_REGION_OVERLAP_PART: {
      n->functorsLock(true);
      XCB_SPIN;
      n->fnDrawClipped(*this);
      XCB_CLEAR;
      n->functorsLock(false);
      ERROR_CHECK(cr);
    } break;
    }
    if (bClearFrame)
      bDone = true;

    DRAWABLES_ON_SPIN;
    if (!bDone) {
      itUnit++;
      bDone = itUnit == viewportOn.end();
    }
  }
  DRAWABLES_ON_CLEAR;
}
/**
\internal
\brief The routine stores error conditions.
*/
void uxdevice::DisplayContext::errorState(const std::string_view &sfunc,
                                          const std::size_t linenum,
                                          const std::string_view &sfile,
                                          const cairo_status_t stat) {
  errorState(sfunc, linenum, sfile,
             std::string_view(cairo_status_to_string(stat)));
}
/**
\internal
\brief The routine stores error conditions.
*/
void uxdevice::DisplayContext::errorState(const std::string_view &sfunc,
                                          const std::size_t linenum,
                                          const std::string_view &sfile,
                                          const std::string &desc) {
  errorState(sfunc, linenum, sfile, std::string_view(desc));
}
/**
\internal
\brief The routine stores error conditions.
*/
void uxdevice::DisplayContext::errorState(const std::string_view &sfunc,
                                          const std::size_t linenum,
                                          const std::string_view &sfile,
                                          const std::string_view &desc) {
  ERRORS_SPIN;
  std::stringstream ss;
  ss << sfile << "\n" << sfunc << "(" << linenum << ") -  " << desc << "\n";
  _errors.emplace_back(ss.str());

  ERRORS_CLEAR;
}
/**
\internal
\brief The routine checks error conditions.
*/
bool uxdevice::DisplayContext::errorState(void) {
  ERRORS_SPIN;
  bool b = !_errors.empty();
  ERRORS_CLEAR;
  return b;
}
/**
\internal
\brief The routine returns a string representing all of the errors that have occurred.
optionally, by default as well, the error queue list is cleared.
*/
std::string uxdevice::DisplayContext::errorText(bool bclear) {
  ERRORS_SPIN;
  std::string ret;
  for (auto s : _errors)
    ret += s;
  if (bclear)
    _errors.clear();

  ERRORS_CLEAR;
  return ret;
}
