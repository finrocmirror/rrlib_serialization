/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2008-2011 Max Reichardt,
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
#include "rrlib/serialization/tGenericObjectInstance.h"

#include "rrlib/serialization/tSerializable.h"
#include "rrlib/serialization/sSerialization.h"
#include <stdexcept>

namespace rrlib
{
namespace serialization
{
template<typename T>
tDataType<T>::tDataTypeInfo::tDataTypeInfo()
{
  type = detail::tListInfo<T>::type;
  rtti_name = typeid(T).name();
  size = sizeof(T);
  generic_object_size = sizeof(tGenericObjectInstance<T>());
  name = detail::tListInfo<T>::GetName();

}

template<typename T>
tGenericObject* tDataType<T>::tDataTypeInfo::CreateInstanceGeneric(void* placement, int manager_size) const
{
  if (placement != NULL)
  {
    if (manager_size <= 8)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<8> >();
    }
    else if (manager_size <= 16)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<16> >();
    }
    else if (manager_size <= 24)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<24> >();
    }
    else if (manager_size <= 32)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<32> >();
    }
    else if (manager_size <= 40)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<40> >();
    }
    else
    {
      throw std::invalid_argument("Management info larger than 40 bytes not allowed");
    }
  }
  else
  {
    if (manager_size <= 8)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<8> >();
    }
    else if (manager_size <= 16)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<16> >();
    }
    else if (manager_size <= 24)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<24> >();
    }
    else if (manager_size <= 32)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<32> >();
    }
    else if (manager_size <= 40)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<40> >();
    }
    else
    {
      throw std::invalid_argument("Management info larger than 40 bytes not allowed");
    }
  }
}

template<typename T>
void tDataType<T>::tDataTypeInfo::DeepCopy(const void* src, void* dest, tFactory* f) const
{
  assert(typeid(*src).name() == typeid(T).name());
  assert(typeid(*dest).name() == typeid(T).name());

  T* s = static_cast<T*>(src);
  T* d = static_cast<T*>(dest);
  sSerialization::DeepCopy(s, d, f);
}

} // namespace rrlib
} // namespace serialization

