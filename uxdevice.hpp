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
#define USE_IMAGE_MAGICK

/**
\def USE_PANGO
\brief THe pango text rendering library is used.
If this is not used the base cairo text rendering functions are used.

*/
#define USE_PANGO

/**
\def USE_CHROMIUM_EMBEDDED_FRAMEWORK
\brief The system will be configured to use the CEF system.
*/
//#define USE_CHROMIUM_EMBEDDED_FRAMEWORK

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
  void fontDescription(const std::string &s);
  void area(float x, float y, float w, float h);
  void drawText(void);
  void drawImage(void);

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
  enum contextUnitIndex : std::size_t {
    AREA_idx,
    STRING_idx,
    IMAGE_idx,
    FONT_idx,
    PEN_idx,
    ALIGN_idx,
    EVENT_idx,
    DRAWTEXT_idx,
    DRAWIMAGE_idx,

    MAX

  };

  using DisplayUnit = class DisplayUnit {
  public:
    virtual ~DisplayUnit() {}
    virtual std::size_t index() = 0;
    virtual void invoke(const DisplayUnitContext &context) = 0;
  };

  using AREA = class AREA : public DisplayUnit {
  public:
    std::size_t index() { return contextUnitIndex::AREA_idx; }
    AREA(void) {}
    AREA(float _x, float _y, float _w, float _h) : x(_x), y(_y), w(_w), h(_h) {}
    ~AREA() {}
    float x = 0.0, y = 0.0, w = 0.0, h = 0.0;
    void invoke(const DisplayUnitContext &context);
  };

  using STRING = class STRING : public DisplayUnit {
  public:
    std::size_t index() { return contextUnitIndex::STRING_idx; }
    constexpr static std::size_t id = 3;
    STRING(const std::string &s) : data(s) {}
    ~STRING() {}
    std::string data;
    void invoke(const DisplayUnitContext &context);
  };

  using IMAGE = class IMAGE : public DisplayUnit {
  public:
    std::size_t index() { return contextUnitIndex::IMAGE_idx; }
    IMAGE(const std::string &_fileName) : fileName(_fileName) {}
    ~IMAGE() {
      if (surface)
        cairo_surface_destroy(surface);
    }
    cairo_surface_t *surface = nullptr;

#if defined(USE_IMAGE_MAGICK)
    Magick::Image data = nullptr;
#endif
    void invoke(const DisplayUnitContext &context);

    std::string fileName;
  };

  using FONT = class FONT : public DisplayUnit {
  public:
    std::size_t index() { return contextUnitIndex::FONT_idx; }
    FONT(const std::string &s)
        : description(s), pointSize(DEFAULT_TEXTSIZE), bProvidedName(false),
          bProvidedSize(false), bProvidedDescription(true) {}
    FONT(const std::string &s, const float &pt)
        : description(s), pointSize(pt) {}
    ~FONT() {

#if defined(USE_PANGO)
      if (fontDescription)
        pango_font_description_free(fontDescription);
#endif // defined

    }
    std::string description = DEFAULT_TEXTFACE;
    float pointSize = DEFAULT_TEXTSIZE;
    bool bProvidedName = false;
    bool bProvidedSize = false;
    bool bProvidedDescription = false;

#if defined(USE_PANGO)
    PangoFontDescription *fontDescription = nullptr;
#endif // defined

    void invoke(const DisplayUnitContext &context);
  };

  using PEN = class PEN : public DisplayUnit {
  public:
    std::size_t index() { return contextUnitIndex::PEN_idx; }
    PEN(u_int32_t c) {
      u_int8_t _r = c >> 16;
      u_int8_t _g = c >> 8;
      u_int8_t _b = c;

      r = _r / 255.0;
      g = _g / 255.0;
      b = _b / 255.0;
    }

    PEN(float _r, float _g, float _b) {
      r = _r;
      g = _g;
      b = _b;
    }

    PEN(const std::string &n){};
    ~PEN() {}
    unsigned int data;
    float r = 0.0, g = 0.0, b = 0.0, a = 0.0;
    void invoke(const DisplayUnitContext &context);
  };
  using ALIGN = class ALIGN : public DisplayUnit {
  public:
    std::size_t index() { return contextUnitIndex::ALIGN_idx; }
    ALIGN(const char _aln) : a(_aln) {}
    ~ALIGN() {}
    char a = 'l';
    void invoke(const DisplayUnitContext &context);
  };

  using EVENT = class EVENT : public DisplayUnit {
  public:
    std::size_t index() { return contextUnitIndex::EVENT_idx; }
    EVENT(eventHandler _eh) : fn(_eh){};
    ~EVENT() {}
    eventHandler fn;
    void invoke(const DisplayUnitContext &context);
  };

  using DRAWTEXT = class DRAWTEXT : public DisplayUnit {
  public:
    std::size_t index() { return contextUnitIndex::DRAWTEXT_idx; }
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
    std::size_t index() { return contextUnitIndex::DRAWIMAGE_idx; }
    DRAWIMAGE(const AREA &a) : src(a) { bEntire = false; }
    DRAWIMAGE(void) {}
    ~DRAWIMAGE() {}
    void invoke(const DisplayUnitContext &context);

  private:
    AREA src;
    bool bEntire = true;
  };

  class DisplayUnitContext {
  public:
    std::array<DisplayUnit *, contextUnitIndex::MAX> currentUnit;

    DisplayUnitContext(void) {
      std::fill(currentUnit.begin(), currentUnit.end(), nullptr);
    }

#define INDEXED_ACCESSOR(NAME)                                                 \
  inline platform::NAME *NAME(void) const {                                           \
    return dynamic_cast<platform::NAME *>(                                     \
        currentUnit[contextUnitIndex::NAME##_idx]);                            \
  }                                                                            \
  inline bool validate##NAME(void) const {                                            \
    return dynamic_cast<platform::NAME *>(                                     \
               currentUnit[contextUnitIndex::NAME##_idx]) != nullptr;          \
  }

    INDEXED_ACCESSOR(AREA);
    INDEXED_ACCESSOR(STRING);
    INDEXED_ACCESSOR(IMAGE);
    INDEXED_ACCESSOR(FONT);
    INDEXED_ACCESSOR(PEN);
    INDEXED_ACCESSOR(ALIGN);
    INDEXED_ACCESSOR(EVENT);
    INDEXED_ACCESSOR(DRAWTEXT);
    INDEXED_ACCESSOR(DRAWIMAGE);

    short windowX = 0;
    short windowY = 0;
    unsigned short windowWidth = 0;
    unsigned short windowHeight = 0;

#if defined(__linux__)
    bool windowOpen = false;
    Display *xdisplay = nullptr;
    xcb_connection_t *connection = nullptr;
    xcb_screen_t *screen = nullptr;
    xcb_drawable_t window = 0;
    xcb_gcontext_t graphics = 0;

    xcb_visualtype_t *visualType = nullptr;

    // xcb -- keyboard
    xcb_key_symbols_t *syms = nullptr;

    cairo_surface_t *xcbSurface = nullptr;
    cairo_t *cr = nullptr;
    bool preclear = true;

#if defined(_WIN64)
    HWND hwnd = 0;

    ID2D1Factory *pD2DFactory = nullptr;
    ID2D1HwndRenderTarget *pRenderTarget = nullptr;
    ID2D1Bitmap *pBitmap = nullptr;

#endif

#elif defined(USE_IMAGE_MAGICK)
    Magick::Image offscreenImage;
    Magick::Quantum *offscreenBuffer = nullptr;

#endif

    void set(DisplayUnit *_ptr) { currentUnit[_ptr->index()] = _ptr; }
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
};

}; // namespace uxdevice
