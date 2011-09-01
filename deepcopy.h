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

#ifndef __rrlib__serialization__deepcopy_h__
#define __rrlib__serialization__deepcopy_h__

#include "rrlib/serialization/sStaticFactory.h"
#include "rrlib/serialization/tMemoryBuffer.h"

/*!
 * \author Max Reichardt
 *
 * This file contains the deepcopy namespace with diverse
 * functions for deep-copying objects.
 *
 * If no function matches, the default serialization mechanism
 * is used as a fallback.
 *
 * More functions for the optimized deep copying of custom types
 * may be added by specializing the sDeepCopy struct.
 */

namespace rrlib
{
namespace serialization
{
namespace detail
{
template <typename T>
inline void SerializationFwdDeepCopy(const T& t, T& t2, tFactory* f);

// Helper to be able to resize vectors of noncopyable types
template <typename C, typename E, bool COPYABLE>
struct tResize
{
  static inline void Resize(C& c, size_t new_size)
  {
    c.resize(new_size);
  }
};

template <typename C, typename E>
struct tResize<C, E, false>
{
  static inline void Resize(C& c, size_t new_size)
  {
    while (c.size() < new_size)
    {
      c.push_back(sStaticFactory<E>::CreateByValue());
    }
    while (c.size() > new_size)
    {
      c.pop_back();
    }
  }
};

}

namespace deepcopy
{

template <typename T>
inline void CopyElement(const T& t, T& t2, tFactory* f)
{
  detail::SerializationFwdDeepCopy(t, t2, f);
}

template <typename T>
inline void CopyElement(const std::shared_ptr<T>& t, std::shared_ptr<T>& t2, tFactory* f)
{
  detail::SerializationFwdDeepCopy(*t, *t2, f);
}

template <typename C, bool b>
struct tSimpleContainerCopy
{
  static inline void Copy(const C& src, C& dest)
  {
    dest = src;
  }
};

template <typename C>
struct tSimpleContainerCopy<C, true>
{
  static inline void Copy(const C& src, C& dest)
  {
    assert(false && "Programming error below (?)");
  }
};


template <typename C, typename T>
inline void CopySTLContainer(const C& src, C& dest, tFactory* f)
{
  typedef detail::tListElemInfo<T> info;
  if (info::is_shared_ptr || std::is_base_of<boost::noncopyable, T>::value)
  {
    //dest.resize(src.size());
    detail::tResize < C, T, !std::is_base_of<boost::noncopyable, T>::value >::Resize(dest, src.size());

    for (size_t i = 0; i < src.size(); i++)
    {
      if (info::IsNull(src[i]))
      {
        info::Reset(dest[i]);
        continue;
      }
      tDataTypeBase needed = info::GetType(src[i]);
      if (info::IsNull(dest[i]) || info::GetType(dest[i]) != needed)
      {
        info::ChangeBufferType(f, dest[i], needed);
      }
      if (needed != info::GetTypeT())
      {
        needed.DeepCopy(&info::GetElem(src[i]), &info::GetElem(dest[i]), f);
      }
      else
      {
        CopyElement(src[i], dest[i], f);
      }
    }
  }
  else
  {
    tSimpleContainerCopy<C, std::is_base_of<boost::noncopyable, T>::value>::Copy(src, dest);
  }
}

template <typename T>
inline void Copy(const std::vector<T>& src, std::vector<T>& dest, tFactory* f)
{
  CopySTLContainer<std::vector<T>, T>(src, dest, f);
}
template <typename T>
inline void Copy(const std::list<T>& src, std::list<T>& dest, tFactory* f)
{
  CopySTLContainer<std::list<T>, T>(src, dest, f);
}
template <typename T>
inline void Copy(const std::deque<T>& src, std::deque<T>& dest, tFactory* f)
{
  CopySTLContainer<std::deque<T>, T>(src, dest, f);
}

} // namespace

/*!
 * Struct that defines how to do deep-copying for objects of type T
 * if they are noncopyable and do not have CopyFrom method.
 *
 * May be specialized for certain types.
 *
 * (reason for this class: implementing alternative deepcopy::Copy methods
 *  did not always work as expected)
 */
template <typename T>
struct sDeepCopy
{
  static void Copy(const T& src, T& dest, tFactory* f)
  {
    tStackMemoryBuffer<65536> buffer;
    tOutputStream os(&buffer);
    os << src;
    os.Close();
    tInputStream ci(&buffer);
    ci.SetFactory(f);
    ci >> dest;
    ci.Close();
  }
};

namespace detail
{

struct tFactoryWrapper
{
  tFactory* factory;
  tFactoryWrapper(tFactory* f) : factory(f) {}
};

struct tSerializableWrapper
{
  tSerializable* s;
  tSerializableWrapper(tSerializable& f) : s(&f) {}
};

// if deepcopy is defined, take this
template <typename T>
inline void DeepCopy(const T& t, T& t2, tFactory* f, decltype(rrlib::serialization::deepcopy::Copy(*((T*)NULL), *((T*)NULL), NULL))* = NULL)
{
  rrlib::serialization::deepcopy::Copy(t, t2, f);
}

// if copyFrom is defined, take this
template <typename T>
inline void DeepCopy(const T& t, T& t2, tFactory* f, decltype(((T*)NULL)->CopyFrom(*((T*)NULL)))* = NULL)
{
  t2.CopyFrom(t);
}

template <typename T, bool B>
struct tCopyImpl
{
  inline static void DeepCopyImpl(const T& t, T& t2, tFactory* f)
  {
    t2 = t;
  }
};

template <typename T>
struct tCopyImpl<T, true>
{
  inline static void DeepCopyImpl(const T& src, T& dest, tFactory* f)
  {
    sDeepCopy<T>::Copy(src, dest, f);
  }
};

// if not, use = operator
template <typename T>
inline void DeepCopy(const T& t, T& t2, tFactoryWrapper f)
{
  tCopyImpl<T, std::is_base_of<boost::noncopyable, T>::value>::DeepCopyImpl(t, t2, f.factory);
}

// if no deepcopy operator and no = operator - try serialization
//template <typename T>
//inline void DeepCopy(const T& src, tSerializableWrapper dest, tFactoryWrapper f) {
//  tInputStream ci(&src);
//  ci.SetFactory(f.factory);
//  tDataType<T>().Deserialize(ci, dest.s);
//  ci.Close();
//}

} // namespace
} // namespace
} // namespace

#include "rrlib/serialization/sSerialization.h"

namespace rrlib
{
namespace serialization
{
namespace detail
{
template <typename T>
inline void SerializationFwdDeepCopy(const T& t, T& t2, tFactory* f)
{
  sSerialization::DeepCopy(t, t2, f);
}

} // namespace
} // namespace
} // namespace

#endif // __rrlib__serialization__deepcopy_h__
