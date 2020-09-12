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
#include "uxdevice.hpp"

using namespace std;
using namespace uxdevice;

cairo_status_t uxdevice::read_contents(const gchar *file_name,
                                       guint8 **contents, gsize *length) {
  GFile *file;
  GFileInputStream *input_stream;
  gboolean success = FALSE;
  cairo_status_t status = CAIRO_STATUS_SUCCESS;

  file = g_file_new_for_commandline_arg(file_name);
  input_stream = g_file_read(file, NULL, NULL);
  if (input_stream) {
    GFileInfo *file_info;

    file_info = g_file_input_stream_query_info(
        input_stream, G_FILE_ATTRIBUTE_STANDARD_SIZE, NULL, NULL);
    if (file_info) {
      gsize bytes_read;

      *length = g_file_info_get_size(file_info);
      *contents = g_new(guint8, *length);
      success = g_input_stream_read_all(G_INPUT_STREAM(input_stream), *contents,
                                        *length, &bytes_read, NULL, NULL);
      if (!success)
        status = CAIRO_STATUS_READ_ERROR;

      g_object_unref(file_info);
    } else {
      status = CAIRO_STATUS_READ_ERROR;
    }
    g_object_unref(input_stream);
  } else {
    status = CAIRO_STATUS_FILE_NOT_FOUND;
  }

  g_object_unref(file);

  return status;
}

/**
\internal
\brief creates an image_block_t surface from an svg.
*/
cairo_surface_t *uxdevice::image_surface_SVG(bool bDataPassed,
                                             std::string &info, double width,
                                             double height) {

  guint8 *contents = nullptr;
  gsize length = 0;
  RsvgHandle *handle = nullptr;
  RsvgDimensionData dimensions;
  cairo_t *cr = nullptr;
  cairo_surface_t *img = nullptr;
  cairo_status_t status = CAIRO_STATUS_SUCCESS;
  double dWidth = 0;
  double dHeight = 0;

  if (bDataPassed) {
    contents = reinterpret_cast<guint8 *>(info.data());
    length = info.size();
  } else {
    // read the file.
    status = read_contents(info.data(), &contents, &length);
    if (status != CAIRO_STATUS_SUCCESS) {
      goto error_exit;
    }
  }
  // create a rsvg handle
  handle = rsvg_handle_new_from_data(contents, length, NULL);
  if (!handle) {
    g_free(contents);
    status = CAIRO_STATUS_READ_ERROR;
    goto error_exit;
  }

  // scale to the image_block_t requested.
  dWidth = width;
  dHeight = height;
  rsvg_handle_get_dimensions(handle, &dimensions);

  if (dWidth < 1) {
    dWidth = dimensions.width;
  } else {
    dWidth /= dimensions.width;
  }

  if (dHeight < 1) {
    dHeight = dimensions.height;
  } else {
    dHeight /= dimensions.height;
  }

  // render the image_block_t to surface
  img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  status = cairo_surface_status(img);
  if (status != CAIRO_STATUS_SUCCESS) {
    goto error_exit;
  }

  cr = cairo_create(img);
  status = cairo_status(cr);
  if (status != CAIRO_STATUS_SUCCESS) {
    goto error_exit;
  }

  cairo_scale(cr, dWidth, dHeight);
  status = cairo_status(cr);
  if (status != CAIRO_STATUS_SUCCESS) {
    goto error_exit;
  }

  if (!rsvg_handle_render_cairo(handle, cr)) {
    status = CAIRO_STATUS_READ_ERROR;
    goto error_exit;
  }

  // clean up
  cairo_destroy(cr);
  if (!bDataPassed)
    g_free(contents);

  g_object_unref(handle);

  return img;

error_exit:
  if (cr)
    cairo_destroy(cr);
  if (img)
    cairo_surface_destroy(img);
  if (!bDataPassed && contents)
    g_free(contents);
  if (handle)
    g_object_unref(handle);

  return nullptr;
}

/**
\internal
\brief reads the image_block_t and creates a cairo surface image_block_t.
*/
cairo_surface_t *uxdevice::read_image(std::string &data, double w, double h) {
  const string dataPNG = string("data:image/png;base64,");
  const string dataSVG = string("<?xml");
  cairo_surface_t *image = nullptr;

  if (data.size() == 0)
    return nullptr;

  // data is passed as base 64 PNG?
  if (data.compare(0, dataPNG.size(), dataPNG) == 0) {

    typedef struct _readInfo {
      unsigned char *data = nullptr;
      size_t dataLen = 0;
      int val = 0;
      int valB = -8;
      size_t decodePos = 0;
    } readInfo;

    readInfo pngData;

    // from base64 decode snippet in c++
    //  stackoverflow.com/base64 decode snippet in c++ - Stack Overflow.html
    pngData.dataLen = data.size();
    pngData.data = reinterpret_cast<unsigned char *>(data.data());
    pngData.val = 0;
    pngData.valB = -8;
    pngData.decodePos = dataPNG.size();

    cairo_read_func_t fn = [](void *closure, unsigned char *data,
                              unsigned int length) -> cairo_status_t {
      static const uint8_t lookup[] = {
          62,  255, 62,  255, 63,  52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
          255, 0,   255, 255, 255, 255, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
          10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
          255, 255, 255, 255, 63,  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
          36,  37,  38,  39,  40,  41,  42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
      static_assert(sizeof(lookup) == 'z' - '+' + 1);
      readInfo *p = reinterpret_cast<readInfo *>(closure);

      size_t bytesDecoded = 0;
      while (bytesDecoded < length) {

        // requesting more than the size?
        if (p->decodePos > p->dataLen)
          return CAIRO_STATUS_READ_ERROR;

        uint8_t c = p->data[p->decodePos];

        if (c < '+' || c > 'z')
          return CAIRO_STATUS_READ_ERROR;

        c -= '+';
        if (lookup[c] >= 64)
          return CAIRO_STATUS_READ_ERROR;

        p->val = (p->val << 6) + lookup[c];
        p->valB += 6;
        if (p->valB >= 0) {
          *data = static_cast<unsigned char>((p->val >> p->valB) & 0xFF);
          data++;
          bytesDecoded++;
          p->valB -= 8;
        }

        p->decodePos++;
      }

      return CAIRO_STATUS_SUCCESS;
    };

    image = cairo_image_surface_create_from_png_stream(fn, &pngData);

    // if not successful read, set the contents to a null pointer.
    if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS)
      image = nullptr;

    // data in passed as a SVG text?
    // use w and h set by caller.
  } else if (data.compare(0, dataSVG.size(), dataSVG) == 0) {
    image = image_surface_SVG(true, data, w, h);

    // file name?
  } else if (data.find(".png") != std::string::npos) {
    image = cairo_image_surface_create_from_png(data.data());

    // if not successful read, set the contents to a null pointer.
    if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS)
      image = nullptr;

  } else if (data.find(".svg") != std::string::npos) {
    image = image_surface_SVG(false, data, w, h);
  }

  return image;
}

#if defined(USE_STACKBLUR)
/// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
/// Stackblur algorithm by Mario Klingemann
/// Details here:
/// http://www.quasimondo.com/StackBlurForCanvas/StackBlurDemo.html
/// C++ implemenation base from:
/// https://gist.github.com/benjamin9999/3809142
/// http://www.antigrain.com/__code/include/agg_blur.h.html
/// This version works only with RGBA color
void uxdevice::blur_image(cairo_surface_t *img, unsigned int radius) {
  static unsigned short const stackblur_mul[255] = {
      512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335,
      292, 512, 454, 405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335,
      312, 292, 273, 512, 482, 454, 428, 405, 383, 364, 345, 328, 312, 298,
      284, 271, 259, 496, 475, 456, 437, 420, 404, 388, 374, 360, 347, 335,
      323, 312, 302, 292, 282, 273, 265, 512, 497, 482, 468, 454, 441, 428,
      417, 405, 394, 383, 373, 364, 354, 345, 337, 328, 320, 312, 305, 298,
      291, 284, 278, 271, 265, 259, 507, 496, 485, 475, 465, 456, 446, 437,
      428, 420, 412, 404, 396, 388, 381, 374, 367, 360, 354, 347, 341, 335,
      329, 323, 318, 312, 307, 302, 297, 292, 287, 282, 278, 273, 269, 265,
      261, 512, 505, 497, 489, 482, 475, 468, 461, 454, 447, 441, 435, 428,
      422, 417, 411, 405, 399, 394, 389, 383, 378, 373, 368, 364, 359, 354,
      350, 345, 341, 337, 332, 328, 324, 320, 316, 312, 309, 305, 301, 298,
      294, 291, 287, 284, 281, 278, 274, 271, 268, 265, 262, 259, 257, 507,
      501, 496, 491, 485, 480, 475, 470, 465, 460, 456, 451, 446, 442, 437,
      433, 428, 424, 420, 416, 412, 408, 404, 400, 396, 392, 388, 385, 381,
      377, 374, 370, 367, 363, 360, 357, 354, 350, 347, 344, 341, 338, 335,
      332, 329, 326, 323, 320, 318, 315, 312, 310, 307, 304, 302, 299, 297,
      294, 292, 289, 287, 285, 282, 280, 278, 275, 273, 271, 269, 267, 265,
      263, 261, 259};

  static unsigned char const stackblur_shr[255] = {
      9,  11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17,
      17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19,
      19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20,
      20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21,
      21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
      21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
      22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
      22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23,
      23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
      23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
      23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24};

  if (radius > 254)
    return;
  if (radius < 2)
    return;

  cairo_surface_flush(img);

  unsigned char *src =
      reinterpret_cast<unsigned char *>(cairo_image_surface_get_data(img));
  unsigned int w = cairo_image_surface_get_width(img);
  unsigned int h = cairo_image_surface_get_height(img);
  unsigned int x, y, xp, yp, i;
  unsigned int sp;
  unsigned int stack_start;
  unsigned char *stack_ptr;

  unsigned char *src_ptr;
  unsigned char *dst_ptr;

  unsigned long sum_r;
  unsigned long sum_g;
  unsigned long sum_b;
  unsigned long sum_a;
  unsigned long sum_in_r;
  unsigned long sum_in_g;
  unsigned long sum_in_b;
  unsigned long sum_in_a;
  unsigned long sum_out_r;
  unsigned long sum_out_g;
  unsigned long sum_out_b;
  unsigned long sum_out_a;

  unsigned int wm = w - 1;
  unsigned int hm = h - 1;
  unsigned int w4 = cairo_image_surface_get_stride(img);
  unsigned int mul_sum = stackblur_mul[radius];
  unsigned char shr_sum = stackblur_shr[radius];

  unsigned int div = (radius * 2) + 1;
  unsigned char *stack = new unsigned char[div * 4];

  unsigned int minY = 0;
  unsigned int maxY = h;

  for (y = minY; y < maxY; y++) {
    sum_r = sum_g = sum_b = sum_a = sum_in_r = sum_in_g = sum_in_b = sum_in_a =
        sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

    src_ptr = src + w4 * y; // start of line (0,y)

    for (i = 0; i <= radius; i++) {
      stack_ptr = &stack[4 * i];
      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];
      sum_r += src_ptr[0] * (i + 1);
      sum_g += src_ptr[1] * (i + 1);
      sum_b += src_ptr[2] * (i + 1);
      sum_a += src_ptr[3] * (i + 1);
      sum_out_r += src_ptr[0];
      sum_out_g += src_ptr[1];
      sum_out_b += src_ptr[2];
      sum_out_a += src_ptr[3];
    }

    for (i = 1; i <= radius; i++) {
      if (i <= wm)
        src_ptr += 4;
      stack_ptr = &stack[4 * (i + radius)];
      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];
      sum_r += src_ptr[0] * (radius + 1 - i);
      sum_g += src_ptr[1] * (radius + 1 - i);
      sum_b += src_ptr[2] * (radius + 1 - i);
      sum_a += src_ptr[3] * (radius + 1 - i);
      sum_in_r += src_ptr[0];
      sum_in_g += src_ptr[1];
      sum_in_b += src_ptr[2];
      sum_in_a += src_ptr[3];
    }

    sp = radius;
    xp = radius;
    if (xp > wm)
      xp = wm;
    src_ptr = src + 4 * (xp + y * w); //   img.pix_ptr(xp, y);
    dst_ptr = src + y * w4;           // img.pix_ptr(0, y);
    for (x = 0; x < w; x++) {
      dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
      dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
      dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
      dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
      dst_ptr += 4;

      sum_r -= sum_out_r;
      sum_g -= sum_out_g;
      sum_b -= sum_out_b;
      sum_a -= sum_out_a;

      stack_start = sp + div - radius;
      if (stack_start >= div)
        stack_start -= div;
      stack_ptr = &stack[4 * stack_start];

      sum_out_r -= stack_ptr[0];
      sum_out_g -= stack_ptr[1];
      sum_out_b -= stack_ptr[2];
      sum_out_a -= stack_ptr[3];

      if (xp < wm) {
        src_ptr += 4;
        ++xp;
      }

      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];

      sum_in_r += src_ptr[0];
      sum_in_g += src_ptr[1];
      sum_in_b += src_ptr[2];
      sum_in_a += src_ptr[3];
      sum_r += sum_in_r;
      sum_g += sum_in_g;
      sum_b += sum_in_b;
      sum_a += sum_in_a;

      ++sp;
      if (sp >= div)
        sp = 0;
      stack_ptr = &stack[sp * 4];

      sum_out_r += stack_ptr[0];
      sum_out_g += stack_ptr[1];
      sum_out_b += stack_ptr[2];
      sum_out_a += stack_ptr[3];
      sum_in_r -= stack_ptr[0];
      sum_in_g -= stack_ptr[1];
      sum_in_b -= stack_ptr[2];
      sum_in_a -= stack_ptr[3];
    }
  }

  unsigned int minX = 0;
  unsigned int maxX = w;

  for (x = minX; x < maxX; x++) {
    sum_r = sum_g = sum_b = sum_a = sum_in_r = sum_in_g = sum_in_b = sum_in_a =
        sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

    src_ptr = src + 4 * x; // x,0
    for (i = 0; i <= radius; i++) {
      stack_ptr = &stack[i * 4];
      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];
      sum_r += src_ptr[0] * (i + 1);
      sum_g += src_ptr[1] * (i + 1);
      sum_b += src_ptr[2] * (i + 1);
      sum_a += src_ptr[3] * (i + 1);
      sum_out_r += src_ptr[0];
      sum_out_g += src_ptr[1];
      sum_out_b += src_ptr[2];
      sum_out_a += src_ptr[3];
    }
    for (i = 1; i <= radius; i++) {
      if (i <= hm)
        src_ptr += w4; // +stride

      stack_ptr = &stack[4 * (i + radius)];
      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];
      sum_r += src_ptr[0] * (radius + 1 - i);
      sum_g += src_ptr[1] * (radius + 1 - i);
      sum_b += src_ptr[2] * (radius + 1 - i);
      sum_a += src_ptr[3] * (radius + 1 - i);
      sum_in_r += src_ptr[0];
      sum_in_g += src_ptr[1];
      sum_in_b += src_ptr[2];
      sum_in_a += src_ptr[3];
    }

    sp = radius;
    yp = radius;
    if (yp > hm)
      yp = hm;
    src_ptr = src + 4 * (x + yp * w); // img.pix_ptr(x, yp);
    dst_ptr = src + 4 * x;            // img.pix_ptr(x, 0);
    for (y = 0; y < h; y++) {
      dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
      dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
      dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
      dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
      dst_ptr += w4;

      sum_r -= sum_out_r;
      sum_g -= sum_out_g;
      sum_b -= sum_out_b;
      sum_a -= sum_out_a;

      stack_start = sp + div - radius;
      if (stack_start >= div)
        stack_start -= div;
      stack_ptr = &stack[4 * stack_start];

      sum_out_r -= stack_ptr[0];
      sum_out_g -= stack_ptr[1];
      sum_out_b -= stack_ptr[2];
      sum_out_a -= stack_ptr[3];

      if (yp < hm) {
        src_ptr += w4; // stride
        ++yp;
      }

      stack_ptr[0] = src_ptr[0];
      stack_ptr[1] = src_ptr[1];
      stack_ptr[2] = src_ptr[2];
      stack_ptr[3] = src_ptr[3];

      sum_in_r += src_ptr[0];
      sum_in_g += src_ptr[1];
      sum_in_b += src_ptr[2];
      sum_in_a += src_ptr[3];
      sum_r += sum_in_r;
      sum_g += sum_in_g;
      sum_b += sum_in_b;
      sum_a += sum_in_a;

      ++sp;
      if (sp >= div)
        sp = 0;
      stack_ptr = &stack[sp * 4];

      sum_out_r += stack_ptr[0];
      sum_out_g += stack_ptr[1];
      sum_out_b += stack_ptr[2];
      sum_out_a += stack_ptr[3];
      sum_in_r -= stack_ptr[0];
      sum_in_g -= stack_ptr[1];
      sum_in_b -= stack_ptr[2];
      sum_in_a -= stack_ptr[3];
    }
  }

  cairo_surface_mark_dirty(img);
  delete[] stack;
}

#elif defined(USE_SVGREN)
// box blur by Ivan Gagis <igagis@gmail.com>
// svgren project.
cairo_surface_t *uxdevice::blur_image(cairo_surface_t *img,
                                      unsigned int radius) {
  std::array<double, 2> stdDeviation = {static_cast<double>(radius),
                                        static_cast<double>(radius)};
  cairo_surface_t *ret = cairoImageSurfaceBlur(img, stdDeviation);
  cairo_surface_mark_dirty(ret);
  return ret;
}

void uxdevice::boxBlurHorizontal(std::uint8_t *dst, const std::uint8_t *src,
                                 unsigned dstStride, unsigned srcStride,
                                 unsigned width, unsigned height,
                                 unsigned boxSize, unsigned boxOffset,
                                 unsigned channel) {
  if (boxSize == 0) {
    return;
  }
  for (unsigned y = 0; y != height; ++y) {
    unsigned sum = 0;
    for (unsigned i = 0; i != boxSize; ++i) {
      int pos = i - boxOffset;
      pos = std::max(pos, 0);
      pos = std::min(pos, int(width - 1));
      sum += src[(srcStride * y) + (pos * sizeof(std::uint32_t)) + channel];
    }
    for (unsigned x = 0; x != width; ++x) {
      int tmp = x - boxOffset;
      int last = std::max(tmp, 0);
      int next = std::min(tmp + boxSize, width - 1);

      dst[(dstStride * y) + (x * sizeof(std::uint32_t)) + channel] =
          sum / boxSize;

      sum += src[(srcStride * y) + (next * sizeof(std::uint32_t)) + channel] -
             src[(srcStride * y) + (last * sizeof(std::uint32_t)) + channel];
    }
  }
}

void uxdevice::boxBlurVertical(std::uint8_t *dst, const std::uint8_t *src,
                               unsigned dstStride, unsigned srcStride,
                               unsigned width, unsigned height,
                               unsigned boxSize, unsigned boxOffset,
                               unsigned channel) {
  if (boxSize == 0) {
    return;
  }
  for (unsigned x = 0; x != width; ++x) {
    unsigned sum = 0;
    for (unsigned i = 0; i != boxSize; ++i) {
      int pos = i - boxOffset;
      pos = std::max(pos, 0);
      pos = std::min(pos, int(height - 1));
      sum += src[(srcStride * pos) + (x * sizeof(std::uint32_t)) + channel];
    }
    for (unsigned y = 0; y != height; ++y) {
      int tmp = y - boxOffset;
      int last = std::max(tmp, 0);
      int next = std::min(tmp + boxSize, height - 1);

      dst[(dstStride * y) + (x * sizeof(std::uint32_t)) + channel] =
          sum / boxSize;

      sum += src[(x * sizeof(std::uint32_t)) + (next * srcStride) + channel] -
             src[(x * sizeof(std::uint32_t)) + (last * srcStride) + channel];
    }
  }
}

cairo_surface_t *
uxdevice::cairoImageSurfaceBlur(cairo_surface_t *img,
                                std::array<double, 2> stdDeviation) {
  // NOTE: see https://www.w3.org/TR/SVG/filters.html#feGaussianBlurElement
  // for Gaussian Blur approximation algorithm.
  int w = cairo_image_surface_get_width(img);
  int h = cairo_image_surface_get_height(img);
  int stride = cairo_image_surface_get_stride(img);
  std::uint8_t *src = cairo_image_surface_get_data(img);

  std::array<unsigned, 2> d;
  for (unsigned i = 0; i != 2; ++i) {
    d[i] = unsigned(float(stdDeviation[i]) * 3 * std::sqrt(2 * PI) / 4 + 0.5f);
  }

  cairo_surface_t *ret = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  std::uint8_t *retData = cairo_image_surface_get_data(ret);

  std::vector<std::uint8_t> tmp(stride * h);

  std::array<unsigned, 3> hBoxSize;
  std::array<unsigned, 3> hOffset;
  std::array<unsigned, 3> vBoxSize;
  std::array<unsigned, 3> vOffset;
  if (d[0] % 2 == 0) {
    hOffset[0] = d[0] / 2;
    hBoxSize[0] = d[0];
    hOffset[1] = d[0] / 2 - 1; // it is ok if d[0] is 0 and -1 will give a large
    // number because box size is also 0 in that case
    // and blur will have no effect anyway
    hBoxSize[1] = d[0];
    hOffset[2] = d[0] / 2;
    hBoxSize[2] = d[0] + 1;
  } else {
    hOffset[0] = d[0] / 2;
    hBoxSize[0] = d[0];
    hOffset[1] = d[0] / 2;
    hBoxSize[1] = d[0];
    hOffset[2] = d[0] / 2;
    hBoxSize[2] = d[0];
  }

  if (d[1] % 2 == 0) {
    vOffset[0] = d[1] / 2;
    vBoxSize[0] = d[1];
    vOffset[1] = d[1] / 2 - 1; // it is ok if d[0] is 0 and -1 will give a large
    // number because box size is also 0 in that case
    // and blur will have no effect anyway
    vBoxSize[1] = d[1];
    vOffset[2] = d[1] / 2;
    vBoxSize[2] = d[1] + 1;
  } else {
    vOffset[0] = d[1] / 2;
    vBoxSize[0] = d[1];
    vOffset[1] = d[1] / 2;
    vBoxSize[1] = d[1];
    vOffset[2] = d[1] / 2;
    vBoxSize[2] = d[1];
  }

  for (auto channel = 0; channel != 4; ++channel) {
    boxBlurHorizontal(tmp.data(), src, stride, stride, w, h, hBoxSize[0],
                      hOffset[0], channel);
  }
  for (auto channel = 0; channel != 4; ++channel) {
    boxBlurHorizontal(retData, tmp.data(), stride, stride, w, h, hBoxSize[1],
                      hOffset[1], channel);
  }
  for (auto channel = 0; channel != 4; ++channel) {
    boxBlurHorizontal(tmp.data(), retData, stride, stride, w, h, hBoxSize[2],
                      hOffset[2], channel);
  }
  for (auto channel = 0; channel != 4; ++channel) {
    boxBlurVertical(retData, tmp.data(), stride, stride, w, h, vBoxSize[0],
                    vOffset[0], channel);
  }
  for (auto channel = 0; channel != 4; ++channel) {
    boxBlurVertical(tmp.data(), retData, stride, stride, w, h, vBoxSize[1],
                    vOffset[1], channel);
  }
  for (auto channel = 0; channel != 4; ++channel) {
    boxBlurVertical(retData, tmp.data(), stride, stride, w, h, vBoxSize[2],
                    vOffset[2], channel);
  }

  return ret;
}

#endif
