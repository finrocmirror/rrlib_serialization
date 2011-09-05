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
#include "rrlib/serialization/tGenericObject.h"

namespace rrlib
{
namespace serialization
{
const double tOutputStream::cBUFFER_COPY_FRACTION = 0.25;

tOutputStream::tOutputStream(tOutputStream::tTypeEncoding encoding_) :
    sink(NULL),
    immediate_flush(false),
    closed(true),
    buffer(),
    cur_skip_offset_placeholder(-1),
    buffer_copy_fraction(0),
    direct_write_support(false),
    encoding(encoding_),
    custom_encoder()
{
}

tOutputStream::tOutputStream(std::shared_ptr<tTypeEncoder> encoder) :
    sink(NULL),
    immediate_flush(false),
    closed(true),
    buffer(),
    cur_skip_offset_placeholder(-1),
    buffer_copy_fraction(0),
    direct_write_support(false),
    encoding(eCustom),
    custom_encoder(encoder)
{
}

tOutputStream::tOutputStream(std::shared_ptr<tSink> sink_, tOutputStream::tTypeEncoding encoding_) :
    sink(NULL),
    immediate_flush(false),
    closed(true),
    buffer(),
    cur_skip_offset_placeholder(-1),
    buffer_copy_fraction(0),
    direct_write_support(false),
    encoding(encoding_),
    custom_encoder()
{
  Reset(sink_);
}

tOutputStream::tOutputStream(tSink* sink_, tOutputStream::tTypeEncoding encoding_) :
    sink(NULL),
    immediate_flush(false),
    closed(true),
    buffer(),
    cur_skip_offset_placeholder(-1),
    buffer_copy_fraction(0),
    direct_write_support(false),
    encoding(encoding_),
    custom_encoder()
{
  Reset(sink_);
}

tOutputStream::tOutputStream(std::shared_ptr<tSink> sink_, std::shared_ptr<tTypeEncoder> encoder) :
    sink(NULL),
    immediate_flush(false),
    closed(true),
    buffer(),
    cur_skip_offset_placeholder(-1),
    buffer_copy_fraction(0),
    direct_write_support(false),
    encoding(eCustom),
    custom_encoder(encoder)
{
  Reset(sink_);
}

tOutputStream::tOutputStream(tSink* sink_, std::shared_ptr<tTypeEncoder> encoder) :
    sink(NULL),
    immediate_flush(false),
    closed(true),
    buffer(),
    cur_skip_offset_placeholder(-1),
    buffer_copy_fraction(0),
    direct_write_support(false),
    encoding(eCustom),
    custom_encoder(encoder)
{
  Reset(sink_);
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
    assert((buffer.Remaining() >= 8));
    buffer_copy_fraction = static_cast<size_t>((buffer.Capacity() * cBUFFER_COPY_FRACTION));
  }
}

void tOutputStream::Println(const std::string& s)
{
  WriteString(s, false);
  WriteByte('\n');
  CheckFlush();
}

void tOutputStream::Reset(tSink* sink_)
{
  Close();
  this->sink = sink_;
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
  assert((cur_skip_offset_placeholder >= 0));
  buffer.buffer->PutInt(cur_skip_offset_placeholder, buffer.position - cur_skip_offset_placeholder - 4);
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

void tOutputStream::WriteObject(const tGenericObject* to)
{
  if (to == NULL)
  {
    WriteType(NULL);
    return;
  }

  //writeSkipOffsetPlaceholder();
  WriteType(to->GetType());
  to->Serialize(*this);
  //skipTargetHere();
}

void tOutputStream::WriteSkipOffsetPlaceholder()
{
  assert((cur_skip_offset_placeholder < 0));
  cur_skip_offset_placeholder = buffer.position;

  WriteInt(0x80000000);
}

void tOutputStream::WriteString(const std::string& s, bool terminate)
{
  size_t len = terminate ? (s.size() + 1) : s.size();
  Write(tFixedBuffer((char*)s.c_str(), len));

}

void tOutputStream::WriteType(tDataTypeBase type)
{
  if (encoding == eLocalUids)
  {
    WriteShort(type.GetUid());
  }
  else if (encoding == eNames)
  {
    WriteString(type.GetName());
  }
  else
  {
    custom_encoder->WriteType(*this, type);
  }
}

} // namespace rrlib
} // namespace serialization

