/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2011 Max Reichardt,
 *   Robotics Research Lab, University of Kaiserslautern
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __rrlib__serialization__detail__tStringInputStreamFallback_h__
#define __rrlib__serialization__detail__tStringInputStreamFallback_h__

#include <string>

namespace rrlib
{
namespace serialization
{
namespace detail
{
/*!
 * \author Max Reichardt
 *
 * Wrapper tStringInputStream to implement fallback mechanism
 * when operators are not overloaded for XML nodes.
 */
class tStringInputStreamFallback : public tStringInputStream
{
public:
  const xml2::tXMLNode& node;

  tStringInputStreamFallback(const xml2::tXMLNode& node_) :
    tStringInputStream(node_.GetTextContent()),
    node(node_)
  {}

};

template <typename T>
struct tIsStringInputSerializable
{
  template < typename U = T >
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

} // namespace
} // namespace
} // namespace

template < typename T, bool ENABLE = (!std::is_base_of<rrlib::serialization::tSerializable, T>::value) && rrlib::serialization::detail::tIsStringInputSerializable<T>::value >
inline const typename std::enable_if<ENABLE, rrlib::xml2::tXMLNode>::type & operator>> (rrlib::serialization::detail::tStringInputStreamFallback && is, T & t)
{
  static_cast<rrlib::serialization::tStringInputStream&>(is) >> t;
  return is.node;
}

#endif // __rrlib__serialization__detail__tStringInputStreamFallback_h__
