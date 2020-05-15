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

  AREA &a = *area;

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
  if (pango_layout_get_width(layout) != a.w * PANGO_SCALE)
    pango_layout_set_width(layout, a.w * PANGO_SCALE);

  if (pango_layout_get_height(layout) != a.h * PANGO_SCALE)
    pango_layout_set_height(layout, a.h * PANGO_SCALE);

  if (context.currentUnits.text->data.compare(
          std::string_view(pango_layout_get_text(layout))) != 0)
    pango_layout_set_text(layout, text->data.data(), -1);

  // any changes
  if (layoutSerial != pango_layout_get_serial(layout)) {
    pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);

    inkRectangle = {(int)a.x, (int)a.y, logical_rect.width,
                  logical_rect.height};
    ret = true;
  }

  return ret;
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
  area = context.currentUnits.area;
  text = context.currentUnits.text;
  font = context.currentUnits.font;

  // check the context parameters before operating
  if (!(pen || textoutline || textfill) && (area && text && font)) {
      const char *s = "A draw text object must include the following "
                      "attributes. A pen or a textoutline or "
                      " textfill. As well, an area, text and font";
      error(s);
      auto fn = [=](DisplayContext &context) {};
      fnDraw = std::bind(fn, _1);
    }
  else {

    setLayoutOptions(context);

    // non using the path layout is faster
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

    // set the drawing function
    if (bUsePathLayout) {
      // set the drawing function to the one that will be used by the rendering
      // options for text.
      if (bFilled && bOutline) {
        auto fn = [=](DisplayContext &context, double x, double y, double w,
                      double h) {
          const AREA &a = *area;
          cairo_rectangle(context.cr, x, y, w, h);
          cairo_clip(context.cr);
          cairo_move_to(context.cr, a.x, a.y);
          pango_cairo_update_layout(context.cr, layout);
          pango_cairo_layout_path(context.cr, layout);
          textfill->emit(context.cr);
          cairo_fill_preserve(context.cr);
          textoutline->emit(context.cr);
          cairo_stroke(context.cr);
          cairo_reset_clip(context.cr);
        };
        fnDraw = std::bind(fn, _1, _2, _3, _4, _5);

        // text is only filled.
      } else if (bFilled) {
        auto fn = [=](DisplayContext &context, double x, double y, double w,
                      double h) {
          const AREA &a = *area;
          cairo_rectangle(context.cr, x, y, w, h);
          cairo_clip(context.cr);
          cairo_move_to(context.cr, a.x, a.y);
          pango_cairo_update_layout(context.cr, layout);
          pango_cairo_layout_path(context.cr, layout);
          textfill->emit(context.cr);
          cairo_fill(context.cr);
          cairo_reset_clip(context.cr);
        };
        fnDraw = std::bind(fn, _1, _2, _3, _4, _5);

        // text is only outlined.
      } else if (bOutline) {
        auto fn = [=](DisplayContext &context, double x, double y, double w,
                      double h) {
          const AREA &a = *area;
          cairo_rectangle(context.cr, x, y, w, h);
          cairo_clip(context.cr);
          cairo_move_to(context.cr, a.x, a.y);
          pango_cairo_update_layout(context.cr, layout);
          pango_cairo_layout_path(context.cr, layout);
          textoutline->emit(context.cr);
          cairo_stroke(context.cr);
          cairo_reset_clip(context.cr);
        };
        fnDraw = std::bind(fn, _1, _2, _3, _4, _5);
      }

    } else {

      // no outline or fill defined, therefore the pen is used.
      // fastest text display uses the function
      //  which uses the rasterizer of the font system
      //        --- see pango_cairo_show_layout
      auto fn = [=](DisplayContext &context, double x, double y, double w,
                    double h) {
        const AREA &a = *area;
        cairo_rectangle(context.cr, x, y, w, h);
        cairo_clip(context.cr);
        cairo_move_to(context.cr, a.x, a.y);
        pango_cairo_update_layout(context.cr, layout);
        pen->emit(context.cr);
        pango_cairo_show_layout(context.cr, layout);
        cairo_reset_clip(context.cr);
      };

      fnDraw = std::bind(fn, _1, _2, _3, _4, _5);
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

  area = context.currentUnits.area;
  image = context.currentUnits.image;

  if (!(area && image)) {
    const char *err = "A draw text object must include the following "
                      "attributes. A pen or a textoutline or "
                      " textfill. As well, an area, text and font";
    error(err);
    fn = [=](DisplayContext &context, double x, double y, double w, double h) {
    };

  } else {

    // set the ink area.
    const AREA &bounds = *area;
    inkRectangle = {(int)bounds.x, (int)bounds.y, (int)bounds.w, (int)bounds.h};
    // set directly callable rendering function.
    fn = [=](DisplayContext &context, double x, double y, double w, double h) {
      cairo_rectangle(context.cr, x, y, w, h);
      cairo_clip(context.cr);
      cairo_set_source_surface(context.cr, image->_image, bounds.x, bounds.y);
      cairo_rectangle(context.cr, bounds.x, bounds.y,bounds.w, bounds.h);
      cairo_fill(context.cr);
      cairo_reset_clip(context.cr);
    };
  }

  fnDraw = std::bind(fn, _1, _2, _3, _4, _5);
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

  // check the context before operating
  if (!(area && (background || pen))) {
    const char *s =
        "The draw area command requires an area to be defined. As well"
        "a background, or a pen.";
    error(s);
    auto fn = [=](DisplayContext &context, double x, double y, double w, double h) {};
    fnDraw = std::bind(fn, _1, _2, _3, _4, _5);

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
    hasInkExtents = true;

    // no outline or fill defined, therefore Displaythe pen is used.
    std::function<void(DisplayContext & context)> fnprolog;

    // set the directly callable rendering function
    if (background && pen) {
      fnprolog = [=](DisplayContext &context) {
        background->emit(context.cr, bounds.x, bounds.y, bounds.w, bounds.h);
        cairo_fill_preserve(context.cr);
        pen->emit(context.cr);
        cairo_stroke(context.cr);
        cairo_reset_clip(context.cr);
      };
    } else if (pen) {
      fnprolog = [=](DisplayContext &context) {
        pen->emit(context.cr);
        cairo_stroke(context.cr);
        cairo_reset_clip(context.cr);
      };
    } else {
      fnprolog = [=](DisplayContext &context) {
        background->emit(context.cr);
        cairo_fill(context.cr);
        cairo_reset_clip(context.cr);
      };
    }

    switch (area->type) {
    case areaType::none:
      break;
    case areaType::rectangle: {
      auto fn = [=](DisplayContext &context, double x, double y, double w,
                    double h) {
        const AREA &a = *area;
        cairo_rectangle(context.cr, x, y, w, h);
        cairo_clip(context.cr);
        cairo_rectangle(context.cr, a.x, a.y, a.w, a.h);
        fnprolog(context);
      };
      fnDraw = std::bind(fn, _1, _2, _3, _4, _5);

      }break;
    case areaType::roundedRectangle: {
      auto fn = [=](DisplayContext &context, double x, double y, double w,
                    double h) {
        // derived  from svgren by Ivan Gagis <igagis@gmail.com>
        const AREA &a = *area;
        cairo_rectangle(context.cr, x, y, w, h);
        cairo_clip(context.cr);

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

      fnDraw = std::bind(fn, _1, _2, _3, _4, _5);
      }break;
    case areaType::circle:{
      auto fn = [=](DisplayContext &context, double x, double y, double w,
                    double h) {
        const AREA &a = *area;
        cairo_rectangle(context.cr, x, y, w, h);
        cairo_clip(context.cr);

        cairo_new_sub_path(context.cr);
        cairo_arc(context.cr, a.x + a.rx, a.y + a.rx, a.rx, 0., 2 * PI);
        cairo_close_path(context.cr);
        fnprolog(context);
      };
      fnDraw = std::bind(fn, _1, _2, _3, _4, _5);
      }break;
    case areaType::ellipse:{
      auto fn = [=](DisplayContext &context, double x, double y, double w,
                    double h) {
        const AREA &a = *area;
        cairo_rectangle(context.cr, x, y, w, h);
        cairo_clip(context.cr);

        cairo_save(context.cr);
        cairo_translate(context.cr, a.x + a.rx / 2., a.y + a.ry / 2.);
        cairo_scale(context.cr, a.rx / 2., a.ry / 2.);
        cairo_new_sub_path(context.cr);
        cairo_arc(context.cr, 0., 0., 1., 0., 2 * PI);
        cairo_close_path(context.cr);
        cairo_restore(context.cr);
        fnprolog(context);
      };
      fnDraw = std::bind(fn, _1, _2, _3, _4, _5);
      } break;
    }
  }

  bprocessed = true;
}
