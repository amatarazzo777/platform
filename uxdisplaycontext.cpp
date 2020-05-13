
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

void uxdevice::DisplayContext::iterate(
    std::function<void(cairo_rectangle_int_t &n)> fn) {
  REGIONS_SPIN;

  for (auto &it : _regions) {
    fn(it.rect);
    it.eval = true;
  }

  REGIONS_CLEAR;
}

void uxdevice::DisplayContext::state(bool on, int x, int y, int w, int h) {
  REGIONS_SPIN;
  if (on)
    _regions.push_back(Region{x, y, w, h});
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
  REGIONS_SPIN;
  bool ret = false;

  if (!_regions.empty())
    for (auto it : _regions)
      if (!it.eval) {
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
cairo_region_overlap_t
uxdevice::DisplayContext::contains(const cairo_rectangle_int_t *rectangle,
                                   RenderList &renderList) {
  cairo_region_overlap_t ret = CAIRO_REGION_OVERLAP_PART;

  REGIONS_SPIN;

  renderList.clear();

  for (auto it : _regions) {
    // eval is set when the background is cleared
    // no need to draw it if -- GLITCH within the logic?
    //if (it.eval)   continue;

    // determine if the object is within the region
    ret = cairo_region_contains_rectangle(it._ptr, rectangle);
    switch (ret) {
    case CAIRO_REGION_OVERLAP_IN:
      renderList.push_back({(double)rectangle->x, (double)rectangle->y,
                            (double)rectangle->width,
                            (double)rectangle->height});
      break;

    case CAIRO_REGION_OVERLAP_PART: {
      cairo_region_t *tmp = cairo_region_copy(it._ptr);
      cairo_region_intersect_rectangle(tmp, rectangle);
      cairo_rectangle_int_t partial;
      cairo_region_get_extents(tmp, &partial);
      renderList.push_back({(double)partial.x, (double)partial.y,
                            (double)partial.width, (double)partial.height});

      cairo_region_destroy(tmp);
    } break;

    case CAIRO_REGION_OVERLAP_OUT:
      break;
    }
  }

  REGIONS_CLEAR;

  return ret;
}
