
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
  if (w != windowWidth || h != windowHeight)
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
  SURFACE_REQUESTS_CLEAR;

  // rectangle of area needs painting background first.
  // these are subareas perhaps multiples exist because of resize
  // coordinates. The information is generated from the
  // paint dispatch event. When the window is opened
  // render work will contain entire window

  REGIONS_SPIN;
  std::size_t cnt = _regions.size();
  auto it = _regions.begin();


  DRAWABLES_ON_SPIN;

  // AFM NOTE it appears the the push and pop
  // cause a memory error, over writing a malloc block
  // header -- os reports a size difernt that prev
    // hides all drawing operations until pop to source.
  //cairo_push_group(cr);

  while (cnt) {

    CairoRegion &r = *it;

    brush.emit(cr);
    cairo_rectangle(cr, r.rect.x, r.rect.y, r.rect.width, r.rect.height);
    cairo_fill(cr);

    plot(r);

    it++;
    cnt--;
  }

  // pop the draw group to the surface.
 // cairo_pop_group_to_source(cr);
  //cairo_paint(cr);
  DRAWABLES_ON_CLEAR;
  REGIONS_CLEAR;
}

void uxdevice::DisplayContext::addDrawable(
    std::shared_ptr<DrawingOutput> _obj) {
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
    DRAWABLES_ON_CLEAR;
    state(_obj);
  }
  _obj->viewportInked = true;
}

void uxdevice::DisplayContext::clear(void) {
  REGIONS_SPIN;
  _regions.clear();
  offsetx = 0;
  offsety = 0;
  viewportRectangle = {(double)offsetx, (double)offsety,
                       (double)offsetx + (double)windowWidth,
                       (double)offsety + (double)windowHeight};

  currentUnits = CurrentUnits();
  DRAWABLES_ON_SPIN;
  viewportOn.clear();
  DRAWABLES_ON_CLEAR;
  REGIONS_CLEAR;

  DRAWABLES_OFF_SPIN;
  viewportOff.clear();
  DRAWABLES_OFF_CLEAR;

  state(true, 0, 0, windowWidth, windowHeight);
}

void uxdevice::DisplayContext::surfaceBrush(Paint &b) {
  brush = b;
  viewportRectangle = {(double)offsetx, (double)offsety,
                       (double)offsetx + (double)windowWidth,
                       (double)offsety + (double)windowHeight};
  state(true, 0, 0, windowWidth, windowHeight);
}

void uxdevice::DisplayContext::state(std::shared_ptr<DrawingOutput> obj) {
  REGIONS_SPIN;
  std::size_t onum = reinterpret_cast<std::size_t>(obj.get());
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
    auto it = std::find_if(_regions.begin(), _regions.end(), [=](auto &n) {
        return n.rect.x == x && n.rect.y == y && n.rect.width == w &&
               n.rect.height == h;
      });

    if(it==_regions.end())
      _regions.push_back(CairoRegion{x, y, w, h});
#if defined(CONSOLE)
    std::cout << "****--- " << x << ", " << y << ", " << w << ", " << h
              << std::endl
              << std::flush;
#endif // defined
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

  REGIONS_SPIN;

  bool ret = false;

  if (!_regions.empty())
    for (auto &it : _regions)
      if (!it.eval) {
        // determine if any offscreen elements are visible
        DRAWABLES_OFF_SPIN;
        DrawingOutputCollection::iterator obj = viewportOff.begin();
        while (obj != viewportOff.end()) {
          std::shared_ptr<DrawingOutput> n = *obj;
          n->intersect(it._rect);
          if (n->overlap != CAIRO_REGION_OVERLAP_OUT) {
            DrawingOutputCollection::iterator next = obj;
            next++;
            DRAWABLES_ON_SPIN;
            viewportOn.push_back(n);
            DRAWABLES_ON_CLEAR;

            viewportOff.erase(obj);

            obj = next;
          } else {
            obj++;
          }
        }
        DRAWABLES_OFF_CLEAR;
        // inform the caller that there is work to be done.
        ret = true;

        break;
      }

  REGIONS_CLEAR;

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
    std::shared_ptr<DrawingOutput> n =
        std::dynamic_pointer_cast<DrawingOutput>(unit);
    n->intersect(plotArea._rect);

    switch (n->overlap) {
    case CAIRO_REGION_OVERLAP_OUT:
      plotArea.eval = true;
      break;
    case CAIRO_REGION_OVERLAP_IN: {
      n->fnBaseSurface(*this);
      n->fnDraw(*this);
      plotArea.eval = true;
    } break;
    case CAIRO_REGION_OVERLAP_PART: {
      n->fnCacheSurface(*this);
      n->fnDrawClipped(*this);
      plotArea.eval = true;
    } break;
    }
  }
}
