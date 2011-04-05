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

#ifndef __rrlib__serialization__tDataType_h__
#define __rrlib__serialization__tDataType_h__

#include "rrlib/serialization/tGenericObjectManager.h"
#include "rrlib/serialization/tDataTypeBase.h"
#include <assert.h>
#include <string>
#include <typeinfo>

#include "rrlib/serialization/detail/tListInfo.h"
#include <boost/type_traits/has_virtual_destructor.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include "rrlib/serialization/tCustomTypeInitialization.h"
#include "rrlib/serialization/tStlContainerSuitable.h"
#include <cstring>

namespace rrlib
{
namespace serialization
{
class tGenericObject;
class tFactory;
class tInputStream;
class tOutputStream;

/*!
 * \author Max Reichardt
 *
 * Objects of this class contain info about the data type T
 */
template<typename T>
class tDataType : public tDataTypeBase
{
  /*!
   * Data type info with factory functions
   */
  class tDataTypeInfo : public tDataTypeBase::tDataTypeInfoRaw
  {
  public:

    tDataTypeInfo();

    template < typename Q = T >
    void InitImpl(typename boost::enable_if_c<boost::is_base_of<tCustomTypeInitialization, Q>::value, tCustomTypeInitialization*>::type d)
    {
      T::CustomTypeInitialization(tDataTypeBase(this), (T*)NULL);
    }

    void InitImpl(void* d) {}

    template <bool B>
    typename boost::enable_if_c<B, tDataTypeBase::tDataTypeInfoRaw*>::type GetListTypeInfo()
    {
      return tDataType<typename detail::tListInfo<T>::tListType>::GetDataTypeInfo();
    }

    template <bool B>
    typename boost::disable_if_c<B, tDataTypeBase::tDataTypeInfoRaw*>::type GetListTypeInfo()
    {
      return NULL;
    }

    template <bool B>
    typename boost::enable_if_c<B, tDataTypeBase::tDataTypeInfoRaw*>::type GetSharedPtrListTypeInfo()
    {
      return tDataType<typename detail::tListInfo<T>::tSharedPtrListType>::GetDataTypeInfo();
    }

    template <bool B>
    typename boost::disable_if_c<B, tDataTypeBase::tDataTypeInfoRaw*>::type GetSharedPtrListTypeInfo()
    {
      return NULL;
    }

    virtual void Init()
    {
      if (type == ePLAIN)
      {
        list_type = GetListTypeInfo<tStlContainerSuitable<T>::value >();
        shared_ptr_list_type = GetSharedPtrListTypeInfo<tCreateSharedPtrListType<T>::value >();
      }
      else
      {
        element_type = tDataType<typename detail::tListInfo<T>::tElementType>::GetDataTypeInfo();
      }
      InitImpl((T*)NULL);
    }

    virtual void* CreateInstance(void* placement) const
    {
      if (placement == NULL)
      {
        placement = operator new(sizeof(T));
      }
      memset(placement, 0, sizeof(T)); // set memory to 0 so that memcmp on class T can be performed cleanly for certain types
      return new(placement) T();
    }

    virtual tGenericObject* CreateInstanceGeneric(void* placement, size_t manager_size) const;

    virtual void DeepCopy(const void* src, void* dest, tFactory* f) const;

    virtual void Deserialize(tInputStream& is, void* obj) const
    {
      T* s = static_cast<T*>(obj);
      if (boost::has_virtual_destructor<T>::value)
      {
        assert(typeid(*s).name() == typeid(T).name());
      }
      is >> *s;
    }

    virtual void Serialize(tOutputStream& os, const void* obj) const
    {
      const T* s = static_cast<const T*>(obj);
      if (boost::has_virtual_destructor<T>::value)
      {
        assert(typeid(*s).name() == typeid(T).name());
      }
      os << *s;
    }

  };

public:
  tDataType() : tDataTypeBase(GetDataTypeInfo()) {}

  // \param name Name data type should get (if different from default)
  tDataType(const std::string& name) : tDataTypeBase(GetDataTypeInfo())
  {
    GetDataTypeInfo()->SetName(name);
  }

  // Lookup data type by rtti name
  // Tries T first
  //
  // \param rtti_name rtti name
  // \return Data type with specified name (== NULL if it could not be found)
  static tDataTypeBase FindTypeByRtti(const char* rtti_name)
  {
    if (rtti_name == GetDataTypeInfo()->rtti_name)
    {
      return tDataType();
    }
    return tDataTypeBase::FindTypeByRtti(rtti_name);
  }

  // \return DataTypeInfo for this type T
  static tDataTypeInfoRaw* GetDataTypeInfo()
  {
    static tDataTypeInfo info;
    return &info;
  }

};

} // namespace rrlib
} // namespace serialization

#include "rrlib/serialization/tDataType.hpp"

namespace rrlib
{
namespace serialization
{
template <>
class tDataType<detail::tNothing> : public tDataTypeBase
{
public:
  tDataType() : tDataTypeBase(NULL) {}
  static tDataTypeInfoRaw* GetDataTypeInfo()
  {
    return NULL;
  }
};

extern template class tDataType<tMemoryBuffer>;
extern template class tDataType<int8_t>;
extern template class tDataType<int16_t>;
extern template class tDataType<int>;
extern template class tDataType<long int>;
extern template class tDataType<long long int>;
extern template class tDataType<uint8_t>;
extern template class tDataType<uint16_t>;
extern template class tDataType<unsigned int>;
extern template class tDataType<unsigned long int>;
extern template class tDataType<unsigned long long int>;
extern template class tDataType<double>;
extern template class tDataType<float>;

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tDataType_h__
