
#include "uxdevice.hpp"

bool uxdevice::DisplayContext::surfacePrime() {
#if defined(__linux__)
  bool bRet = false;

  SURFACE_REQUESTS_SPIN;

  // no surface allocated yet
  if (!xcbSurface) {
    SURFACE_REQUESTS_CLEAR;
    return bRet;
  }
  // if there are no surface requests but there are
  // region paint requests.
  if (_surfaceRequests.empty()) {
    SURFACE_REQUESTS_CLEAR;
    return state();
  }
  // processing surface requests
  auto flat = _surfaceRequests.back();
  _surfaceRequests.clear();
  SURFACE_REQUESTS_CLEAR;

  cairo_surface_flush(xcbSurface);
  cairo_xcb_surface_set_size(xcbSurface, std::get<0>(flat), std::get<1>(flat));
  windowWidth = std::get<0>(flat);
  windowHeight = std::get<1>(flat);

  // surface requests processed, now
  // determine if painting should also occur
  bRet = state();

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

  return bRet;
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

void uxdevice::DisplayContext::iterate(RegionFunc fn) {
  REGIONS_SPIN;

  for (auto &it : _regions) {
    fn(it);
    it.eval = true;
  }

  REGIONS_CLEAR;
}

void uxdevice::DisplayContext::state(bool on, int x, int y, int w, int h) {
  REGIONS_SPIN;
  if (on)
    _regions.push_back(CairoRegion{x, y, w, h});
  else {
    while (!_regions.empty()) {
      if (!_regions.front().eval)
        break;
      _regions.pop_front();
    }
  }
  REGIONS_CLEAR;
}

bool uxdevice::DisplayContext::state(void) {
  bool ret = false;
  REGIONS_SPIN;

  if (!_regions.empty())
    for (auto &it : _regions)
      if (!it.eval) {
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
        ret = true;
        break;
      }

  REGIONS_CLEAR;
  return ret;
}

/**
 \details Routine iterates each of the render work regions and tests if
 the rectangle is within the region.

*/
void uxdevice::DisplayContext::collectables(DisplayUnitCollection *obj) {
  // first time will not be set, so start at the beginning of the list
  if (collection == nullptr)
    itunitCollectables = obj->begin();
  collection = obj;

  viewportRectangle = {(double)offsetx, (double)offsety,
                       (double)offsetx + (double)windowWidth,
                       (double)offsety + (double)windowHeight};

  itunitCollectables =
      std::find_if(itunitCollectables, collection->end(),
                   [](DisplayUnit *&n) { return n->viewportInked == false; });
  while (itunitCollectables != collection->end()) {
    DrawingOutput *n = dynamic_cast<DrawingOutput *>(*itunitCollectables);
    n->intersect(viewportRectangle);
    if (n->overlap == CAIRO_REGION_OVERLAP_OUT) {
      viewportOff.push_back(n);
    } else {
      viewportOn.push_back(n);
    }
    n->viewportInked = true;
    itunitCollectables++;
  }
}

/**
 \details Routine iterates each of the render work regions and tests if
 the rectangle is within the region.

*/
void uxdevice::DisplayContext::plot(CairoRegion &plotArea) {

  for (auto &unit : viewportOn) {
    DrawingOutput *n = dynamic_cast<DrawingOutput *>(unit);
    n->intersect(plotArea._rect);
    switch (n->overlap) {
    case CAIRO_REGION_OVERLAP_OUT:
      break;
    case CAIRO_REGION_OVERLAP_IN: {
      n->fnDraw(*this);
    } break;
    case CAIRO_REGION_OVERLAP_PART: {
      n->fnDrawClipped(*this);
    } break;
    }
  }
}
