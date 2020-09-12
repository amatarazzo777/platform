/**
\author Anthony Matarazzo
\file uxenums.hpp
\date 5/12/20
\version 1.0
 \details  options for parameters

*/
#pragma once
namespace uxdevice {
enum class antialias {
  def = CAIRO_ANTIALIAS_DEFAULT,

  /* method */
  none = CAIRO_ANTIALIAS_NONE,
  gray = CAIRO_ANTIALIAS_GRAY,
  subPixel = CAIRO_ANTIALIAS_SUBPIXEL,

  /* hints */
  fast = CAIRO_ANTIALIAS_FAST,
  good = CAIRO_ANTIALIAS_GOOD,
  best = CAIRO_ANTIALIAS_BEST
};

enum class filterType {
  fast = CAIRO_FILTER_FAST,
  good = CAIRO_FILTER_GOOD,
  best = CAIRO_FILTER_BEST,
  nearest = CAIRO_FILTER_NEAREST,
  bilinear = CAIRO_FILTER_BILINEAR,
  gaussian = CAIRO_FILTER_GAUSSIAN
};

enum class extendType {
  none = CAIRO_EXTEND_NONE,
  repeat = CAIRO_EXTEND_REPEAT,
  reflect = CAIRO_EXTEND_REFLECT,
  pad = CAIRO_EXTEND_PAD
};

enum class lineCap {
  butt = CAIRO_LINE_CAP_BUTT,
  round = CAIRO_LINE_CAP_ROUND,
  square = CAIRO_LINE_CAP_SQUARE
};

enum class lineJoin {
  miter = CAIRO_LINE_JOIN_MITER,
  round = CAIRO_LINE_JOIN_ROUND,
  bevel = CAIRO_LINE_JOIN_BEVEL
};
using areaType = enum class areaType {
  none,
  circle,
  ellipse,
  rectangle,
  roundedRectangle
};

enum op_t {
  opClear = CAIRO_OPERATOR_CLEAR,
  opSource = CAIRO_OPERATOR_SOURCE,
  opOver = CAIRO_OPERATOR_OVER,
  opIn = CAIRO_OPERATOR_IN,
  opOut = CAIRO_OPERATOR_OUT,
  opAtop = CAIRO_OPERATOR_ATOP,
  opDest = CAIRO_OPERATOR_DEST,
  opDestOver = CAIRO_OPERATOR_DEST_OVER,
  opDestIn = CAIRO_OPERATOR_DEST_IN,
  opDestOut = CAIRO_OPERATOR_DEST_OUT,
  opDestAtop = CAIRO_OPERATOR_DEST_ATOP,
  opXor = CAIRO_OPERATOR_XOR,
  opAdd = CAIRO_OPERATOR_ADD,
  opSaturate = CAIRO_OPERATOR_SATURATE,
  opMultiply = CAIRO_OPERATOR_MULTIPLY,
  opScreen = CAIRO_OPERATOR_SCREEN,
  opOverlay = CAIRO_OPERATOR_OVERLAY,
  opDarken = CAIRO_OPERATOR_DARKEN,
  opLighten = CAIRO_OPERATOR_LIGHTEN,
  opColorDodge = CAIRO_OPERATOR_COLOR_DODGE,
  opColorBurn = CAIRO_OPERATOR_COLOR_BURN,
  opHardLight = CAIRO_OPERATOR_HARD_LIGHT,
  opSoftLight = CAIRO_OPERATOR_SOFT_LIGHT,
  opDifference = CAIRO_OPERATOR_DIFFERENCE,
  opExclusion = CAIRO_OPERATOR_EXCLUSION,
  opHSLHUE = CAIRO_OPERATOR_HSL_HUE,
  opHSLSaturation = CAIRO_OPERATOR_HSL_SATURATION,
  opHSLColor = CAIRO_OPERATOR_HSL_COLOR,
  opHSLLuminosity = CAIRO_OPERATOR_HSL_LUMINOSITY
};
enum class alignment {
  left = PangoAlignment::PANGO_ALIGN_LEFT,
  center = PangoAlignment::PANGO_ALIGN_CENTER,
  right = PangoAlignment::PANGO_ALIGN_RIGHT,
  justified = 4
};
enum class ellipsize {
  none = PANGO_ELLIPSIZE_NONE,
  start = PANGO_ELLIPSIZE_START,
  middle = PANGO_ELLIPSIZE_MIDDLE,
  end = PANGO_ELLIPSIZE_END
};
enum class content {
  color = CAIRO_CONTENT_COLOR,
  alpha = CAIRO_CONTENT_ALPHA,
  all = CAIRO_CONTENT_COLOR_ALPHA
};
} // namespace uxdevice
