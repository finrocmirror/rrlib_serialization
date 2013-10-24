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
/*!\file    rrlib/serialization/detail/tStringInputStreamFallback.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-18
 *
 * \brief   Contains tStringInputStreamFallback
 *
 * \b tStringInputStreamFallback
 *
 * Wrapper tStringInputStream to implement fallback mechanism
 * when operators are not overloaded for XML nodes.
 *
 * This way, string serializable types can seamlessly be deserialized from XML nodes.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__detail__tStringInputStreamFallback_h__
#define __rrlib__serialization__detail__tStringInputStreamFallback_h__

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
//! String input stream fallback for deserialization from XML
/*!
 * Wrapper tStringInputStream to implement fallback mechanism
 * when operators are not overloaded for XML nodes.
 *
 * This way, string serializable types can seamlessly be deserialized from XML nodes.
 */
class tStringInputStreamFallback : public tStringInputStream
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  const xml::tNode& node;

  tStringInputStreamFallback(const xml::tNode& node_) :
    tStringInputStream(node_.GetTextContent()),
    node(node_)
  {}

};

template <typename T>
struct tIsStringInputSerializable
{
  template <typename U = T>
  static int16_t Test(decltype((*(tStringInputStream*)(NULL)) >> (*(U*)(NULL))))
  {
    return 0;
  }

  static int32_t Test(...)
  {
    return 0;
  }

  enum { value = sizeof(Test(*(tStringInputStream*)(NULL))) == sizeof(int16_t) };
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

template <typename T, bool ENABLE = rrlib::serialization::detail::tIsStringInputSerializable<T>::value>
inline const typename std::enable_if<ENABLE, rrlib::xml::tNode>::type & operator>> (rrlib::serialization::detail::tStringInputStreamFallback && is, T & t)
{
  static_cast<rrlib::serialization::tStringInputStream&>(is) >> t;
  return is.node;
}

#endif
