/**
\author Anthony Matarazzo
\file uxdisplaycontext.hpp
\date 5/12/20
\version 1.0
 \details CLass holds the display window context, gui drawing, cairo
 context, and provides an interface for threads running to
 invalidate part of the surface, resize the surface. The
 CurrentUnits class is within the public members and holds
 the state of the last used data parameters.

*/
#pragma once

namespace uxdevice {

class AREA;
class STRING;
class IMAGE;
class FONT;
class ANTIALIAS;
class TEXTSHADOW;
class TEXTFILL;
class TEXTOUTLINE;
class PEN;
class BACKGROUND;
class ALIGN;
class EVENT;
class DRAWTEXT;
class DRAWIMAGE;
class DRAWAREA;
class FUNCTION;

class CurrentUnits {
public:
  AREA *area = nullptr;
  STRING *text = nullptr;
  IMAGE *image = nullptr;
  FONT *font = nullptr;
  ANTIALIAS *antialias = nullptr;
  TEXTSHADOW *textshadow = nullptr;
  TEXTFILL *textfill = nullptr;
  TEXTOUTLINE *textoutline = nullptr;
  PEN *pen = nullptr;
  BACKGROUND *background = nullptr;
  ALIGN *align = nullptr;
  EVENT *event = nullptr;
};

class DisplayContext {
public:
  DisplayContext(void) {}
  DisplayContext(const DisplayContext &other) { *this = other; }
  DisplayContext &operator=(const DisplayContext &other) {
    windowX = other.windowX;
    windowY = other.windowY;
    windowWidth = other.windowWidth;
    windowHeight = other.windowHeight;
    windowOpen = other.windowOpen;
    if (other.cr)
      cr = cairo_reference(other.cr);
    _regions = other._regions;
    _surfaceRequests = other._surfaceRequests;

#if defined(__linux__)
    xdisplay = other.xdisplay;
    connection = other.connection;
    screen = other.screen;
    window = other.window;
    graphics = other.graphics;
    visualType = other.visualType;
    syms = other.syms;
    xcbSurface = other.xcbSurface;
    preclear = other.preclear;

#elif defined(_WIN64)
    HWND hwnd = 0;

    ID2D1Factory *pD2DFactory = nullptr;
    ID2D1HwndRenderTarget *pRenderTarget = nullptr;
    ID2D1Bitmap *pBitmap = nullptr;

#endif

    return *this;
  }
  bool surfacePrime(void);
  void flush(void);
  void resizeSurface(const int w, const int h);
  void iterate(std::function<void(cairo_rectangle_int_t &n)> fn);
  void state(bool on, int x = 0, int y = 0, int w = 0, int h = 0);

  bool state(void);
  typedef std::list<cairo_rectangle_t> RenderList;
  cairo_region_overlap_t contains(const cairo_rectangle_int_t *rectangle,
                                  RenderList &renderList);

  CurrentUnits currentUnits = CurrentUnits();
  void setUnit(AREA *_area) { currentUnits.area = _area; };
  void setUnit(STRING *_text) { currentUnits.text = _text; };
  void setUnit(IMAGE *_image) { currentUnits.image = _image; };
  void setUnit(FONT *_font) { currentUnits.font = _font; };
  void setUnit(ANTIALIAS *_antialias) { currentUnits.antialias = _antialias; };
  void setUnit(TEXTSHADOW *_textshadow) {
    currentUnits.textshadow = _textshadow;
  };
  void setUnit(TEXTFILL *_textfill) { currentUnits.textfill = _textfill; };
  void setUnit(TEXTOUTLINE *_textoutline) {
    currentUnits.textoutline = _textoutline;
  };
  void setUnit(PEN *_pen) { currentUnits.pen = _pen; };
  void setUnit(BACKGROUND *_background) {
    currentUnits.background = _background;
  };
  void setUnit(ALIGN *_align) { currentUnits.align = _align; };
  void setUnit(EVENT *_event) { currentUnits.event = _event; };

public:
  short windowX = 0;
  short windowY = 0;
  unsigned short windowWidth = 0;
  unsigned short windowHeight = 0;
  bool windowOpen = false;

  cairo_t *cr = nullptr;

private:
  class Region {
  public:
    Region() = delete;
    Region(int x, int y, int w, int h) {
      rect = {x, y, w, h};
      _ptr = cairo_region_create_rectangle(&rect);
    }
    Region(const Region &other) { *this = other; }
    Region &operator=(const Region &other) {
      _ptr = cairo_region_reference(other._ptr);
      rect = other.rect;
      return *this;
    }
    ~Region() {
      if (_ptr)
        cairo_region_destroy(_ptr);
    }
    cairo_rectangle_int_t rect = cairo_rectangle_int_t();
    cairo_region_t *_ptr = nullptr;
    bool eval = false;
  };

private:
  std::list<Region> _regions = {};
  typedef std::list<Region>::iterator RegionIter;

  typedef std::tuple<int, int> _WH;
  std::list<_WH> _surfaceRequests = {};
  typedef std::list<_WH>::iterator SurfaceRequestsIter;

  std::atomic_flag lockRegions = ATOMIC_FLAG_INIT;
#define REGIONS_SPIN while (lockRegions.test_and_set(std::memory_order_acquire))
#define REGIONS_CLEAR lockRegions.clear(std::memory_order_release)

  std::atomic_flag lockSurfaceRequests = ATOMIC_FLAG_INIT;
#define SURFACE_REQUESTS_SPIN                                                  \
  while (lockSurfaceRequests.test_and_set(std::memory_order_acquire))
#define SURFACE_REQUESTS_CLEAR                                                 \
  lockSurfaceRequests.clear(std::memory_order_release)

public:
#if defined(__linux__)
  Display *xdisplay = nullptr;
  xcb_connection_t *connection = nullptr;
  xcb_screen_t *screen = nullptr;
  xcb_drawable_t window = 0;
  xcb_gcontext_t graphics = 0;

  xcb_visualtype_t *visualType = nullptr;

  // xcb -- keyboard
  xcb_key_symbols_t *syms = nullptr;

  cairo_surface_t *xcbSurface = nullptr;
  bool preclear = false;

#elif defined(_WIN64)
  HWND hwnd = 0;

  ID2D1Factory *pD2DFactory = nullptr;
  ID2D1HwndRenderTarget *pRenderTarget = nullptr;
  ID2D1Bitmap *pBitmap = nullptr;

#endif
};
} // namespace uxdevice