//
// You received this file as part of RRLib
// Robotics Research Library
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//----------------------------------------------------------------------
/*!\file    rrlib/serialization/tFixedBuffer.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * \brief   Contains tFixedBuffer
 *
 * \b tFixedBuffer
 *
 * This is a simple fixed-size memory buffer.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tFixedBuffer_h__
#define __rrlib__serialization__tFixedBuffer_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/util/tNoncopyable.h"
#include <cstring>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace rrlib
{
namespace serialization
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Simple fixed-size memory buffer.
/*!
 * This is a simple fixed-size memory buffer.
 */
class tFixedBuffer : private util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Wraps arbitrary memory as fixed buffer.
   *
   * \param buffer_memory pointer to buffer start
   * \param capacity capacity of wrapped buffer
   */
  tFixedBuffer(char* buffer_memory, size_t capacity) :
    buffer_memory(buffer_memory),
    capacity(capacity),
    owns_buffer(false)
  {}

  /*!
   * Creates new buffer with specified capacity.
   * (Owns this buffer.)
   *
   * \param capacity Capacity of this buffer
   */
  tFixedBuffer(size_t capacity = 0);

  /*! move constructor */
  tFixedBuffer(tFixedBuffer && fb);

  /*! move assignment */
  tFixedBuffer& operator=(tFixedBuffer && o);

  ~tFixedBuffer();


  /*!
   * \return Capacity of buffer (in bytes)
   */
  inline size_t Capacity() const
  {
    return capacity;
  }

  /*!
   * Copy data to destination buffer
   *
   * \param offset Offset in this buffer
   * \param destination Destination buffer
   * \param length Number of bytes to copy
   */
  void Get(size_t offset, void* destination, size_t length) const
  {
    assert(offset + length <= capacity);
    memcpy(destination, buffer_memory + offset, length);
  }

  /*!
   * Copy Data to destination buffer
   *
   * \param offset Offset in this buffer
   * \param destination Destination buffer
   * \param destination_offset Offset in destination buffer
   * \param length Number of bytes to copy
   */
  inline void Get(size_t offset, tFixedBuffer& destination, size_t destination_offset, size_t length) const
  {
    assert(destination_offset + length <= destination.Capacity());
    Get(offset, destination.GetPointer() + destination_offset, length);
  }

  /*!
   * Copy Data to destination buffer
   * (whole buffer is filled)
   *
   * \param offset Offset in this buffer
   * \param destination Destination buffer
   */
  inline void Get(size_t offset, tFixedBuffer& destination) const
  {
    Get(offset, destination.GetPointer(), destination.Capacity());
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
    return GetGeneric<int8_t>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return Character
   */
  inline char GetChar(size_t offset) const
  {
    return GetGeneric<char>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return 64 bit floating point
   */
  inline double GetDouble(size_t offset) const
  {
    return GetGeneric<double>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return 32 bit floating point
   */
  inline float GetFloat(size_t offset) const
  {
    return GetGeneric<float>(offset);
  }

  /*!
   * Generic get method...
   *
   * \param offset absolute offset
   * \return Data read at specified offset
   */
  template <typename T>
  inline T GetGeneric(size_t offset) const
  {
    assert(offset + sizeof(T) <= capacity);
    T value;
    memcpy(&value, buffer_memory + offset, sizeof(T));
    return value;
  }

  /*!
   * \param offset absolute offset
   * \return 32 bit integer
   */
  inline int GetInt(size_t offset) const
  {
    return GetGeneric<int>(offset);
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
    return GetGeneric<int64_t>(offset);
  }

  /*!
   * \return Returns raw pointer to buffer memory
   */
  const char* GetPointer() const
  {
    return buffer_memory;
  }
  char* GetPointer()
  {
    return buffer_memory;
  }

  /*!
   * \param offset absolute offset
   * \return 16 bit integer
   */
  inline int16_t GetShort(size_t offset) const
  {
    return GetGeneric<int16_t>(offset);
  }

  /*!
   * Read null-terminated string (8 Bit Characters - Suited for ASCII)
   *
   * \param offset absolute offset in buffer
   */
  inline std::string GetString(size_t offset) const
  {
    return std::string(buffer_memory + offset);
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
    return GetGeneric<uint8_t>(offset);
  }

  /*!
   * \param offset absolute offset
   * \return unsigned 2 byte integer
   */
  inline uint GetUnsignedShort(size_t offset) const
  {
    return GetGeneric<uint16_t>(offset);
  }

  /*!
   * Copy data from source buffer
   *
   * \param offset Absolute offset in this buffer
   * \param source_buffer Source buffer
   * \param length Number of bytes to copy
   */
  void Put(size_t offset, const void* source_buffer, size_t length)
  {
    assert(offset + length <= capacity);
    memcpy(buffer_memory + offset, source_buffer, length);
  }

  /*!
   * Copy Data from source buffer
   *
   * \param offset Offset in this buffer
   * \param source_buffer Source Buffer
   * \param source_offset offset in source array
   * \param length number of bytes to copy
   */
  inline void Put(size_t offset, const tFixedBuffer& source_buffer, size_t source_offset, size_t length)
  {
    assert(source_offset + length <= source_buffer.Capacity());
    Put(offset, source_buffer.GetPointer() + source_offset, length);
  }

  /*!
   * Copy Data from source buffer
   * (whole buffer is copied)
   *
   * \param offset Offset in this buffer
   * \param source_buffer Source Buffer
   */
  inline void Put(size_t offset, const tFixedBuffer& source_buffer)
  {
    Put(offset, source_buffer.GetPointer(), source_buffer.Capacity());
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
    PutGeneric<int8_t>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v Character
   */
  inline void PutChar(size_t offset, char v)
  {
    PutGeneric<char>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v 64 bit floating point
   */
  inline void PutDouble(size_t offset, double v)
  {
    PutGeneric<double>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v 32 bit floating point
   */
  inline void PutFloat(size_t offset, float v)
  {
    PutGeneric<float>(offset, v);
  }

  /*!
   * Generic put method...
   *
   * \param offset absolute offset
   * \param t Data to write
   */
  template <typename T>
  void PutGeneric(size_t offset, T t)
  {
    assert(offset + sizeof(T) <= capacity);
    memcpy(buffer_memory + offset, &t, sizeof(T));
  }

  /*!
   * \param offset absolute offset
   * \param v 32 bit integer
   */
  inline void PutInt(size_t offset, int v)
  {
    PutGeneric<int>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v 64 bit integer
   */
  inline void PutLong(size_t offset, int64_t v)
  {
    PutGeneric<int64_t>(offset, v);
  }

  /*!
   * \param offset absolute offset
   * \param v 16 bit integer
   */
  inline void PutShort(size_t offset, int v)
  {
    PutGeneric<int16_t>(offset, v);
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
    assert(offset + length <= capacity);
    memset(buffer_memory + offset, 0, length);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Pointer to buffer start */
  char* buffer_memory;

  /*! Buffer capacity */
  size_t capacity;

  /*! Owned buffers are deleted when this class is */
  bool owns_buffer;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
