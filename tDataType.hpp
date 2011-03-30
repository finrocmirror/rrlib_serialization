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
namespace detail
{
template <typename T, size_t MSIZE>
tGenericObject* CreateInstanceGeneric(void* placement)
{
  size_t size = sizeof(tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<MSIZE> >);
  if (placement == NULL)
  {
    placement = operator new(size);
  }
  memset(placement, 0, size); // set memory to 0 so that memcmp on class T can be performed cleanly for certain types
  return new(placement) tGenericObjectInstance<T, tGenericObjectManagerPlaceHolder<MSIZE> >();
}
}

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
  if (manager_size <= 8)
  {
    return detail::CreateInstanceGeneric<T, 8>(placement);
  }
  else if (manager_size <= 16)
  {
    return detail::CreateInstanceGeneric<T, 16>(placement);
  }
  else if (manager_size <= 24)
  {
    return detail::CreateInstanceGeneric<T, 24>(placement);
  }
  else if (manager_size <= 32)
  {
    return detail::CreateInstanceGeneric<T, 32>(placement);
  }
  else if (manager_size <= 40)
  {
    return detail::CreateInstanceGeneric<T, 40>(placement);
  }
  else if (manager_size <= 48)
  {
    return detail::CreateInstanceGeneric<T, 48>(placement);
  }
  else if (manager_size <= 56)
  {
    return detail::CreateInstanceGeneric<T, 56>(placement);
  }
  else if (manager_size <= 64)
  {
    return detail::CreateInstanceGeneric<T, 64>(placement);
  }
  else if (manager_size <= 72)
  {
    return detail::CreateInstanceGeneric<T, 72>(placement);
  }
  else if (manager_size <= 80)
  {
    return detail::CreateInstanceGeneric<T, 80>(placement);
  }
  else if (manager_size <= 88)
  {
    return detail::CreateInstanceGeneric<T, 88>(placement);
  }
  else if (manager_size <= 96)
  {
    return detail::CreateInstanceGeneric<T, 96>(placement);
  }
  else if (manager_size <= 104)
  {
    return detail::CreateInstanceGeneric<T, 104>(placement);
  }
  else if (manager_size <= 112)
  {
    return detail::CreateInstanceGeneric<T, 112>(placement);
  }
  else if (manager_size <= 120)
  {
    return detail::CreateInstanceGeneric<T, 120>(placement);
  }
  else
  {
    throw std::invalid_argument("Management info larger than 120 bytes not allowed");
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

