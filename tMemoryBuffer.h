/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2007-2011 Max Reichardt,
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

#ifndef __rrlib__serialization__tMemoryBuffer_h__
#define __rrlib__serialization__tMemoryBuffer_h__

#include "rrlib/serialization/tSerializable.h"
#include "rrlib/serialization/tConstSource.h"
#include "rrlib/serialization/tSink.h"
#include "rrlib/serialization/tBufferInfo.h"
#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/tFixedBuffer.h"
#include <boost/utility.hpp>
#include <stdint.h>

namespace rrlib
{
namespace serialization
{
class tOutputStream;

/*!
 * \author Max Reichardt
 *
 * Memory buffer that can be used as source and concurrent sink.
 *
 * When used as sink, it can grow when required. A resizeFactor <= 1 indicates that this should not happen.
 * The buffer size is limited to 2GB with respect to serialization.
 *
 * Writing and reading concurrently is not supported - due to resize.
 */
class tMemoryBuffer : public tSerializable, public tConstSource, public tSink, public boost::noncopyable
{
protected:

  /*! Wrapped memory buffer */
  tFixedBuffer backend;

  /*! When buffer needs to be reallocated, new size is multiplied with this factor to have some bytes in reserve */
  float resize_reserve_factor;

  /*! Current size of buffer */
  size_t cur_size;

public:

  /*! Size of temporary array */
  static const size_t cTEMP_ARRAY_SIZE = 2048u;

  /*! Default size of temp buffer */
  static const size_t cDEFAULT_SIZE = 8192u;

  /*! Default factor for buffer size increase */
  static const int cDEFAULT_RESIZE_FACTOR = 2;

protected:

  /*!
   * Ensure that memory buffer has at least this size.
   * If not, backend will be reallocated.
   *
   * \param new_size New Size in bytes
   * \param keep_contents Keep contents when reallocating?
   * \param old_size Old Size (only relevant if contents are to be kept)
   */
  void EnsureCapacity(int new_size, bool keep_contents, size_t old_size);

  /*!
   * Reallocate backend
   *
   * \param new_size New size
   * \param keep_contents Keep contents of backend?
   * \param old_size Old Size (only relevant of contents are to be kept)
   */
  void Reallocate(size_t new_size, bool keep_contents, size_t old_size);

public:

  /*! move constructor */
  tMemoryBuffer(tMemoryBuffer && o) :
    backend(),
    resize_reserve_factor(cDEFAULT_RESIZE_FACTOR),
    cur_size(0)
  {
    std::swap(backend, o.backend);
    std::swap(resize_reserve_factor, o.resize_reserve_factor);
    std::swap(cur_size, o.cur_size);
  }

  /*!
   * \param size Initial buffer size
   * \param resize_factor When buffer needs to be reallocated, new size is multiplied with this factor to have some bytes in reserve
   */
  tMemoryBuffer(size_t size = cDEFAULT_SIZE, float resize_factor = cDEFAULT_RESIZE_FACTOR);

  /*!
   * Wraps existing buffer
   *
   * \param empty Is this an empty buffer of the specified size?
   */
  tMemoryBuffer(void* buffer, size_t size, bool empty = false);

  /*! move assignment */
  tMemoryBuffer& operator=(tMemoryBuffer && o)
  {
    std::swap(backend, o.backend);
    std::swap(resize_reserve_factor, o.resize_reserve_factor);
    std::swap(cur_size, o.cur_size);
    return *this;
  }

  void ApplyChange(const tMemoryBuffer& t, int64_t offset, int64_t dummy);

  /*!
   * Clear buffer
   */
  inline void Clear()
  {
    cur_size = 0u;
  }

  // ConstSource implementation

  virtual void Close(tInputStream* input_stream_buffer, tBufferInfo& buffer) const
  {
    // do nothing, really
    buffer.Reset();
  }

  // Sink implementation

  virtual void Close(tOutputStream* output_stream_buffer, tBufferInfo& buffer)
  {
    // do nothing, really
    buffer.Reset();
  }

  void CopyFrom(const tMemoryBuffer& source);

  virtual ~tMemoryBuffer()
  {
  }

  // CustomSerializable implementation

  virtual void Deserialize(tInputStream& rv)
  {
    int size = rv.ReadInt();  // Buffer size is limited to 2 GB
    Deserialize(rv, size);
  }

  /*!
   * Reset this buffer and
   * copy data from stream to it
   *
   * \param size Number of bytes to
   */
  void Deserialize(tInputStream& rv, size_t size);

  virtual void DirectRead(tInputStream* input_stream_buffer, tFixedBuffer& buffer, size_t offset, size_t len) const;

  virtual bool DirectReadSupport() const
  {
    return false;
  }

  virtual void DirectWrite(tOutputStream* output_stream_buffer, const tFixedBuffer& buffer, size_t offset, size_t len);

  virtual bool DirectWriteSupport()
  {
    return false;
  }

  /*!
   * \param other Other memory buffer
   * \return Is content of this memory buffer equal to other memory buffer
   */
  bool Equals(const tMemoryBuffer& other) const;

  virtual void Flush(tOutputStream* output_stream_buffer, const tBufferInfo& buffer)
  {
    cur_size = buffer.position;  // update buffer size
  }

  //! returns buffer backend
  inline tFixedBuffer* GetBuffer()
  {
    return &backend;
  }

  //! returns pointer to buffer backend - with specified offset in bytes
  inline char* GetBufferPointer(int offset)
  {
    return GetBuffer()->GetPointer() + offset;
  }

  //! returns pointer to buffer backend - with specified offset in bytes
  inline const char* GetBufferPointer(int offset) const
  {
    return GetBuffer()->GetPointer() + offset;
  }

  /*!
   * \return Raw buffer backend
   */
  inline const tFixedBuffer* GetBuffer() const
  {
    return &backend;
  }

  /*!
   * \return Buffer capacity
   */
  inline int GetCapacity() const
  {
    return backend.Capacity();
  }

  /*!
   * \return description
   */
  inline const char* GetDescription() const
  {
    return "MemoryBuffer";
  }

  /*!
   * \return the resizeReserveFactor
   */
  inline float GetResizeReserveFactor() const
  {
    return resize_reserve_factor;
  }

  /*!
   * \return Buffer size
   */
  inline int GetSize() const
  {
    return cur_size;
  }

  virtual bool MoreDataAvailable(tInputStream* input_stream_buffer, tBufferInfo& buffer) const
  {
    return buffer.end < cur_size;
  }

  virtual void Read(tInputStream* input_stream_buffer, tBufferInfo& buffer, size_t len) const;

  virtual void Reset(tInputStream* input_stream_buffer, tBufferInfo& buffer) const;

  virtual void Reset(tOutputStream* output_stream_buffer, tBufferInfo& buffer);

  virtual void Serialize(tOutputStream& sb) const;

  /*!
   * \param resize_reserve_factor the resizeReserveFactor to set
   */
  inline void SetResizeReserveFactor(float resize_reserve_factor_)
  {
    this->resize_reserve_factor = resize_reserve_factor_;
  }

  virtual bool Write(tOutputStream* output_stream_buffer, tBufferInfo& buffer, int hint);

  bool operator=(const tMemoryBuffer& o) const
  {
    return Equals(o);
  }
};

/*!
 * memory buffer with initial buffer allocated from stack (with size SIZE)
 */
template <size_t SIZE>
class tStackMemoryBuffer : public tMemoryBuffer
{
  char initial_buffer[SIZE];

public:
  tStackMemoryBuffer(float resize_factor = cDEFAULT_RESIZE_FACTOR) : tMemoryBuffer(initial_buffer, SIZE), initial_buffer()
  {
    resize_reserve_factor = resize_factor;
  }
};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tMemoryBuffer_h__
