//
// You received this file as part of RRLib
// Robotics Research Library
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tFixedBuffer_h__
#define __rrlib__serialization__tFixedBuffer_h__

#include <assert.h>
#include <boost/utility.hpp>
#include <cstring>
#include <stdint.h>
#include <string>

namespace rrlib
{
namespace serialization
{
/*!
 * \author Max Reichardt
 *
 * This is a simple fixed-size memory buffer.
 *
 * In Java it wraps a java.nio.ByteBuffer (this allows sharing the buffer with C++).
 * Its methods are also very similar to ByteBuffer's - which makes ByteBuffer
 * a good source for documentation ;-)
 *
 * Writing arrays/buffers to the buffer is not thread-safe in Java. Everything else is.
 */
class tFixedBuffer : public boost::noncopyable
{
protected:

  /*! Actual (wrapped) buffer - may be replaced by subclasses */
  friend class tReadView;

  char* buffer; // pointer to buffer start
  size_t capacity_x; // buffer capacity
  bool owns_buf; // owned buffers are deleted when this class is

  /*!
   * Set wrapped buffer to buffer contained in other fixed buffer
   * (only call, when FixedBuffer doesn't own a buffer himself)
   *
   * \param fb
   */
  void SetCurrentBuffer(tFixedBuffer* fb);

public:

  // \param buffer_ pointer to buffer start
  // \param capacity_ capacity of wrapped buffer
  tFixedBuffer(char* buffer_, size_t capacity_) :
    buffer(buffer_),
    capacity_x(capacity_),
    owns_buf(false)
  {}

  tFixedBuffer(size_t capacity_ = 0);

  // move constructor
  tFixedBuffer(tFixedBuffer && fb);

  // move assignment
  tFixedBuffer& operator=(tFixedBuffer && o);

  virtual ~tFixedBuffer();

  /*!
   * \return Capacity of buffer (in bytes)
   */
  inline size_t Capacity() const
  {
    return capacity_x;
  }

  // returns raw pointer to buffer start
  const char* GetPointer() const
  {
    return buffer;
  }

  // returns raw pointer to buffer start
  char* GetPointer()
  {
    return buffer;
  }

  // Generic put method...
  // \param offset absolute offset
  // \param t Data to write
  template <typename T>
  void PutImpl(size_t offset, T t)
  {
    assert(offset + sizeof(T) <= capacity_x);
    T* ptr = (T*)(buffer + offset);
    (*ptr) = t;
  }

  // Generic get method...
  // \param offset absolute offset
  // \return t Data at offset
  template <typename T>
  T GetImpl(size_t offset) const
  {
    assert(offset + sizeof(T) <= capacity_x);
    T* ptr = (T*)(buffer + offset);
    return *ptr;
  }

  // Copy data from source buffer
  // \param off absolute offset
  // \param other source buffer
  // \param len number of bytes to copy
  void Put(size_t off, const void* other, size_t len)
  {
    assert(off + len <= capacity_x);
    memcpy(buffer + off, other, len);
  }

  // Copy data to destination buffer
  // \param off absolute offset
  // \param other destination buffer
  // \param len number of bytes to copy
  void Get(size_t off, void* other, size_t len) const
  {
    assert(off + len <= capacity_x);
    memcpy(other, buffer + off, len);
  }

  /*!
   * Copy Data to destination buffer
   *
   * \param offset Offset in this buffer
   * \param dst Destination array
   * \param off offset in source array
   * \param length number of bytes to copy
   */
  inline void Get(size_t offset, tFixedBuffer& dst, size_t off, size_t len) const
  {
    assert(off + len <= dst.Capacity());
    Get(offset, dst.GetPointer() + off, len);
  }

  /*!
   * Copy Data to destination buffer
   * (whole buffer is filled)
   *
   * \param offset Offset in this buffer
   * \param dst Destination array
   */
  inline void Get(size_t offset, tFixedBuffer& dst) const
  {
    Get(offset, dst.GetPointer(), dst.Capacity());
  }

  /*!
   * \param offset absolute offset
   * \return (1-byte) boolean
   */
  inline bool GetBoolean(size_t offset) const
  {
    return GetByte(offset) != 0;
  }

  /*!
   * \param offset absolute offset
   * \return 8 bit integer
   */
  inline int8_t GetByte(size_t offset) const
  {
    return GetImpl<int8_t>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return Character
   */
  inline char GetChar(size_t offset) const
  {
    return GetImpl<char>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return 64 bit floating point
   */
  inline double GetDouble(size_t offset) const
  {
    return GetImpl<double>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return 32 bit floating point
   */
  inline float GetFloat(size_t offset) const
  {
    return GetImpl<float>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return 32 bit integer
   */
  inline int GetInt(size_t offset) const
  {
    return GetImpl<int>(offset);
  }

  /*!
   * Read String/Line from buffer (ends either at line delimiter or 0-character - 8bit)
   *
   * \param offset absolute offset in buffer
   */
  std::string GetLine(size_t offset) const;

  /*!
   * \param offset absolute offset
   * \return 64 bit integer
   */
  inline int64_t GetLong(size_t offset) const
  {
    return GetImpl<int64_t>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return 16 bit integer
   */
  inline int16_t GetShort(size_t offset) const
  {
    return GetImpl<int16_t>(offset);
  }

  /*!
   * Read null-terminated string (8 Bit Characters - Suited for ASCII)
   *
   * \param offset absolute offset in buffer
   */
  inline std::string GetString(size_t offset) const
  {
    return std::string(buffer + offset);
  }

  /*!
   * Read string (8 Bit Characters - Suited for ASCII). Stops at null-termination or specified length.
   *
   * \param offset absolute offset
   * \param length Length of string to read
   */
  std::string GetString(size_t offset, size_t length) const;

  /*!
   * \param offset absolute offset
   * \return unsigned 1 byte integer
   */
  inline uint GetUnsignedByte(size_t offset) const
  {
    return GetImpl<uint8_t>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return unsigned 2 byte integer
   */
  inline uint GetUnsignedShort(size_t offset) const
  {
    return GetImpl<uint16_t>(offset);
  }

  /*!
   * Copy Data from source buffer
   *
   * \param offset Offset in this buffer
   * \param src Source Buffer
   * \param off offset in source array
   * \param length number of bytes to copy
   */
  inline void Put(size_t offset, const tFixedBuffer& src, size_t off, size_t len)
  {
    assert(off + len <= src.Capacity());
    Put(offset, src.GetPointer() + off, len);
  }

  /*!
   * Copy Data from source buffer
   * (whole buffer is copied)
   *
   * \param offset Offset in this buffer
   * \param src Source Buffer
   */
  inline void Put(size_t offset, const tFixedBuffer& src)
  {
    Put(offset, src.GetPointer(), src.Capacity());
  }

  /*!
   * \param offset absolute offset
   * \param v (1-byte) boolean
   */
  inline void PutBoolean(size_t offset, bool v)
  {
    PutByte(offset, v ? 1 : 0);
  }

  /*!
   * \param offset absolute offset
   * \param v 8 bit integer
   */
  inline void PutByte(size_t offset, int v)
  {
    PutImpl<int8_t>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v Character
   */
  inline void PutChar(size_t offset, char v)
  {
    PutImpl<char>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v 64 bit floating point
   */
  inline void PutDouble(size_t offset, double v)
  {
    PutImpl<double>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v 32 bit floating point
   */
  inline void PutFloat(size_t offset, float v)
  {
    PutImpl<float>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v 32 bit integer
   */
  inline void PutInt(size_t offset, int v)
  {
    PutImpl<int>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v 64 bit integer
   */
  inline void PutLong(size_t offset, int64_t v)
  {
    PutImpl<int64_t>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v 16 bit integer
   */
  inline void PutShort(size_t offset, int v)
  {
    PutImpl<int16_t>(offset, v);
  }

  /*!
   * Write null-terminated string (8 Bit Characters - Suited for ASCII)
   *
   * \param offset absolute offset in buffer
   * \param s String
   */
  inline void PutString(size_t offset, const std::string& s)
  {
    PutString(offset, s, true);
  }

  /*!
   * Write string (8 Bit Characters - Suited for ASCII)
   *
   * \param offset absolute offset in buffer
   * \param s String
   * \param terminate Terminate string with zero?
   */
  inline void PutString(size_t offset, const std::string& s, bool terminate)
  {
    Put(offset, s.c_str(), terminate ? s.size() + 1 : s.size());
  }

  /*!
   * Zero out specified bytes
   *
   * \param offset Offset in buffer to start at
   * \param length Length of area to zero out
   */
  inline void ZeroOut(size_t offset, size_t length)
  {
    assert(offset + length <= capacity_x);
    memset(buffer + offset, 0, length);

  }

};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tFixedBuffer_h__
