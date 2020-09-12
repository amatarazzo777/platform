/**
\author Anthony Matarazzo
\file uxdevice.hpp
\date 3/26/20
\version 1.0
\brief interface for the platform.

*/
#pragma once

#define PI (3.14159265358979323846264338327f)

/**
\addtogroup Library Build Options
\brief Library Options
\details These options provide the selection to configure selection
options when compiling the source.
@{
*/
//#define USE_STACKBLUR

#define USE_SVGREN

#define DEFAULT_TEXTFACE "arial"
#define DEFAULT_TEXTSIZE 12
#define DEFAULT_TEXTCOLOR 0

//#define CLIP_OUTLINE
/**
\def USE_DEBUG_CONSOLE
*/
#define USE_DEBUG_CONSOLE
#define CONSOLE
/** @} */

#include "uxbase.hpp"

#include "uxevent.hpp"
#include "uxmatrix.hpp"
#include "uxpaint.hpp"

#include "uxdisplaycontext.hpp"
#include "uxdisplayunits.hpp"

#include "uxcairoimage.hpp"

std::string _errorReport(std::string sourceFile, int ln, std::string sfunc,
                         std::string cond, std::string ecode);
typedef std::function<void(const std::string &err)> errorHandler;

namespace uxdevice {

class event;

/**
 \details
*/

using bounds = class bounds {
public:
  double x = 0, y = 0, w = 0, h = 0;
};

using point = class point {
public:
  double x = 0, y = 0;
};

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
                  const unsigned short height,
                  Paint background = Paint("white"));
  void surfaceBrush(Paint &b);
  void closeWindow(void);
  void backgroundBrush(Paint &p) { context.brush = p; }
  bool processing(void) { return bProcessing; }

  void startProcessing(void);

  void clear(void);
  void notifyComplete(void);

  void text(const std::string &s);
  void text(const std::stringstream &s);
  void image(const std::string &s);

  void pen(const Paint &p);
  void pen(u_int32_t c);
  void pen(const std::string &c);
  void pen(const std::string &c, double w, double h);
  void pen(double _r, double _g, double _b);
  void pen(double _r, double _g, double _b, double _a);
  void pen(double x0, double y0, double x1, double y1, const ColorStops &cs);
  void pen(double cx0, double cy0, double radius0, double cx1, double cy1,
           double radius1, const ColorStops &cs);

  void background(const Paint &p);
  void background(u_int32_t c);
  void background(const std::string &c);
  void background(const std::string &c, double w, double h);
  void background(double _r, double _g, double _b);
  void background(double _r, double _g, double _b, double _a);
  void background(double x0, double y0, double x1, double y1,
                  const ColorStops &cs);
  void background(double cx0, double cy0, double radius0, double cx1,
                  double cy1, double radius1, const ColorStops &cs);

  void textOutline(const Paint &p, double dWidth = 1);
  void textOutline(u_int32_t c, double dWidth = 1);
  void textOutline(const std::string &c, double dWidth = 1);
  void textOutline(const std::string &c, double w, double h, double dWidth = 1);
  void textOutline(double _r, double _g, double _b, double dWidth = 1);
  void textOutline(double _r, double _g, double _b, double _a,
                   double dWidth = 1);
  void textOutline(double x0, double y0, double x1, double y1,
                   const ColorStops &cs, double dWidth = 1);
  void textOutline(double cx0, double cy0, double radius0, double cx1,
                   double cy1, double radius1, const ColorStops &cs,
                   double dWidth = 1);

  void textOutlineNone(void);

  void textFill(const Paint &p);
  void textFill(u_int32_t c);
  void textFill(const std::string &c);
  void textFill(const std::string &c, double w, double h);
  void textFill(double _r, double _g, double _b);
  void textFill(double _r, double _g, double _b, double _a);
  void textFill(double x0, double y0, double x1, double y1,
                const ColorStops &cs);
  void textFill(double cx0, double cy0, double radius0, double cx1, double cy1,
                double radius1, const ColorStops &cs);

  void textFillNone(void);

  void textShadow(const Paint &p, int r = 3, double xOffset = 1.0,
                  double yOffset = 1.0);
  void textShadow(u_int32_t c, int r = 3, double xOffset = 1.0,
                  double yOffset = 1.0);
  void textShadow(const std::string &c, int r = 3, double xOffset = 1.0,
                  double yOffset = 1.0);
  void textShadow(const std::string &c, double w, double h, int r,
                  double xOffset, double yOffset);

  void textShadow(double _r, double _g, double _b, int r = 3,
                  double xOffset = 1.0, double yOffset = 1.0);
  void textShadow(double _r, double _g, double _b, double _a, int r = 3,
                  double xOffset = 1.0, double yOffset = 1.0);

  void textShadow(double x0, double y0, double x1, double y1,
                  const ColorStops &cs, int r = 3, double xOffset = 1.0,
                  double yOffset = 1.0);
  void textShadow(double cx0, double cy0, double radius0, double cx1,
                  double cy1, double radius1, const ColorStops &cs, int r = 3,
                  double xOffset = 1.0, double yOffset = 1.0);

  void textShadowNone(void);

  void textAlignment(alignment aln);
  void indent(double space);
  double indent(void);
  void ellipse(ellipsize e);
  ellipsize ellipse(void);
  void lineSpace(double dSpace);
  double lineSpace(void);
  void tabStops(const std::vector<double> &tabs);

  void font(const std::string &s);
  void area(double x, double y, double w, double h);
  void area(double x, double y, double w, double h, double rx, double ry);
  void area(double cx, double cy, double r) { areaCircle(cx, cy, r); }
  void areaCircle(double x, double y, double d);
  void areaEllipse(double cx, double cy, double rx, double ry);
  void areaLines(std::vector<double> lines);
  // void areaPath(std::vector<PathStep> path);

  void drawText(void);
  void drawImage(void);
  void drawArea(void);
  void antiAlias(antialias antialias);

  void save(void);
  void restore(void);

  void push(content _content = content::all);
  void pop(bool bToSource = false);

  void translate(double x, double y);
  void rotate(double angle);
  void scale(double x, double y);
  void transform(const Matrix &mat);
  void matrix(const Matrix &mat);
  void identity(void);
  void device(double &x, double &y);
  void deviceDistance(double &x, double &y);
  void user(double &x, double &y);
  void userDistance(double &x, double &y);

  void source(Paint &p);

  void cap(lineCap c);
  void join(lineJoin j);
  void lineWidth(double dWidth);
  void miterLimit(double dLimit);
  void dashes(const std::vector<double> &dashes, double offset);
  void tollerance(double _t);
  void op(op_t _op);

  void arc(double xc, double yc, double radius, double angle1, double angle2,
           bool bNegative = false);
  void curve(double x1, double y1, double x2, double y2, double x3, double y3,
             bool bRelative = false);

  void line(double x, double y, bool bRelative = false);
  void move(double x, double y, bool bRelative = false);
  void rectangle(double x, double y, double width, double height);
  point location(void);

  void mask(Paint &p);
  void mask(Paint &p, double x, double y);
  void paint(double alpha = 1.0);
  void stroke(bool bPreserve = false);
  bounds stroke(void);
  bool inStroke(double x, double y);
  void fill(bool bPreserve = false);
  bool inFill(double x, double y);

  bounds clip(void);
  void clip(bool bPreserve = false);
  bool inClip(double x, double y);

private:
  void renderLoop(void);
  void exposeRegions(void);
  void dispatchEvent(const event &e);
  void drawCaret(const int x, const int y, const int h);

  void messageLoop(void);

  void flip(void);

#if defined(__linux__)

#elif defined(_WIN64)
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
  bool initializeVideo(void);
  void terminateVideo(void);
#endif

private:
  DisplayContext context = DisplayContext();
  std::atomic<bool> bProcessing = false;
  int framesPerSecond = 60;
  errorHandler fnError = nullptr;
  eventHandler fnEvents = nullptr;

  typedef std::list<std::shared_ptr<DisplayUnit>> DisplayUnitStorage;
  DisplayUnitStorage DL = {};
  DisplayUnitStorage::iterator itDL_Processed = DL.begin();

  std::atomic_flag DL_readwrite = ATOMIC_FLAG_INIT;

#define DL_SPIN while (DL_readwrite.test_and_set(std::memory_order_acquire))
#define DL_CLEAR DL_readwrite.clear(std::memory_order_release)

  std::vector<eventHandler> onfocus = {};
  std::vector<eventHandler> onblur = {};
  std::vector<eventHandler> onresize = {};
  std::vector<eventHandler> onkeydown = {};
  std::vector<eventHandler> onkeyup = {};
  std::vector<eventHandler> onkeypress = {};
  std::vector<eventHandler> onmouseenter = {};
  std::vector<eventHandler> onmouseleave = {};
  std::vector<eventHandler> onmousemove = {};
  std::vector<eventHandler> onmousedown = {};
  std::vector<eventHandler> onmouseup = {};
  std::vector<eventHandler> onclick = {};
  std::vector<eventHandler> ondblclick = {};
  std::vector<eventHandler> oncontextmenu = {};
  std::vector<eventHandler> onwheel = {};

  std::vector<eventHandler> &getEventVector(eventType evtType);
}; // namespace uxdevice

} // namespace uxdevice
