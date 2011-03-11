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
#include "rrlib/serialization/tMemoryBuffer.h"
#include "rrlib/serialization/tDataType.h"
#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/tOutputStream.h"
#include <stdexcept>

namespace rrlib
{
namespace serialization
{
const size_t tMemoryBuffer::cTEMP_ARRAY_SIZE;
const size_t tMemoryBuffer::cDEFAULT_SIZE;
const float tMemoryBuffer::cDEFAULT_RESIZE_FACTOR;
const tDataTypeBase tMemoryBuffer::cTYPE = tDataType<tMemoryBuffer>();

tMemoryBuffer::tMemoryBuffer(size_t size, float resize_factor) :
    backend(new tFixedBuffer(size)),
    resize_reserve_factor(resize_factor),
    cur_size(0)
{
}

void tMemoryBuffer::ApplyChange(const tMemoryBuffer& t, int64_t offset, int64_t dummy)
{
  EnsureCapacity(static_cast<int>((t.GetSize() + offset)), true, GetSize());
  backend->Put(static_cast<size_t>(offset), *t.backend, 0u, t.GetSize());
  size_t required_size = static_cast<size_t>(offset + t.GetSize());
  cur_size = std::max(cur_size, required_size);
}

void tMemoryBuffer::CopyFrom(tMemoryBuffer* source)
{
  EnsureCapacity(source->GetSize(), false, GetSize());
  backend->Put(0u, *source->backend, 0u, source->GetSize());
  cur_size = source->GetSize();
}

void tMemoryBuffer::Deserialize(tInputStream& rv)
{
  int size = rv.ReadInt();  // Buffer size is limited to 2 GB
  cur_size = 0u;
  Reallocate(size, false, -1u);
  rv.ReadFully(*backend, 0u, size);
  cur_size = size;
}

void tMemoryBuffer::DirectRead(tInputStream* input_stream_buffer, tFixedBuffer& buffer, size_t offset, size_t len) const
{
  throw std::logic_error("Unsupported - shouldn't be called");
}

void tMemoryBuffer::DirectWrite(tOutputStream* output_stream_buffer, const tFixedBuffer& buffer, size_t offset, size_t len)
{
  throw std::logic_error("Unsupported - shouldn't be called");
}

void tMemoryBuffer::EnsureCapacity(int new_size, bool keep_contents, size_t old_size)
{
  if (resize_reserve_factor <= 1)
  {
    throw std::out_of_range("Attempt to write outside of buffer");
  }
  if (resize_reserve_factor <= 1.2)
  {
    //System.out.println("warning: small resizeReserveFactor");
    RRLIB_LOG_STREAM(logging::eLL_DEBUG_WARNING, log_domain, "warning: small resizeReserveFactor");
  }

  Reallocate(new_size, keep_contents, old_size);
}

void tMemoryBuffer::Read(tInputStream* input_stream_buffer, tBufferInfo& buffer, size_t len) const
{
  buffer.SetRange(0u, cur_size);
  if (buffer.position >= cur_size)
  {
    throw std::out_of_range("Attempt to read outside of buffer");
  }
}

void tMemoryBuffer::Reallocate(size_t new_size, bool keep_contents, size_t old_size)
{
  if (new_size <= backend->Capacity())
  {
    return;
  }

  tFixedBuffer* new_buffer = new tFixedBuffer(new_size);

  if (keep_contents)
  {
    // Copy old contents
    new_buffer->Put(0u, *backend, 0u, old_size);
  }

  DeleteOldBackend(backend);
  backend = new_buffer;
}

void tMemoryBuffer::Reset(tInputStream* input_stream_buffer, tBufferInfo& buffer) const
{
  buffer.buffer = backend;
  buffer.position = 0u;
  buffer.SetRange(0u, cur_size);
}

void tMemoryBuffer::Reset(tOutputStream* output_stream_buffer, tBufferInfo& buffer)
{
  buffer.buffer = backend;
  buffer.position = 0u;
  buffer.SetRange(0u, backend->Capacity());
}

void tMemoryBuffer::Serialize(tOutputStream& sb) const
{
  sb.WriteInt(cur_size);
  sb.Write(*backend, 0u, cur_size);
}

bool tMemoryBuffer::Write(tOutputStream* output_stream_buffer, tBufferInfo& buffer, int hint)
{
  // do we need size increase?
  if (hint >= 0)
  {
    size_t new_size = std::max(8, static_cast<int>(((backend->Capacity() + hint) * resize_reserve_factor)));
    EnsureCapacity(new_size, true, buffer.position);
    buffer.buffer = backend;
  }
  buffer.end = backend->Capacity();  // don't modify buffer start
  return false;
}

} // namespace rrlib
} // namespace serialization

