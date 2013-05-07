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
/*!\file    rrlib/serialization/tInputStream.h
 *
 * \author  Max Reichardt
 *
 * \date    2011-02-01
 *
 * \brief
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tInputStream_h__
#define __rrlib__serialization__tInputStream_h__

#include "rrlib/time/time.h"
#include "rrlib/serialization/tSerializable.h"
#include "rrlib/serialization/tBufferInfo.h"
#include "rrlib/serialization/tFixedBuffer.h"
#include "rrlib/serialization/tTypeEncoder.h"
#include "rrlib/serialization/tSource.h"
#include "rrlib/serialization/tConstSource.h"
#include <assert.h>
#include <boost/utility.hpp>
#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <endian.h>
#include <chrono>

namespace rrlib
{
namespace rtti
{
class tFactory;
}
namespace serialization
{
class tStringOutputStream;

/*!
 * \author Max Reichardt
 *
 * Reasonably efficient, flexible, universal reading interface.
 * A manager class customizes its behaviour (whether it reads from file, memory block, chunked buffer, etc.)
 * It handles, where the data blocks actually come from.
 */
class tInputStream : public boost::noncopyable, public tSource
{
  friend class tOutputStream;

  // for "locking" object source as long as this buffer exists
  std::shared_ptr<const void> source_lock;

  /*! Buffer that is managed by source */
  tBufferInfo source_buffer;

  /*! Small buffer to enable reading data that crosses buffer boundaries */
  tBufferInfo boundary_buffer;

  /*! Actual boundary buffer backend - symmetric layout: 7 bit old bytes - 7 bit new bytes */
  tFixedBuffer boundary_buffer_backend;

  /*! Current buffer - either sourceBuffer or boundary buffer */
  tBufferInfo* cur_buffer;

  /*! Manager that handles, where the data blocks come from etc. */
  tSource* source;

  /*! Manager that handles, where the data blocks come from etc. */
  const tConstSource* const_source;

  /*! Current absolute buffer read position of buffer start - relevant when using Source; 64bit value, because we might transfer several GB over a stream */
  int64_t absolute_read_pos;

  /*! (Absolute) skip offset target - if one has been read - otherwise -1 */
  int64_t cur_skip_offset_target;

  /*! Has stream/source been closed? */
  bool closed;

  /*! Is direct read support available with this sink? */
  bool direct_read_support;

  /*! timeout for blocking calls (<= 0 when disabled) */
  rrlib::time::tDuration timeout;

  /*! Factory to use for creating objects. */
  rtti::tFactory* factory;

  /*! Data type encoding that is used */
  tTypeEncoding encoding;

  /*! Custom type encoder */
  tTypeEncoder* custom_encoder;

private:

  /*!
   * \return Is current buffer currently set to boundaryBuffer?
   */
  inline bool UsingBoundaryBuffer()
  {
    return cur_buffer->buffer == boundary_buffer.buffer;
  }

protected:

  /*!
   * Ensures that the specified number of bytes is available for reading
   */
  inline void EnsureAvailable(size_t required)
  {
    assert((!closed));
    size_t available = Remaining();
    if (available < required)
    {
      // copy rest to beginning and get next bytes from input
      FetchNextBytes(required - available);
      assert((Remaining() >= required));
      //  throw new RuntimeException("Attempt to read outside of buffer");
    }
  }

  /*!
   * Fills buffer with contents from input stream
   *
   * \param min_required2 Minimum number of bytes to read (block otherwise)
   */
  virtual void FetchNextBytes(size_t min_required2);

public:

  tInputStream(tTypeEncoding encoding_ = tTypeEncoding::LOCAL_UIDS);

  tInputStream(tTypeEncoder& encoder);

  template <typename T>
  tInputStream(T && source, tTypeEncoding encoding = tTypeEncoding::LOCAL_UIDS, decltype(source.DirectReadSupport()) dummy = true) :
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

    Reset(source);
  }

  template <typename T>
  tInputStream(T && source, tTypeEncoder& encoder) :
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

    Reset(source);
  }

  /*!
   * In case of source change: Cleanup
   */
  void Close();

  // Source methods for efficient chaining of buffers

  virtual void Close(tInputStream* buf, tBufferInfo& buffer)
  {
    Close();
  }

  virtual ~tInputStream()
  {
    Close();
  }

  virtual void DirectRead(tInputStream* input_stream_buffer, tFixedBuffer& buffer, size_t offset, size_t len)
  {
    DirectRead(this, buffer, offset, len);
  }

  virtual bool DirectReadSupport() const
  {
    return source != NULL ? source->DirectReadSupport() : const_source->DirectReadSupport();
  }

  /*!
   * \return Number of bytes ever read from this stream
   */
  inline int64_t GetAbsoluteReadPosition()
  {
    return absolute_read_pos + cur_buffer->position;
  }

  /*!
   * \return Custom type encoder
   */
  tTypeEncoder* GetCustomTypeEncoder() const
  {
    return custom_encoder;
  }

  /*!
   * \return Factory to use for creating objects.
   */
  inline rtti::tFactory* GetFactory()
  {
    return factory;
  }

  /*!
   * \return timeout for blocking calls (<= 0 when disabled)
   */
  inline rrlib::time::tDuration GetTimeout()
  {
    return timeout;
  }

  /*!
   * \return Data type encoding that is used
   */
  tTypeEncoding GetTypeEncoding() const
  {
    return encoding;
  }

  virtual bool MoreDataAvailable(tInputStream* input_stream_buffer, tBufferInfo& buffer);

  /*!
   * \return Is further data available?
   */
  bool MoreDataAvailable();

  /*!
   * \return Next byte - without forwarding read position though
   */
  int8_t Peek();

  virtual void Read(tInputStream* buf, tBufferInfo& buffer, size_t len);

  /*!
   * \return boolean value (byte is read from stream and compared against zero)
   */
  inline bool ReadBoolean()
  {
    return ReadByte() != 0;
  }

  /*!
   * \return 8 bit integer
   */
  int8_t ReadByte();

  /*!
   * \return 64 bit floating point
   */
  double ReadDouble();

  /*!
   * \return Enum value
   */
  template <typename ENUM>
  inline ENUM ReadEnum()
  {

#ifndef _LIB_ENUM_STRINGS_PRESENT_
    return static_cast<ENUM>(ReadInt());
#endif

    size_t enum_strings_dimension = make_builder::GetEnumStringsDimension<ENUM>();

    if (enum_strings_dimension <= 0x100)
    {
      return static_cast<ENUM>(ReadByte());
    }

    if (enum_strings_dimension <= 0x1000)
    {
      return static_cast<ENUM>(ReadShort());
    }

    assert(enum_strings_dimension < 0x7FFFFFFF && "What?");
    return static_cast<ENUM>(ReadInt());
  }

  /*!
   * \return 32 bit floating point
   */
  float ReadFloat();

  void ReadFully(void* address, size_t size)
  {
    tFixedBuffer tmp((char*)address, size);
    ReadFully(tmp);
  }

  /*!
   * Fill destination buffer (complete buffer)
   *
   * \param b destination buffer
   */
  inline void ReadFully(tFixedBuffer& bb)
  {
    ReadFully(bb, 0u, bb.Capacity());
  }

  /*!
   * Fill destination buffer
   *
   * \param bb destination buffer
   * \param offset offset in buffer
   * \param len number of bytes to copy
   */
  void ReadFully(tFixedBuffer& bb, size_t off, size_t len);

  /*!
   * \return 32 bit integer
   */
  inline int ReadInt()
  {
    return ReadNumber<int32_t>();
  }

  /*!
   * \return String/Line from stream (ends either at line delimiter or 0-character)
   */
  std::string ReadLine();

  /*!
   * \return 8 byte integer
   */
  inline int64_t ReadLong()
  {
    return ReadNumber<int64_t>();
  }

  /*!
   * \return 2 byte integer
   */
  inline int16_t ReadShort()
  {
    return ReadNumber<int16_t>();
  }

  // stream operator
  template <typename T>
  T ReadNumber()
  {
    EnsureAvailable(sizeof(T));
    T t = cur_buffer->buffer->GetImpl<T>(cur_buffer->position);
    cur_buffer->position += sizeof(T);

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

    return t;
  }

  /*public int read(FixedBuffer buffer, int offset) {
      ensureAvailable(1);
      int size = Math.min(buffer.capacity() - offset, remaining());
      readFully(buffer, offset, size);
      return size;
  }*/

  /*!
   * Read "skip offset" at current position and store it internally
   */
  void ReadSkipOffset();

  /*!
   * Read null-terminated string (8 Bit Characters - Suited for ASCII)
   *
   * \return String
   */
  std::string ReadString();

  /*!
   * Read null-terminated string (8 Bit Characters - Suited for ASCII)
   *
   * \param sb StringOutputStream object to write result to
   */
  void ReadString(tStringOutputStream& sb);

  /*!
   * Read string (8 Bit Characters - Suited for ASCII). Stops at null-termination or specified length.
   *
   * \param length Length of string to read (including possible termination)
   * \return String
   */
  std::string ReadString(size_t length);

  /*!
   * Read string (8 Bit Characters - Suited for ASCII). Stops at null-termination or specified length.
   *
   * \param sb StringBuilder object to write result to
   * \param length Length of string to read (including possible termination)
   */
  void ReadString(tStringOutputStream& sb, size_t length);

  /*!
   * Read null-terminated string (8 Bit Characters - Suited for ASCII)
   *
   * \param sb StringBuilder object to write result to
   */
  template <typename T>
  inline void ReadStringImpl(T& sb)
  {
    while (true)
    {
      int8_t b = ReadByte();
      if (b == 0)
      {
        break;
      }
      sb.Append(static_cast<char>(b));
    }
  }

  /*!
   * \return unsigned 1 byte integer
   */
  inline int ReadUnsignedByte()
  {
    return ReadNumber<uint8_t>();
  }

  /*!
   * \return unsigned 2 byte integer
   */
  inline int ReadUnsignedShort()
  {
    return ReadNumber<uint16_t>();
  }

  /*!
   * \return Remaining size of data in wrapped current intermediate buffer.
   * There might actually be more to read in following buffers though.
   * (Therefore, this is usually pretty useless - some internal implementations use it though)
   */
  inline size_t Remaining()
  {
    return cur_buffer->Remaining();
  }

  /*!
   * Resets buffer for reading - may not be supported by all managers
   */
  void Reset();

  template <typename T>
  void Reset(std::shared_ptr<T> source)
  {
    source_lock = source;
    Reset(source.get());
  }

  /*!
   * Use this object with different source.
   * Current source will be closed.
   *
   * \param source New Source
   */
  void Reset(const tConstSource& source);

  /*!
   * Use this object with different source.
   * Current source will be closed.
   *
   * \param source New Source
   */
  void Reset(tSource& source);

  virtual void Reset(tInputStream* buf, tBufferInfo& buffer)
  {
    Reset();
    buffer.Assign(*cur_buffer);
  }

  /*!
   * When deserializing pointer list, for example, buffers are needed.
   * This factory provides them.
   *
   * It may be reset for more efficient buffer management.
   *
   * \param factory Factory to use for creating objects. (will not be deleted by this class)
   */
  inline void SetFactory(rtti::tFactory* factory)
  {
    this->factory = factory;
  }

  /*!
   * \param timeout for blocking calls (<= 0 when disabled)
   */
  inline void SetTimeout(const rrlib::time::tDuration& timeout)
  {
    this->timeout = timeout;
  }

  /*!
   * Skip specified number of bytes
   */
  void Skip(size_t n);

  /*!
   * Skips null-terminated string (8 Bit Characters)
   */
  void SkipString();

  /*!
   * Move to target of last read skip offset
   */
  void ToSkipTarget();

};


inline tInputStream& operator>> (tInputStream& is, char& t)
{
  t = is.ReadNumber<char>();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, int8_t& t)
{
  t = is.ReadNumber<int8_t>();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, int16_t& t)
{
  t = is.ReadNumber<int16_t>();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, int32_t& t)
{
  t = is.ReadNumber<int32_t>();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, long int& t)
{
  t = static_cast<long int>(is.ReadNumber<int64_t>()); /* for 32/64-bit safety */
  return is;
}
inline tInputStream& operator>> (tInputStream& is, long long int& t)
{
  t = is.ReadNumber<long long int>();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, uint8_t& t)
{
  t = is.ReadNumber<uint8_t>();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, uint16_t& t)
{
  t = is.ReadNumber<uint16_t>();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, uint32_t& t)
{
  t = is.ReadNumber<uint32_t>();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, long unsigned int& t)
{
  t = static_cast<long unsigned int>(is.ReadNumber<uint64_t>()); /* for 32/64-bit safety */
  return is;
}
inline tInputStream& operator>> (tInputStream& is, long long unsigned int& t)
{
  t = is.ReadNumber<long long unsigned int>();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, float& t)
{
  t = is.ReadFloat();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, double& t)
{
  t = is.ReadDouble();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, bool& t)
{
  t = is.ReadBoolean();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, std::vector<bool>::reference t)  // for std::vector<bool> support
{
  t = is.ReadBoolean();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, std::string& t)
{
  t = is.ReadString();
  return is;
}
template <typename R, typename P>
inline tInputStream& operator>> (tInputStream& is, std::chrono::duration<R, P>& t)
{
  t = std::chrono::duration_cast<std::chrono::duration<R, P>>(std::chrono::nanoseconds(is.ReadLong()));
  return is;
}
template <typename D>
inline tInputStream& operator>> (tInputStream& is, std::chrono::time_point<std::chrono::system_clock, D>& t)
{
  D d;
  is >> d;
  t = std::chrono::time_point<std::chrono::system_clock, D>(d);
  return is;
}
inline tInputStream& operator>> (tInputStream& is, tSerializable& t)
{
  t.Deserialize(is);
  return is;
}

template <typename T>
inline tInputStream& operator>> (typename std::enable_if<std::is_enum<T>::value, tInputStream>::type& is, T& t)
{
  tInputStream& is2 = is;
  t = is2.ReadEnum<T>();
  return is;
}

template <typename T1, typename T2>
inline tInputStream& operator>> (tInputStream& is, std::pair<T1, T2>& pair)
{
  is >> pair.first >> pair.second;
  return is;
}

namespace internal
{
template <int ELEMENT, typename ... TArgs>
struct tTupleDeserializer
{
  static void DeserializeTuple(tInputStream& stream, std::tuple<TArgs...>& tuple)
  {
    tTupleDeserializer < ELEMENT - 1, TArgs... >::DeserializeTuple(stream, tuple);
    stream >> std::get<ELEMENT>(tuple);
  }
};

template <typename ... TArgs>
struct tTupleDeserializer < -1, TArgs... >
{
  static void DeserializeTuple(tInputStream& stream, std::tuple<TArgs...>& tuple)
  {
  }
};
} // namespace internal

template <typename ... TArgs>
inline tInputStream& operator>> (tInputStream& stream, std::tuple<TArgs...>& tuple)
{
  internal::tTupleDeserializer < static_cast<int>(std::tuple_size<std::tuple<TArgs...>>::value) - 1, TArgs... >::DeserializeTuple(stream, tuple);
  return stream;
}

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tInputStream_h__
