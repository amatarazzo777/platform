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
/**
\author Anthony Matarazzo
\file uxenums.hpp
\date 5/12/20
\version 1.0
 \details  options for parameters

*/
#pragma once
namespace uxdevice {
enum class antialias_options_t {
  def = CAIRO_ANTIALIAS_DEFAULT,

  /* method */
  off = CAIRO_ANTIALIAS_NONE,
  gray = CAIRO_ANTIALIAS_GRAY,
  subPixel = CAIRO_ANTIALIAS_SUBPIXEL,

  /* hints */
  fast = CAIRO_ANTIALIAS_FAST,
  good = CAIRO_ANTIALIAS_GOOD,
  best = CAIRO_ANTIALIAS_BEST
};

enum class filter_options_t {
  fast = CAIRO_FILTER_FAST,
  good = CAIRO_FILTER_GOOD,
  best = CAIRO_FILTER_BEST,
  nearest = CAIRO_FILTER_NEAREST,
  bilinear = CAIRO_FILTER_BILINEAR,
  gaussian = CAIRO_FILTER_GAUSSIAN
};

enum class extend_options_t {
  off = CAIRO_EXTEND_NONE,
  repeat = CAIRO_EXTEND_REPEAT,
  reflect = CAIRO_EXTEND_REFLECT,
  pad = CAIRO_EXTEND_PAD
};

enum class line_cap_options_t {
  butt = CAIRO_LINE_CAP_BUTT,
  round = CAIRO_LINE_CAP_ROUND,
  square = CAIRO_LINE_CAP_SQUARE
};

enum class line_join_options_t {
  miter = CAIRO_LINE_JOIN_MITER,
  round = CAIRO_LINE_JOIN_ROUND,
  bevel = CAIRO_LINE_JOIN_BEVEL
};

enum class graphic_operator_options_t {
  op_clear = CAIRO_OPERATOR_CLEAR,
  op_source = CAIRO_OPERATOR_SOURCE,
  op_over = CAIRO_OPERATOR_OVER,
  op_in = CAIRO_OPERATOR_IN,
  op_out = CAIRO_OPERATOR_OUT,
  op_at = CAIRO_OPERATOR_ATOP,
  op_dest = CAIRO_OPERATOR_DEST,
  op_dest_over = CAIRO_OPERATOR_DEST_OVER,
  op_dest_in = CAIRO_OPERATOR_DEST_IN,
  op_dest_out = CAIRO_OPERATOR_DEST_OUT,
  op_dest_at = CAIRO_OPERATOR_DEST_ATOP,
  op_xor = CAIRO_OPERATOR_XOR,
  op_add = CAIRO_OPERATOR_ADD,
  op_saturate = CAIRO_OPERATOR_SATURATE,
  op_multiply = CAIRO_OPERATOR_MULTIPLY,
  op_screen = CAIRO_OPERATOR_SCREEN,
  op_overlay = CAIRO_OPERATOR_OVERLAY,
  op_darken = CAIRO_OPERATOR_DARKEN,
  op_lighten = CAIRO_OPERATOR_LIGHTEN,
  op_color_dodge = CAIRO_OPERATOR_COLOR_DODGE,
  op_color_burn = CAIRO_OPERATOR_COLOR_BURN,
  op_hard_light = CAIRO_OPERATOR_HARD_LIGHT,
  op_soft_light = CAIRO_OPERATOR_SOFT_LIGHT,
  op_difference = CAIRO_OPERATOR_DIFFERENCE,
  op_exclusion = CAIRO_OPERATOR_EXCLUSION,
  op_hsl_hue = CAIRO_OPERATOR_HSL_HUE,
  op_hsl_saturation = CAIRO_OPERATOR_HSL_SATURATION,
  op_hsl_color = CAIRO_OPERATOR_HSL_COLOR,
  op_hsl_luminosity = CAIRO_OPERATOR_HSL_LUMINOSITY
};

enum class text_alignment_options_t {
  left = PangoAlignment::PANGO_ALIGN_LEFT,
  center = PangoAlignment::PANGO_ALIGN_CENTER,
  right = PangoAlignment::PANGO_ALIGN_RIGHT,
  justified = 4
};
enum class text_ellipsize_options_t {
  off = PANGO_ELLIPSIZE_NONE,
  start = PANGO_ELLIPSIZE_START,
  middle = PANGO_ELLIPSIZE_MIDDLE,
  end = PANGO_ELLIPSIZE_END
};

enum class content_options_t {
  color = CAIRO_CONTENT_COLOR,
  alpha = CAIRO_CONTENT_ALPHA,
  all = CAIRO_CONTENT_COLOR_ALPHA
};
} // namespace uxdevice
