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
\file uxevent.hpp
\date 9/7/20
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

/**
\class event

\brief the event class provides the communication between the event system and
the caller. There is one event class for all of the distinct events. Simply
different constructors are selected based upon the necessity of information
given within the parameters.
*/
using event_t = class event_t {
public:
  event_t(const std::type_index &et) : type(et) {}
  event_t(const std::type_index &et, const char &k) : type(et), key(k) {}
  event_t(const std::type_index &et, const unsigned int &vk)
      : type(et), virtualKey(vk), isVirtualKey(true) {}

  event_t(const std::type_index &et, const short &mx, const short &my,
          const short &mb_dis)
      : type(et), x(mx), y(my) {
    distance = mb_dis;
    button = static_cast<char>(mb_dis);
  }
  event_t(const std::type_index &et, const short &_w, const short &_h)
      : type(et), x(_w), y(_h), w(_w), h(_h) {}

  event_t(const std::type_index &et, const short &_x, const short &_y,
          const short &_w, const short &_h)
      : type(et), x(_x), y(_y), w(_w), h(_h) {}
  event_t(const std::type_index &et, const short &_distance)
      : type(et), distance(_distance) {}
  ~event_t(){};

public:
  std::type_index type = std::type_index(typeid(this));

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

/// \typedef event_handler_t is used to note and declare a lambda function for
/// the specified event.
typedef std::function<void(const event_t &et)> event_handler_t;

} // namespace uxdevice
