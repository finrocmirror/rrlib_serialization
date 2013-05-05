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
#include <thread>

#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/tStringOutputStream.h"

namespace rrlib
{
namespace serialization
{
tInputStream::tInputStream(tTypeEncoding encoding) :
  source_lock(),
  source_buffer(),
  boundary_buffer(),
  boundary_buffer_backend(14u),
  cur_buffer(NULL),
  source(NULL),
  const_source(NULL),
  absolute_read_pos(0),
  cur_skip_offset_target(-1),
  closed(false),
  direct_read_support(false),
  timeout(rrlib::time::tDuration::zero()),
  factory(NULL),
  encoding(encoding),
  custom_encoder(NULL)
{
  boundary_buffer.buffer = &(boundary_buffer_backend);
}

tInputStream::tInputStream(tTypeEncoder& encoder) :
  source_lock(),
  source_buffer(),
  boundary_buffer(),
  boundary_buffer_backend(14u),
  cur_buffer(NULL),
  source(NULL),
  const_source(NULL),
  absolute_read_pos(0),
  cur_skip_offset_target(-1),
  closed(false),
  direct_read_support(false),
  timeout(rrlib::time::tDuration::zero()),
  factory(NULL),
  encoding(tTypeEncoding::CUSTOM),
  custom_encoder(&encoder)
{
  boundary_buffer.buffer = &(boundary_buffer_backend);
}

void tInputStream::Close()
{
  if (!closed)
  {
    if (source != NULL)
    {
      source->Close(this, source_buffer);
    }
    else if (const_source != NULL)
    {
      const_source->Close(this, source_buffer);
    }
    source_lock.reset();
  }
  closed = true;
}

void tInputStream::FetchNextBytes(size_t min_required2)
{
  assert((min_required2 <= 8));
  assert((source != NULL || const_source != NULL));

  // are we finished using boundary buffer?
  if (UsingBoundaryBuffer() && boundary_buffer.position >= 7)
  {
    absolute_read_pos += 7;
    cur_buffer = &(source_buffer);
    EnsureAvailable(min_required2);
    return;
  }

  // put any remaining bytes in boundary buffer
  int remain = Remaining();
  absolute_read_pos += (cur_buffer->end - cur_buffer->start);
  if (remain > 0)
  {
    boundary_buffer.position = 7u - remain;
    boundary_buffer.start = 0u;
    absolute_read_pos -= 7;
    //boundaryBuffer.buffer.put(boundaryBuffer.position, sourceBuffer.buffer, sourceBuffer.position, remain);
    source_buffer.buffer->Get(source_buffer.position, *boundary_buffer.buffer, boundary_buffer.position, remain);  // equivalent, but without problem that SourceBuffer limit is changed in java
    cur_buffer = &(boundary_buffer);
  }

  // if we have a timeout set - wait until more data is available
  // TODO: this doesn't ensure that there are minRequired2 bytes available. However, it should be sufficient in 99.9% of the cases.
  if (timeout > rrlib::time::tDuration::zero())
  {
    rrlib::time::tDuration initial_sleep = std::chrono::milliseconds(20);  // timeout-related
    rrlib::time::tDuration slept = rrlib::time::tDuration::zero();  // timeout-related
    while (timeout > rrlib::time::tDuration::zero() && (!(source != NULL ? source->MoreDataAvailable(this, source_buffer) : const_source->MoreDataAvailable(this, source_buffer))))
    {
      initial_sleep *= 2;

      std::this_thread::sleep_for(initial_sleep);

      slept += initial_sleep;
      if (slept > timeout)
      {
        throw std::runtime_error("Read Timeout");
      }
    }
  }

  // read next block
  if (source != NULL)
  {
    source->Read(this, source_buffer, min_required2);
  }
  else
  {
    const_source->Read(this, source_buffer, min_required2);
  }
  assert((source_buffer.Remaining() >= min_required2));

  // (possibly) fill up boundary buffer
  if (remain > 0)
  {
    //boundaryBuffer.buffer.put(7, sourceBuffer.buffer, 0, minRequired2);
    source_buffer.buffer->Get(0u, *boundary_buffer.buffer, 7u, min_required2);
    boundary_buffer.end = 7u + min_required2;
    source_buffer.position += min_required2;
  }
}

bool tInputStream::MoreDataAvailable(tInputStream* input_stream_buffer, tBufferInfo& buffer)
{
  if (source != NULL)
  {
    return source->MoreDataAvailable(this, buffer);
  }
  else
  {
    return const_source->MoreDataAvailable(this, buffer);
  }
}

bool tInputStream::MoreDataAvailable()
{
  if (Remaining() > 0)
  {
    return true;
  }
  //System.out.println("Not here");
  return source != NULL ? source->MoreDataAvailable(this, source_buffer) : const_source->MoreDataAvailable(this, source_buffer);
}

int8_t tInputStream::Peek()
{
  EnsureAvailable(1u);
  int8_t b = cur_buffer->buffer->GetByte(cur_buffer->position);
  return b;
}

void tInputStream::Read(tInputStream* buf, tBufferInfo& buffer, size_t len)
{
  // read next chunk - and copy this info to chained input stream buffer
  cur_buffer->position = cur_buffer->end;  // move marker to end so that ensureAvailable fetches next bytes
  EnsureAvailable(len);
  buffer.Assign(*cur_buffer);
}

int8_t tInputStream::ReadByte()
{
  EnsureAvailable(1u);
  int8_t b = cur_buffer->buffer->GetByte(cur_buffer->position);
  cur_buffer->position++;
  return b;
}

double tInputStream::ReadDouble()
{
  EnsureAvailable(8u);
  double d = cur_buffer->buffer->GetDouble(cur_buffer->position);
  cur_buffer->position += 8u;
  return d;
}

float tInputStream::ReadFloat()
{
  EnsureAvailable(4u);
  float f = cur_buffer->buffer->GetFloat(cur_buffer->position);
  cur_buffer->position += 4u;
  return f;
}

void tInputStream::ReadFully(tFixedBuffer& bb, size_t off, size_t len)
{
  while (true)
  {
    int read = std::min(cur_buffer->Remaining(), len);
    cur_buffer->buffer->Get(cur_buffer->position, bb, off, read);
    len -= read;
    off += read;
    cur_buffer->position += read;
    assert((len >= 0));
    if (len == 0)
    {
      break;
    }
    if (UsingBoundaryBuffer() || (!direct_read_support))
    {
      FetchNextBytes(1u);
    }
    else
    {
      source->DirectRead(this, bb, off, len);  // shortcut
      absolute_read_pos += len;
      assert((cur_buffer->position == cur_buffer->end));
      break;
    }
  }
}

std::string tInputStream::ReadLine()
{
  tStringOutputStream sb;
  while (true)
  {
    int8_t b = ReadByte();
    if (b == 0 || b == '\n')
    {
      break;
    }
    sb.Append(static_cast<char>(b));
  }
  return sb.ToString();
}

void tInputStream::ReadSkipOffset()
{
  cur_skip_offset_target = absolute_read_pos + cur_buffer->position;
  cur_skip_offset_target += ReadInt();
  cur_skip_offset_target += 4;  // from readInt()
}

std::string tInputStream::ReadString()
{
  tStringOutputStream sb;  // no shortcut in C++, since String could be in this chunk only partly
  ReadStringImpl(sb);
  return sb.ToString();
}

void tInputStream::ReadString(tStringOutputStream& sb)
{
  sb.Clear();
  ReadStringImpl(sb);
}

std::string tInputStream::ReadString(size_t length)
{
  tStringOutputStream sb;  // no shortcut in C++, since String could be in this chunk only partly
  ReadString(sb, length);
  return sb.ToString();
}

void tInputStream::ReadString(tStringOutputStream& sb, size_t length)
{
  size_t count = 0u;
  while (true)
  {
    int8_t b = ReadByte();
    if (b == 0 || count == length)
    {
      break;
    }
    sb.Append(b);
    count++;
  }
}

void tInputStream::Reset()
{
  if (source != NULL)
  {
    source->Reset(this, source_buffer);
    direct_read_support = source->DirectReadSupport();
    closed = false;
  }
  else if (const_source != NULL)
  {
    const_source->Reset(this, source_buffer);
    direct_read_support = const_source->DirectReadSupport();
    closed = false;
  }
  cur_buffer = &(source_buffer);
  absolute_read_pos = 0;
}

void tInputStream::Reset(const tConstSource& source)
{
  Close();
  this->source = NULL;
  this->const_source = &source;
  Reset();
}

void tInputStream::Reset(tSource& source)
{
  Close();
  this->source = &source;
  this->const_source = NULL;
  Reset();
}

void tInputStream::Skip(size_t n)
{
  while (true)
  {
    if (Remaining() >= n)
    {
      cur_buffer->position += n;
      return;
    }
    n -= Remaining();
    cur_buffer->position = cur_buffer->end;
    FetchNextBytes(1u);
  }
}

void tInputStream::SkipString()
{
  while (true)
  {
    int8_t c = ReadByte();
    if (c == 0)
    {
      break;
    }
  }
}

void tInputStream::ToSkipTarget()
{
  int64_t pos = cur_buffer->position;
  assert((cur_skip_offset_target >= absolute_read_pos + pos));
  Skip(static_cast<size_t>((cur_skip_offset_target - absolute_read_pos - pos)));
  cur_skip_offset_target = 0;
}

} // namespace rrlib
} // namespace serialization

