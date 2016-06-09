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
/*!\file    rrlib/serialization/tMemoryBuffer.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tMemoryBuffer.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/logging/messages.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tOutputStream.h"
#include "rrlib/serialization/tInputStream.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
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
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

const size_t tMemoryBuffer::cDEFAULT_SIZE;
const int tMemoryBuffer::cDEFAULT_RESIZE_FACTOR;

tMemoryBuffer::tMemoryBuffer(size_t size, float resize_factor) :
  backend(size),
  resize_reserve_factor(resize_factor),
  cur_size(0)
{
}

tMemoryBuffer::tMemoryBuffer(void* buffer, size_t size, bool empty) :
  backend((char*)buffer, size),
  resize_reserve_factor(1),
  cur_size(empty ? 0u : size)
{
}

void tMemoryBuffer::ApplyChange(const tMemoryBuffer& t, int64_t offset, int64_t dummy)
{
  EnsureCapacity(static_cast<int>((t.GetSize() + offset)), true, GetSize());
  backend.Put(static_cast<size_t>(offset), t.backend, 0u, t.GetSize());
  size_t required_size = static_cast<size_t>(offset + t.GetSize());
  cur_size = std::max(cur_size, required_size);
}

void tMemoryBuffer::CopyFrom(const tMemoryBuffer& source)
{
  EnsureCapacity(source.GetSize(), false, GetSize());
  backend.Put(0u, source.backend, 0u, source.GetSize());
  cur_size = source.GetSize();
}

void tMemoryBuffer::DirectRead(tInputStream& input_stream_buffer, tFixedBuffer& buffer, size_t offset, size_t len) const
{
  throw std::logic_error("Unsupported - shouldn't be called");
}

void tMemoryBuffer::DirectWrite(tOutputStream& output_stream_buffer, const tFixedBuffer& buffer, size_t offset, size_t len)
{
  throw std::logic_error("Unsupported - shouldn't be called");
}

void tMemoryBuffer::EnsureCapacity(size_t new_size, bool keep_contents, size_t old_size)
{
  if (new_size > backend.Capacity())
  {
    if (resize_reserve_factor <= 1)
    {
      throw std::out_of_range("Attempt to write outside of buffer");
    }
    if (resize_reserve_factor <= 1.2)
    {
      RRLIB_LOG_PRINT(DEBUG_WARNING, "Small resize_reserve_factor");
    }
  }

  Reallocate(new_size, keep_contents, old_size);
}

bool tMemoryBuffer::Equals(const tMemoryBuffer& other) const
{
  if (cur_size != other.cur_size)
  {
    return false;
  }

  return this == &other || memcmp(backend.GetPointer(), other.backend.GetPointer(), cur_size) == 0;
}


void tMemoryBuffer::Read(tInputStream& input_stream_buffer, tBufferInfo& buffer, size_t len) const
{
  buffer.SetRange(0u, cur_size);
  if (buffer.position >= cur_size)
  {
    throw std::out_of_range("Attempt to read outside of buffer");
  }
}

void tMemoryBuffer::Reallocate(size_t new_size, bool keep_contents, size_t old_size)
{
  if (new_size <= backend.Capacity())
  {
    return;
  }

  tFixedBuffer new_buffer(new_size);

  if (keep_contents)
  {
    // Copy old contents
    new_buffer.Put(0u, backend, 0u, old_size);
  }

  std::swap(backend, new_buffer);
}

void tMemoryBuffer::Reset(tInputStream& input_stream_buffer, tBufferInfo& buffer) const
{
  buffer.buffer = const_cast<tFixedBuffer*>(&backend);
  buffer.position = 0u;
  buffer.SetRange(0u, cur_size);
}

void tMemoryBuffer::Reset(tOutputStream& output_stream_buffer, tBufferInfo& buffer)
{
  EnsureCapacity(16, false, 0); // buffer should have at least space for 8+ bytes (in order to avoid assertion)
  buffer.buffer = &backend;
  buffer.position = 0u;
  buffer.SetRange(0u, backend.Capacity());
}

void tMemoryBuffer::Seek(tInputStream& input_stream, tBufferInfo& buffer, uint64_t position) const
{
  // Since the input stream has the complete buffer, seeking can only occur out of range
  throw std::out_of_range("Position out of range: " + std::to_string(position));
}

bool tMemoryBuffer::Write(tOutputStream& output_stream_buffer, tBufferInfo& buffer, int hint)
{
  // do we need size increase?
  if (hint >= 0)
  {
    size_t new_size = std::max(8, static_cast<int>(((backend.Capacity() + hint) * resize_reserve_factor)));
    EnsureCapacity(new_size, true, buffer.position);
    buffer.buffer = &backend;
  }
  buffer.end = backend.Capacity();  // don't modify buffer start
  return false;
}

tOutputStream& operator << (tOutputStream& stream, const tMemoryBuffer& buffer)
{
  stream.WriteLong(buffer.GetSize());
  if (buffer.GetSize())
  {
    stream.Write(buffer.GetBuffer(), 0u, buffer.GetSize());
  }
  return stream;
}

tInputStream& operator >> (tInputStream& stream, tMemoryBuffer& buffer)
{
  size_t size = stream.ReadLong();
  buffer.cur_size = 0u;
  buffer.Reallocate(size, false, -1u);
  if (size)
  {
    stream.ReadFully(buffer.backend, 0u, size);
  }
  buffer.cur_size = size;
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
