/**
\file uxdisplayunits.cpp

\author Anthony Matarazzo

\date 5/12/20
\version 1.0

\brief The modules extends the uxdevice namespace. The objects
provided are the base objects for which the caller may instantiate to
draw. While most of these objects are parameters to drawing functions,
the implementation with in this file provides the functional logic.
All objects derive from the DisplayUnit class which contains the
virtual invoke method. Objects that provide drawing operations
can inherit from the DrawingOutput base class which enables visibility
query. As well, a particular note is that parameters that describe colors,
shading or texturing derive and publish the Paint class interface.

*/

#include "uxdevice.hpp"

// error check for this file
#ifdef ERROR_CHECK
#undef ERROR_CHECK
#endif // ERROR_CHECK

#define ERROR_CHECK(obj)                                                       \
  {                                                                            \
    cairo_status_t stat = context.errorCheck(obj);                             \
    if (stat)                                                                  \
      context.errorState(__func__, __LINE__, __FILE__, stat);                  \
  }
#define ERROR_DRAW_PARAM(s) \
  context.errorState(__func__, __LINE__, __FILE__, std::string_view(s)); \
  error(s);

#define ERROR_DESC(s) \
  context.errorState(__func__, __LINE__, __FILE__, std::string_view(s));

void uxdevice::DrawingOutput::invoke(cairo_t *cr) {

  // for (auto &fn : options)
  // fn->fnOption(cr);
  bprocessed = true;
}

void uxdevice::DrawingOutput::intersect(cairo_rectangle_t &r) {
  if (!hasInkExtents)
    return;
  cairo_rectangle_int_t rInt = {(int)r.x, (int)r.y, (int)r.width,
                                (int)r.height};
  cairo_region_t *rectregion = cairo_region_create_rectangle(&rInt);
  cairo_rectangle_int_t objrect = {inkRectangle.x, inkRectangle.y,
                                   inkRectangle.width, inkRectangle.height};

  overlap = cairo_region_contains_rectangle(rectregion, &objrect);
  if (overlap == CAIRO_REGION_OVERLAP_PART) {
    cairo_region_t *dst = cairo_region_create_rectangle(&objrect);
    cairo_region_intersect(dst, rectregion);
    cairo_region_get_extents(dst, &intersection);
    _intersection = {(double)intersection.x, (double)intersection.y,
                     (double)intersection.width, (double)intersection.height};
    cairo_region_destroy(dst);
  }

  cairo_region_destroy(rectregion);
}
void uxdevice::DrawingOutput::intersect(CairoRegion &rectregion) {
  if (!hasInkExtents)
    return;

  cairo_region_t *dst = cairo_region_create_rectangle(&inkRectangle);
  cairo_region_intersect(dst, rectregion._ptr);
  cairo_region_get_extents(dst, &intersection);
  _intersection = {(double)intersection.x, (double)intersection.y,
                   (double)intersection.width, (double)intersection.height};
  cairo_region_destroy(dst);
}

void uxdevice::DrawingOutput::evaluateCache(DisplayContext &context) {
return;
  if (bRenderBufferCached) {
    lastRenderTime = std::chrono::high_resolution_clock::now();
    if (oncethread)
      oncethread.reset();
    return;
  }

  // evaluate Rendering from cache
  if (bFirstTimeRendered) {
    bFirstTimeRendered = false;

  } else if (!oncethread) {
    auto currentPoint = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff =
        currentPoint - lastRenderTime;
    // if rendering requests are for more than 2 frames
    bool useCache = diff.count() < context.cacheThreshold;
    if (useCache) {
      oncethread = std::make_unique<std::thread>(
          [=, &context]() { fnCacheSurface(context); });
      oncethread->detach();
    }
  }
  lastRenderTime = std::chrono::high_resolution_clock::now();
}

void uxdevice::OPTION_FUNCTION::invoke(DisplayContext &context) {
  auto optType = fnOption.target_type().hash_code();

  context.currentUnits.options.remove_if([=](auto &n) {
    return n->fnOption.target_type().hash_code() == optType;
  });

  context.currentUnits.options.emplace_back(this);
}

void uxdevice::AREA::shrink(double a) {
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
\brief emits the alignment setting within the pango layout object.
*/
void uxdevice::ALIGN::emit(PangoLayout *layout) {
  // only change setting if changed, this saves on unnecessary
  // layout context rendering internal to pango
  if (setting == alignment::justified && !pango_layout_get_justify(layout)) {
    pango_layout_set_justify(layout, true);
  } else if (static_cast<alignment>(pango_layout_get_alignment(layout)) !=
             setting) {
    pango_layout_set_justify(layout, false);
    pango_layout_set_alignment(layout, static_cast<PangoAlignment>(setting));
  }
}

/**
\internal
\brief creates if need be and sets options that differ.
*/
bool uxdevice::DRAWTEXT::setLayoutOptions(cairo_t *cr) {
  bool ret = false;

  AREA &a = *area;

  // create layout
  if (!layout)
    layout = pango_cairo_create_layout(cr);

  guint layoutSerial = pango_layout_get_serial(layout);

  const PangoFontDescription *originalDescription =
      pango_layout_get_font_description(layout);
  if (!originalDescription ||
      !pango_font_description_equal(originalDescription, font->fontDescription))
    pango_layout_set_font_description(layout, font->fontDescription);

  if (align) {
    align->emit(layout);
  }

  // set the width and height of the layout.
  if (pango_layout_get_width(layout) != a.w * PANGO_SCALE)
    pango_layout_set_width(layout, a.w * PANGO_SCALE);

  if (pango_layout_get_height(layout) != a.h * PANGO_SCALE)
    pango_layout_set_height(layout, a.h * PANGO_SCALE);

  std::string_view sinternal = std::string_view(pango_layout_get_text(layout));
  if (text->data.compare(sinternal) != 0)
    pango_layout_set_text(layout, text->data.data(), -1);

  // any changes
  if (layoutSerial != pango_layout_get_serial(layout)) {
    pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);
    int tw = std::min((double)logical_rect.width, a.w);
    int th = std::min((double)logical_rect.height, a.h);
    inkRectangle = {(int)a.x, (int)a.y, tw, th};
    _inkRectangle = {(double)inkRectangle.x, (double)inkRectangle.y,
                     (double)inkRectangle.width, (double)inkRectangle.height};

    hasInkExtents = true;
    ret = true;
  }

  return ret;
}

void uxdevice::DRAWTEXT::createShadow(void) {
  if (!shadowImage) {
    shadowImage = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, _inkRectangle.width + textshadow->x,
        _inkRectangle.height + textshadow->y);
    shadowCr = cairo_create(shadowImage);
    // offset text by the parameter amounts
    cairo_move_to(shadowCr, textshadow->x, textshadow->y);
    if (setLayoutOptions(shadowCr))
      pango_cairo_update_layout(shadowCr, layout);
    textshadow->emit(shadowCr);

    pango_cairo_show_layout(shadowCr, layout);

#if defined(USE_STACKBLUR)
    blurImage(shadowImage, textshadow->radius);

#elif defined(USE_SVGREN)
    cairo_surface_t *blurred = blurImage(shadowImage, textshadow->radius);
    cairo_surface_destroy(shadowImage);
    shadowImage = blurred;
#endif
  }
}

/**
\internal
\brief


*/
void uxdevice::DRAWTEXT::invoke(DisplayContext &context) {
  using namespace std::placeholders;

  pen = context.currentUnits.pen;
  textoutline = context.currentUnits.textoutline;
  textfill = context.currentUnits.textfill;
  textshadow = context.currentUnits.textshadow;
  area = context.currentUnits.area;
  text = context.currentUnits.text;
  font = context.currentUnits.font;
  align = context.currentUnits.align;
  options = context.currentUnits.options;

  // check the context parameters before operating
  if (!((pen || textoutline || textfill) && area && text && font)) {
    const char *s = "A draw text object must include the following "
                    "attributes. A pen or a textoutline or "
                    " textfill. As well, an area, text and font";
    ERROR_DRAW_PARAM(s);
    auto fn = [=](DisplayContext &context) {};

    fnBaseSurface = std::bind(fn, _1);
    fnCacheSurface = std::bind(fn, _1);
    fnDraw = std::bind(fn, _1);
    fnDrawClipped = std::bind(fn, _1);
    return;
  }
  // not using the path layout is faster
  // these options change rendering and pango api usage
  bool bUsePathLayout = false;
  bool bOutline = false;
  bool bFilled = false;

  // if the text is drawn with an outline
  if (textoutline) {
    bUsePathLayout = true;
    bOutline = true;
  }

  // if the text is filled with a texture or gradient
  if (textfill) {
    bFilled = true;
    bUsePathLayout = true;
  }

  std::function<void(cairo_t * cr, AREA & a)> fnShadow;
  std::function<void(cairo_t * cr, AREA & a)> fn;

  if (textshadow) {
    fnShadow = [=](cairo_t *cr, AREA &a) {
      createShadow();
      cairo_set_source_surface(cr, shadowImage, a.x, a.y);
      cairo_rectangle(cr, a.x, a.y, a.w, a.h);
      cairo_fill(cr);
    };
  } else {

    fnShadow = [=](cairo_t *cr, AREA &a) {};
  }

  // set the drawing function
  if (bUsePathLayout) {
    // set the drawing function to the one that will be used by the rendering
    // options for text. These functions accept five parameters.
    // These are the clipping versions that have coordinates.
    if (bFilled && bOutline) {
      fn = [=](cairo_t *cr, AREA a) {
        DrawingOutput::invoke(cr);
        if (setLayoutOptions(cr))
          pango_cairo_update_layout(cr, layout);
        fnShadow(cr, a);
        cairo_move_to(cr, a.x, a.y);
        pango_cairo_layout_path(cr, layout);
        textfill->emit(cr, a.x, a.y, a.w, a.h);
        cairo_fill_preserve(cr);
        textoutline->emit(cr, a.x, a.y, a.w, a.h);
        cairo_stroke(cr);
      };

      // text is only filled.
    } else if (bFilled) {
      fn = [=](cairo_t *cr, AREA a) {
        DrawingOutput::invoke(cr);
        if (setLayoutOptions(cr))
          pango_cairo_update_layout(cr, layout);
        fnShadow(cr, a);
        cairo_move_to(cr, a.x, a.y);
        pango_cairo_layout_path(cr, layout);
        textfill->emit(cr, a.x, a.y, a.w, a.h);
        cairo_fill(cr);
      };

      // text is only outlined.
    } else if (bOutline) {
      fn = [=](cairo_t *cr, AREA a) {
        DrawingOutput::invoke(cr);
        if (setLayoutOptions(cr))
          pango_cairo_update_layout(cr, layout);
        fnShadow(cr, a);
        cairo_move_to(cr, a.x, a.y);
        pango_cairo_layout_path(cr, layout);
        textoutline->emit(cr, a.x, a.y, a.w, a.h);
        cairo_stroke(cr);
      };
    }

  } else {

    // no outline or fill defined, therefore the pen is used.
    // fastest text display uses the function
    //  which uses the raster of the font system
    //        --- see pango_cairo_show_layout
    fn = [=](cairo_t *cr, AREA a) {
      DrawingOutput::invoke(cr);
      if (setLayoutOptions(cr))
        pango_cairo_update_layout(cr, layout);
      fnShadow(cr, a);
      cairo_move_to(cr, a.x, a.y);
      pen->emit(cr, a.x, a.y, a.w, a.h);
      pango_cairo_show_layout(cr, layout);
    };
  }

  auto fnCache = [=](DisplayContext &context) {
    // if the item is already cached, return.
    if (bRenderBufferCached)
      return;

    // create off screen buffer
    context.lock(true);
    setLayoutOptions(context.cr);
    context.lock(false);

    ERROR_CHECK(context.cr);

    _buf = context.allocateBuffer(_inkRectangle.width, _inkRectangle.height);

    setLayoutOptions(_buf.cr);
    ERROR_CHECK(_buf.cr);

    AREA a = *area;
#if 0
    if(textfill)
      textfill->translate(-a.x,-a.y);
    if(textoutline)
      textoutline->translate(-a.x,-a.y);
#endif // 0
    a.x = 0;
    a.y = 0;

    fn(_buf.cr, a);
    ERROR_CHECK(_buf.cr);

    cairo_surface_flush(_buf.rendered);
    ERROR_CHECK(_buf.rendered);

    auto drawfn = [=](DisplayContext &context) {
      DrawingOutput::invoke(context.cr);
      cairo_set_source_surface(context.cr, _buf.rendered, area->x, area->y);
      double tw, th;
      tw = std::min(_inkRectangle.width, area->w);
      th = std::min(_inkRectangle.height, area->h);

      cairo_rectangle(context.cr, _inkRectangle.x, _inkRectangle.y, tw, th);
      cairo_fill(context.cr);
    };
    auto fnClipping = [=](DisplayContext &context) {
      DrawingOutput::invoke(context.cr);
      cairo_set_source_surface(context.cr, _buf.rendered, area->x, area->y);
      cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                      _intersection.width, _intersection.height);
      cairo_fill(context.cr);
    };
    functorsLock(true);
    fnDraw = std::bind(drawfn, _1);
    fnDrawClipped = std::bind(fnClipping, _1);
    functorsLock(false);
    bRenderBufferCached = true;
  };

  // two function provide mode switching for the rendering.
  // a cache surface is a new xcb surface that can be threaded in creation
  // base surface issues the drawing commands to the base window drawing cairo
  // context. base surface creation is not threaded.
  fnCacheSurface = fnCache;

  // the base option rendered contains two functions that rendering using the
  // cairo api to the base surface context. One is for clipping and one without.
  auto fnBase = [=](DisplayContext &context) {
    auto drawfn = [=](DisplayContext &context) {
      DrawingOutput::invoke(context.cr);
      fn(context.cr, *area);
      evaluateCache(context);
    };
    auto fnClipping = [=](DisplayContext &context) {
      cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                      _intersection.width, _intersection.height);
      cairo_clip(context.cr);
      DrawingOutput::invoke(context.cr);
      fn(context.cr, *area);
      cairo_reset_clip(context.cr);
      evaluateCache(context);
    };
    functorsLock(true);
    fnDraw = std::bind(drawfn, _1);
    fnDrawClipped = std::bind(fnClipping, _1);
    functorsLock(false);
    if (bRenderBufferCached) {
      context.destroyBuffer(_buf);
      bRenderBufferCached = false;
    }
  };
  context.lock(true);
  setLayoutOptions(context.cr);
  context.lock(false);
  fnBaseSurface = fnBase;
  fnBaseSurface(context);

  bprocessed = true;
}

/**
\internal
\brief reads the image and creates a cairo surface image.
*/
void uxdevice::IMAGE::invoke(DisplayContext &context) {

  if (bLoaded)
    return;
  area = context.currentUnits.area;
  if (!area) {
    const char *s= "An image requires an area size to be defined. ";
    ERROR_DRAW_PARAM(s);
    return;
  }

  auto fnthread = [=,&context]() {
    _image = readImage(_data, area->w, area->h);

    if (_image)
      bLoaded = true;
    else {
      const char *s="The image could not be processed or loaded. ";
      ERROR_DRAW_PARAM(s);
      ERROR_DESC(_data);
    }
  };

  fnthread();
  bprocessed = true;
}

/**
\internal
\brief
*/
void uxdevice::DRAWIMAGE::invoke(DisplayContext &context) {
  using namespace std::placeholders;

  area = context.currentUnits.area;
  image = context.currentUnits.image;
  options = context.currentUnits.options;
  if (!(area && image && image->valid())) {
    const char *s = "A draw image object must include the following "
                      "attributes. A an area and an image.";
    ERROR_DRAW_PARAM(s);
    auto fn = [=](DisplayContext &context) {};

    fnBaseSurface = std::bind(fn, _1);
    fnCacheSurface = std::bind(fn, _1);
    fnDraw = std::bind(fn, _1);
    fnDrawClipped = std::bind(fn, _1);
    return;
  }
  // set the ink area.
  const AREA &a = *area;
  inkRectangle = {(int)a.x, (int)a.y, (int)a.w, (int)a.h};
  _inkRectangle = {(double)inkRectangle.x, (double)inkRectangle.y,
                   (double)inkRectangle.width, (double)inkRectangle.height};
  hasInkExtents = true;
  auto fnCache = [=](DisplayContext &context) {

    // set directly callable rendering function.
    auto fn = [=](DisplayContext &context) {
      if(!image->valid())
        return;
      DrawingOutput::invoke(context.cr);
      cairo_set_source_surface(context.cr, image->_image, a.x, a.y);
      cairo_rectangle(context.cr, a.x, a.y, a.w, a.h);
      cairo_fill(context.cr);
    };
    auto fnClipping = [=](DisplayContext &context) {
      if(!image->valid())
        return;
      DrawingOutput::invoke(context.cr);
      cairo_set_source_surface(context.cr, image->_image, a.x, a.y);
      cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                      _intersection.width, _intersection.height);
      cairo_fill(context.cr);
    };
    functorsLock(true);
    fnDraw = std::bind(fn, _1);
    fnDrawClipped = std::bind(fnClipping, _1);
    functorsLock(false);
    bRenderBufferCached = true;
  };

  // two function provide mode switching for the rendering.
  // a cache surface is a new xcb surface that can be threaded in creation
  // base surface issues the drawing commands to the base window drawing cairo
  // context
  fnCacheSurface = fnCache;
  fnBaseSurface = fnCache;
  fnBaseSurface(context);

  bprocessed = true;
}

/**
\internal
\brief
*/
void uxdevice::DRAWAREA::invoke(DisplayContext &context) {
  using namespace std::placeholders;

  area = context.currentUnits.area;
  background = context.currentUnits.background;
  pen = context.currentUnits.pen;
  options = context.currentUnits.options;

  // check the context before operating
  if (!(area && (background || pen))) {
    const char *s =
        "The draw area command requires an area to be defined. As well"
        "a background, or a pen.";
    ERROR_DRAW_PARAM(s);
    auto fn = [=](DisplayContext &context) {};

    fnBaseSurface = std::bind(fn, _1);
    fnCacheSurface = std::bind(fn, _1);
    fnDraw = std::bind(fn, _1);
    fnDrawClipped = std::bind(fn, _1);
    return;
  }

  // set the ink area.
  const AREA &bounds = *area;

  switch (bounds.type) {
  case areaType::none:
    break;
  case areaType::rectangle:
    inkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.w, (int)bounds.h};

    break;
  case areaType::roundedRectangle:
    inkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.w, (int)bounds.h};

    break;
  case areaType::circle:
    inkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.rx * 2,
                    (int)bounds.rx * 2};

    break;
  case areaType::ellipse:
    inkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.rx,
                    (int)bounds.ry};

    break;
  }
  _inkRectangle = {(double)inkRectangle.x, (double)inkRectangle.y,
                   (double)inkRectangle.width, (double)inkRectangle.height};
  hasInkExtents = true;

  // no outline or fill defined, therefore Display the pen is used.
  std::function<void(cairo_t * cr)> fnprolog;
  std::function<void(cairo_t * cr, AREA & a)> fnadjustForStroke;
  std::function<void(cairo_t * cr, AREA & a)> fn;

  // set the directly callable rendering function
  if (background && pen) {
    fnadjustForStroke = [=](cairo_t *cr, AREA &a) {
      a.shrink(cairo_get_line_width(cr) / 2);
    };
    fnprolog = [=](cairo_t *cr) {
      background->emit(cr, bounds.x, bounds.y, bounds.w, bounds.h);
      cairo_fill_preserve(cr);
      pen->emit(cr);
      cairo_stroke(cr);
    };
  } else if (pen) {
    fnadjustForStroke = [=](cairo_t *cr, AREA &a) {
      a.shrink(cairo_get_line_width(cr) / 2);
    };

    fnprolog = [=](cairo_t *cr) {
      pen->emit(cr);
      cairo_stroke(cr);
    };
  } else {
    fnadjustForStroke = [=](cairo_t *cr, AREA &a) {};
    fnprolog = [=](cairo_t *cr) {
      background->emit(cr, bounds.x, bounds.y, bounds.w, bounds.h);
      cairo_fill(cr);
    };
  }

  switch (area->type) {
  case areaType::none:
    break;

  case areaType::rectangle: {
    fn = [=](cairo_t *cr, AREA a) {
      DrawingOutput::invoke(cr);
      fnadjustForStroke(cr, a);
      cairo_rectangle(cr, a.x, a.y, a.w, a.h);
      fnprolog(cr);
    };

  } break;

  case areaType::roundedRectangle: {
    // from SVGREN
    fn = [=](cairo_t *cr, AREA a) {
      DrawingOutput::invoke(cr);
      fnadjustForStroke(cr, a);

      cairo_move_to(cr, a.x + a.rx, a.y);
      cairo_line_to(cr, a.x + a.w - a.rx, a.y);

      cairo_save(cr);
      cairo_translate(cr, a.x + a.w - a.rx, a.y + a.ry);
      cairo_scale(cr, a.rx, a.ry);
      cairo_arc(cr, 0, 0, 1, -PI / 2, 0);
      cairo_restore(cr);

      cairo_line_to(cr, a.x + a.w, a.y + a.h - a.ry);

      cairo_save(cr);
      cairo_translate(cr, a.x + a.w - a.rx, a.y + a.h - a.ry);
      cairo_scale(cr, a.rx, a.ry);
      cairo_arc(cr, 0, 0, 1, 0, PI / 2);
      cairo_restore(cr);

      cairo_line_to(cr, a.x + a.rx, a.y + a.h);

      cairo_save(cr);
      cairo_translate(cr, a.x + a.rx, a.y + a.h - a.ry);
      cairo_scale(cr, a.rx, a.ry);
      cairo_arc(cr, 0, 0, 1, PI / 2, PI);
      cairo_restore(cr);

      cairo_line_to(cr, a.x, a.y + a.ry);

      cairo_save(cr);
      cairo_translate(cr, a.x + a.rx, a.y + a.ry);
      cairo_scale(cr, a.rx, a.ry);
      cairo_arc(cr, 0, 0, 1, PI, PI * 3 / 2);
      cairo_restore(cr);

      cairo_close_path(cr);
      fnprolog(cr);
    };

  } break;

  case areaType::circle: {
    fn = [=](cairo_t *cr, AREA a) {
      DrawingOutput::invoke(cr);
      fnadjustForStroke(cr, a);
      cairo_new_sub_path(cr);
      cairo_arc(cr, a.x + a.rx, a.y + a.rx, a.rx, 0., 2 * PI);
      cairo_close_path(cr);
      fnprolog(cr);
    };

  } break;

  case areaType::ellipse: {
    fn = [=](cairo_t *cr, AREA a) {
      DrawingOutput::invoke(cr);
      fnadjustForStroke(cr, a);

      cairo_save(cr);
      cairo_translate(cr, a.x + a.rx / 2., a.y + a.ry / 2.);
      cairo_scale(cr, a.rx / 2., a.ry / 2.);
      cairo_new_sub_path(cr);
      cairo_arc(cr, 0, 0, 1., 0., 2 * PI);
      cairo_close_path(cr);
      cairo_restore(cr);
      fnprolog(cr);
    };

  } break;
  }

  // two function provide mode switching for the rendering.
  // a cache surface is a new xcb surface that can be threaded in creation
  auto fnCache = [=](DisplayContext &context) {
    if (bRenderBufferCached)
      return;
    _buf = context.allocateBuffer(_inkRectangle.width, _inkRectangle.height);

    AREA a = *area;
    a.x = 0;
    a.y = 0;

    fn(_buf.cr, a);
    cairo_surface_flush(_buf.rendered);

    auto drawfn = [=](DisplayContext &context) {
      DrawingOutput::invoke(context.cr);
      cairo_set_source_surface(context.cr, _buf.rendered, area->x, area->y);
      cairo_rectangle(context.cr, _inkRectangle.x, _inkRectangle.y,
                      _inkRectangle.width, _inkRectangle.height);
      cairo_fill(context.cr);
    };
    auto fnClipping = [=](DisplayContext &context) {
      DrawingOutput::invoke(context.cr);
      cairo_set_source_surface(context.cr, _buf.rendered, area->x, area->y);
      cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                      _intersection.width, _intersection.height);
      cairo_fill(context.cr);
    };
    functorsLock(true);
    fnDraw = std::bind(drawfn, _1);
    fnDrawClipped = std::bind(fnClipping, _1);
    functorsLock(false);
    bRenderBufferCached = true;
  };
  fnCacheSurface = fnCache;

  // base surface issues the drawing commands to the base window drawing
  // cairo context

  auto fnBase = [=](DisplayContext &context) {
    auto drawfn = [=](DisplayContext &context) {
      DrawingOutput::invoke(context.cr);
      fn(context.cr, *area);
      evaluateCache(context);
    };
    auto fnClipping = [=](DisplayContext &context) {
      DrawingOutput::invoke(context.cr);
      cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                      _intersection.width, _intersection.height);
      cairo_clip(context.cr);
      fn(context.cr, *area);
      cairo_reset_clip(context.cr);
      evaluateCache(context);
    };

    functorsLock(true);
    fnDraw = std::bind(drawfn, _1);
    fnDrawClipped = std::bind(fnClipping, _1);
    functorsLock(false);

    if (bRenderBufferCached) {
      context.destroyBuffer(_buf);
      bRenderBufferCached = false;
    }
  };

  fnBaseSurface = fnBase;
  fnBaseSurface(context);

  bprocessed = true;
}
