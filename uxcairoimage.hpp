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
\file uxworkstate.hpp
\date 5/12/20
\version 1.0
 \details  Routines for cairo image_block_t

*/
#pragma once

namespace uxdevice {

cairo_surface_t *read_image(std::string &data, double w = -1, double h = -1);
cairo_status_t read_contents(const gchar *file_name, guint8 **contents,
                             gsize *length);

cairo_surface_t *image_surface_SVG(bool bDataPassed, std::string &data,
                                   double width = -1, double height = -1);

#if defined(USE_STACKBLUR)
void blur_image(cairo_surface_t *img, unsigned int radius);

#elif defined(USE_SVGREN)
cairo_surface_t *blurImage(cairo_surface_t *img, unsigned int radius);

cairo_surface_t *cairoImageSurfaceBlur(cairo_surface_t *img,
                                       std::array<double, 2> stdDeviation);
void boxBlurHorizontal(std::uint8_t *dst, const std::uint8_t *src,
                       unsigned dstStride, unsigned srcStride, unsigned width,
                       unsigned height, unsigned boxSize, unsigned boxOffset,
                       unsigned channel);
void boxBlurVertical(std::uint8_t *dst, const std::uint8_t *src,
                     unsigned dstStride, unsigned srcStride, unsigned width,
                     unsigned height, unsigned boxSize, unsigned boxOffset,
                     unsigned channel);

#endif // defined

} // namespace uxdevice
