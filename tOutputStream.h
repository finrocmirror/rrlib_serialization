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
/*!\file    rrlib/serialization/tOutputStream.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * \brief   Contains tOutputStream
 *
 * \b tOutputStream
 *
 * Binary output stream.
 *
 * Writes binary data to a sink.
 * This can be a file, memory block, network stream etc.
 * The sink manages the memory blocks the stream operates on.
 *
 * Implementation is reasonably efficient and flexible
 * (no virtual function calls in C++; except of committing/fetching data chunks from sink).
 *
 * The output stream takes care of endianness for all writes of integral types.
 *
 * Size checking is performed for every write operation.
 * For maximum performance, arrays/buffers can be used to write data to stream
 * (endianness, however, needs to be dealt with manually in this case).
 * Buffers can be forwarded to a sink directly (they don't need to be buffered) -
 * avoiding additional copying operations.
 *
 * The class is explicitly _not_ thread-safe for writing - meaning multiple threads may
 * not write to the same object at any given point in time.
 *
 * There are two modes of operation with respect to print-methods:
 *  1) flush immediately
 *  2) flush when requested or full
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tOutputStream_h__
#define __rrlib__serialization__tOutputStream_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/util/tNoncopyable.h"
#include "rrlib/time/time.h"
#include "rrlib/util/tEnumBasedFlags.h"
#include <bitset>
#include <atomic>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tSerializationInfo.h"
#include "rrlib/serialization/tBufferInfo.h"
#include "rrlib/serialization/tSink.h"

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
template <typename T>
struct ContainerSerialization;
template <typename T>
class IsBinarySerializable;
template <typename T>
class IsSerializableContainer;
template <typename T>
class IsSerializableMap;
class tInputStream;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Binary output stream
/*!
 * Writes binary data to a sink.
 * This can be a file, memory block, network stream etc.
 * The sink manages the memory blocks the stream operates on.
 *
 * Implementation is reasonably efficient and flexible
 * (no virtual function calls in C++; except of committing/fetching data chunks from sink).
 *
 * The output stream takes care of endianness for all writes of integral types.
 *
 * Size checking is performed for every write operation.
 * For maximum performance, arrays/buffers can be used to write data to stream
 * (endianness, however, needs to be dealt with manually in this case).
 * Buffers can be forwarded to a sink directly (they don't need to be buffered) -
 * avoiding additional copying operations.
 *
 * The class is explicitly _not_ thread-safe for writing - meaning multiple threads may
 * not write to the same object at any given point in time.
 *
 * There are two modes of operation with respect to print-methods:
 *  1) flush immediately
 *  2) flush when requested or full
 */
class tOutputStream : private util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param sink Sink to write to
   * \param serialization_target Info on target that serialization is created for
   */
  tOutputStream(tSink& sink, const tSerializationInfo& serialization_target = tSerializationInfo()) : tOutputStream()
  {
    Reset(sink, serialization_target);
  }

  /*!
   * \param sink Sink to write to
   * \param shared_serialization_info_from Serialization info (tSerializationInfo and published registers) is taken from and shared with this output stream.
   */
  tOutputStream(tSink& sink, tOutputStream& shared_serialization_info_from) : tOutputStream()
  {
    Reset(sink, shared_serialization_info_from);
  }

  /*!
   * Creates output stream not attached to any sink.
   * Reset() with a sink must be called, before data can be written
   */
  tOutputStream() :
    sink(nullptr),
    immediate_flush(false),
    closed(true),
    buffer(),
    cur_skip_offset_placeholder(-1),
    short_skip_offset(false),
    buffer_copy_fraction(0),
    direct_write_support(false)
  {}

  ~tOutputStream()
  {
    Close();
  }

  /*!
   * Close output stream.
   * Flushes all bytes written to sink.
   */
  void Close();

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
    sink->Flush(*this, buffer);
  }

  /*!
   * \return Position in current internal buffer provided by the Sink.
   * With a tMemoryBuffer sink, this is the actual position in the tMemoryBuffer
   * (and also the size of the data that was written to it).
   * For most other sink types this method is not useful.
   */
  inline size_t GetPosition() const
  {
    return buffer.position - buffer.start;
  }

  /*!
   * \return Info on target that serialization is created for
   */
  inline const tSerializationInfo& GetTargetInfo() const
  {
    return shared_serialization_info.serialization_target;
  }

  /*!
   * \return Whether auto-updated published local registers have more entries than have been transferred (transferring any register entry will cause an update)
   */
  bool IsPublishedRegisterUpdatePending() const
  {
    return shared_serialization_info.published_register_status ? shared_serialization_info.published_register_status->on_register_change_update_counter > shared_serialization_info.published_register_status->counter_on_last_update : false;
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
   * Resets/clears buffer for writing (to same sink)
   */
  void Reset();

  /*!
   * Use buffer with different sink (closes old one)
   *
   * \param sink New Sink to use
   * \param serialization_target Info on target that serialization is created for
   */
  void Reset(tSink& sink, const tSerializationInfo& serialization_target = tSerializationInfo());

  /*!
   * Use buffer with different sink (closes old one)
   *
   * \param sink New Sink to use
   * \param shared_serialization_info_from Serialization info (tSerializationInfo and published registers) is taken from and shared with this output stream.
   */
  void Reset(tSink& sink, tOutputStream& shared_serialization_info_from);

  /*!
   * Seeks the specified position in the current internal buffer provided by the Sink.
   * With a tMemoryBuffer sink, this is the actual position in the tMemoryBuffer.
   * For most other sink types this method is not useful because flushing etc.
   * causes switching to other internal buffers from time to time.
   *
   * Note: If the stream is closed or flushed, the current position is considered as
   * the end of the data written. Thus after seeking backwards, you might want to set
   * the position back to the furthest position before flushing or closing the stream.
   *
   * Note 2: Seeking beyond the internal buffers bounds is not possible. In particular,
   * the size of an underlying tMemoryBuffer will not be automatically increased.
   * Instead, an exception is thrown.
   *
   * \param position Position to seek
   * \exception std::invalid_argument is thrown if an invalid position is specified
   */
  void Seek(size_t position);

  /*!
   * Set target for last "skip offset" to this position.
   */
  void SkipTargetHere();

  /*!
   * Writes specified byte buffer contents to stream
   * Regarding streams:
   *   Large buffers are directly copied to output device
   *   avoiding an unnecessary copying operation.
   *
   * \param address Address of data to write
   * \param size Length (in bytes) of data to write
   */
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

  /*!
   * Write all available data from input stream to this output stream buffer
   *
   * \param input_stream Input Stream
   */
  void WriteAllAvailable(tInputStream& input_stream);

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
  void WriteEnum(ENUM e)
  {
    // could be added if ever needed
    /*#ifndef _LIB_ENUM_STRINGS_PRESENT_
        static_assert(sizeof(ENUM) <= 8, "Unexpected size of enum constants");
        if (sizeof(ENUM) <= 4)
        {
          WriteInt((int)e);
        }
        else
        {
          WriteLong((int64_t)e);
        }
    #endif*/

    const make_builder::internal::tEnumStrings& enum_strings = make_builder::internal::GetEnumStrings<ENUM>();
    size_t enum_strings_dimension = enum_strings.size;
    size_t enum_index = static_cast<size_t>(e);
    if (enum_strings.non_standard_values)
    {
      bool found = false;
      for (size_t i = 0; i < enum_strings_dimension; i++)
      {
        if (e == static_cast<const ENUM*>(enum_strings.non_standard_values)[i])
        {
          enum_index = i;
          found = true;
          break;
        }
      }
      if (!found)
      {
        throw std::runtime_error("Invalid enum value");
      }
    }

    if (enum_strings_dimension <= 0x100)
    {
      WriteByte((int8_t)enum_index);
    }
    else if (enum_strings_dimension <= 0x10000)
    {
      WriteShort((int16_t)enum_index);
    }
    else
    {
      assert(enum_strings_dimension < 0x7FFFFFFF);
      WriteInt((int)enum_index);
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
   * Write integer to stream - taking care of endianness
   *
   * \param t Integer to write to stream
   */
  template <typename T>
  void WriteNumber(T t)
  {
    EnsureAdditionalCapacity(sizeof(T));

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
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

    buffer.buffer->PutGeneric<T>(buffer.position, t);
    buffer.position += sizeof(T);
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
   * As soon as the stream has reached the position to which a reader might want to skip
   * call SkipTargetHere()
   *
   * (Is equivalent to writing the size of the data until SkipTargetHere() to stream)
   *
   * \param short_skip_offset If skip offset will be smaller than 256, can be set to true, to make stream 3 bytes shorter
   */
  void WriteSkipOffsetPlaceholder(bool short_skip_offset = false);

  void WriteString(const char* s)
  {
    Write(s, strlen(s) + 1);
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

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Committed buffers are buffered/copied (not forwarded directly), when smaller than 1/(2^n) of buffer capacity */
  static const double cBUFFER_COPY_FRACTION;

  /*! Source that determines where buffers that are written to come from and how they are handled */
  rrlib::serialization::tSink* sink;

  /*! Immediately flush buffer after printing? */
  const bool immediate_flush;

  /*! Has stream been closed? */
  bool closed;

  /*! Buffer that is currently written to - is managed by sink */
  tBufferInfo buffer;

  /*! -1 by default - buffer position when a skip offset placeholder has been set/written */
  int64_t cur_skip_offset_placeholder;

  /*! if true, indicates that only 1 byte has been reserved for skip offset placeholder */
  bool short_skip_offset;

  /*! Whole Buffers are only buffered/copied when they are smaller than this */
  size_t buffer_copy_fraction;

  /*! Is direct write support available with this sink? */
  bool direct_write_support;


  template <typename TEntry, size_t Tchunk_count, size_t Tchunk_size, typename THandle, typename TMutex>
  friend class tRegister;

  /*! Info on a single published registers */
  struct tPublishedRegisterStatus : util::tNoncopyable
  {
    /*! Contains number of entries from register that were already written to stream */
    std::array<uint, cMAX_PUBLISHED_REGISTERS> elements_written;

    /*! Counts updates to registers that are update on change (for efficient change detection) */
    std::atomic<uint> on_register_change_update_counter;

    /*! Counter value when registers updated on change were last updated (for efficient change detection) */
    uint counter_on_last_update;

    /*! Where this object registered as listener */
    std::bitset<cMAX_PUBLISHED_REGISTERS> registered_listeners;

    tPublishedRegisterStatus() :
      on_register_change_update_counter(0),
      counter_on_last_update(0)
    {
      elements_written.fill(0);
    }

    void OnRegisterUpdate()
    {
      on_register_change_update_counter++;
    }

    void Reset();

    ~tPublishedRegisterStatus()
    {
      Reset();
    }
  };

  /*! Serialization possibly shared with substreams */
  struct tSharedSerializationInfo
  {
    /*! Info on target that serialization is created for */
    tSerializationInfo serialization_target;

    /*! Info on published registers */
    std::shared_ptr<tPublishedRegisterStatus> published_register_status;

  } shared_serialization_info;


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

  /*!
   * \return Bytes remaining (for writing) in current internal buffer provided by the Sink
   */
  inline size_t Remaining() const
  {
    return buffer.Remaining();
  }

  /*!
   * Writes any required updates to stream
   *
   * \param register_uid UID of register whose entry is to be serialized after call
   * \param entry_handle Handle of entry to be serialized after call
   * \param handle_size Size of register's handle (in bytes)
   */
  void WriteRegisterUpdates(uint register_uid, uint entry_handle, size_t handle_size)
  {
    size_t current_counter = shared_serialization_info.published_register_status->on_register_change_update_counter.load();
    if (entry_handle >= shared_serialization_info.published_register_status->elements_written[register_uid] || current_counter > shared_serialization_info.published_register_status->counter_on_last_update)
    {
      shared_serialization_info.published_register_status->counter_on_last_update = current_counter;
      WriteRegisterUpdatesImplementation(register_uid, entry_handle, handle_size);
    }
  }

  /*!
   * Sends updates on all published registers on-change and possibly the register triggering this call
   *
   * \param register_uid UID of register whose entry is to be serialized after call
   * \param entry_handle Handle of entry to be serialized after call
   * \param handle_size Size of register's handle (in bytes)
   */
  void WriteRegisterUpdatesImplementation(uint register_uid, uint entry_handle, size_t handle_size);
};

// stream operators for various standard types

inline tOutputStream& operator<< (tOutputStream& stream, char t)
{
  stream.WriteNumber(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, int8_t t)
{
  stream.WriteNumber(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, int16_t t)
{
  stream.WriteNumber(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, int t)
{
  stream.WriteNumber(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, long int t)
{
  stream.WriteNumber<int64_t>(t); /* for 32/64-bit safety */
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, long long int t)
{
  stream.WriteNumber<int64_t>(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, uint8_t t)
{
  stream.WriteNumber(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, uint16_t t)
{
  stream.WriteNumber(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, unsigned int t)
{
  stream.WriteNumber(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, long unsigned int t)
{
  stream.WriteNumber<uint64_t>(t); /* for 32/64-bit safety */
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, long long unsigned int t)
{
  stream.WriteNumber(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, float t)
{
  stream.WriteFloat(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, double t)
{
  stream.WriteDouble(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, bool t)
{
  stream.WriteBoolean(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, const char* t)
{
  stream.WriteString(t);
  return stream;
}
inline tOutputStream& operator<< (tOutputStream& stream, const std::string& t)
{
  stream.WriteString(t);
  return stream;
}
template <typename R, typename P>
inline tOutputStream& operator<< (tOutputStream& stream, const std::chrono::duration<R, P>& t)
{
  std::chrono::nanoseconds ns = t;
  stream.WriteLong(ns.count());
  return stream;
}
template <typename D>
inline tOutputStream& operator<< (tOutputStream& stream, const std::chrono::time_point<std::chrono::system_clock, D>& t)
{
  stream << t.time_since_epoch();
  return stream;
}
template <typename T>
inline tOutputStream& operator<< (typename std::enable_if<std::is_enum<T>::value, tOutputStream&>::type stream, const T& t)
{
  tOutputStream& stream_reference = stream;
  stream_reference.WriteEnum<T>(t);
  return stream;
}

template <typename TFlag, typename TStorage>
inline tOutputStream& operator<< (tOutputStream& stream, const util::tEnumBasedFlags<TFlag, TStorage>& flags)
{
  stream << flags.Raw();
  return stream;
}

template <typename T1, typename T2>
inline tOutputStream& operator<< (tOutputStream& stream, const std::pair<T1, T2>& pair)
{
  stream << pair.first << pair.second;
  return stream;
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

template <typename T, size_t N>
inline tOutputStream& operator<< (tOutputStream& stream, const std::array<T, N>& array)
{
  for (const T & element : array)
  {
    stream << element;
  }
  return stream;
}

template <typename T>
inline typename std::enable_if < IsSerializableContainer<T>::value && (!IsSerializableMap<T>::value) &&
ContainerSerialization<typename IsSerializableContainer<T>::tValue>::cBINARY_SERIALIZABLE, tOutputStream >::type&
operator<< (tOutputStream& stream, const T& t)
{
  static_assert(!std::is_same<std::string, T>::value, "This is not supposed to be used for std::string");
  ContainerSerialization<typename IsSerializableContainer<T>::tValue>::Serialize(stream, t);
  return stream;
}

template <typename T>
inline typename std::enable_if < IsSerializableMap<T>::value &&
ContainerSerialization<typename IsSerializableContainer<T>::tValue>::cBINARY_SERIALIZABLE, tOutputStream >::type&
operator<< (tOutputStream& stream, const T& map)
{
  ContainerSerialization<typename IsSerializableMap<T>::tMapped>::SerializeMap(stream, map);
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#endif
