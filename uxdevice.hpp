/**
\author Anthony Matarazzo
\file uxdevice.hpp
\date 11/19/19
\version 1.0
\brief interface for the platform.
*/
#pragma once

/**
\addtogroup Library Build Options
\brief Library Options
\details These options provide the selection to configure selection
options when compiling the source.
@{
*/

#define DEFAULT_TEXTFACE "arial"
#define DEFAULT_TEXTSIZE 12
#define DEFAULT_TEXTCOLOR 0

/**
\def USE_IMAGE_MAGICK
\brief the ImageMagick library is used for image
 * loading and processing.

*/
//#define USE_IMAGE_MAGICK

/**
\def USE_PANGO
\brief THe pango text rendering library is used.
If this is not used the base cairo text rendering functions are used.

*/
#define USE_PANGO

#define USE_FRAMEBUFFER

/**
\def USE_CHROMIUM_EMBEDDED_FRAMEWORK
\brief The system will be configured to use the CEF system.
*/
//#define USE_CHROMIUM_EMBEDDED_FRAMEWORK

/**
\def USE_DEBUG_CONSOLE
*/
#define USE_DEBUG_CONSOLE

/** @} */

#include <algorithm>
#include <any>
#include <array>
#include <cstdint>

#if defined(_WIN64)
typedef unsigned char u_int8_t;
#endif

#include <assert.h>
#include <cctype>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

/*************************************
OS SPECIFIC HEADERS
*************************************/

#if defined(__linux__)
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include <sys/types.h>
#include <xcb/xcb_keysyms.h>



#elif defined(_WIN64)
// Windows Header Files:
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

// auto linking of direct x
#pragma comment(lib, "d2d1.lib")

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#endif

#ifdef USE_IMAGE_MAGICK
#include <Magick++.h>
#endif // image processing

#include <cairo-xcb.h>
#include <cairo.h>

#if defined(USE_PANGO)
#include <pango/pangocairo.h>
#endif // defined

std::string _errorReport(std::string sourceFile, int ln, std::string sfunc,
                         std::string cond, std::string ecode);
typedef std::function<void(const std::string &err)> errorHandler;

namespace uxdevice {

class event;

/**
\enum eventType
\brief the eventType enumeration contains a sequenced value for all of the
events that can be dispatched by the system.
*/
enum class eventType : uint8_t {
  paint,
  focus,
  blur,
  resize,
  keydown,
  keyup,
  keypress,
  mouseenter,
  mousemove,
  mousedown,
  mouseup,
  click,
  dblclick,
  contextmenu,
  wheel,
  mouseleave
};

/// \typedef eventHandler is used to note and declare a lambda function for
/// the specified event.
typedef std::function<void(const event &et)> eventHandler;

/**
\class event

\brief the event class provides the communication between the event system and
the caller. There is one event class for all of the distinct events. Simply
different constructors are selected based upon the necessity of information
given within the parameters.
*/
using event = class event {
public:
  event(const eventType &et) {
    evtType = et;
    bVirtualKey = false;
  }
  event(const eventType &et, const char &k) {
    evtType = et;
    key = k;
    bVirtualKey = false;
  }
  event(const eventType &et, const unsigned int &vk) {
    evtType = et;
    virtualKey = vk;
    bVirtualKey = true;
  }

  event(const eventType &et, const short &mx, const short &my,
        const short &mb_dis) {
    evtType = et;
    mousex = mx;
    mousey = my;
    if (et == eventType::wheel)
      wheelDistance = mb_dis;
    else
      mouseButton = static_cast<char>(mb_dis);
    bVirtualKey = false;
  }
  event(const eventType &et, const short &w, const short &h) {
    evtType = et;
    width = w;
    height = h;
    mousex = w;
    mousey = h;
    bVirtualKey = false;
  }

  event(const eventType &et, const short &_x, const short &_y, const short &w,
        const short &h) {
    evtType = et;
    x = _x;
    y = _y;
    width = w;
    height = h;
    bVirtualKey = false;
  }
  event(const eventType &et, const short &distance) {
    evtType = et;
    wheelDistance = distance;
    bVirtualKey = false;
  }
  ~event(){};

public:
  eventType evtType;
  bool bVirtualKey;
  char key;
  unsigned int virtualKey;
  std::wstring unicodeKeys;
  short mousex;
  short mousey;
  char mouseButton;
  short width;
  short height;
  short wheelDistance;
  short x, y;
};

/**
 \details
*/

/**
\internal
\class platform
\brief The platform contains logic to connect the document object model to the
local operating system.
*/
class platform {
public:
  platform(const eventHandler &evtDispatcher, const errorHandler &fn);
  ~platform();
  void openWindow(const std::string &sWindowTitle, const unsigned short width,
                  const unsigned short height);
  void closeWindow(void);

  void render(const event &evt);
  void processEvents(void);
  void dispatchEvent(const event &e);

  void clear(void);
  void text(const std::string &s);
  void text(const std::stringstream &s);
  void image(const std::string &s);
  void pen(u_int32_t c);
  void pen(double _r, double _g, double _b);
  void pen(double _r, double _g, double _b, double _a);
  void pen(const std::string &c);
  void background(u_int32_t c);
  void background(double _r, double _g, double _b);
  void background(double _r, double _g, double _b, double _a);
  void background(const std::string &c);

  void fontDescription(const std::string &s);
  void area(double x, double y, double w, double h);
  void drawText(void);
  void drawImage(void);
  void drawBox(void);

  void translate(double x, double y);
  void rotate(double angle);
  void scale(double x, double y);

  void arc(double xc, double yc, double radius, double angle1, double angle2);
  void arc_negative(double xc, double yc, double radius, double angle1,
                    double angle2);
  void curve_to(double x1, double y1, double x2, double y2, double x3,
                double y3);

  void line_to(double x, double y);
  void move_to(double x, double y);
  void rectangle(double x, double y, double width, double height);
  void stroke(void);

#if defined(USE_IMAGE_MAGICK)
  void addNoise(Magick::NoiseType noiseType_);
  void addNoiseChannel(const Magick::ChannelType channel_,
                       const Magick::NoiseType noiseType_);
  void blur(const double radius_ = 1, const double sigma_ = 0.5);
  void blurChannel(const Magick::ChannelType channel_,
                   const double radius_ = 0.0, const double sigma_ = 1.0);
  void charcoal(const double radius_ = 1, const double sigma_ = 0.5);

  void colorize(const unsigned int alpha_, const Magick::Color &penColor_);
  void colorize(const unsigned int alphaRed_, const unsigned int alphaGreen_,
                const unsigned int alphaBlue_, const Magick::Color &penColor_);

  void contrast(bool sharpen_);
  void cycleColormap(int amount_);
  void despeckle(void);
  void distort(const Magick::DistortMethod method,
               const std::vector<double> &args, const bool bestfit = false);
  void equalize(void);
  void enhance(void);
  void gaussianBlur(const double width_, const double sigma_);
  void gaussianBlurChannel(const Magick::ChannelType channel_,
                           const double radius_ = 0.0,
                           const double sigma_ = 1.0);
  void implode(const double factor_);
  void medianFilter(const double radius_ = 0.0);
  void modulate(double brightness_, double saturation_, double hue_);
  void motionBlur(const double radius_, const double sigma_,
                  const double angle_);
  void negate(bool grayscale_ = false);
  void normalize(void);
  void oilPaint(const double radius_ = 3, const double sigma_ = 1);
  void raise(const Magick::Geometry &geometry_ = "6x6+0+0",
             bool raisedFlag_ = false);
  void shade(double azimuth_ = 30, double elevation_ = 30,
             bool colorShading_ = false);
  void shadow(const double percent_opacity = 80, const double sigma_ = 0.5,
              const ssize_t x_ = 0, const ssize_t y_ = 0);
  void sharpen(const double radius_ = 1, const double sigma_ = 0.5);
#endif // defined

private:
  void drawCaret(const int x, const int y, const int h);

  void messageLoop(void);
  void test(int x, int y);

  void flip(void);
  void resize(const int w, const int h);

#if defined(__linux__)

#elif defined(_WIN64)
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
  bool initializeVideo(void);
  void terminateVideo(void);
#endif

private:
  /**
   \details
  */

  class DisplayUnitContext;
#define IDX(NAME) platform::contextUnitIndex::NAME##_idx

  enum contextUnitIndex : std::size_t {
    AREA_idx,
    STRING_idx,
    IMAGE_idx,
    FONT_idx,
    PEN_idx,
    BACKGROUND_idx,
    ALIGN_idx,
    EVENT_idx,
    DRAWTEXT_idx,
    DRAWIMAGE_idx,
    DRAWBOX_idx,

    IMAGEPROCESS_idx,
    FUNCTION_idx,
    IMAGECHAIN_idx,

    MAX_idx

  };
#define VIRTUAL_INDEX(NAME)                                                    \
  inline std::size_t index(void) { return contextUnitIndex::NAME##_idx; }

  using DisplayUnit = class DisplayUnit {
  public:
    virtual std::size_t index() = 0;
    virtual ~DisplayUnit() {}
    virtual void invoke(const DisplayUnitContext &context) = 0;
  };

  using AREA = class AREA : public DisplayUnit {
  public:
    VIRTUAL_INDEX(AREA);

    AREA(void) {}
    AREA(double _x, double _y, double _w, double _h)
        : x(_x), y(_y), w(_w), h(_h) {}
    ~AREA() {}
    double x = 0.0, y = 0.0, w = 0.0, h = 0.0;
    void invoke(const DisplayUnitContext &context);
  };

  using STRING = class STRING : public DisplayUnit {
  public:
    VIRTUAL_INDEX(STRING);
    STRING(const std::string &s) : data(s) {}
    ~STRING() {}
    std::string data;
    void invoke(const DisplayUnitContext &context);
  };

  using FONT = class FONT : public DisplayUnit {
  public:
    VIRTUAL_INDEX(FONT);

    FONT(const std::string &s)
        : description(s), pointSize(DEFAULT_TEXTSIZE), bProvidedName(false),
          bProvidedSize(false), bProvidedDescription(true) {}
    FONT(const std::string &s, const double &pt)
        : description(s), pointSize(pt) {}
    ~FONT() {

#if defined(USE_PANGO)
      if (fontDescription)
        pango_font_description_free(fontDescription);
#endif // defined
    }
    std::string description = DEFAULT_TEXTFACE;
    double pointSize = DEFAULT_TEXTSIZE;
    bool bProvidedName = false;
    bool bProvidedSize = false;
    bool bProvidedDescription = false;

#if defined(USE_PANGO)
    PangoFontDescription *fontDescription = nullptr;
#endif // defined

    void invoke(const DisplayUnitContext &context);
  };

  using BASECOLOR = class BASECOLOR {
  public:
    BASECOLOR(u_int32_t c) {
      u_int8_t _r = c >> 16;
      u_int8_t _g = c >> 8;
      u_int8_t _b = c;
      u_int8_t _a = c >> 24;

      r = _r / 255.0;
      g = _g / 255.0;
      b = _b / 255.0;
      a = 1.0;
    }

    BASECOLOR(double _r, double _g, double _b) {
      r = _r;
      g = _g;
      b = _b;
      a = 1.0;
    }

    BASECOLOR(double _r, double _g, double _b, double _a) {
      r = _r;
      g = _g;
      b = _b;
      a = _a;
    }

    BASECOLOR(const std::string &n) {
#if defined(USE_PANGO) && defined(USE_IMAGE_MAGICK)
      if (pango_color_parse(&pangoColor, n.data())) {
        r = pangoColor.red / 65535.0;
        g = pangoColor.blue / 65535.0;
        b = pangoColor.green / 65535.0;
        // a = pangoColor.alpha / 65535.0;
      }
      magicColor = Magick::Color(r * QuantumRange, g * QuantumRange,
                                 b * QuantumRange, a * QuantumRange);

#elif defined(USE_IMAGE_MAGICK)
      magicColor = Magick::Color(n);
      r = magicColor.quantumRed() / QuantumRange;
      g = magicColor.quantumGreen() / QuantumRange;
      b = magicColor.quantumBlue() / QuantumRange;
      a = magicColor.quantumAlpha() / QuantumRange;

#endif
    }

    ~BASECOLOR() {}
    unsigned int data;
    double r = 0.0, g = 0.0, b = 0.0, a = 1.0;

#if defined(USE_PANGO)
    PangoColor pangoColor;
#endif // defined

#if defined(USE_IMAGE_MAGICK)
    Magick::Color magicColor;
#endif
  };

  using PEN = class PEN : public DisplayUnit, public BASECOLOR {
  public:
    VIRTUAL_INDEX(PEN);
    PEN(u_int32_t c) : BASECOLOR(c) {}
    PEN(double _r, double _g, double _b) : BASECOLOR(_r, _g, _b) {}
    PEN(double _r, double _g, double _b, double _a)
        : BASECOLOR(_r, _g, _b, _a) {}
    PEN(const std::string &n) : BASECOLOR(n) {}
    ~PEN() {}
    void invoke(const DisplayUnitContext &context);
  };

  using BACKGROUND = class BACKGROUND : public DisplayUnit, public BASECOLOR {
  public:
  public:
    VIRTUAL_INDEX(BACKGROUND);
    BACKGROUND(u_int32_t c) : BASECOLOR(c) {}
    BACKGROUND(double _r, double _g, double _b) : BASECOLOR(_r, _g, _b) {}
    BACKGROUND(double _r, double _g, double _b, double _a)
        : BASECOLOR(_r, _g, _b, _a) {}
    BACKGROUND(const std::string &n) : BASECOLOR(n) {}
    ~BACKGROUND() {}
    void invoke(const DisplayUnitContext &context);
  };

  using ALIGN = class ALIGN : public DisplayUnit {
  public:
    VIRTUAL_INDEX(ALIGN);

    ALIGN(const char _aln) : a(_aln) {}
    ~ALIGN() {}
    char a = 'l';
    void invoke(const DisplayUnitContext &context);
  };

  using EVENT = class EVENT : public DisplayUnit {
  public:
    VIRTUAL_INDEX(EVENT);

    EVENT(eventHandler _eh) : fn(_eh){};
    ~EVENT() {}
    eventHandler fn;
    void invoke(const DisplayUnitContext &context);
  };

  using DRAWTEXT = class DRAWTEXT : public DisplayUnit {
  public:
    VIRTUAL_INDEX(DRAWTEXT);

    DRAWTEXT(void) : beginIndex(0), endIndex(0), bEntire(true) {}
    DRAWTEXT(std::size_t _b, std::size_t _e)
        : beginIndex(_b), endIndex(_e), bEntire(false) {}
    ~DRAWTEXT() {
#if defined(USE_PANGO)
      if (layout)
        g_object_unref(layout);
#endif
    }
    std::size_t beginIndex = 0;
    std::size_t endIndex = 0;
    bool bEntire = true;
#if defined(USE_PANGO)
    PangoLayout *layout = nullptr;
#endif // defined

    void invoke(const DisplayUnitContext &context);
  };
  using DRAWIMAGE = class DRAWIMAGE : public DisplayUnit {
  public:
    VIRTUAL_INDEX(DRAWIMAGE);
    DRAWIMAGE(const AREA &a) : src(a) { bEntire = false; }
    DRAWIMAGE(void) {}
    ~DRAWIMAGE() {}
    void invoke(const DisplayUnitContext &context);

  private:
    AREA src;
    bool bEntire = true;
  };

  using DRAWBOX = class DRAWBOX : public DisplayUnit {
  public:
    VIRTUAL_INDEX(DRAWBOX);
    DRAWBOX() {}
    ~DRAWBOX() {}
    void invoke(const DisplayUnitContext &context);

  private:
  };

  typedef std::function<void(cairo_t *cr)> CAIRO_FUNCTION;
  using FUNCTION = class FUNCTION : public DisplayUnit {
  public:
    VIRTUAL_INDEX(FUNCTION);
    FUNCTION(CAIRO_FUNCTION _func) : func(_func) {}
    ~FUNCTION() {}
    void invoke(const DisplayUnitContext &context);

  private:
    CAIRO_FUNCTION func;
  };

  using IMAGE = class IMAGE : public DisplayUnit {
  public:
    enum ImageSystemType { none, cairo_type, magick_type };

    VIRTUAL_INDEX(IMAGE);
    IMAGE(const std::string &_fileName) : fileName(_fileName) {}
    ~IMAGE() {
      if (cairo)
        cairo_surface_destroy(cairo);
    }
    cairo_surface_t *cairo = nullptr;

#if defined(USE_IMAGE_MAGICK)
    Magick::Image magick = nullptr;
#endif
    void invoke(const DisplayUnitContext &context);

#if defined(USE_IMAGE_MAGICK)
    void toCairo(void);
    void toMagick(void);
#endif

    std::string fileName;
    bool bLoaded = false;
#if defined(USE_IMAGE_MAGICK)
    ImageSystemType type = ImageSystemType::none;
#endif
  };

#if defined(USE_IMAGE_MAGICK)
  typedef std::function<void(Magick::Image *img)> ImageFunction;

  using IMAGEPROCESS = class IMAGEPROCESS : public DisplayUnit {
  public:
    VIRTUAL_INDEX(IMAGEPROCESS);

    IMAGEPROCESS(ImageFunction _func) : func(_func){};
    ~IMAGEPROCESS() {}

    void invoke(const DisplayUnitContext &context);

  private:
    ImageFunction func;
    bool bCompleted = false;
  };
#endif

  class DisplayUnitContext {
  public:
    std::array<DisplayUnit *, contextUnitIndex::MAX_idx> currentUnit;

    DisplayUnitContext(void) {
      std::fill(currentUnit.begin(), currentUnit.end(), nullptr);
    }

#define INDEXED_ACCESSOR(NAME)                                                 \
  inline platform::NAME *NAME(void) const {                                    \
    return dynamic_cast<platform::NAME *>(                                     \
        currentUnit[contextUnitIndex::NAME##_idx]);                            \
  }                                                                            \
  inline bool validate##NAME(void) const {                                     \
    return dynamic_cast<platform::NAME *>(                                     \
               currentUnit[contextUnitIndex::NAME##_idx]) != nullptr;          \
  }

    INDEXED_ACCESSOR(AREA);
    INDEXED_ACCESSOR(STRING);
    INDEXED_ACCESSOR(IMAGE);
    INDEXED_ACCESSOR(FONT);
    INDEXED_ACCESSOR(PEN);
    INDEXED_ACCESSOR(BACKGROUND);
    INDEXED_ACCESSOR(ALIGN);
    INDEXED_ACCESSOR(EVENT);
    INDEXED_ACCESSOR(DRAWTEXT);
    INDEXED_ACCESSOR(DRAWIMAGE);
    INDEXED_ACCESSOR(DRAWBOX);

#if defined(USE_IMAGE_MAGICK)
    INDEXED_ACCESSOR(IMAGEPROCESS);
#endif // defined

    INDEXED_ACCESSOR(FUNCTION);

    /**
      \def The set routine stores the passed object within the slot context.
      This reduces the number of parameters and offers a state.
    */
    void set(DisplayUnit *_ptr) { currentUnit[_ptr->index()] = _ptr; }

    short windowX = 0;
    short windowY = 0;
    unsigned short windowWidth = 0;
    unsigned short windowHeight = 0;
    bool windowOpen = false;
    cairo_t *cr = nullptr;

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
    bool preclear = true;

#elif defined(_WIN64)
    HWND hwnd = 0;

    ID2D1Factory *pD2DFactory = nullptr;
    ID2D1HwndRenderTarget *pRenderTarget = nullptr;
    ID2D1Bitmap *pBitmap = nullptr;

#endif

#if defined(USE_IMAGE_MAGICK)
    Magick::Image offscreenImage;
    Magick::Quantum *offscreenBuffer = nullptr;

#endif
  };

  DisplayUnitContext context;

private:
  errorHandler fnError;
  eventHandler fnEvents;
  std::vector<std::unique_ptr<DisplayUnit>> DL;

  std::vector<eventHandler> onfocus;
  std::vector<eventHandler> onblur;
  std::vector<eventHandler> onresize;
  std::vector<eventHandler> onkeydown;
  std::vector<eventHandler> onkeyup;
  std::vector<eventHandler> onkeypress;
  std::vector<eventHandler> onmouseenter;
  std::vector<eventHandler> onmouseleave;
  std::vector<eventHandler> onmousemove;
  std::vector<eventHandler> onmousedown;
  std::vector<eventHandler> onmouseup;
  std::vector<eventHandler> onclick;
  std::vector<eventHandler> ondblclick;
  std::vector<eventHandler> oncontextmenu;
  std::vector<eventHandler> onwheel;

  std::vector<eventHandler> &getEventVector(eventType evtType);
}; // namespace uxdevice
}; // namespace uxdevice
