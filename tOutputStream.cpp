/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2009-2011 Max Reichardt,
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
#include "rrlib/serialization/tOutputStream.h"
#include "rrlib/serialization/tInputStream.h"

namespace rrlib
{
namespace serialization
{
const double tOutputStream::cBUFFER_COPY_FRACTION = 0.25;

tOutputStream::tOutputStream(tTypeEncoding encoding) :
  sink(NULL),
  immediate_flush(false),
  closed(true),
  buffer(),
  cur_skip_offset_placeholder(-1),
  short_skip_offset(false),
  buffer_copy_fraction(0),
  direct_write_support(false),
  encoding(encoding),
  custom_encoder(NULL)
{
}

tOutputStream::tOutputStream(tTypeEncoder& encoder) :
  sink(NULL),
  immediate_flush(false),
  closed(true),
  buffer(),
  cur_skip_offset_placeholder(-1),
  short_skip_offset(false),
  buffer_copy_fraction(0),
  direct_write_support(false),
  encoding(tTypeEncoding::CUSTOM),
  custom_encoder(&encoder)
{
}

tOutputStream::tOutputStream(std::shared_ptr<tSink> sink, tTypeEncoding encoding) :
  sink(NULL),
  immediate_flush(false),
  closed(true),
  buffer(),
  cur_skip_offset_placeholder(-1),
  short_skip_offset(false),
  buffer_copy_fraction(0),
  direct_write_support(false),
  encoding(encoding),
  custom_encoder(NULL)
{
  Reset(sink);
}

tOutputStream::tOutputStream(tSink& sink, tTypeEncoding encoding) :
  sink(NULL),
  immediate_flush(false),
  closed(true),
  buffer(),
  cur_skip_offset_placeholder(-1),
  short_skip_offset(false),
  buffer_copy_fraction(0),
  direct_write_support(false),
  encoding(encoding),
  custom_encoder(NULL)
{
  Reset(sink);
}

tOutputStream::tOutputStream(std::shared_ptr<tSink> sink, tTypeEncoder& encoder) :
  sink(NULL),
  immediate_flush(false),
  closed(true),
  buffer(),
  cur_skip_offset_placeholder(-1),
  buffer_copy_fraction(0),
  direct_write_support(false),
  encoding(tTypeEncoding::CUSTOM),
  custom_encoder(&encoder)
{
  Reset(sink);
}

tOutputStream::tOutputStream(tSink& sink, tTypeEncoder& encoder) :
  sink(NULL),
  immediate_flush(false),
  closed(true),
  buffer(),
  cur_skip_offset_placeholder(-1),
  buffer_copy_fraction(0),
  direct_write_support(false),
  encoding(tTypeEncoding::CUSTOM),
  custom_encoder(&encoder)
{
  Reset(sink);
}

void tOutputStream::Close()
{
  if (!closed)
  {
    Flush();
    sink->Close(this, buffer);
    sink_lock.reset();
  }
  closed = true;
}

void tOutputStream::CommitData(int add_size_hint)
{
  if (GetWriteSize() > 0)
  {
    if (sink->Write(this, buffer, add_size_hint))
    {
      assert((cur_skip_offset_placeholder < 0));
    }
    assert(add_size_hint < 0 || buffer.Remaining() >= 8);
    buffer_copy_fraction = static_cast<size_t>((buffer.Capacity() * cBUFFER_COPY_FRACTION));
  }
}

void tOutputStream::Println(const std::string& s)
{
  WriteString(s, false);
  WriteByte('\n');
  CheckFlush();
}

void tOutputStream::Reset(tSink& sink)
{
  Close();
  this->sink = &sink;
  Reset();
}

void tOutputStream::Reset()
{
  sink->Reset(this, buffer);
  assert((buffer.Remaining() >= 8));
  closed = false;
  buffer_copy_fraction = static_cast<size_t>((buffer.Capacity() * cBUFFER_COPY_FRACTION));
  direct_write_support = sink->DirectWriteSupport();
}

void tOutputStream::SkipTargetHere()
{
  assert(cur_skip_offset_placeholder >= 0);
  if (short_skip_offset)
  {
    buffer.buffer->PutByte(cur_skip_offset_placeholder, buffer.position - cur_skip_offset_placeholder - 1);
  }
  else
  {
    buffer.buffer->PutInt(cur_skip_offset_placeholder, buffer.position - cur_skip_offset_placeholder - 4);
  }
  cur_skip_offset_placeholder = -1;
}

void tOutputStream::Write(const tFixedBuffer& bb, size_t off, size_t len)
{
  if ((Remaining() >= len) && (len < GetCopyFraction() || cur_skip_offset_placeholder >= 0))
  {
    buffer.buffer->Put(buffer.position, bb, off, len);
    buffer.position += len;
  }
  else
  {
    if (direct_write_support && cur_skip_offset_placeholder < 0)
    {
      CommitData(-1);
      sink->DirectWrite(this, bb, off, len);
    }
    else
    {
      while (len > 0)
      {
        int write = std::min(len, Remaining());
        buffer.buffer->Put(buffer.position, bb, off, write);
        buffer.position += write;
        len -= write;
        off += write;
        assert((len >= 0));
        if (len == 0)
        {
          return;
        }
        CommitData(len);
      }
    }
  }
}

bool tOutputStream::Write(tOutputStream* output_stream_buffer, tBufferInfo& buffer_, int size_hint)
{
  bool result = sink->Write(this, buffer_, size_hint);
  this->buffer.Assign(buffer_);  // synchronize
  return result;
}

void tOutputStream::WriteAllAvailable(tInputStream* input_stream)
{
  while (input_stream->MoreDataAvailable())
  {
    input_stream->EnsureAvailable(1u);
    Write(*input_stream->cur_buffer->buffer, input_stream->cur_buffer->position, input_stream->cur_buffer->Remaining());
    input_stream->cur_buffer->position = input_stream->cur_buffer->end;
  }
}

void tOutputStream::WriteSkipOffsetPlaceholder(bool short_skip_offset)
{
  assert((cur_skip_offset_placeholder < 0));
  cur_skip_offset_placeholder = buffer.position;
  this->short_skip_offset = short_skip_offset;

  if (short_skip_offset)
  {
    WriteByte(0x80);
  }
  else
  {
    WriteInt(0x80000000);
  }
}

void tOutputStream::WriteString(const std::string& s, bool terminate)
{
  size_t len = terminate ? (s.size() + 1) : s.size();
  Write(tFixedBuffer((char*)s.c_str(), len));

}

} // namespace rrlib
} // namespace serialization

