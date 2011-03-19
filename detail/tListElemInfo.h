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

#ifndef __rrlib__serialization__detail__tListElemInfo_h__
#define __rrlib__serialization__detail__tListElemInfo_h__

#include "rrlib/serialization/tDataTypeBase.h"
#include "rrlib/serialization/tFactory.h"
//#include "rrlib/serialization/tDataType.h"
#include <memory>

namespace rrlib
{
namespace serialization
{

template <typename T>
class tDataType;

namespace detail
{

/*!
 * \author Max Reichardt
 *
 * List-element-related information for data type T
 *
 * is_shared_ptr  : Is shared pointer list?
 */
template <typename T>
struct tListElemInfo
{
  static const bool is_shared_ptr = false;
  static tDataTypeBase GetType(const T& t)
  {
    return tDataType<T>();
  }
  static tDataTypeBase GetTypeT()
  {
    return tDataType<T>();
  }
  static const T& GetElem(const T& t)
  {
    return t;
  }
  static T& GetElem(T& t)
  {
    return t;
  }
  static void ChangeBufferType(tFactory* f, const T& t, tDataTypeBase type) {}
  static bool IsNull(const T& t)
  {
    return false;
  }
  static void Reset(const T& t) {}
};

template <typename T>
struct tListElemInfo<std::shared_ptr<T>>
{
  static const bool is_shared_ptr = true;
  static tDataTypeBase GetType(const std::shared_ptr<T>& t)
  {
    if (t.get() == NULL)
    {
      return tDataTypeBase(NULL);
    }
    return tDataType<T>::FindTypeByRtti(typeid(*t).name());
  }
  static tDataTypeBase GetTypeT()
  {
    return tDataType<T>();
  }
  static T& GetElem(const std::shared_ptr<T>& t)
  {
    return *t;
  }
  static void ChangeBufferType(tFactory* f, std::shared_ptr<T>& t, tDataTypeBase type)
  {
    f->CreateBuffer(t, type);
  }
  static bool IsNull(const std::shared_ptr<T>& t)
  {
    return t.get() == NULL;
  }
  static void Reset(std::shared_ptr<T>& t)
  {
    t.reset();
  }
};

} // namespace
} // namespace
} // namespace

#endif // __rrlib__serialization__detail__tListElemInfo_h__
