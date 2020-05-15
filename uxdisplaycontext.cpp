
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

void uxdevice::DisplayContext::iterate(RegionFunc fn) {
  REGIONS_SPIN;

  for (auto &it : _regions)
    fn(it);

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
void uxdevice::DisplayContext::collectables(DisplayUnitCollection *obj) {
  collection = obj;
  viewportRectangle = {offsetx, offsety, offsetx + windowWidth,
                       offsety + windowHeight};
  if (!viewport)
    viewport = cairo_region_create_rectangle(&viewportRectangle);

  // first time or regenerate
  if (!objectRegion) {
    for (auto unit : *collection) {
      DrawingOutput &n=*dynamic_cast<DrawingOutput *>(unit);
      inkedAreas.push_back(n.inkExtents());
      std::size_t loc = reinterpret_cast<std::size_t>(inkedAreas.back());
      inkedAreaAssociated[loc] = n.fnDraw;
    }

    objectRegion =
        cairo_region_create_rectangles(*inkedAreas.data(), inkedAreas.size());

    cairo_region_intersect(viewport, objectRegion);
    inkedAreasTotal = collection->size();
    itcollection = collection->end();
  } else {

    while (itcollection != collection->end()) {
      DrawingOutput &n=*dynamic_cast<DrawingOutput *>(*itcollection);
      cairo_rectangle_int_t *r=n.inkExtents();
      inkedAreas.push_back(r);
      std::size_t loc = reinterpret_cast<std::size_t>(r);
      inkedAreaAssociated[loc] = n.fnDraw;

      cairo_region_union_rectangle(objectRegion,r);
      cairo_region_xor_rectangle(viewport, r);

      itcollection++;
    }
    inkedAreasTotal = collection->size();
  }
}

/**
 \details Routine iterates each of the render work regions and tests if
 the rectangle is within the region.

*/
void uxdevice::DisplayContext::plot(CairoRegion plotArea) {

  // used to store
  cairo_region_t *dst = cairo_region_copy(viewport);

  // compute rectangles, values are uploaded through ptr
  cairo_region_intersect(dst, plotArea._ptr);

  if (!cairo_region_is_empty(dst)) {
    int numIntersects = cairo_region_num_rectangles(dst);
    for (int nth = 0; nth < numIntersects; nth++) {
      cairo_rectangle_int_t rectangle;
      cairo_region_get_rectangle(dst, nth, &rectangle);
      std::size_t key = reinterpret_cast<std::size_t>(&rectangle);
      AssociatedInkFN::iterator itfn = inkedAreaAssociated.find(key);
      if(itfn != inkedAreaAssociated.end()) {
        DrawLogic &fn = itfn->second;
        fn(*this, (double)rectangle.x, (double)rectangle.y,
               (double)rectangle.width, (double)rectangle.height);
      }
    }
  }


  return;
}
