//
// You received this file as part of RRLib
// Robotics Research Library
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//----------------------------------------------------------------------
/*!\file    rrlib/serialization/detail/tStringOutputStreamFallback.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-18
 *
 * \brief   Contains tStringOutputStreamFallback
 *
 * \b tStringOutputStreamFallback
 *
 * Wrapper tStringOutputStream to implement fallback mechanism
 * when operators are not overloaded for XML nodes.
 *
 * This way, string serializable types can seamlessly be serialized to XML nodes.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__detail__tStringOutputStreamFallback_h__
#define __rrlib__serialization__detail__tStringOutputStreamFallback_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/xml/tNode.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace rrlib
{
namespace serialization
{
namespace detail
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! String output stream fallback for serialization to XML
/*!
 * Wrapper tStringOutputStream to implement fallback mechanism
 * when operators are not overloaded for XML nodes.
 *
 * This way, string serializable types can seamlessly be serialized to XML nodes.
 */
class tStringOutputStreamFallback : public tStringOutputStream
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  xml::tNode& wrapped;

  tStringOutputStreamFallback(xml::tNode& node) :
    wrapped(node)
  {}

  ~tStringOutputStreamFallback()
  {
    wrapped.SetContent(ToString());
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

};


template <typename T>
struct IsStringOutputSerializable
{
  template <typename U>
  static U &Make();

  template <typename U = T>
  static int16_t TestOutput(decltype(Make<tStringOutputStream>() << Make<U>()));

  static int32_t TestOutput(...);

  enum { value = sizeof(TestOutput(Make<tStringOutputStream>())) == sizeof(int16_t) };
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

template <typename T, bool ENABLE = rrlib::serialization::detail::IsStringOutputSerializable<T>::value>
inline typename std::enable_if<ENABLE, rrlib::xml::tNode>::type & operator<< (rrlib::serialization::detail::tStringOutputStreamFallback && os, const T & t)
{
  os << t;
  return os.wrapped;
}

#endif
