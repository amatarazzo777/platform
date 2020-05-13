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

bool uxdevice::DrawingOutput::visible(DisplayContext &context) {
  bool ret = false;

  cairo_rectangle_int_t *extents = nullptr;
  if (hasInkExtents) {
    extents = inkExtents();
  } else if (!context.currentUnits.area) {
    return false;
  } else {
    extents = context.currentUnits.area->inkExtents();
  }

  // when rendering work is established by region allocation,
  // find the paint intersection. cairoRenderRectangle
  // contains the full or partial area to paint
  overlap = context.contains(extents, renderRectangles);

  switch (overlap) {
  case CAIRO_REGION_OVERLAP_IN:
    bCompleted = false;
    ret = true;
    break;

  case CAIRO_REGION_OVERLAP_PART:
    bCompleted = false;
    ret = true;
    break;

  case CAIRO_REGION_OVERLAP_OUT:
    ret = false;
    bCompleted = false;
    break;
  }
  return ret;
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
\brief The drawText function provides textual character rendering.
*/
bool uxdevice::DRAWTEXT::setLayoutOptions(DisplayContext &context) {
  bool ret = false;

  AREA &area = *context.currentUnits.area;

  // create layout
  if (!layout)
    layout = pango_cairo_create_layout(context.cr);

  guint layoutSerial = pango_layout_get_serial(layout);

  const PangoFontDescription *originalDescription =
      pango_layout_get_font_description(layout);
  if (!originalDescription ||
      !pango_font_description_equal(originalDescription,
                                    context.currentUnits.font->fontDescription))
    pango_layout_set_font_description(
        layout, context.currentUnits.font->fontDescription);

  if (context.currentUnits.align) {
    context.currentUnits.align->emit(layout);
  }

  // set the width and height of the layout.
  if (pango_layout_get_width(layout) != area.w * PANGO_SCALE)
    pango_layout_set_width(layout, area.w * PANGO_SCALE);

  if (pango_layout_get_height(layout) != area.h * PANGO_SCALE)
    pango_layout_set_height(layout, area.h * PANGO_SCALE);

  if (context.currentUnits.text->data.compare(
          std::string_view(pango_layout_get_text(layout))) != 0)
    pango_layout_set_text(layout, context.currentUnits.text->data.data(), -1);

  // any changes
  if (layoutSerial != pango_layout_get_serial(layout)) {
    pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);

    cairoInkRectangle = {(int)area.x, (int)area.y, logical_rect.width,
                         logical_rect.height};
    hasInkExtents = true;
    ret = true;
  }

  return ret;
}
/**
\internal
\brief   draws a shadow image of the text within the
offscreen buffer.

*/
void uxdevice::DRAWTEXT::drawShadowOffscreen(DisplayContext &context) {

  // the routine returns true of the layout has been created or changed.
  setLayoutOptions(context);

  // cairoInkRectangle is set by the layout
  if(!textImage)
    textImage = cairo_image_surface_create(
          CAIRO_FORMAT_ARGB32, cairoInkRectangle.width, cairoInkRectangle.height);

  cairo_t *cr = cairo_create(textImage);

  // move to shadow offset
  cairo_move_to(cr, context.currentUnits.textshadow->x,
                context.currentUnits.textshadow->y);

  context.currentUnits.textshadow->emit(cr);
  pango_cairo_update_layout(cr, layout);
  pango_cairo_show_layout(cr, layout);
  cairo_destroy(cr);

#if defined(USE_STACKBLUR)
  blurImage(textImage, context.currentUnits.textshadow->radius);

#elif defined(USE_SVGREN)
  cairo_surface_t *blurred =
      blurImage(textImage, context.currentUnits.textshadow->radius);
  cairo_surface_destroy(textImage);
  textImage = blurred;
#endif
}

/**
\internal
\brief
  Depending on the parameters set for the textual character rendering,
  different pango apis may be used in conjunction with cairo.
  essentially this optimizes the textual rendering
  such that less calls are made and more automatic filling
  occurs within the rendering api.

*/
void uxdevice::DRAWTEXT::drawTextOffscreen(DisplayContext &context) {

  // already manufactured.
  if (bTextImageCompleted)
    return;

  bool bUsePathLayout = false;
  bool bOutline = false;
  bool bFilled = false;

  // if the text is drawn with an outline
  if (context.currentUnits.textoutline) {
    bUsePathLayout = true;
    bOutline = true;
  }

  // if the text is filled with a texture or gradient
  if (context.currentUnits.textfill) {
    bFilled = true;
    bUsePathLayout = true;
  }

  setLayoutOptions(context);

  // cairoInkRectangle is set by the layout
  if(!textImage)
    textImage = cairo_image_surface_create(
          CAIRO_FORMAT_ARGB32, cairoInkRectangle.width, cairoInkRectangle.height);

  cairo_t *cr = cairo_create(textImage);
  cairo_move_to(cr, 0, 0);

  pango_cairo_update_layout(cr, layout);

  // path layout is used for outline and filled options.
  if (bUsePathLayout) {
    // draw the glyph shapes
    pango_cairo_layout_path(cr, layout);

    // text is filled and outlined.
    if (bFilled && bOutline) {
      context.currentUnits.textfill->emit(cr);
      cairo_fill_preserve(cr);
      context.currentUnits.textoutline->emit(cr);
      cairo_stroke(cr);

      // text is only filled.
    } else if (bFilled) {
      context.currentUnits.textfill->emit(cr);
      cairo_fill(cr);

      // text is only outlined.
    } else if (bOutline) {
      context.currentUnits.textoutline->emit(cr);
      cairo_stroke(cr);
    }

  } else {
    // no outline or fill defined, therefore the pen is used.
    context.currentUnits.pen->emit(cr);
    pango_cairo_show_layout(cr, layout);
  }

  cairo_destroy(cr);
}

/**
\internal
\brief
  Renders portions of the textImage to the surface. If a
  textImage does not exist, it is created first. Note that the shadow
  is drawn first. When the images have been rendered,
  bTextImageCompleted is set to true.

*/
void uxdevice::DRAWTEXT::invoke(DisplayContext &context) {

  // check the context parameters before operating
  bool bColor = context.currentUnits.pen || (context.currentUnits.textoutline ||
                                             context.currentUnits.textfill);

  if (!(bColor && (context.currentUnits.area && context.currentUnits.text &&
                   context.currentUnits.font))) {
    std::stringstream sError;
    sError << "ERR DRAWTEXT AREA, STRING, FONT, "
           << "PEN, TEXTOUTLINE or TEXTFILL not set."
           << "  " << __FILE__ << " " << __func__;
    return;
  }

  // checks to see if ink bounds is
  // within view by rectangle test
  // if items are partial, then bComplete is also cleared
  // the renderRectangles is modified so that it will contain
  // the areas that need rendering. There can be one,
  // or multiple of these to note dirty spots.
  if (!visible(context))
    return;

  // means already drawn previously
  if (bCompleted)
    return;

  if (!bTextImageCompleted) {

    if (context.currentUnits.textshadow)
      drawShadowOffscreen(context);

    // create the image of the text.
    // if a shadow exists, it is composited on the surface before the
    // text is rendered.
    drawTextOffscreen(context);
    bTextImageCompleted = true;

  }

  // draw requested portions of the text image.
  for (auto it : renderRectangles) {
    cairo_rectangle(context.cr, it.x, it.y, it.width, it.height);
    cairo_set_source_surface(context.cr, textImage, cairoInkRectangle.x, cairoInkRectangle.y);
    cairo_fill(context.cr);
  }

  bCompleted = true;
}

/**
\internal
\brief The function draws the image
*/
void uxdevice::DRAWIMAGE::invoke(DisplayContext &context) {
  // check the context before operating
  if (!(context.currentUnits.area && context.currentUnits.image)) {
    std::stringstream sError;
    sError << "ERR_DRAWIMAGE AREA or IMAGE not set. "
           << "  " << __FILE__ << " " << __func__;
    return;
  }

  if (!visible(context))
    return;

  // means already drawn previously
  if (bCompleted)
    return;

  IMAGE &image = *context.currentUnits.image;

  if (image._image) {

    // only draw portions requested for paint.
    for (auto it : renderRectangles) {
      cairo_set_source_surface(context.cr, image._image,
                               context.currentUnits.area->x,
                               context.currentUnits.area->y);
      cairo_rectangle(context.cr, it.x, it.y, it.width, it.height);
      cairo_fill(context.cr);
    }

    bCompleted = true;
  }
}

/**
\internal
\brief The function draws an area
*/
void uxdevice::DRAWAREA::invoke(DisplayContext &context) {
  // check the context before operating
  if (!(context.currentUnits.area &&
        (context.currentUnits.background || context.currentUnits.pen))) {
    std::stringstream sError;
    sError << "ERR_DRAWBOX AREA or IMAGE not set. "
           << "  " << __FILE__ << " " << __func__;
    return;
  }

  // include line width for the size of the object
  // if the area will be stroked. Note area is copied
  AREA area = *context.currentUnits.area;
  const AREA &bounds = *context.currentUnits.area;
  if (context.currentUnits.pen) {
    double dWidth = cairo_get_line_width(context.cr) / 2;
    area.shrink(dWidth);
  }

  // set the ink area.
  switch (area.type) {
  case areaType::none:
    break;
  case areaType::rectangle:
    cairoInkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.w,
                         (int)bounds.h};

    break;
  case areaType::roundedRectangle:
    cairoInkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.w,
                         (int)bounds.h};

    break;
  case areaType::circle:
    cairoInkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.rx * 2,
                         (int)bounds.rx * 2};

    break;
  case areaType::ellipse:
    cairoInkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.rx,
                         (int)bounds.ry};

    break;
  }
  hasInkExtents = true;

  if (!visible(context))
    return;

  // means already drawn previously
  if (bCompleted)
    return;



  // draw eveything but clip it.
  for (auto it : renderRectangles) {
    cairo_rectangle(context.cr, it.x,it.y,it.width,it.height);
    cairo_clip(context.cr);

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
    if (context.currentUnits.background) {
      context.currentUnits.background->emit(context.cr, area.x, area.y, area.w,
                                            area.h);
      cairo_fill_preserve(context.cr);
    }
    // use original non adjusted area.
    area = *context.currentUnits.area;
    if (context.currentUnits.pen) {
      context.currentUnits.pen->emit(context.cr, area.x, area.y, area.w, area.h);
      cairo_stroke(context.cr);
    }

    cairo_reset_clip(context.cr);
  }

  bCompleted = true;
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


}
