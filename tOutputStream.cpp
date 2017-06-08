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
/*!\file    rrlib/serialization/tOutputStream.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tOutputStream.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/PublishedRegisters.h"

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
const double tOutputStream::cBUFFER_COPY_FRACTION = 0.25;

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

void tOutputStream::Close()
{
  if (!closed)
  {
    Flush();
    sink->Close(*this, buffer);
  }
  closed = true;
}

void tOutputStream::CommitData(int add_size_hint)
{
  if (GetPosition() > 0)
  {
    if (sink->Write(*this, buffer, add_size_hint))
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

void tOutputStream::Reset(tSink& sink, const tSerializationInfo& serialization_target)
{
  Close();
  this->sink = &sink;
  this->shared_serialization_info.serialization_target = serialization_target;
  // TODO: a possible optimization would be to not remove/add listeners if set of registers to be published on_change did not change (seems negligible currently)
  if (!this->shared_serialization_info.published_register_status.unique())
  {
    this->shared_serialization_info.published_register_status.reset(serialization_target.HasPublishedRegisters() ? new tPublishedRegisterStatus() : nullptr);
  }
  else if (serialization_target.HasPublishedRegisters())
  {
    this->shared_serialization_info.published_register_status->Reset();
  }
  else
  {
    this->shared_serialization_info.published_register_status.reset();
  }
  if (serialization_target.HasPublishedRegisters())
  {
    tPublishedRegisterStatus& status = *shared_serialization_info.published_register_status;
    std::function<void()> change_function = std::bind(&tPublishedRegisterStatus::OnRegisterUpdate, &status);
    for (uint i = 0; i < cMAX_PUBLISHED_REGISTERS; i++)
    {
      if (serialization_target.GetRegisterEntryEncoding(i) == tRegisterEntryEncoding::PUBLISH_REGISTER_ON_CHANGE)
      {
        PublishedRegisters::AddListener(i, change_function, &status);
        status.OnRegisterUpdate();
        status.registered_listeners.set(i, true);
      }
    }
  }
  Reset();
}

void tOutputStream::Reset(tSink& sink, tOutputStream& shared_serialization_info_from)
{
  Close();
  this->sink = &sink;
  this->shared_serialization_info = shared_serialization_info_from.shared_serialization_info;
  Reset();
}

void tOutputStream::Reset()
{
  sink->Reset(*this, buffer);
  assert((buffer.Remaining() >= 8));
  closed = false;
  buffer_copy_fraction = static_cast<size_t>((buffer.Capacity() * cBUFFER_COPY_FRACTION));
  direct_write_support = sink->DirectWriteSupport();
}

void tOutputStream::Seek(size_t position)
{
  size_t desired_position = buffer.start + position;
  if (desired_position > buffer.end)
  {
    throw std::invalid_argument("Position is out of bounds");
  }
  buffer.position = desired_position;
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
      sink->DirectWrite(*this, bb, off, len);
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

void tOutputStream::WriteAllAvailable(tInputStream& input_stream)
{
  while (input_stream.MoreDataAvailable())
  {
    input_stream.EnsureAvailable(1u);
    Write(*input_stream.cur_buffer->buffer, input_stream.cur_buffer->position, input_stream.cur_buffer->Remaining());
    input_stream.cur_buffer->position = input_stream.cur_buffer->end;
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

bool tOutputStream::WriteRegisterUpdatesImplementation(uint register_uid, size_t handle_size)
{
  bool escape_signal_written = false;

  // Update on_change registers
  for (size_t i = 0; i < cMAX_PUBLISHED_REGISTERS; i++)
  {
    tPublishedRegisterStatus& status = *shared_serialization_info.published_register_status;
    bool may_require_update = i == register_uid || shared_serialization_info.serialization_target.GetRegisterEntryEncoding(i) == tRegisterEntryEncoding::PUBLISH_REGISTER_ON_CHANGE;
    if (may_require_update)
    {
      size_t current_size = PublishedRegisters::Size(i);
      if (current_size > status.elements_written[i])
      {
        if (!escape_signal_written)
        {
          if (handle_size == 1)
          {
            WriteByte(-2);
          }
          else if (handle_size == 2)
          {
            WriteShort(-2);
          }
          else if (handle_size == 4)
          {
            WriteInt(-2);
          }
          escape_signal_written = true;
        }

        if (GetTargetInfo().revision == 0)
        {
          if (status.elements_written[i] == 0)
          {
            WriteShort(40);
          }

          // compatibility with legacy parts
          PublishedRegisters::SerializeEntries(*this, i, status.elements_written[i], current_size);
          WriteShort(-1);
        }
        else
        {
          WriteByte(i);
          WriteInt(current_size - status.elements_written[i]);
          PublishedRegisters::SerializeEntries(*this, i, status.elements_written[i], current_size);
        }
        status.elements_written[i] = current_size;
      }
    }
  }
  if (escape_signal_written && GetTargetInfo().revision) // non-legacy parts require this terminator
  {
    WriteByte(-1);
  }
  return escape_signal_written;
}

void tOutputStream::WriteString(const std::string& s, bool terminate)
{
  size_t len = terminate ? (s.size() + 1) : s.size();
  Write(tFixedBuffer((char*)s.c_str(), len));
}

void tOutputStream::tPublishedRegisterStatus::Reset()
{
  for (int i = 0; i < cMAX_PUBLISHED_REGISTERS; i++)
  {
    if (registered_listeners[i])
    {
      bool removed __attribute__((unused)) = PublishedRegisters::RemoveListener(i, this);
      assert(removed);
    }
  }
  registered_listeners.reset();
  elements_written.fill(0);
  on_register_change_update_counter = 0;
  counter_on_last_update = 0;
}


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
