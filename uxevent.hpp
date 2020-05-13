/**
\author Anthony Matarazzo
\file uxevent.hpp
\date 5/12/20
\version 1.0
 \details  event class

*/
#pragma once

namespace uxdevice {
/**
\enum eventType
\brief the eventType enumeration contains a sequenced value for all of the
events that can be dispatched by the system.
*/
enum class eventType : uint8_t {
  none,
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

/**
\class event

\brief the event class provides the communication between the event system and
the caller. There is one event class for all of the distinct events. Simply
different constructors are selected based upon the necessity of information
given within the parameters.
*/
using event = class event {
public:
  event(const eventType &et) : type(et) {}
  event(const eventType &et, const char &k) : type(et), key(k) {}
  event(const eventType &et, const unsigned int &vk)
      : type(et), virtualKey(vk), isVirtualKey(true) {}

  event(const eventType &et, const short &mx, const short &my,
        const short &mb_dis)
      : type(et), x(mx), y(my) {

    if (et == eventType::wheel)
      distance = mb_dis;
    else
      button = static_cast<char>(mb_dis);
  }
  event(const eventType &et, const short &_w, const short &_h)
      : type(et), x(_w), y(_h), w(_w), h(_h) {}

  event(const eventType &et, const short &_x, const short &_y, const short &_w,
        const short &_h)
      : type(et), x(_x), y(_y), w(_w), h(_h) {}
  event(const eventType &et, const short &_distance)
      : type(et), distance(_distance) {}
  ~event(){};

public:
  eventType type = eventType::none;

  unsigned int virtualKey = 0;
  std::wstring unicodeKeys = L"";
  bool isVirtualKey = false;
  char key = 0x00;

  char button = 0;

  short x = 0;
  short y = 0;
  short w = 0;
  short h = 0;
  short distance = 0;
};

/// \typedef eventHandler is used to note and declare a lambda function for
/// the specified event.
typedef std::function<void(const event &et)> eventHandler;

} // namespace uxdevice
