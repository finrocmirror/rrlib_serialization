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
  name = detail::tListInfo<T>::GetName();

}

template<typename T>
tGenericObject* tDataType<T>::tDataTypeInfo::CreateInstanceGeneric(void* placement, size_t manager_size) const
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
    else if (manager_size <= 48)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<48> >();
    }
    else if (manager_size <= 56)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<56> >();
    }
    else if (manager_size <= 64)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<64> >();
    }
    else if (manager_size <= 72)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<72> >();
    }
    else if (manager_size <= 80)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<80> >();
    }
    else if (manager_size <= 88)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<88> >();
    }
    else if (manager_size <= 96)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<96> >();
    }
    else if (manager_size <= 104)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<104> >();
    }
    else if (manager_size <= 112)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<112> >();
    }
    else if (manager_size <= 120)
    {
      return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<120> >();
    }
    else
    {
      throw std::invalid_argument("Management info larger than 120 bytes not allowed");
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
    else if (manager_size <= 48)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<48> >();
    }
    else if (manager_size <= 56)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<56> >();
    }
    else if (manager_size <= 64)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<64> >();
    }
    else if (manager_size <= 72)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<72> >();
    }
    else if (manager_size <= 80)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<80> >();
    }
    else if (manager_size <= 88)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<88> >();
    }
    else if (manager_size <= 96)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<96> >();
    }
    else if (manager_size <= 104)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<104> >();
    }
    else if (manager_size <= 112)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<112> >();
    }
    else if (manager_size <= 120)
    {
      return new tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<120> >();
    }
    else
    {
      throw std::invalid_argument("Management info larger than 120 bytes not allowed");
    }
  }
}

template<typename T>
void tDataType<T>::tDataTypeInfo::DeepCopy(const void* src, void* dest, tFactory* f) const
{
  const T* s = static_cast<const T*>(src);
  T* d = static_cast<T*>(dest);

  if (boost::has_virtual_destructor<T>::value)
  {
    assert(typeid(*s).name() == typeid(T).name());
    assert(typeid(*d).name() == typeid(T).name());
  }

  sSerialization::DeepCopy(*s, *d, f);
}

} // namespace rrlib
} // namespace serialization

