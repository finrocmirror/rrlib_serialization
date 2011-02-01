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
tGenericObject* tDataType<T>::tDataTypeInfo::CreateInstanceGeneric(void* placement) const
{
  if (placement != NULL)
  {
    return new(placement) tGenericObjectInstance<T>();
  }
  else
  {
    return new tGenericObjectInstance<T>();
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

