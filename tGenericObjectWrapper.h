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

#ifndef __rrlib__serialization__tGenericObjectWrapper_h__
#define __rrlib__serialization__tGenericObjectWrapper_h__

#include "rrlib/serialization/tStringInputStream.h"
#include "rrlib/serialization/tStringOutputStream.h"
#include "rrlib/serialization/tGenericObject.h"
#include <assert.h>

#include "rrlib/serialization/clear.h"

namespace rrlib
{
namespace xml2
{
class tXMLNode;
} // namespace rrlib
} // namespace xml2

namespace rrlib
{
namespace serialization
{
class tFactory;
class tInputStream;
class tOutputStream;

/*!
 * \author Max Reichardt
 *
 * Allows wrapping any object as GenericObject
 */
template < typename T, typename M = tGenericObjectManagerPlaceHolder<8> >
class tGenericObjectWrapper : public tGenericObject
{
private:

  /*! Manager */
  M manager;

protected:

  virtual void DeepCopyFrom(const void* source, tFactory* f)
  {
    DeepCopyFromImpl(*static_cast<const T*>(source), f);
  }

public:

  virtual void Clear()
  {
    clear::Clear(GetData<T>());
  }

  /*!
   * Deep copy source object to this object
   *
   * \param source Source object
   */
  inline void DeepCopyFromImpl(const T& source, tFactory* f = NULL)
  {
    sSerialization::DeepCopy(source, *GetData<T>(), f);
  }

  virtual void Deserialize(tInputStream& is)
  {
    is >> *GetData<T>();
  }

  virtual void Deserialize(tStringInputStream& is)
  {
    is >> *GetData<T>();
  }

  virtual void Deserialize(const xml2::tXMLNode& node)
  {
    node >> *GetData<T>();
  }

  tGenericObjectWrapper(T* wrapped_object) : tGenericObject(tDataType<T>()), manager()
  {
    assert((reinterpret_cast<char*>(&manager) - reinterpret_cast<char*>(this)) == cMANAGER_OFFSET && "Manager offset invalid");
    wrapped = wrapped_object;
  }

  virtual void Serialize(tOutputStream& os) const
  {
    os << *GetData<T>();
  }

  virtual void Serialize(tStringOutputStream& os) const
  {
    os << *GetData<T>();
  }

  virtual void Serialize(xml2::tXMLNode& node) const
  {
    node << *GetData<T>();
  }

};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tGenericObjectWrapper_h__
