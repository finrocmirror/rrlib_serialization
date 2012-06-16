/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2012 Max Reichardt,
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

#ifndef __rrlib__serialization__type_traits_h__
#define __rrlib__serialization__type_traits_h__

#include "rrlib/serialization/tStringOutputStream.h"

namespace rrlib
{
namespace serialization
{

/*!
 * This type-trait-like struct is used to determine whether a type is binary serializable
 */
template <typename T>
struct tIsBinarySerializable
{
  template < typename U = T >
  static int16_t Test(decltype((*(tOutputStream*)(NULL)) << (*(U*)(NULL))))
  {
    return 0;
  }

  static int32_t Test(...)
  {
    return 0;
  }

  enum { value = sizeof(Test(*(tOutputStream*)(NULL))) == sizeof(int16_t) };
};

/*!
 * This type-trait-like struct is used to determine whether a type is string serializable
 */
template <typename T>
struct tIsStringSerializable
{
  enum { value = detail::tIsStringOutputSerializable<T>::value };
};

/*!
 * This type-trait-like struct is used to determine whether a type is string serializable
 */
template <typename T>
struct tIsXMLSerializable
{
  template < typename U = T >
  static int16_t Test(decltype((*(xml::tNode*)(NULL)) << (*(U*)(NULL))))
  {
    return 0;
  }

  static int32_t Test(...)
  {
    return 0;
  }

  enum { value = sizeof(Test(*(xml::tNode*)(NULL))) == sizeof(int16_t) };
};

} // namespace
} // namespace

#endif // __rrlib__serialization__type_traits_h__
