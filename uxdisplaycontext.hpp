/**
\author Anthony Matarazzo
\file uxdisplaycontext.hpp
\date 5/12/20
\version 1.0
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
class OPTION_FUNCTION;
class DisplayUnit;
class DrawingOutput;
typedef std::list<DisplayUnit *> DisplayUnitCollection;
typedef std::list<DisplayUnit *>::iterator DisplayUnitCollectionIter;
typedef std::list<DrawingOutput *> DrawingOutputCollection;
typedef std::list<DrawingOutput *>::iterator DrawingOutputCollectionIter;

class DisplayContext;
typedef std::function<void(DisplayContext &context)> DrawLogic;

typedef std::list<OPTION_FUNCTION *> CairoOptionFn;

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
  CairoOptionFn options = {};
};

class DisplayContext {
public:
  class CairoRegion {
  public:
    CairoRegion() = delete;
    CairoRegion(int x, int y, int w, int h) {
      rect = {x, y, w, h};
      _rect = {(double)x, (double)y, (double)w, (double)h};

      _ptr = cairo_region_create_rectangle(&rect);
    }
    CairoRegion(std::size_t _obj, int x, int y, int w, int h) : obj(_obj) {
      rect = {x, y, w, h};
      _rect = {(double)x, (double)y, (double)w, (double)h};

      _ptr = cairo_region_create_rectangle(&rect);
    }

    CairoRegion(const CairoRegion &other) { *this = other; }
    CairoRegion &operator=(const CairoRegion &other) {
      _ptr = cairo_region_reference(other._ptr);
      rect = other.rect;
      _rect = other._rect;
      obj = other.obj;
      return *this;
    }
    ~CairoRegion() {
      if (_ptr)
        cairo_region_destroy(_ptr);
    }
    cairo_rectangle_int_t rect = cairo_rectangle_int_t();
    cairo_rectangle_t _rect = cairo_rectangle_t();

    cairo_region_t *_ptr = nullptr;
    bool eval = false;
    std::size_t obj = 0;
  };

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

  DrawingOutputCollection viewportOff = {};
  std::atomic_flag drawables_off_readwrite = ATOMIC_FLAG_INIT;
#define DRAWABLES_OFF_SPIN                                                     \
  while (drawables_off_readwrite.test_and_set(std::memory_order_acquire))
#define DRAWABLES_OFF_CLEAR                                                    \
  drawables_off_readwrite.clear(std::memory_order_release)

  DrawingOutputCollection viewportOn = {};
  std::atomic_flag drawables_on_readwrite = ATOMIC_FLAG_INIT;
#define DRAWABLES_ON_SPIN                                                      \
  while (drawables_on_readwrite.test_and_set(std::memory_order_acquire))
#define DRAWABLES_ON_CLEAR                                                     \
  drawables_on_readwrite.clear(std::memory_order_release)

  bool surfacePrime(void);
  void plot(CairoRegion &plotArea);
  void flush(void);

  void resizeSurface(const int w, const int h);
  void offsetPosition(const int x, const int y);
  void collectables(DisplayUnitCollection *obj);

  void render(void);
  void state(DrawingOutput *obj);
  void state(bool on, int x = 0, int y = 0, int w = 0, int h = 0);
  bool state(void);
  void clear(void);

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
  Paint brush = Paint("white");
  void addDrawable(DrawingOutput *_obj);

  cairo_t *cr = nullptr;

  cairo_rectangle_t viewportRectangle = cairo_rectangle_t();

private:
  std::list<CairoRegion> _regions = {};
  typedef std::list<CairoRegion>::iterator RegionIter;

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

  int offsetx = 0, offsety = 0;

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
