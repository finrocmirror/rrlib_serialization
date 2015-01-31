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
/*!\file    rrlib/serialization/tMemoryBuffer.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * \brief   Contains tMemoryBuffer
 *
 * \b tMemoryBuffer
 *
 * Memory buffer that can be used as (concurrent) source and as sink.
 *
 * When used as sink, it can grow when required.
 * A resize_factor <= 1 indicates that this should not happen.
 *
 * Writing concurrently to reading is not supported - e.g. due to resizing etc..
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tMemoryBuffer_h__
#define __rrlib__serialization__tMemoryBuffer_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tBufferInfo.h"
#include "rrlib/serialization/tConstSource.h"
#include "rrlib/serialization/tFixedBuffer.h"
#include "rrlib/serialization/tSink.h"

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
class tInputStream;
class tOutputStream;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Memory buffer suitable for binary (de)serialization
/*!
 * Memory buffer that can be used as (concurrent) source and as sink.
 *
 * When used as sink, it can grow when required.
 * A resize_factor <= 1 indicates that this should not happen.
 *
 * Writing concurrently to reading is not supported (e.g. due to resizing etc.).
 *
 * Notably, a memory buffer has a capacity (allocated memory) and a size (the portion
 * of memory that is currently used) - similar to e.g. std::vector.
 */
class tMemoryBuffer : public tConstSource, public tSink, public util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Default size of temp buffer */
  static const size_t cDEFAULT_SIZE = 8192u;

  /*! Default factor for buffer size increase */
  static const int cDEFAULT_RESIZE_FACTOR = 2;

  /*!
   * \param size Initial buffer size
   * \param resize_factor When buffer needs to be reallocated, new size is multiplied with this factor to have some bytes in reserve
   */
  tMemoryBuffer(size_t size = cDEFAULT_SIZE, float resize_factor = cDEFAULT_RESIZE_FACTOR);

  /*!
   * Wraps existing buffer
   *
   * \param buffer Pointer to start of buffer to wrap
   * \param size Size (== capacity) of buffer to wrap
   * \param empty Is this an empty buffer of the specified size? If true, GetSize() returns zero. Otherwise, the buffer's size is equal to its capacity.
   */
  tMemoryBuffer(void* buffer, size_t size, bool empty = false);

  /*! move constructor */
  tMemoryBuffer(tMemoryBuffer && o) :
    backend(0u),
    resize_reserve_factor(cDEFAULT_RESIZE_FACTOR),
    cur_size(0)
  {
    std::swap(backend, o.backend);
    std::swap(resize_reserve_factor, o.resize_reserve_factor);
    std::swap(cur_size, o.cur_size);
  }

  /*! move assignment */
  tMemoryBuffer& operator=(tMemoryBuffer && o)
  {
    std::swap(backend, o.backend);
    std::swap(resize_reserve_factor, o.resize_reserve_factor);
    std::swap(cur_size, o.cur_size);
    return *this;
  }

  /*!
   * Overwrites parts of this memory buffer with contents of specified memory buffer.
   *
   * \param t Memory buffer containing new contents (is written completely to specified offset)
   * \param offset Offset in this memory buffer where to start writing at
   * \param unused Unused parameter
   */
  void ApplyChange(const tMemoryBuffer& t, int64_t offset, int64_t unused = 0);

  /*!
   * Clear buffer
   */
  inline void Clear()
  {
    cur_size = 0u;
  }

  /*!
   * Makes this memory buffer a (deep) copy of provided memory buffer
   *
   * \param source Provided memory buffer
   */
  void CopyFrom(const tMemoryBuffer& source);

  /*!
   * \param other Other memory buffer
   * \return Is content of this memory buffer equal to other memory buffer
   */
  bool Equals(const tMemoryBuffer& other) const;

  /*!
   * \return Returns fixed-size buffer used as backend
   */
  inline tFixedBuffer& GetBuffer()
  {
    return backend;
  }
  inline const tFixedBuffer& GetBuffer() const
  {
    return backend;
  }

  /*!
   * \return Returns pointer to buffer backend - with specified offset in bytes
   */
  inline char* GetBufferPointer(int offset = 0)
  {
    return GetBuffer().GetPointer() + offset;
  }
  inline const char* GetBufferPointer(int offset = 0) const
  {
    return GetBuffer().GetPointer() + offset;
  }

  /*!
   * \return Buffer capacity
   */
  inline int GetCapacity() const
  {
    return backend.Capacity();
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
  inline size_t GetSize() const
  {
    return cur_size;
  }

  /*!
   * \param resize_reserve_factor the resizeReserveFactor to set
   */
  inline void SetResizeReserveFactor(float resize_reserve_factor)
  {
    this->resize_reserve_factor = resize_reserve_factor;
  }

  bool operator==(const tMemoryBuffer& o) const
  {
    return Equals(o);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend tInputStream& operator >> (tInputStream& stream, tMemoryBuffer& buffer);


  /*! Wrapped memory buffer */
  tFixedBuffer backend;

  /*! When buffer needs to be reallocated, new size is multiplied with this factor to have some bytes in reserve */
  float resize_reserve_factor;

  /*! Current size of buffer */
  size_t cur_size;


  virtual void Close(tInputStream& input_stream_buffer, tBufferInfo& buffer) const override
  {
    // do nothing, really
    buffer.Reset();
  }

  virtual void Close(tOutputStream& output_stream_buffer, tBufferInfo& buffer) override
  {
    // do nothing, really
    buffer.Reset();
  }

  virtual void DirectRead(tInputStream& input_stream_buffer, tFixedBuffer& buffer, size_t offset, size_t len) const override;

  virtual bool DirectReadSupport() const override
  {
    return false;
  }

  virtual void DirectWrite(tOutputStream& output_stream_buffer, const tFixedBuffer& buffer, size_t offset, size_t len) override;

  virtual bool DirectWriteSupport() override
  {
    return false;
  }

  /*!
   * Ensure that memory buffer has at least this size.
   * If not, backend will be reallocated.
   *
   * \param new_size New Size in bytes
   * \param keep_contents Keep contents when reallocating?
   * \param old_size Old Size (only relevant if contents are to be kept)
   */
  void EnsureCapacity(size_t new_size, bool keep_contents, size_t old_size);

  virtual void Flush(tOutputStream& output_stream_buffer, const tBufferInfo& buffer) override
  {
    cur_size = buffer.position;  // update buffer size
  }

  /*!
   * \return description
   */
  inline const char* GetDescription() const
  {
    return "MemoryBuffer";
  }

  /*!
   * Reallocate backend
   *
   * \param new_size New size
   * \param keep_contents Keep contents of backend?
   * \param old_size Old Size (only relevant of contents are to be kept)
   */
  void Reallocate(size_t new_size, bool keep_contents, size_t old_size);

  virtual bool MoreDataAvailable(tInputStream& input_stream_buffer, tBufferInfo& buffer) const override
  {
    return buffer.end < cur_size;
  }

  virtual void Read(tInputStream& input_stream_buffer, tBufferInfo& buffer, size_t len) const override;

  virtual void Reset(tInputStream& input_stream_buffer, tBufferInfo& buffer) const override;

  virtual void Reset(tOutputStream& output_stream_buffer, tBufferInfo& buffer) override;

  virtual void Seek(tInputStream& input_stream, tBufferInfo& buffer, uint64_t position) const override;

  virtual bool SeekSupport() const override
  {
    return true;
  }

  virtual bool Write(tOutputStream& output_stream_buffer, tBufferInfo& buffer, int hint) override;
};

tOutputStream& operator << (tOutputStream& stream, const tMemoryBuffer& buffer);

tInputStream& operator >> (tInputStream& stream, tMemoryBuffer& buffer);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
