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

/*!
 * \author Max Reichardt
 *
 * This file contains the deepcopy namespace with diverse
 * functions for deep-copying objects.
 *
 * If no function matches, the default serialization mechanism
 * is used as a fallback.
 *
 * More functions for custom types may be added to this namespace
 * by a user of this library.
 */

namespace rrlib
{
namespace serialization
{
namespace deepcopy
{

struct any_source
{
  tDataTypeBase type;
  tStackMemoryBuffer<65536> buffer;

  template <typename T>
  any_source(const T& t) : type(NULL), buffer()
  {
    type = tDataType<T>();
    tOutputStream os(&buffer);
    os << t;
    os.Close();
  }
};

struct any_dest
{
  tDataTypeBase type;
  void* dest_ptr;

  template <typename T>
  any_dest(T& t) : type(NULL), dest_ptr(&t)
  {
    type = tDataType<T>();
  }
};

// fallback: use serialization, if nothing else matches
// (uses wrappers, so that compiler chooses this last)
inline void Copy(any_source src, any_dest dest, tFactory* f)
{
  assert(src.type == dest.type);
  tInputStream ci(&(src.buffer));
  ci.SetFactory(f);
  dest.type.Deserialize(ci, dest.dest_ptr);
  ci.Close();
}

inline void Copy(const char src, char dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const int8_t src, int8_t dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const int16_t src, int16_t dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const int32_t src, int32_t dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const int64_t src, int64_t dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const uint8_t src, uint8_t dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const uint16_t src, uint16_t dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const uint32_t src, uint32_t dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const uint64_t src, uint64_t dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const float src, float dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const double src, double dest, tFactory* f)
{
  dest = src;
}
inline void Copy(const std::string src, std::string dest, tFactory* f)
{
  dest = src;
}

template <typename C, typename T>
inline void CopySTLContainer(const C& src, C& dest, tFactory* f)
{
  typedef detail::tListElemInfo<T> info;
  if (info::is_shared_ptr)
  {
    dest.resize(src.size());
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
        copy(src[i], dest[i], f);
      }
    }
  }
  else
  {
    dest = src;
  }
}

template <typename T>
inline void Copy(const std::vector<T> src, std::vector<T> dest, tFactory* f)
{
  CopySTLContainer<std::vector<T>, T>(src, dest, f);
}
template <typename T>
inline void Copy(const std::list<T> src, std::list<T> dest, tFactory* f)
{
  CopySTLContainer<std::list<T>, T>(src, dest, f);
}
template <typename T>
inline void Copy(const std::deque<T> src, std::deque<T> dest, tFactory* f)
{
  CopySTLContainer<std::deque<T>, T>(src, dest, f);
}



} // namespace
} // namespace
} // namespace

#endif // __rrlib__serialization__deepcopy_h__
