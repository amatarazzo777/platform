/**
\author Anthony Matarazzo
\file uxevent.hpp
\date 5/12/20
\version 1.0
 \details  paint class

*/
#pragma once

namespace uxdevice {
using Matrix = class Matrix {
public:
  Matrix() { cairo_matrix_init_identity(&_matrix); }
  virtual ~Matrix() {}
  void initIdentity(void) { cairo_matrix_init_identity(&_matrix); };
  void initTranslate(double tx, double ty) {
    cairo_matrix_init_translate(&_matrix, tx, ty);
  }
  void initScale(double sx, double sy) {
    cairo_matrix_init_scale(&_matrix, sx, sy);
  }
  void initRotate(double radians) {
    cairo_matrix_init_rotate(&_matrix, radians);
  }
  void translate(double tx, double ty) {
    cairo_matrix_translate(&_matrix, tx, ty);
  }
  void scale(double sx, double sy) { cairo_matrix_scale(&_matrix, sx, sy); }
  void rotate(double radians) { cairo_matrix_rotate(&_matrix, radians); }
  void invert(void) { cairo_matrix_invert(&_matrix); }
  void multiply(const Matrix &operand, Matrix &result) {
    cairo_matrix_multiply(&result._matrix, &_matrix, &operand._matrix);
  }
  void transformDistance(double &dx, double &dy) {
    double _dx = dx;
    double _dy = dy;
    cairo_matrix_transform_distance(&_matrix, &_dx, &_dy);
    dx = _dx;
    dy = _dy;
  }
  void transformPoint(double &x, double &y) {
    double _x = x;
    double _y = y;
    cairo_matrix_transform_point(&_matrix, &_x, &_y);
    x = _y;
    y = _y;
  }
  cairo_matrix_t _matrix = {0, 0, 0, 0, 0, 0};
};
} // namespace uxdevice
