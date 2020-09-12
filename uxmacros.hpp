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
\file uxmacros.hpp
\date 5/12/20
\version 1.0
\details  macros that minimize the maintained source code. These macros provide
functionality inheirient within display units such as hashing and object
constructor, copy, assignment, and move.

*/

#pragma once

/**
\internal
\def UX_ERROR_CHECK(obj)
Checks the state of the object to determine the error status. These are
typically reported using an error code. The context object contains all of
the error handling and capture of error text. When errors occur, they are placed
into a list. The function bool error_state(void) can be used to
determine if errors exist.


Generally the two macro provide necessary functionality. UX_ERROR_CHECK and
UX_ERROR_DESC. UX_ERROR_CHECK may be used to check objects and report errors. It
checks and adds them automatically. The UX_ERROR_DESC(std::string_view) can be
used to create an error condition for a named description.

std::string error_text(bool bclear) may be used to get a list of the error
descriptions. All error descriptions are concatenated and returned. Optionally
sending a boolean value of true to the function will clear the status.

*/

#define UX_ERROR_CHECK(obj)                                                    \
  {                                                                            \
    cairo_status_t stat = context.error_check(obj);                            \
    if (stat)                                                                  \
      context.error_state(__func__, __LINE__, __FILE__, stat);                 \
  }

#define UX_ERROR_DESC(s)                                                       \
  context.error_state(__func__, __LINE__, __FILE__, std::string_view(s));

#define UX_ERRORS_SPIN                                                         \
  while (lockErrors.test_and_set(std::memory_order_acquire))

#define UX_ERRORS_CLEAR lockErrors.clear(std::memory_order_release);

#define UX_DECLARE_ERROR_HANDLING                                              \
  std::atomic_flag lockErrors = ATOMIC_FLAG_INIT;                              \
  std::list<std::string> _errors = {};                                         \
                                                                               \
  cairo_status_t error_check(cairo_surface_t *sur) {                           \
    return cairo_surface_status(sur);                                          \
  }                                                                            \
  cairo_status_t error_check(cairo_t *cr) { return cairo_status(cr); }         \
                                                                               \
  void error_state(const std::string_view &sfunc, const std::size_t linenum,   \
                   const std::string_view &sfile, const cairo_status_t stat) { \
    error_state(sfunc, linenum, sfile,                                         \
                std::string_view(cairo_status_to_string(stat)));               \
  }                                                                            \
                                                                               \
  void error_state(const std::string_view &sfunc, const std::size_t linenum,   \
                   const std::string_view &sfile, const std::string &desc) {   \
    error_state(sfunc, linenum, sfile, std::string_view(desc));                \
  }                                                                            \
                                                                               \
  void error_state(const std::string_view &sfunc, const std::size_t linenum,   \
                   const std::string_view &sfile,                              \
                   const std::string_view &desc) {                             \
    UX_ERRORS_SPIN;                                                            \
    std::stringstream ss;                                                      \
    ss << sfile << "\n" << sfunc << "(" << linenum << ") -  " << desc << "\n"; \
    _errors.emplace_back(ss.str());                                            \
                                                                               \
    UX_ERRORS_CLEAR;                                                           \
  }                                                                            \
                                                                               \
  bool error_state(void) {                                                     \
    UX_ERRORS_SPIN;                                                            \
    bool b = !_errors.empty();                                                 \
    UX_ERRORS_CLEAR;                                                           \
    return b;                                                                  \
  }                                                                            \
                                                                               \
  std::string error_text(bool bclear = false) {                                \
    UX_ERRORS_SPIN;                                                            \
    std::string ret;                                                           \
    for (auto s : _errors)                                                     \
      ret += s;                                                                \
    if (bclear)                                                                \
      _errors.clear();                                                         \
                                                                               \
    UX_ERRORS_CLEAR;                                                           \
    return ret;                                                                \
  }

// from -
// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
namespace uxdevice {
template <typename T, typename... Rest>
void hash_combine(std::size_t &seed, const T &v, const Rest &... rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
}
} // namespace uxdevice

/**
\internal
\def UX_REGISTER_STD_HASH_SPECIALIZATION(CLASS_NAME)
\param CLASS_NAME to make std::hash aware.
\brief creates an operator() that is accessible from std::hash<>
These objects must expose a method named hash_code() which returns a
std::size_t.
*/
#define UX_REGISTER_STD_HASH_SPECIALIZATION(CLASS_NAME)                        \
  template <> struct std::hash<CLASS_NAME> {                                   \
    std::size_t operator()(CLASS_NAME const &o) const noexcept {               \
      return o.hash_code();                                                    \
    }                                                                          \
  }

/**
\internal
\def UX_HASH_TYPE_ID_THIS
\brief hashes the type id of the this pointer.
*/
#define UX_HASH_TYPE_ID_THIS std::type_index(typeid(this)).hash_code()

/**
\internal
\def UX_HASH_TYPE_ID_OBJECT
\param OBJECT_NAME - the variable name
\brief hashes the type id of the this pointer.
*/
#define UX_HASH_TYPE_ID_OBJECT(OBJECT_NAME)                                    \
  std::type_index(typeid(OBJECT_NAME)).hash_code()

/**
\internal
\def UX_HASH_OBJECT_MEMBERS
\param ... - variadic parameter expanding to hash combine each listed.
\brief Creates interface routines for the hashing system and change detection
logic. Hashes each of the listed values within the macro parameters.
*/
#define UX_HASH_OBJECT_MEMBERS(...)                                            \
  std::size_t hash_code(void) const noexcept {                                 \
    std::size_t __value = {};                                                  \
    hash_combine(__value, __VA_ARGS__);                                        \
    return __value;                                                            \
  }                                                                            \
  std::size_t __used_hash_code = {};                                           \
  void state_hash_code(void) { __used_hash_code = hash_code(); }               \
  bool is_different_hash() { return hash_code() != __used_hash_code; }

#define UX_DECLARE_HASH_MEMBERS_INTERFACE                                      \
  std::size_t hash_code(void) const noexcept;                                  \
  std::size_t __used_hash_code = {};                                           \
  void state_hash_code(void) { __used_hash_code = hash_code(); }               \
  bool is_different_hash() { return hash_code() != __used_hash_code; }

/**
\internal
\def UX_HASH_VECTOR_OBJECTS
\param VARIABLE_NAME
\brief invokes a lambda that calls the hash code member of the objects.
Objects declared with the macros in uxdisplayunitbase.hpp have these functions.

*/
#define UX_HASH_VECTOR_OBJECTS(VARIABLE_NAME)                                  \
  std::invoke([&]() {                                                          \
    std::size_t __value_vec = {};                                              \
    for (auto n : VARIABLE_NAME)                                               \
      hash_combine(__value_vec, n);                                            \
    return __value_vec;                                                        \
  })

/**
\internal
\def UX_DECLARE_TYPE_INDEX_MEMORY
\tparam typename T
\param const std::shared_ptr<T> ptr)

\brief sets a value at the specific type info. The storage is
std::any however can be correlated directly back into the type.

*/
typedef std::function<std::size_t(void)> hash_function_t;
typedef std::tuple<std::any, hash_function_t> unit_memory_storage_object_t;
typedef std::unordered_map<std::type_index, unit_memory_storage_object_t>
    unit_memory_storage_t;

#define UX_DECLARE_TYPE_INDEX_MEMORY(FUNCTION_NAME)                            \
  unit_memory_storage_t FUNCTION_NAME##_storage = {};                          \
                                                                               \
  template <typename T> void FUNCTION_NAME(const std::shared_ptr<T> ptr) {     \
    auto ti = std::type_index(typeid(T));                                      \
    FUNCTION_NAME##_storage[ti] =                                              \
        std::make_tuple<unit_memory_storage_object_t>(                         \
            ptr, [&]() { return ptr->hash_code(); });                          \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  void FUNCTION_NAME(const std::shared_ptr<display_unit_t> ptr) {              \
    auto ti = std::type_index(typeid(T));                                      \
    FUNCTION_NAME##_storage[ti] =                                              \
        std::make_tuple<unit_memory_storage_object_t>(                         \
            std::dynamic_pointer_cast<T>(ptr),                                 \
            [&]() { return std::dynamic_pointer_cast<T>(ptr)->hash_code(); }); \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  auto FUNCTION_NAME(void) const noexcept->const std::shared_ptr<T> {          \
    std::shared_ptr<T> ptr = {};                                               \
    auto ti = std::type_index(typeid(T));                                      \
    auto item = FUNCTION_NAME##_storage.find(ti);                              \
    if (item != FUNCTION_NAME##_storage.end()) {                               \
      auto obj_data = std::get<std::any>(item->second);                        \
      ptr = std::any_cast<std::shared_ptr<T>>(obj_data);                       \
    }                                                                          \
    return ptr;                                                                \
  }                                                                            \
                                                                               \
  std::size_t FUNCTION_NAME##_hash_code_all(void) const noexcept {             \
    std::size_t value = {};                                                    \
    for (auto &n : FUNCTION_NAME##_storage) {                                  \
      hash_combine(value, std::get<hash_function_t>(n.second)());              \
    }                                                                          \
    return value;                                                              \
  }                                                                            \
                                                                               \
  template <typename T> auto FUNCTION_NAME##_hash_code(void)->std::size_t {    \
    std::size_t value = {};                                                    \
    auto ti = std::type_index(typeid(T));                                      \
    auto item = FUNCTION_NAME##_storage.find(ti);                              \
    if (item != FUNCTION_NAME##_storage.end()) {                               \
      value = std::get<hash_function_t>(item->second)();                       \
    }                                                                          \
    return value;                                                              \
  }                                                                            \
  void FUNCTION_NAME##_clear(void) { FUNCTION_NAME##_storage.clear(); }
