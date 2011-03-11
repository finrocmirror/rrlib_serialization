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

#ifndef __rrlib__serialization__tInputStream_h__
#define __rrlib__serialization__tInputStream_h__

#include "rrlib/serialization/tSerializable.h"
#include "rrlib/serialization/tBufferInfo.h"
#include "rrlib/serialization/tFixedBuffer.h"
#include "rrlib/serialization/tDefaultFactory.h"
#include "rrlib/serialization/tSource.h"
#include "rrlib/serialization/tConstSource.h"
#include "rrlib/serialization/tDataTypeBase.h"
#include <assert.h>
#include <boost/utility.hpp>
#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <string>

#include "rrlib/serialization/detail/tListElemInfo.h"
#include <vector>
#include <list>
#include <deque>
#include <endian.h>

namespace rrlib
{
namespace serialization
{
class tFactory;
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
public:

  enum tTypeEncoding { eLocalUids, eNames, eSubClass };

  friend class tOutputStream;

protected:

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
  ::rrlib::serialization::tSource* source;

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
  int timeout;

  /*! Default Factory to use for creating objects. */
  tDefaultFactory default_factory;

  /*! Factory to use for creating objects. */
  tFactory* factory;

  /*! Data type encoding that is used */
  tInputStream::tTypeEncoding encoding;

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

  /*!
   * May be overridden by subclass to realize custom type serialization
   *
   * \return Data type that was read
   */
  virtual tDataTypeBase ReadTypeSpecialization();

public:

  tInputStream(tInputStream::tTypeEncoding encoding_ = eLocalUids);

  template <typename T>
  tInputStream(T source_, tInputStream::tTypeEncoding encoding_ = eLocalUids) :
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
      timeout(-1),
      default_factory(),
      factory(&(default_factory)),
      encoding()
  {
    boundary_buffer.buffer = &(boundary_buffer_backend);

    Reset(source_);
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
   * \return Factory to use for creating objects.
   */
  inline tFactory* GetFactory()
  {
    return factory;
  }

  /*!
   * \return timeout for blocking calls (<= 0 when disabled)
   */
  inline int GetTimeout()
  {
    return timeout;
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
   * \return 32 bit floating point
   */
  float ReadFloat();

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

  // Deserialize STL container (must either have pass-by-value type or shared pointers)
  template <typename C, typename T>
  void ReadSTLContainer(C& container)
  {
    typedef detail::tListElemInfo<T> info;

    size_t size = ReadInt();
    bool const_type = ReadBoolean();
    if (const_type == info::is_shared_ptr)
    {
      throw std::runtime_error("Wrong list type");
    }
    container.resize(size);
    typename C::iterator it;
    for (it = container.begin(); it != container.end(); it++)
    {
      if (!const_type)
      {
        tDataTypeBase needed = ReadType();
        tDataTypeBase current = info::GetType(*it);
        if (needed != current)
        {
          if (needed == NULL)
          {
            info::Reset(*it);
          }
          else
          {
            info::ChangeBufferType(factory, *it, needed);
          }
        }
        if (needed != info::GetTypeT())
        {
          needed.Deserialize(*this, &info::GetElem(*it));
          continue;
        }
      }
      *this >> info::GetElem(*it);
    }
  }

  /*!
   * \return Reads type from stream
   */
  virtual tDataTypeBase ReadType();

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
  void Reset(const tConstSource* source_);

  /*!
   * Use this object with different source.
   * Current source will be closed.
   *
   * \param source New Source
   */
  void Reset(tSource* source_);

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
   * \param factory Factory to use for creating objects.
   */
  inline void SetFactory(tFactory* factory_)
  {
    this->factory = factory_;
  }

  /*!
   * \param timeout for blocking calls (<= 0 when disabled)
   */
  inline void SetTimeout(int timeout_)
  {
    this->timeout = timeout_;
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

} // namespace rrlib
} // namespace serialization

namespace rrlib
{
namespace serialization
{
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
inline tInputStream& operator>> (tInputStream& is, std::string& t)
{
  t = is.ReadString();
  return is;
}
inline tInputStream& operator>> (tInputStream& is, tSerializable& t)
{
  t.Deserialize(is);
  return is;
}
template <typename T>
inline tInputStream& operator>> (tInputStream& is, std::vector<T>& t)
{
  is.ReadSTLContainer<std::vector<T>, T>(t);
  return is;
}
template <typename T>
inline tInputStream& operator>> (tInputStream& is, std::list<T>& t)
{
  is.ReadSTLContainer<std::list<T>, T>(t);
  return is;
}
template <typename T>
inline tInputStream& operator>> (tInputStream& is, std::deque<T>& t)
{
  is.ReadSTLContainer<std::deque<T>, T>(t);
  return is;
}

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tInputStream_h__
