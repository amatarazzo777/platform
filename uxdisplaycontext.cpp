
#include "uxdevice.hpp"

bool uxdevice::DisplayContext::surfacePrime() {
#if defined(__linux__)
  bool bRet = false;

  // no surface allocated yet
  if (!xcbSurface) {
    return bRet;
  }

  // determine if painting should also occur
  bRet = state();

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

void uxdevice::DisplayContext::flush() {
  state(false);

  if (xcbSurface)
    cairo_surface_flush(xcbSurface);
  if (connection)
    xcb_flush(connection);
}

void uxdevice::DisplayContext::resizeSurface(const int w, const int h) {
  SURFACE_REQUESTS_SPIN;
  _surfaceRequests.push_back(std::make_tuple(w, h));
  SURFACE_REQUESTS_CLEAR;
}

void uxdevice::DisplayContext::render(void) {
  SURFACE_REQUESTS_SPIN;

  // processing surface requests
  if (!_surfaceRequests.empty()) {
    auto flat = _surfaceRequests.back();
    _surfaceRequests.clear();

    cairo_surface_flush(xcbSurface);
    cairo_xcb_surface_set_size(xcbSurface, std::get<0>(flat),
                               std::get<1>(flat));
    windowWidth = std::get<0>(flat);
    windowHeight = std::get<1>(flat);
  }

  // rectangle of area needs painting background first.
  // these are subareas perhaps multiples exist because of resize
  // coordinates. The information is generated from the
  // paint dispatch event. When the window is opened
  // render work will contain entire window
  DRAWABLES_ON_SPIN;

  REGIONS_SPIN;
  std::size_t cnt = _regions.size();
  auto it = _regions.begin();

  while (cnt) {

    CairoRegion &r = *it;
    // hides all drawing operations until pop to source.
    cairo_push_group(cr);

    brush.emit(cr);
    cairo_rectangle(cr, r.rect.x, r.rect.y, r.rect.width, r.rect.height);
    cairo_fill(cr);

    plot(r);
    // pop the draw group to the surface.
    cairo_pop_group_to_source(cr);
    cairo_paint(cr);

    it++;
    cnt--;
  }
  REGIONS_CLEAR;
  DRAWABLES_ON_CLEAR;
  SURFACE_REQUESTS_CLEAR;
}
void uxdevice::DisplayContext::addDrawable(DrawingOutput *_obj) {

  viewportRectangle = {(double)offsetx, (double)offsety,
                       (double)offsetx + (double)windowWidth,
                       (double)offsety + (double)windowHeight};

  _obj->intersect(viewportRectangle);
  if (_obj->overlap == CAIRO_REGION_OVERLAP_OUT) {
    DRAWABLES_OFF_SPIN;
    viewportOff.push_back(_obj);
    DRAWABLES_OFF_CLEAR;
  } else {
    DRAWABLES_ON_SPIN;
    viewportOn.push_back(_obj);
    state(_obj);
    DRAWABLES_ON_CLEAR;
  }
  _obj->viewportInked = true;
}
void uxdevice::DisplayContext::clear(void) {
  DRAWABLES_ON_SPIN;
  DRAWABLES_OFF_SPIN;
  REGIONS_SPIN;
  SURFACE_REQUESTS_SPIN;

  _regions.clear();

  // processing surface requests
  if (!_surfaceRequests.empty()) {
    auto flat = _surfaceRequests.back();
    _surfaceRequests.clear();

    cairo_surface_flush(xcbSurface);
    cairo_xcb_surface_set_size(xcbSurface, std::get<0>(flat),
                               std::get<1>(flat));
    windowWidth = std::get<0>(flat);
    windowHeight = std::get<1>(flat);
  }

  viewportOff.clear();
  viewportOn.clear();

  // hides all drawing operations until pop to source.
  cairo_push_group(cr);

  offsetx = 0;
  offsety = 0;

  brush.emit(cr);
  viewportRectangle = {(double)offsetx, (double)offsety,
                       (double)offsetx + (double)windowWidth,
                       (double)offsety + (double)windowHeight};
  cairo_rectangle(cr, viewportRectangle.x, viewportRectangle.y,
                  viewportRectangle.width, viewportRectangle.height);
  cairo_fill(cr);

  // pop the draw group to the surface.
  cairo_pop_group_to_source(cr);
  cairo_paint(cr);

  REGIONS_CLEAR;
  DRAWABLES_ON_CLEAR;
  DRAWABLES_OFF_CLEAR;
  SURFACE_REQUESTS_CLEAR;

}

void uxdevice::DisplayContext::state(DrawingOutput *obj) {
  REGIONS_SPIN;
  std::size_t onum = reinterpret_cast<std::size_t>(obj);
#if defined(CONSOLE)
  std::cout << "**--- " << onum << ", " << obj->inkRectangle.x << ", "
            << obj->inkRectangle.y << ", " << obj->inkRectangle.width << ", "
            << obj->inkRectangle.height << std::endl
            << std::flush;
#endif // defined

  _regions.push_back(CairoRegion(onum, obj->inkRectangle.x, obj->inkRectangle.y,
                                 obj->inkRectangle.width,
                                 obj->inkRectangle.height));

  REGIONS_CLEAR;
}

void uxdevice::DisplayContext::state(bool on, int x, int y, int w, int h) {
  REGIONS_SPIN;

  if (on) {
    _regions.push_back(CairoRegion{x, y, w, h});

  } else {
    while (!_regions.empty()) {
      if (!_regions.front().eval) {
        break;
      }
      _regions.pop_front();
    }
  }
  REGIONS_CLEAR;
}

bool uxdevice::DisplayContext::state(void) {
  DRAWABLES_ON_SPIN;
  DRAWABLES_OFF_SPIN;
  REGIONS_SPIN;

  bool ret = false;

  if (!_regions.empty())
    for (auto &it : _regions)
      if (!it.eval) {
        // determine if any offscreen elements are visible
        DrawingOutputCollection::iterator obj = viewportOff.begin();
        while (obj != viewportOff.end()) {
          DrawingOutput *n = *obj;
          n->intersect(it._rect);
          if (n->overlap != CAIRO_REGION_OVERLAP_OUT) {
            DrawingOutputCollection::iterator next = obj;
            next++;

            viewportOn.push_back(n);

            viewportOff.erase(obj);
            obj = next;
          } else {
            obj++;
          }
        }
        // inform the caller that there is work to be done.
        ret = true;

        break;
      }

  REGIONS_CLEAR;
  DRAWABLES_OFF_CLEAR;
  DRAWABLES_ON_CLEAR;
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

  for (auto &unit : viewportOn) {
    DrawingOutput *n = dynamic_cast<DrawingOutput *>(unit);
    n->intersect(plotArea._rect);

    switch (n->overlap) {
    case CAIRO_REGION_OVERLAP_OUT:
      plotArea.eval = true;
      break;
    case CAIRO_REGION_OVERLAP_IN: {
      n->fnDraw(*this);
      plotArea.eval = true;
    } break;
    case CAIRO_REGION_OVERLAP_PART: {
      n->fnDrawClipped(*this);
      plotArea.eval = true;
    } break;
    }
  }
}
