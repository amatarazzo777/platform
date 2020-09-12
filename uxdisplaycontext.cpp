/*
 * This file is part of the PLATFORM_OBJ distribution
 * {https://github.com/amatarazzo777/platform_obj). Copyright (c) 2020 Anthony
 * Matarazzo.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
\author Anthony Matarazzo
\file uxdisplayunits.hpp
\date 9/7/20
\version 1.0
\brief
*/
#include "uxdevice.hpp"

// error macro for this source
#ifdef UX_ERROR_CHECK
#undef UX_ERROR_CHECK
#endif // UX_ERROR_CHECK

#define UX_ERROR_CHECK(obj)                                                    \
  {                                                                            \
    cairo_status_t stat = error_check(obj);                                    \
    if (stat)                                                                  \
      error_state(__func__, __LINE__, __FILE__, stat);                         \
  }
/**
\internal
\brief The routine checks the system for render work which primarily
arrives to the thread via the regions list. However, when no official work
exists, the condition variable cvRenderWork is placed in a wait state. The
condition may be awoken by calling the routine state_notify_complete().
*/
bool uxdevice::display_context_t::surface_prime() {
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
  // the state routines could easily produce region rectangular information
  // along the notification but do not. The user should call notify_complete.
  if (!bRet) {
    std::unique_lock<std::mutex> lk(mutexRenderWork);
    cvRenderWork.wait(lk);
    lk.unlock();
  }

  return bRet;
}
/**
\internal
\brief The routine provides the syncronization of the xcb cairo surface
and the video system of xcb.
*/
void uxdevice::display_context_t::flush() {

  XCB_SPIN;
  if (xcbSurface) {
    cairo_surface_flush(xcbSurface);
    UX_ERROR_CHECK(xcbSurface);
  }
  XCB_CLEAR;

  if (connection)
    xcb_flush(connection);
}
/**
\internal
\brief The routine
*/
void uxdevice::display_context_t::device_offset(double x, double y) {
  XCB_SPIN;
  cairo_surface_set_device_offset(xcbSurface, x, y);
  XCB_CLEAR;
  state(0, 0, window_width, window_height);
}
/**
\internal
\brief The routine
*/
void uxdevice::display_context_t::device_scale(double x, double y) {
  XCB_SPIN;
  cairo_surface_set_device_scale(xcbSurface, x, y);
  XCB_CLEAR;
  state(0, 0, window_width, window_height);
}

/**
\internal
\brief The routine is called by a client to resize the window surface.
In addition, other work may be applied such as paint, hwoever those go
into a separate list as the operating system provides these message
independently, that is Configure window event and a separate paint
rectangle.
*/
void uxdevice::display_context_t::resize_surface(const int w, const int h) {
  SURFACE_REQUESTS_SPIN;
  if (w != window_width || h != window_height)
    _surfaceRequests.emplace_back(w, h);
  SURFACE_REQUESTS_CLEAR;
}
/**
\internal
\brief The routine applies resize requests of a window.
The underlying cairo surface is sized with the very last
one.
*/
void uxdevice::display_context_t::apply_surface_requests(void) {
  SURFACE_REQUESTS_SPIN;
  // take care of surface requests
  if (!_surfaceRequests.empty()) {
    auto flat = _surfaceRequests.back();
    _surfaceRequests.clear();

    XCB_SPIN;
    cairo_surface_flush(xcbSurface);
    cairo_xcb_surface_set_size(xcbSurface, flat.w, flat.h);
    UX_ERROR_CHECK(xcbSurface);
    XCB_CLEAR;

    window_width = flat.w;
    window_height = flat.h;
  }
  SURFACE_REQUESTS_CLEAR;
}

void uxdevice::display_context_t::render(void) {
  clearing_frame = false;

  // rectangle of area needs painting background first.
  // these are subareas perhaps multiples exist because of resize
  // coordinate_t. The information is generated from the
  // paint dispatch event. When the window is opened
  // render work will contain entire window

  apply_surface_requests();

  // partitionVisibility();

  REGIONS_SPIN;
  cairo_region_t *current = nullptr;
  while (!_regions.empty()) {
    context_cairo_region_t r = _regions.front();
    _regions.pop_front();
    REGIONS_CLEAR;
    // os surface requests are ideally full screen block coordinate_t
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
    UX_ERROR_CHECK(cr);
    XCB_CLEAR;

    XCB_SPIN;
    cairo_rectangle(cr, r.rect.x, r.rect.y, r.rect.width, r.rect.height);
    cairo_fill(cr);
    UX_ERROR_CHECK(cr);
    XCB_CLEAR;

    plot(r);

    XCB_SPIN;
    cairo_pop_group_to_source(cr);
    cairo_paint(cr);
    UX_ERROR_CHECK(cr);
    XCB_CLEAR;

    flush();

    // processing surface requests
    apply_surface_requests();
    REGIONS_SPIN;
    if (clearing_frame) {
      clearing_frame = false;
      break;
    }
  }
  REGIONS_CLEAR;
  if (current)
    cairo_region_destroy(current);
}
/**
\internal
\brief The allocates an xcb and cairo image_block_t surface.
*/
uxdevice::draw_buffer_t
uxdevice::display_context_t::allocate_buffer(int width, int height) {
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
  UX_ERROR_CHECK(rendered);

  cairo_t *cr = cairo_create(rendered);
  UX_ERROR_CHECK(cr);

  return draw_buffer_t{cr, rendered};
}
/**
\internal
\brief The routine frees the buffer.
*/
void uxdevice::display_context_t::destroy_buffer(draw_buffer_t &_buffer) {
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
void uxdevice::display_context_t::add_drawable(
    std::shared_ptr<drawing_output_t> _obj) {
  viewport_rectangle = {(double)offsetx, (double)offsety,
                        (double)offsetx + (double)window_width,
                        (double)offsety + (double)window_height};
  _obj->intersect(viewport_rectangle);
  if (_obj->overlap == CAIRO_REGION_OVERLAP_OUT) {
    DRAWABLES_OFF_SPIN;
    viewport_off.emplace_back(_obj);
    DRAWABLES_OFF_CLEAR;
  } else {
    DRAWABLES_ON_SPIN;
    viewport_on.emplace_back(_obj);
    DRAWABLES_ON_CLEAR;
    state(_obj);
  }
  _obj->viewport_inked = true;
}
/**
\internal
\brief The routine scans the offscreen list to see if any are now visible.
*/
void uxdevice::display_context_t::partition_visibility(void) {
  // determine if any off screen elements are visible
  DRAWABLES_OFF_SPIN;

  viewport_rectangle = {(double)offsetx, (double)offsety,
                        (double)offsetx + (double)window_width,
                        (double)offsety + (double)window_height};
  if (viewport_off.empty()) {
    DRAWABLES_OFF_CLEAR;
    return;
  }

  drawing_output_collection_t::iterator obj = viewport_off.begin();
  while (obj != viewport_off.end()) {
    std::shared_ptr<drawing_output_t> n = *obj;
    DRAWABLES_OFF_CLEAR;

    n->intersect(viewport_rectangle);
    if (clearing_frame) {
      clearing_frame = false;
      break;
    }

    if (n->overlap != CAIRO_REGION_OVERLAP_OUT) {
      drawing_output_collection_t::iterator next = obj;
      next++;
      DRAWABLES_ON_SPIN;
      viewport_on.emplace_back(n);
      DRAWABLES_ON_CLEAR;

      DRAWABLES_OFF_SPIN;
      if (clearing_frame || viewport_off.empty()) {
        clearing_frame = false;
        DRAWABLES_OFF_CLEAR;
        break;
      }

      viewport_off.erase(obj);
      DRAWABLES_OFF_CLEAR;
      obj = next;
    } else {
      obj++;
    }

    if (clearing_frame) {
      clearing_frame = false;
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
void uxdevice::display_context_t::clear(void) {
  clearing_frame = true;

  REGIONS_SPIN;
  _regions.remove_if([](auto &n) { return !n.bOSsurface; });

  offsetx = 0;
  offsety = 0;
  unit_memory_storage.clear();

  REGIONS_CLEAR;

  DRAWABLES_ON_SPIN;
  viewport_on.clear();
  DRAWABLES_ON_CLEAR;

  DRAWABLES_OFF_SPIN;
  viewport_off.clear();
  DRAWABLES_OFF_CLEAR;

  state(0, 0, window_width, window_height);
}
/**
\internal
\brief The routine sets the background surface brush.
*/
void uxdevice::display_context_t::surface_brush(painter_brush_t &b) {
  BRUSH_SPIN;
  brush = b;
  BRUSH_CLEAR;
  state(0, 0, window_width, window_height);
}
/**
\internal
\brief The routine accepts a drawing output object and adds the
associated render work with the object's coordinate_t.
note stateNotifyComplete must be called after this to inform the renderer
there is work.
*/
void uxdevice::display_context_t::state(std::shared_ptr<drawing_output_t> obj) {
  REGIONS_SPIN;
  std::size_t onum = reinterpret_cast<std::size_t>(obj.get());

  _regions.emplace_back(context_cairo_region_t(
      onum, obj->ink_rectangle.x, obj->ink_rectangle.y,
      obj->ink_rectangle.width, obj->ink_rectangle.height));
  REGIONS_CLEAR;
}
/**
\internal
\brief The routine adds a generalized region area paint for the rendered to
find. note stateNotifyComplete must be called after this to inform the renderer
there is work.
*/
void uxdevice::display_context_t::state(int x, int y, int w, int h) {
  REGIONS_SPIN;
  _regions.emplace_back(context_cairo_region_t{false, x, y, w, h});
  REGIONS_CLEAR;
}
/**
\internal
\brief The routine adds a surface oriented painting request to the render queue.
the items are inserted first before any other so that painting
of a newly resized window area occurs first.
*/
void uxdevice::display_context_t::state_surface(int x, int y, int w, int h) {
  REGIONS_SPIN;
  auto it = std::find_if(_regions.begin(), _regions.end(),
                         [](auto &n) { return !n.bOSsurface; });
  if (it != _regions.end())
    _regions.insert(it, context_cairo_region_t{true, x, y, w, h});
  else
    _regions.emplace_back(context_cairo_region_t{true, x, y, w, h});

  REGIONS_CLEAR;
}
/**
\internal
\brief The routine notifies the condition variable that work
has been requested and should immediately being to render.
Having this as a separate function provides the ability
to add work without rendering occurring. However, message
queue calls this when a resize occurs.
*/
void uxdevice::display_context_t::state_notify_complete(void) {
  cvRenderWork.notify_one();
}

/**
\internal
\brief The routine returns whether work is within the system.
*/
bool uxdevice::display_context_t::state(void) {

  // determine if any on screen elements, or their attribute shared pointers
  // have changed.
  for (auto n : viewport_on) {
    // if (n->is_different_hash())      add_drawable(n);
  }

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
void uxdevice::display_context_t::plot(context_cairo_region_t &plotArea) {
  // if an object is named as what should be updated.
  // setting the flag informs that the contents
  // has been evaluated and ma be removed
  DRAWABLES_ON_SPIN;
  if (viewport_on.empty()) {
    DRAWABLES_ON_CLEAR;
    return;
  }

  auto itUnit = viewport_on.begin();
  bool bDone = false;
  while (!bDone) {

    std::shared_ptr<drawing_output_t> n = *itUnit;
    DRAWABLES_ON_CLEAR;
    n->intersect(plotArea._rect);

    switch (n->overlap) {
    case CAIRO_REGION_OVERLAP_OUT:
      break;
    case CAIRO_REGION_OVERLAP_IN: {
      n->functors_lock(true);
      XCB_SPIN;
      n->fn_draw(*this);
      XCB_CLEAR;
      n->functors_lock(false);
      UX_ERROR_CHECK(cr);
    } break;
    case CAIRO_REGION_OVERLAP_PART: {
      n->functors_lock(true);
      XCB_SPIN;
      n->fn_draw_clipped(*this);
      XCB_CLEAR;
      n->functors_lock(false);
      UX_ERROR_CHECK(cr);
    } break;
    }
    if (clearing_frame)
      bDone = true;

    DRAWABLES_ON_SPIN;
    if (!bDone) {
      itUnit++;
      bDone = itUnit == viewport_on.end();
    }
  }
  DRAWABLES_ON_CLEAR;
}
