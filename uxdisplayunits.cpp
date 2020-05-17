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
void uxdevice::DrawingOutput::invoke(DisplayContext &context) {

  for (auto &fn : options)
    fn->fnOption(context.cr);
}

void uxdevice::DrawingOutput::intersect(cairo_rectangle_t &r) {
  if (!hasInkExtents)
    return;
  cairo_rectangle_int_t rInt = {(int)r.x, (int)r.y, (int)r.width,
                                (int)r.height};
  cairo_region_t *rectregion = cairo_region_create_rectangle(&rInt);
  overlap = cairo_region_contains_rectangle(rectregion, &inkRectangle);
  if (overlap == CAIRO_REGION_OVERLAP_PART) {
    cairo_region_t *dst = cairo_region_create_rectangle(&inkRectangle);
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

void uxdevice::OPTION_FUNCTION::invoke(DisplayContext &context) {
  auto optType = fnOption.target_type().hash_code();

  context.currentUnits.options.remove_if([=](auto &n) {
    return n->fnOption.target_type().hash_code() == optType;
  });

  context.currentUnits.options.push_back(this);
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
bool uxdevice::DRAWTEXT::setLayoutOptions(DisplayContext &context) {
  bool ret = false;

  AREA &a = *area;

  // create layout
  if (!layout)
    layout = pango_cairo_create_layout(context.cr);

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

    inkRectangle = {(int)a.x, (int)a.y, logical_rect.width,
                    logical_rect.height};
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
    cairo_t *cr = cairo_create(shadowImage);
    // offset text by the parameter amounts
    cairo_move_to(cr, textshadow->x, textshadow->y);
    pango_cairo_update_layout(cr, layout);
    textshadow->emit(cr);

    pango_cairo_show_layout(cr, layout);

#if defined(USE_STACKBLUR)
    blurImage(shadowImage, textshadow->radius);

#elif defined(USE_SVGREN)
    cairo_surface_t *blurred = blurImage(shadowImage, textshadow->radius);
    cairo_surface_destroy(shadowImage);
    shadowImage = blurred;
#endif

    cairo_destroy(cr);
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
  if (!(pen || textoutline || textfill) && (area && text && font)) {
    const char *s = "A draw text object must include the following "
                    "attributes. A pen or a textoutline or "
                    " textfill. As well, an area, text and font";
    error(s);
    auto fn = [=](DisplayContext &context) {};
    fnDraw = std::bind(fn, _1);
  } else {

    setLayoutOptions(context);

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

    DrawLogic fnShadow = DrawLogic();
    if (textshadow) {
      createShadow();
      fnShadow = [=](DisplayContext &context) {
        cairo_set_source_surface(context.cr, shadowImage, _inkRectangle.x,
                                 _inkRectangle.y);
        if (overlap == CAIRO_REGION_OVERLAP_IN)
          cairo_rectangle(context.cr, _inkRectangle.x, _inkRectangle.y,
                          _inkRectangle.width, _inkRectangle.height);

        else if (overlap == CAIRO_REGION_OVERLAP_PART)
          cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                          _intersection.width, _intersection.height);

        cairo_fill(context.cr);
      };
    } else {
      fnShadow = [=](DisplayContext &context) {};
    }

    // set the drawing function
    if (bUsePathLayout) {
      // set the drawing function to the one that will be used by the rendering
      // options for text. These functions accept five parameters.
      // These are the clipping versions that have coordinates.
      if (bFilled && bOutline) {
        auto fn = [=](DisplayContext &context) {
          const AREA &a = *area;
          DrawingOutput::invoke(context);
          fnShadow(context);
          cairo_move_to(context.cr, a.x, a.y);
          if (setLayoutOptions(context))
            pango_cairo_update_layout(context.cr, layout);
          pango_cairo_layout_path(context.cr, layout);
          textfill->emit(context.cr, a.x, a.y, a.w, a.h);
          cairo_fill_preserve(context.cr);
          textoutline->emit(context.cr, a.x, a.y, a.w, a.h);
          cairo_stroke(context.cr);
        };
        auto fnClipping = [=](DisplayContext &context) {
          cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                          _intersection.width, _intersection.height);
          cairo_clip(context.cr);
          fn(context);
          cairo_reset_clip(context.cr);
        };
        fnDraw = std::bind(fn, _1);
        fnDrawClipped = std::bind(fnClipping, _1);

        // text is only filled.
      } else if (bFilled) {
        auto fn = [=](DisplayContext &context) {
          const AREA &a = *area;
          DrawingOutput::invoke(context);
          fnShadow(context);
          cairo_move_to(context.cr, a.x, a.y);
          if (setLayoutOptions(context))
            pango_cairo_update_layout(context.cr, layout);
          pango_cairo_layout_path(context.cr, layout);
          textfill->emit(context.cr, a.x, a.y, a.w, a.h);
          cairo_fill(context.cr);
        };
        auto fnClipping = [=](DisplayContext &context) {
          cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                          _intersection.width, _intersection.height);
          cairo_clip(context.cr);
          fn(context);
          cairo_reset_clip(context.cr);
        };
        fnDraw = std::bind(fn, _1);
        fnDrawClipped = std::bind(fnClipping, _1);

        // text is only outlined.
      } else if (bOutline) {
        auto fn = [=](DisplayContext &context) {
          const AREA &a = *area;
          DrawingOutput::invoke(context);
          fnShadow(context);
          cairo_move_to(context.cr, a.x, a.y);
          if (setLayoutOptions(context))
            pango_cairo_update_layout(context.cr, layout);
          pango_cairo_layout_path(context.cr, layout);
          textoutline->emit(context.cr, a.x, a.y, a.w, a.h);
          cairo_stroke(context.cr);
        };
        auto fnClipping = [=](DisplayContext &context) {
          cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                          _intersection.width, _intersection.height);
          cairo_clip(context.cr);
          fn(context);
          cairo_reset_clip(context.cr);
        };
        fnDraw = std::bind(fn, _1);
        fnDrawClipped = std::bind(fnClipping, _1);
      }

    } else {

      // no outline or fill defined, therefore the pen is used.
      // fastest text display uses the function
      //  which uses the rasterizer of the font system
      //        --- see pango_cairo_show_layout
      auto fn = [=](DisplayContext &context) {
        const AREA &a = *area;
        DrawingOutput::invoke(context);
        fnShadow(context);
        cairo_move_to(context.cr, a.x, a.y);
        if (setLayoutOptions(context))
          pango_cairo_update_layout(context.cr, layout);
        pen->emit(context.cr, a.x, a.y, a.w, a.h);
        pango_cairo_show_layout(context.cr, layout);
      };
      auto fnClipping = [=](DisplayContext &context) {
        cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                        _intersection.width, _intersection.height);
        cairo_clip(context.cr);
        fn(context);
        cairo_reset_clip(context.cr);
      };
      fnDraw = std::bind(fn, _1);
      fnDrawClipped = std::bind(fnClipping, _1);
    }
  }
  bprocessed = true;
}

/**
\internal
\brief reads the image and creates a cairo surface image.
*/
void uxdevice::IMAGE::invoke(DisplayContext &context) {
  // set the unit
  context.setUnit(this);

  if (bLoaded)
    return;

  if (!context.currentUnits.area) {
    std::stringstream sError;
    sError << "ERR IMAGE AREA not set. "
           << "  " << __FILE__ << " " << __func__;
    return;
  }

  _image = readImage(_data, context.currentUnits.area->w,
                     context.currentUnits.area->h);

  if (_image)
    bLoaded = true;
  bprocessed = true;
}

/**
\internal
\brief
*/
void uxdevice::DRAWIMAGE::invoke(DisplayContext &context) {
  using namespace std::placeholders;
  DrawLogic fn;
  DrawLogic fnClipping;

  area = context.currentUnits.area;
  image = context.currentUnits.image;
  options = context.currentUnits.options;
  if (!(area && image)) {
    const char *err = "A draw text object must include the following "
                      "attributes. A pen or a textoutline or "
                      " textfill. As well, an area, text and font";
    error(err);

  } else {

    // set the ink area.
    const AREA &a = *area;
    inkRectangle = {(int)a.x, (int)a.y, (int)a.w, (int)a.h};
    _inkRectangle = {(double)inkRectangle.x, (double)inkRectangle.y,
                     (double)inkRectangle.width, (double)inkRectangle.height};
    hasInkExtents = true;
    // set directly callable rendering function.
    fn = [=](DisplayContext &context) {
      DrawingOutput::invoke(context);
      cairo_set_source_surface(context.cr, image->_image, a.x, a.y);
      cairo_rectangle(context.cr, a.x, a.y, a.w, a.h);
      cairo_fill(context.cr);
    };
    fnClipping = [=](DisplayContext &context) {
      cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                      _intersection.width, _intersection.height);
      cairo_clip(context.cr);
      fn(context);
      cairo_reset_clip(context.cr);
    };
  }

  fnDraw = std::bind(fn, _1);
  fnDrawClipped = std::bind(fnClipping, _1);

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
    error(s);
  } else {

    // set the ink area.
    const AREA &bounds = *area;

    switch (bounds.type) {
    case areaType::none:
      break;
    case areaType::rectangle:
      inkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.w,
                      (int)bounds.h};

      break;
    case areaType::roundedRectangle:
      inkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.w,
                      (int)bounds.h};

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

    // no outline or fill defined, therefore Displaythe pen is used.
    std::function<void(DisplayContext & context)> fnprolog;
    std::function<void(AREA & a)> fnadjustForStroke;

    // set the directly callable rendering function
    if (background && pen) {
      fnadjustForStroke = [=](AREA &a) {
        a.shrink(cairo_get_line_width(context.cr) / 2);
      };
      fnprolog = [=](DisplayContext &context) {
        background->emit(context.cr, bounds.x, bounds.y, bounds.w, bounds.h);
        cairo_fill_preserve(context.cr);
        pen->emit(context.cr);
        cairo_stroke(context.cr);
      };
    } else if (pen) {
      fnadjustForStroke = [=](AREA &a) {
        a.shrink(cairo_get_line_width(context.cr));
      };

      fnprolog = [=](DisplayContext &context) {
        pen->emit(context.cr);
        cairo_stroke(context.cr);
      };
    } else {
      fnadjustForStroke = [=](AREA &a) {};
      fnprolog = [=](DisplayContext &context) {
        background->emit(context.cr, bounds.x, bounds.y, bounds.w, bounds.h);
        cairo_fill(context.cr);
      };
    }

    switch (area->type) {
    case areaType::none:
      break;
    case areaType::rectangle: {
      auto fn = [=](DisplayContext &context) {
        AREA a = *area;
        DrawingOutput::invoke(context);
        fnadjustForStroke(a);
        cairo_rectangle(context.cr, a.x, a.y, a.w, a.h);
        fnprolog(context);
      };
      auto fnClipping = [=](DisplayContext &context) {
        DrawingOutput::invoke(context);
        cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                        _intersection.width, _intersection.height);
        cairo_clip(context.cr);
        fn(context);
        fnprolog(context);
        cairo_reset_clip(context.cr);
      };
      fnDraw = std::bind(fn, _1);
      fnDrawClipped = std::bind(fnClipping, _1);

    } break;
    case areaType::roundedRectangle: {
      auto fn = [=](DisplayContext &context) {
        // derived  from svgren by Ivan Gagis <igagis@gmail.com>
        AREA a = *area;
        DrawingOutput::invoke(context);
        fnadjustForStroke(a);

        cairo_move_to(context.cr, a.x + a.rx, a.y);
        cairo_line_to(context.cr, a.x + a.w - a.rx, a.y);

        cairo_save(context.cr);
        cairo_translate(context.cr, a.x + a.w - a.rx, a.y + a.ry);
        cairo_scale(context.cr, a.rx, a.ry);
        cairo_arc(context.cr, 0, 0, 1, -PI / 2, 0);
        cairo_restore(context.cr);

        cairo_line_to(context.cr, a.x + a.w, a.y + a.h - a.ry);

        cairo_save(context.cr);
        cairo_translate(context.cr, a.x + a.w - a.rx, a.y + a.h - a.ry);
        cairo_scale(context.cr, a.rx, a.ry);
        cairo_arc(context.cr, 0, 0, 1, 0, PI / 2);
        cairo_restore(context.cr);

        cairo_line_to(context.cr, a.x + a.rx, a.y + a.h);

        cairo_save(context.cr);
        cairo_translate(context.cr, a.x + a.rx, a.y + a.h - a.ry);
        cairo_scale(context.cr, a.rx, a.ry);
        cairo_arc(context.cr, 0, 0, 1, PI / 2, PI);
        cairo_restore(context.cr);

        cairo_line_to(context.cr, a.x, a.y + a.ry);

        cairo_save(context.cr);
        cairo_translate(context.cr, a.x + a.rx, a.y + a.ry);
        cairo_scale(context.cr, a.rx, a.ry);
        cairo_arc(context.cr, 0, 0, 1, PI, PI * 3 / 2);
        cairo_restore(context.cr);

        cairo_close_path(context.cr);
        fnprolog(context);
      };

      auto fnClipping = [=](DisplayContext &context) {
        DrawingOutput::invoke(context);
        cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                        _intersection.width, _intersection.height);
        cairo_clip(context.cr);
        fn(context);
        fnprolog(context);
        cairo_reset_clip(context.cr);
      };
      fnDraw = std::bind(fn, _1);
      fnDrawClipped = std::bind(fnClipping, _1);
    } break;
    case areaType::circle: {
      auto fn = [=](DisplayContext &context) {
        AREA a = *area;
        DrawingOutput::invoke(context);
        fnadjustForStroke(a);
        cairo_new_sub_path(context.cr);
        cairo_arc(context.cr, a.x + a.rx, a.y + a.rx, a.rx, 0., 2 * PI);
        cairo_close_path(context.cr);
        fnprolog(context);
      };
      auto fnClipping = [=](DisplayContext &context) {
        cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                        _intersection.width, _intersection.height);
        cairo_clip(context.cr);
        fn(context);
        cairo_reset_clip(context.cr);
      };
      fnDraw = std::bind(fn, _1);
      fnDrawClipped = std::bind(fnClipping, _1);
    } break;
    case areaType::ellipse: {
      auto fn = [=](DisplayContext &context) {
        AREA a = *area;
        DrawingOutput::invoke(context);
        fnadjustForStroke(a);
        cairo_save(context.cr);
        cairo_translate(context.cr, a.x + a.rx / 2., a.y + a.ry / 2.);
        cairo_scale(context.cr, a.rx / 2., a.ry / 2.);
        cairo_new_sub_path(context.cr);
        cairo_arc(context.cr, 0., 0., 1., 0., 2 * PI);
        cairo_close_path(context.cr);
        cairo_restore(context.cr);
        fnprolog(context);
      };
      auto fnClipping = [=](DisplayContext &context) {
        cairo_rectangle(context.cr, _intersection.x, _intersection.y,
                        _intersection.width, _intersection.height);
        cairo_clip(context.cr);
        fn(context);
        cairo_reset_clip(context.cr);
      };
      fnDraw = std::bind(fn, _1);
      fnDrawClipped = std::bind(fnClipping, _1);

    } break;
    }
  }

  bprocessed = true;
}
