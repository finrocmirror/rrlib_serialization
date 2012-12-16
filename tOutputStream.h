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

#ifndef __rrlib__serialization__tOutputStream_h__
#define __rrlib__serialization__tOutputStream_h__

#include "rrlib/time/time.h"
#include "rrlib/serialization/tSerializable.h"
#include "rrlib/serialization/tBufferInfo.h"
#include "rrlib/serialization/tTypeEncoder.h"
#include "rrlib/serialization/tSink.h"
#include "rrlib/serialization/tFixedBuffer.h"
#include <assert.h>
#include <boost/utility.hpp>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <endian.h>

namespace rrlib
{
namespace serialization
{
class tInputStream;

/*!
 * \author Max Reichardt
 *
 * Reasonably efficient, flexible, universal writing interface.
 *
 * Flexible "all-in-one" output stream memory buffer interface that implements various interfaces.
 * (in Java it can be combined with Streams and ByteBuffers, in C++ with output streams and boost iostreams)
 *
 * This class provides a universal data writing interface for memory buffers.
 * A manager class needs to be specified, which will customize what is actually done with the data.
 *
 * The implementation is designed to be reasonably efficient (no virtual function calls in C++; except of committing/fetching
 * data chunks from streams... however, this doesn't happen often and should not harm performance) -
 * that's why no interfaces are used for serialization, but rather the class itself. Support for further
 * read and write methods can be easily added.
 *
 * Size checking is performed for every write and read operation. For maximum performance, arrays/buffers can be used to
 * write and read data to/from this class. Buffers can be forwarded to a sink directly (they don't need to be buffered) avoiding
 * additional copying operations.
 *
 * The Class is explicitly _not_ thread-safe for writing - meaning multiple threads may not write to the same object at any given
 * moment in time.
 *
 * There are two modes of operation with respect to print-methods:
 *  1) flush immediately
 *  2) flush when requested or full
 */
class tOutputStream : public boost::noncopyable, public tSink
{
  // for "locking" object sink as long as this buffer exists
  std::shared_ptr<const tSink> sink_lock;

  /*! Committed buffers are buffered/copied (not forwarded directly), when smaller than 1/(2^n) of buffer capacity */
  static const double cBUFFER_COPY_FRACTION;

  /*! Source that determines where buffers that are written to come from and how they are handled */
  ::rrlib::serialization::tSink* sink;

  /*! Immediately flush buffer after printing? */
  const bool immediate_flush;

  /*! Has stream been closed? */
  bool closed;

  /*! Buffer that is currently written to - is managed by sink */
  tBufferInfo buffer;

  /*! -1 by default - buffer position when a skip offset placeholder has been set/written */
  int cur_skip_offset_placeholder;

  /*! hole Buffers are only buffered/copied, when they are smaller than this */
  size_t buffer_copy_fraction;

  /*! Is direct write support available with this sink? */
  bool direct_write_support;

  /*! Data type encoding that is used */
  tTypeEncoding encoding;

  /*! Custom type encoder */
  std::shared_ptr<tTypeEncoder> custom_encoder;

protected:

  /*!
   * Immediately flush buffer if appropriate option is set
   * Used in print methods
   */
  inline void CheckFlush()
  {
    if (immediate_flush)
    {
      Flush();
    }
  }

  /*!
   * Write current buffer contents to sink and clear buffer.
   *
   * \param add_size_hint Hint at how many additional bytes we want to write; -1 indicates manual flush without need for size increase
   */
  void CommitData(int add_size_hint);

  /*!
   * \return Whole Buffers are only buffered/copied, when they are smaller than this
   */
  inline size_t GetCopyFraction() const
  {
    assert((buffer_copy_fraction > 0));
    return buffer_copy_fraction;
  }

public:

  /*!
   * \param encoding Data type encoding that is used
   */
  tOutputStream(tTypeEncoding encoding_ = tTypeEncoding::LOCAL_UIDS);

  tOutputStream(std::shared_ptr<tTypeEncoder> encoder);

  /*!
   * \param sink_ Sink to write to
   * \param encoding Data type encoding that is used
   */
  tOutputStream(std::shared_ptr<tSink> sink_, tTypeEncoding encoding_);

  tOutputStream(tSink* sink_, tTypeEncoding encoding_ = tTypeEncoding::LOCAL_UIDS);

  /*!
   * \param sink_ Sink to write to
   * \param encoder Custom type encoder
   */
  tOutputStream(std::shared_ptr<tSink> sink_, std::shared_ptr<tTypeEncoder> encoder);

  tOutputStream(tSink* sink_, std::shared_ptr<tTypeEncoder> encoder);

  void Close();

  // Sink implementation - for chaining OutputStreamBuffers together

  virtual void Close(tOutputStream* output_stream_buffer, tBufferInfo& buffer_)
  {
    sink->Close(this, buffer_);
  }

  virtual ~tOutputStream()
  {
    Close();
  }

  virtual void DirectWrite(tOutputStream* output_stream_buffer, const tFixedBuffer& buffer_, size_t offset, size_t len)
  {
    sink->DirectWrite(this, buffer_, offset, len);
  }

  virtual bool DirectWriteSupport()
  {
    return sink->DirectWriteSupport();
  }

  /*!
   * Ensure that the specified number of bytes is available in buffer.
   * Possibly resize or flush.
   *
   * \param c Number of Bytes.
   */
  inline void EnsureAdditionalCapacity(size_t c)
  {
    if (Remaining() < c)
    {
      CommitData(c - Remaining());
    }
  }

  /*!
   * Flush current buffer contents to sink and clear buffer.
   * (with no immediate intent to write further data to buffer)
   */
  inline void Flush()
  {
    CommitData(-1);
    sink->Flush(this, buffer);
  }

  virtual void Flush(tOutputStream* output_stream_buffer, const tBufferInfo& info)
  {
    Flush();
  }

  /*!
   * \return Custom type encoder
   */
  tTypeEncoder* GetCustomTypeEncoder() const
  {
    return custom_encoder.get();
  }

  /*!
   * \return Data type encoding that is used
   */
  tTypeEncoding GetTypeEncoding() const
  {
    return encoding;
  }

  /*!
   * \return Size of data that was written to buffer
   * (typically useless - because of flushing etc. - only used by some internal stuff)
   */
  inline size_t GetWriteSize() const
  {
    return buffer.position - buffer.start;
  }

  /*!
   * Print String to StreamBuffer.
   *
   * \param s Line to print
   */
  inline void Print(const std::string& s)
  {
    WriteString(s, false);
    CheckFlush();
  }

  /*!
   * Print line to StreamBuffer.
   *
   * \param s Line to print
   */
  void Println(const std::string& s);

  /*!
   * \return Bytes remaining (for writing) in this buffer
   */
  inline size_t Remaining() const
  {
    return buffer.Remaining();
  }

  void Reset(std::shared_ptr<tSink> sink)
  {
    sink_lock = sink;
    Reset(sink.get());
  }

  /*!
   * Use buffer with different sink (closes old one)
   *
   * \param sink New Sink to use
   */
  void Reset(tSink* sink_);

  /*!
   * Resets/clears buffer for writing
   */
  void Reset();

  virtual void Reset(tOutputStream* output_stream_buffer, tBufferInfo& buffer_)
  {
    sink->Reset(this, this->buffer);
    buffer_.Assign(this->buffer);
  }

  /*!
   * Set target for last "skip offset" to this position.
   */
  void SkipTargetHere();

  inline void Write(const void* address, size_t size)
  {
    tFixedBuffer fb((char*)address, size);
    Write(fb);
  }

  /*!
   * Writes specified byte buffer contents to stream
   * Regarding streams:
   *   Large buffers are directly copied to output device
   *   avoiding an unnecessary copying operation.
   *
   * \param bb ByteBuffer (whole buffer will be copied)
   */
  inline void Write(const tFixedBuffer& bb)
  {
    Write(bb, 0u, bb.Capacity());
  }

  /*!
   * (Non-virtual variant of above)
   * Writes specified byte buffer contents to stream
   * Regarding streams:
   *   Large buffers are directly copied to output device
   *   avoiding an unnecessary copying operation.
   *
   * \param bb ByteBuffer
   * \param off Offset in buffer
   * \param len Number of bytes to write
   */
  void Write(const tFixedBuffer& bb, size_t off, size_t len);

  virtual bool Write(tOutputStream* output_stream_buffer, tBufferInfo& buffer_, int size_hint);

  /*!
   * Write all available data from input stream to this output stream buffer
   *
   * \param input_stream Input Stream
   */
  void WriteAllAvailable(tInputStream* input_stream);

  template <typename T>
  void WriteNumber(T t)
  {
    EnsureAdditionalCapacity(sizeof(T));

#if __BYTE_ORDER == __ORDER_BIG_ENDIAN
    T tmp = t;
    char* dest = reinterpret_cast<char*>(&t);
    char* src = reinterpret_cast<char*>(&tmp);
    src += sizeof(T);
    for (size_t i = 0; i < sizeof(T); i++)
    {
      src--;
      *dest = *src;
      dest++;
    }
#endif

    buffer.buffer->PutImpl<T>(buffer.position, t);
    buffer.position += sizeof(T);
  }

  /*!
   * \param v (1-byte) boolean
   */
  inline void WriteBoolean(bool v)
  {
    WriteByte(v ? 1 : 0);
  }

  /*!
   * \param v 8 bit integer
   */
  inline void WriteByte(int v)
  {
    WriteNumber<int8_t>(static_cast<int8_t>(v));
  }

  /*!
   * \param v 64 bit floating point
   */
  inline void WriteDouble(double v)
  {
    EnsureAdditionalCapacity(8u);
    buffer.buffer->PutDouble(buffer.position, v);
    buffer.position += 8u;
  }

  /*!
   * \param e Enum constant to serialize
   */
  template <typename ENUM>
  inline void WriteEnum(ENUM e)
  {

#ifndef _LIB_ENUM_STRINGS_PRESENT_
    WriteInt((int)e);
    return;
#endif

    const make_builder::tEnumStrings &enum_strings = make_builder::GetEnumStrings<ENUM>();

    if (enum_strings.size <= 0x100)
    {
      WriteByte((int8_t)e);
    }
    else if (enum_strings.size <= 0x1000)
    {
      WriteShort((int16_t)e);
    }
    else
    {
      assert(enum_strings.size < 0x7FFFFFFF && "What?");
      WriteInt((int)e);
    }
  }

  /*!
   * \param v 32 bit floating point
   */
  inline void WriteFloat(float v)
  {
    EnsureAdditionalCapacity(4u);
    buffer.buffer->PutFloat(buffer.position, v);
    buffer.position += 4u;
  }

  /*!
   * \param v 32 bit integer
   */
  inline void WriteInt(int v)
  {
    WriteNumber(v);
  }

  /*!
   * \param v 64 bit integer
   */
  inline void WriteLong(int64_t v)
  {
    WriteNumber(v);
  }

  /*!
   * \param v 16 bit integer
   */
  inline void WriteShort(int v)
  {
    WriteNumber<int16_t>(static_cast<int16_t>(v));
  }

  /*!
   * A "skip offset" will be written to this position in the stream.
   *
   * (only one such position can be set/remembered at a time)
   *
   * As soon as the stream has reached the position to which are reader might want to skip
   * call setSkipTargetHere()
   */
  void WriteSkipOffsetPlaceholder();

  void WriteString(const char* s)
  {
    Write(const_cast<char*>(s), strlen(s) + 1);
  }

  /*!
   * Write null-terminated string (8 Bit Characters - Suited for ASCII)
   *
   * \param s String
   */
  inline void WriteString(const std::string& s)
  {
    WriteString(s, true);
  }

  /*!
   * Write string (8 Bit Characters - Suited for ASCII)
   *
   * \param s String
   * \param terminate Terminate string with zero?
   */
  void WriteString(const std::string& s, bool terminate);

};


inline tOutputStream& operator<< (tOutputStream& os, char t)
{
  os.WriteNumber(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, int8_t t)
{
  os.WriteNumber(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, int16_t t)
{
  os.WriteNumber(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, int t)
{
  os.WriteNumber(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, long int t)
{
  os.WriteNumber<int64_t>(t); /* for 32/64-bit safety */
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, long long int t)
{
  os.WriteNumber<int64_t>(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, uint8_t t)
{
  os.WriteNumber(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, uint16_t t)
{
  os.WriteNumber(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, uint32_t t)
{
  os.WriteNumber(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, long unsigned int t)
{
  os.WriteNumber<uint64_t>(t); /* for 32/64-bit safety */
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, long long unsigned int t)
{
  os.WriteNumber(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, float t)
{
  os.WriteFloat(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, double t)
{
  os.WriteDouble(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, bool t)
{
  os.WriteBoolean(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, const char* t)
{
  os.WriteString(t);
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, const std::string& t)
{
  os.WriteString(t);
  return os;
}
template <typename R, typename P>
inline tOutputStream& operator<< (tOutputStream& os, const std::chrono::duration<R, P>& t)
{
  std::chrono::nanoseconds ns = t;
  os.WriteLong(ns.count());
  return os;
}
template <typename D>
inline tOutputStream& operator<< (tOutputStream& os, const std::chrono::time_point<std::chrono::system_clock, D>& t)
{
  os << t.time_since_epoch();
  return os;
}
inline tOutputStream& operator<< (tOutputStream& os, const tSerializable& t)
{
  t.Serialize(os);
  return os;
}
template <typename T>
inline tOutputStream& operator<< (typename std::enable_if<std::is_enum<T>::value, tOutputStream&>::type os, const T& t)
{
  tOutputStream& os2 = os;
  os2.WriteEnum<T>(t);
  return os;
}

namespace internal
{
template <int ELEMENT, typename ... TArgs>
struct tTupleSerializer
{
  static void SerializeTuple(tOutputStream& stream, const std::tuple<TArgs...>& tuple)
  {
    tTupleSerializer < ELEMENT - 1, TArgs... >::SerializeTuple(stream, tuple);
    stream << std::get<ELEMENT>(tuple);
  }
};

template <typename ... TArgs>
struct tTupleSerializer < -1, TArgs... >
{
  static void SerializeTuple(tOutputStream& stream, const std::tuple<TArgs...>& tuple)
  {
  }
};
} // namespace internal

template <typename ... TArgs>
inline tOutputStream& operator<< (tOutputStream& stream, const std::tuple<TArgs...>& tuple)
{
  internal::tTupleSerializer < static_cast<int>(std::tuple_size<std::tuple<TArgs...>>::value) - 1, TArgs... >::SerializeTuple(stream, tuple);
  return stream;
}


} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tOutputStream_h__
