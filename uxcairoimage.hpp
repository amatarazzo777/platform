/**
\author Anthony Matarazzo
\file uxworkstate.hpp
\date 5/12/20
\version 1.0
 \details  Routines for cairo image

*/
#pragma once

namespace uxdevice {

cairo_surface_t *readImage(std::string &data, double w = -1, double h = -1);
cairo_status_t read_contents(const gchar *file_name, guint8 **contents,
                             gsize *length);

cairo_surface_t *imageSurfaceSVG(bool bDataPassed, std::string &data,
                                 double width = -1, double height = -1);

#if defined(USE_STACKBLUR)
void blurImage(cairo_surface_t *img, unsigned int radius);

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
