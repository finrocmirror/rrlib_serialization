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
/*!\file    rrlib/serialization/tInputStream.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tInputStream.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <thread>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tStringOutputStream.h"

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

tInputStream::tInputStream(tTypeEncoding encoding) :
  source_buffer(),
  boundary_buffer(),
  boundary_buffer_memory(),
  boundary_buffer_backend(boundary_buffer_memory, 14u),
  cur_buffer(NULL),
  source(NULL),
  const_source(NULL),
  absolute_read_pos(0),
  cur_skip_offset_target(-1),
  closed(false),
  direct_read_support(false),
  timeout(rrlib::time::tDuration::zero()),
  encoding(encoding),
  custom_encoder(NULL)
{
  boundary_buffer.buffer = &(boundary_buffer_backend);
}

void tInputStream::Close()
{
  if (!closed)
  {
    if (source != NULL)
    {
      source->Close(*this, source_buffer);
    }
    else if (const_source != NULL)
    {
      const_source->Close(*this, source_buffer);
    }
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
    while (timeout > rrlib::time::tDuration::zero() && (!(source != NULL ? source->MoreDataAvailable(*this, source_buffer) : const_source->MoreDataAvailable(*this, source_buffer))))
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
    source->Read(*this, source_buffer, min_required2);
  }
  else
  {
    const_source->Read(*this, source_buffer, min_required2);
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

bool tInputStream::MoreDataAvailable()
{
  if (Remaining() > 0)
  {
    return true;
  }
  //System.out.println("Not here");
  return source != NULL ? source->MoreDataAvailable(*this, source_buffer) : const_source->MoreDataAvailable(*this, source_buffer);
}

int8_t tInputStream::Peek()
{
  EnsureAvailable(1u);
  int8_t b = cur_buffer->buffer->GetByte(cur_buffer->position);
  return b;
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
      source->DirectRead(*this, bb, off, len);  // shortcut
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

std::string tInputStream::ReadString(size_t max_length)
{
  std::stringstream string_buffer;
  ReadString(string_buffer, max_length);
  return string_buffer.str();
}

void tInputStream::ReadString(std::string& string_buffer, size_t max_length)
{
  if (string_buffer.length())
  {
    size_t max_length_for_buffer = std::min(string_buffer.length() + 1, max_length);
    char temp_buffer[max_length_for_buffer + 1];
    size_t read = ReadString(temp_buffer, max_length_for_buffer, false);
    bool terminated = (temp_buffer[read - 1] == 0);
    temp_buffer[read] = 0;
    if (terminated || read == max_length)
    {
      string_buffer.assign(temp_buffer);
    }
    else
    {
      std::stringstream string_stream;
      string_stream << temp_buffer;
      ReadString(string_stream, max_length - read);
      string_buffer = string_stream.str();
    }
  }
  else
  {
    string_buffer = ReadString(max_length);
  }
}

void tInputStream::ReadString(std::stringstream& string_stream, size_t max_length)
{
  const size_t BUFFER_LENGTH = 1024;
  char temp_buffer[BUFFER_LENGTH + 1];
  while (max_length)
  {
    size_t max_length_for_buffer = std::min(BUFFER_LENGTH, max_length);
    size_t read = ReadString(temp_buffer, max_length_for_buffer, false);
    bool terminated = (temp_buffer[read - 1] == 0);
    temp_buffer[read] = 0;
    if (terminated)
    {
      string_stream << temp_buffer;
      return;
    }
    string_stream << temp_buffer;

    max_length -= read;
  }
}

size_t tInputStream::ReadString(char* buffer, size_t max_length, bool terminate_if_length_exceeded)
{
  size_t read = 0;
  while (max_length)
  {
    EnsureAvailable(1u);
    if (terminate_if_length_exceeded && max_length == 1)
    {
      buffer[read] = 0;
      if (Peek() == 0)
      {
        ReadByte();
      }
      return read + 1;;
    }

    size_t length = std::min(terminate_if_length_exceeded ? (max_length - 1) : max_length, Remaining());
    char* start_pointer = cur_buffer->buffer->GetPointer() + cur_buffer->position;
    void* terminator = memchr(start_pointer, 0, length);
    if (terminator)
    {
      length = (static_cast<char*>(terminator) - start_pointer) + 1; // include terminator
      memcpy(&(buffer[read]), start_pointer, length);
      cur_buffer->position += length;
      return read + length;
    }
    memcpy(&(buffer[read]), start_pointer, length);
    max_length -= length;
    read += length;
    cur_buffer->position += length;
  }
  return read;
}

void tInputStream::Reset()
{
  if (source != NULL)
  {
    source->Reset(*this, source_buffer);
    direct_read_support = source->DirectReadSupport();
    closed = false;
  }
  else if (const_source != NULL)
  {
    const_source->Reset(*this, source_buffer);
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

void tInputStream::Seek(int64_t position)
{
  int64_t offset = position - this->absolute_read_pos;
  if ((source && source->SeekSupport()) || (const_source && const_source->SeekSupport()))
  {
    if (offset >= 0 && static_cast<size_t>(offset) < this->cur_buffer->Capacity())
    {
      // goal position lies in the current buffer, read it from there
      this->cur_buffer->position = this->cur_buffer->start + offset;
    }
    else
    {
      // need to seek the source
      if (source)
      {
        source->Seek(*this, source_buffer, position);
      }
      else
      {
        const_source->Seek(*this, source_buffer, position);
      }
      this->cur_buffer = &source_buffer;
      this->absolute_read_pos = position - cur_buffer->position;
    }
  }
  else
  {
    if (offset > 0)
    {
      this->Skip(offset);
    }
    else
    {
      throw std::runtime_error("Cannot seek backwards in stream");
    }
  }
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
  // TODO: optimize
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

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
