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

#ifndef __rrlib__serialization__tGenericObject_h__
#define __rrlib__serialization__tGenericObject_h__

#include "rrlib/serialization/tDataTypeBase.h"
#include "rrlib/serialization/tSerializable.h"
#include <assert.h>
#include <boost/utility.hpp>
#include <typeinfo>

namespace rrlib
{
namespace serialization
{
class tFactory;
class tGenericObjectManager;

/*!
 * \author Max Reichardt
 *
 * Container/wrapper for an arbitrary object.
 *
 * Provides base functionality such as deep copying, type information
 * and serialization.
 * It also assert that casting back is only possible to the original type.
 *
 * This allows to handle objects in a uniform way.
 *
 * Memory Layout of all subclasses: vtable ptr | datatype ptr | object ptr | management info raw memory of size M
 */
class tGenericObject : public boost::noncopyable, public tSerializable
{
private:

  /*! Data Type of wrapped object */
  tDataTypeBase type;

protected:

  /*! Wrapped object */
  void* wrapped;

  /*!
   * Deep copy source object to this object
   * (types MUST match)
   *
   * \param source Source object
   */
  virtual void DeepCopyFrom(const void* source, tFactory* f = NULL) = 0;

  /*!
   * \return Management information for this generic object.
   */
  inline tGenericObjectManager* GetManager()
  {
    return reinterpret_cast<tGenericObjectManager*>(reinterpret_cast<char*>(this) + cMANAGER_OFFSET);
  }

public:

  static const size_t cMANAGER_OFFSET = (sizeof(void*) == 4) ? 16 : 24;

  /*!
   * \param wrapped Wrapped object
   * \param dt Data Type of wrapped object
   */
  tGenericObject(tDataTypeBase dt) :
      type(dt),
      wrapped()
  {
  }

  /*!
   * Deep copy source object to this object
   * (types MUST match)
   *
   * \param source Source object
   */
  inline void DeepCopyFrom(const tGenericObject* source, tFactory* f = NULL)
  {
    assert(((source->type == type)) && "Types must match");

    DeepCopyFrom(source->wrapped, f);
  }

  /*!
   * Deep copy source object to this object
   * (types MUST match)
   *
   * \param source Source object
   */
  template <typename T>
  inline void DeepCopyFrom(const T* source, tFactory* f = NULL)
  {
    assert(typeid(T).name() == type.GetRttiName());
    DeepCopyFrom((void*)source);
  }

  template <typename T>
  T* GetData()
  {
    assert(typeid(T).name() == type.GetRttiName());
    return static_cast<T*>(wrapped);
  }

  /*!
   * \return Wrapped object (type T must match original type)
   */
  template <typename T>
  inline const T* GetData() const
  {
    assert(typeid(T).name() == type.GetRttiName());
    return static_cast<const T*>(wrapped);
  }

  /*!
   * \return Data Type of wrapped object
   */
  inline tDataTypeBase GetType() const
  {
    return type;
  }

};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tGenericObject_h__
