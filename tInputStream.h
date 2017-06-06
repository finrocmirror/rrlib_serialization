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
/*!\file    rrlib/serialization/tInputStream.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * \brief   Contains tInputStream
 *
 * \b tInputStream
 *
 * Binary input stream.
 *
 * Reads binary data from a source.
 * This can be a file, memory block, network stream etc.
 * The source manages the memory blocks the stream operates on.
 *
 * Implementation is reasonably efficient and flexible.
 *
 * The input stream takes care of endianness for all reads of integral types.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__tInputStream_h__
#define __rrlib__serialization__tInputStream_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/util/tNoncopyable.h"
#include "rrlib/time/time.h"
#include "rrlib/util/tEnumBasedFlags.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tSerializationInfo.h"
#include "rrlib/serialization/tBufferInfo.h"
#include "rrlib/serialization/tConstSource.h"
#include "rrlib/serialization/tSource.h"

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
class tStringOutputStream;
template <typename T>
struct ContainerSerialization;
template <typename T>
class IsBinarySerializable;
template <typename T>
class IsSerializableContainer;
template <typename T>
class IsSerializableMap;
class PublishedRegisters;
class tRegisterUpdate;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Binary input stream
/*!
 * Reads binary data from a source.
 * This can be a file, memory block, network stream etc.
 * The source manages the memory blocks the stream operates on.
 *
 * Implementation is reasonably efficient and flexible.
 *
 * The input stream takes care of endianness for all reads of integral types.
 */
class tInputStream : private util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param source Source to read from
   * \param source_info Info on source that created data currently read (at least the included revision is required for deserialization)
   */
  tInputStream(tSource& source, const tSerializationInfo& source_info = tSerializationInfo()) : tInputStream()
  {
    Reset(source, source_info);
  }

  /*!
   * \param source Source to read from
   * \param source_info Info on source that created data currently read (at least the included revision is required for deserialization)
   */
  tInputStream(const tConstSource& source, const tSerializationInfo& source_info = tSerializationInfo()) : tInputStream()
  {
    Reset(source, source_info);
  }

  /*!
   * \param source Source to read from
   * \param shared_serialization_info_from Serialization info (tSerializationInfo and remote registers) is taken from and shared with this input stream.
   */
  tInputStream(tSource& source, tInputStream& shared_serialization_info_from) : tInputStream()
  {
    Reset(source, shared_serialization_info_from);
  }

  /*!
   * \param source Source to read from
   * \param shared_serialization_info_from Serialization info (tSerializationInfo and remote registers) is taken from and shared with this input stream.
   */
  tInputStream(const tConstSource& source, tInputStream& shared_serialization_info_from) : tInputStream()
  {
    Reset(source, shared_serialization_info_from);
  }

  /*!
   * Note: Reset() with a source needs to be called, before data can be read.
   *
   * \param encoding Data type encoding to use when data types from rrlib::rtti are deserialized (optional)
   */
  tInputStream();


  ~tInputStream()
  {
    Close();
  }

  /*!
   * In case of source change: Cleanup
   */
  void Close();

  /*!
   * \return Number of bytes ever read from this stream
   */
  inline int64_t GetAbsoluteReadPosition()
  {
    return absolute_read_pos + cur_buffer->position;
  }

  /*!
   * \return Info on source that created data currently read (at least the included revision is required for deserialization)
   */
  inline const tSerializationInfo& GetSourceInfo() const
  {
    return shared_serialization_info.serialization_source;
  }

  /*!
   * \return timeout for blocking calls (<= 0 when disabled)
   */
  inline rrlib::time::tDuration GetTimeout()
  {
    return timeout;
  }

  /*!
   * \return Is further data available?
   */
  bool MoreDataAvailable();

  /*!
   * \return Next byte - without forwarding read position though
   */
  int8_t Peek();

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
  ENUM ReadEnum()
  {
    // could be added if ever needed
    /*#ifndef _LIB_ENUM_STRINGS_PRESENT_
        static_assert(sizeof(ENUM) <= 8, "Unexpected size of enum constants");
        if (sizeof(ENUM) <= 4)
        {
          return static_cast<ENUM>(ReadInt());
        }
        else
        {
          return static_cast<ENUM>(ReadLong());
        }
    #endif*/

    const make_builder::internal::tEnumStrings& enum_strings = make_builder::internal::GetEnumStrings<ENUM>();
    size_t enum_strings_dimension = enum_strings.size;
    size_t enum_index = 0;
    if (enum_strings_dimension <= 0x100)
    {
      enum_index = ReadByte();
    }
    else if (enum_strings_dimension <= 0x10000)
    {
      enum_index = ReadShort();
    }
    else
    {
      assert(enum_strings_dimension < 0x7FFFFFFF);
      enum_index = ReadInt();
    }
    if (enum_index > enum_strings.size)
    {
      throw std::runtime_error("Received invalid enum value");
    }
    if (enum_strings.non_standard_values)
    {
      return static_cast<ENUM>(static_cast<const ENUM*>(enum_strings.non_standard_values)[enum_index]);
    }
    return static_cast<ENUM>(enum_index);
  }

  /*!
   * \return 32 bit floating point
   */
  float ReadFloat();

  /*!
   * Fill destination buffer (complete buffer)
   *
   * \param address buffer address
   * \param size buffer length
   */
  void ReadFully(void* address, size_t size)
  {
    tFixedBuffer tmp((char*)address, size);
    ReadFully(tmp);
  }

  /*!
   * Fill destination buffer (complete buffer)
   *
   * \param buffer destination buffer
   */
  inline void ReadFully(tFixedBuffer& buffer)
  {
    ReadFully(buffer, 0u, buffer.Capacity());
  }

  /*!
   * Fill destination buffer
   *
   * \param buffer destination buffer
   * \param offset offset in buffer
   * \param length number of bytes to copy
   */
  void ReadFully(tFixedBuffer& buffer, size_t offset, size_t length);

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

  // stream operator
  template <typename T>
  T ReadNumber()
  {
    EnsureAvailable(sizeof(T));
    T t = cur_buffer->buffer->GetGeneric<T>(cur_buffer->position);
    cur_buffer->position += sizeof(T);

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

    return t;
  }

  /*!
   * \return Remote register entry of specified type. It is safe to keep a reference/pointer to returned object as long as shared deserialization info exists.
   */
  template <typename TRemoteEntry>
  inline const TRemoteEntry& ReadRegisterEntry();

  /*!
   * \return 2 byte integer
   */
  inline int16_t ReadShort()
  {
    return ReadNumber<int16_t>();
  }

  /*!
   * Read "skip offset" at current position and store it internally
   */
  void ReadSkipOffset();

  /*!
   * Read null-terminated string (8 Bit Characters - Suited for ASCII). Stops at null-termination or specified length.
   *
   * \param length Maximum length of string to read (including possible termination character)
   * \return String
   */
  std::string ReadString(size_t max_length = std::string::npos);

  /*!
   * Read null-terminated string (8 Bit Characters - Suited for ASCII). Stops at null-termination or specified length.
   *
   * \param string_stream String buffer to write string to. Is cleared before writing to it.
   * \param length Maximum length of string to read (including possible termination character)
   */
  void ReadString(std::string& string_buffer, size_t max_length = std::string::npos);

  /*!
   * Read null-terminated string (8 Bit Characters - Suited for ASCII). Stops at null-termination or specified length.
   *
   * \param string_stream String stream to write string to. Stream is cleared before writing to it.
   * \param length Maximum length of string to read (including possible termination character)
   */
  void ReadString(std::stringstream& string_stream, size_t max_length = std::string::npos);

  /*!
   * Read string (8 Bit Characters - Suited for ASCII). Stops at null-termination or length BUFFER_SIZE.
   *
   * \tparam BUFFER_SIZE Length of string to read (including possible termination)
   * \param buffer Char array with buffer.
   * \param terminate_if_length_exceeded Terminate string with null character if max_length is exceeded?
   * \return Number of chars read (including null-termination character)
   */
  template <size_t BUFFER_SIZE>
  size_t ReadString(char(&buffer)[BUFFER_SIZE], bool terminate_if_length_exceeded)
  {
    return ReadString(buffer, BUFFER_SIZE, terminate_if_length_exceeded);
  }

  /*!
   * Read string (8 Bit Characters - Suited for ASCII). Stops at null-termination or specified length.
   *
   * \param buffer Char array with buffer.
   * \param max_length Length of string to read (including possible termination)
   * \param terminate_if_length_exceeded Terminate string with null character if max_length is exceeded?
   * \return Number of chars read (including null-termination character)
   */
  size_t ReadString(char* buffer, size_t max_length, bool terminate_if_length_exceeded);

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
   * Resets buffer for reading - may not be supported by all sources
   */
  void Reset();

  /*!
   * Use this object with different source.
   * Current source will be closed.
   *
   * \param source New Source
   * \param source_info Info on source that created data currently read (at least the included revision is required for deserialization)
   */
  void Reset(const tConstSource& source, const tSerializationInfo& source_info = tSerializationInfo())
  {
    ResetSource(source);
    ResetInfo(source_info);
  }
  void Reset(tSource& source, const tSerializationInfo& source_info = tSerializationInfo())
  {
    ResetSource(source);
    ResetInfo(source_info);
  }

  /*!
   * Use this object with different source.
   * Current source will be closed.
   *
   * \param source New Source
   * \param shared_serialization_info_from Serialization info (tSerializationInfo and remote registers) is taken from and shared with this input stream.
   */
  void Reset(const tConstSource& source, tInputStream& shared_serialization_info_from)
  {
    ResetSource(source);
    ResetInfo(shared_serialization_info_from);
  }
  void Reset(tSource& source, tInputStream& shared_serialization_info_from)
  {
    ResetSource(source);
    ResetInfo(shared_serialization_info_from);
  }

  /*!
   * Seek to specified absolute position in the stream
   *
   * If the underlying source does not support seeking,
   * forward seeks will be emulated using Skip() (which may be very slow)
   * and backward seeks throw a std::runtime_error.
   *
   * If seeking fails (e.g. invalid position), the underlying source
   * throws an exception (the exception type is source-specific).
   *
   * \param position the absolute position in the stream
   */
  void Seek(int64_t position);

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

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend class tOutputStream;
  friend class PublishedRegisters;
  friend tInputStream& operator >> (tInputStream& stream, tRegisterUpdate& update);

  /*! Buffer that is managed by source */
  tBufferInfo source_buffer;

  /*! Small buffer to enable reading data that crosses buffer boundaries */
  tBufferInfo boundary_buffer;

  /*! Memory for boundary buffer */
  char boundary_buffer_memory[14];

  /*! Actual boundary buffer backend - symmetric layout: 7 bit old bytes - 7 bit new bytes */
  tFixedBuffer boundary_buffer_backend;

  /*! Current buffer - either sourceBuffer or boundary buffer */
  tBufferInfo* cur_buffer;

  /*! Manager that handles, where the data blocks come from etc. */
  tSource* source;

  /*! Manager that handles, where the data blocks come from etc. */
  const tConstSource* const_source;

  /*!
   * Current absolute buffer read position of buffer start
   * Relevant when using Source; 64bit value, because we might transfer several GB over a stream
   */
  int64_t absolute_read_pos;

  /*! (Absolute) skip offset target - if one has been read - otherwise -1 */
  int64_t cur_skip_offset_target;

  /*! Has stream/source been closed? */
  bool closed;

  /*! Is direct read support available with this sink? */
  bool direct_read_support;

  /*! timeout for blocking calls (<= 0 when disabled) */
  rrlib::time::tDuration timeout;

  struct tRemoteRegister
  {
    virtual ~tRemoteRegister() = default;
  };
  typedef std::array<std::unique_ptr<tRemoteRegister>, cMAX_PUBLISHED_REGISTERS> tRemoteRegisters;

  /*! Serialization possibly shared with substreams */
  struct tSharedSerializationInfo
  {
    /*! Info on source that created data currently read (at least the included revision is required for deserialization) */
    tSerializationInfo serialization_source;

    /*! Replicated remote registers */
    std::shared_ptr<tRemoteRegisters> remote_registers;

  } shared_serialization_info;


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
   * \param min_required Minimum number of bytes to read (depending on source, might block until enough data is available)
   */
  void FetchNextBytes(size_t min_required);

  /*!
   * Reads any register updates from stream
   * (counterpart to tOutputStream::WriteRegisterUpdatesImplementation)
   *
   */
  void ReadRegisterUpdatesImplementation();

  /*!
   * Set shared serialization info
   *
   * \param source_info Info on source that created data currently read (at least the included revision is required for deserialization)
   */
  void ResetInfo(const tSerializationInfo& source_info)
  {
    this->shared_serialization_info.serialization_source = source_info;
    if (this->shared_serialization_info.remote_registers.unique())
    {
      for (auto & entry : (*this->shared_serialization_info.remote_registers))
      {
        entry.reset();
      }
    }
    else if (source_info.HasPublishedRegisters())
    {
      this->shared_serialization_info.remote_registers.reset(new tRemoteRegisters());
    }
  }

  /*!
   * Set shared serialization info
   *
   * \param shared_serialization_info_from Serialization info (tSerializationInfo and remote registers) is taken from and shared with this input stream.
   */
  void ResetInfo(tInputStream& shared_serialization_info_from)
  {
    this->shared_serialization_info = shared_serialization_info_from.shared_serialization_info;
  }

  /*!
   * Use this object with different source.
   * Current source will be closed.
   *
   * \param source New Source
   */
  void ResetSource(tSource& source);
  void ResetSource(const tConstSource& source);

  /*!
   * \return Is current buffer currently set to boundaryBuffer?
   */
  inline bool UsingBoundaryBuffer()
  {
    return cur_buffer->buffer == boundary_buffer.buffer;
  }
};

// stream operators for various standard types

inline tInputStream& operator>> (tInputStream& stream, char& t)
{
  t = stream.ReadNumber<char>();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, int8_t& t)
{
  t = stream.ReadNumber<int8_t>();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, int16_t& t)
{
  t = stream.ReadNumber<int16_t>();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, int& t)
{
  t = stream.ReadNumber<int>();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, long int& t)
{
  t = static_cast<long int>(stream.ReadNumber<int64_t>()); /* for 32/64-bit safety */
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, long long int& t)
{
  t = stream.ReadNumber<long long int>();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, uint8_t& t)
{
  t = stream.ReadNumber<uint8_t>();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, uint16_t& t)
{
  t = stream.ReadNumber<uint16_t>();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, unsigned int& t)
{
  t = stream.ReadNumber<unsigned int>();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, long unsigned int& t)
{
  t = static_cast<long unsigned int>(stream.ReadNumber<uint64_t>()); /* for 32/64-bit safety */
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, long long unsigned int& t)
{
  t = stream.ReadNumber<long long unsigned int>();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, float& t)
{
  t = stream.ReadFloat();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, double& t)
{
  t = stream.ReadDouble();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, bool& t)
{
  t = stream.ReadBoolean();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, typename std::vector<bool>::reference t)  // for std::vector<bool> support
{
  t = stream.ReadBoolean();
  return stream;
}
inline tInputStream& operator>> (tInputStream& stream, std::string& t)
{
  stream.ReadString(t);
  return stream;
}
template <typename R, typename P>
inline tInputStream& operator>> (tInputStream& stream, std::chrono::duration<R, P>& t)
{
  t = std::chrono::duration_cast<std::chrono::duration<R, P>>(std::chrono::nanoseconds(stream.ReadLong()));
  return stream;
}
template <typename D>
inline tInputStream& operator>> (tInputStream& stream, std::chrono::time_point<std::chrono::system_clock, D>& t)
{
  D d;
  stream >> d;
  t = std::chrono::time_point<std::chrono::system_clock, D>(d);
  return stream;
}

template <typename T>
inline tInputStream& operator>> (typename std::enable_if<std::is_enum<T>::value, tInputStream>::type& stream, T& t)
{
  tInputStream& stream_reference = stream;
  t = stream_reference.ReadEnum<T>();
  return stream;
}

template <typename TFlag, typename TStorage>
inline tInputStream& operator>> (tInputStream& stream, util::tEnumBasedFlags<TFlag, TStorage>& flags)
{
  TStorage raw_value;
  stream >> raw_value;
  flags = util::tEnumBasedFlags<TFlag, TStorage>(raw_value);
  return stream;
}

template <typename T1, typename T2>
inline tInputStream& operator>> (tInputStream& stream, std::pair<T1, T2>& pair)
{
  stream >> pair.first >> pair.second;
  return stream;
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

template <typename T, size_t N>
inline tInputStream& operator>> (tInputStream& stream, std::array<T, N>& array)
{
  for (T & element : array)
  {
    stream >> element;
  }
  return stream;
}

template <typename T>
inline typename std::enable_if < IsSerializableContainer<T>::value && (!IsSerializableMap<T>::value) &&
ContainerSerialization<typename IsSerializableContainer<T>::tValue>::cBINARY_SERIALIZABLE, tInputStream >::type&
operator>> (tInputStream& stream, T& t)
{
  static_assert(!std::is_same<std::string, T>::value, "This is not supposed to be used for std::string");
  ContainerSerialization<typename IsSerializableContainer<T>::tValue>::Deserialize(stream, t);
  return stream;
}


template <typename T>
inline typename std::enable_if < IsSerializableMap<T>::value &&
ContainerSerialization<typename IsSerializableContainer<T>::tValue>::cBINARY_SERIALIZABLE, tInputStream >::type&
operator>> (tInputStream& stream, T& map)
{
  ContainerSerialization<typename IsSerializableMap<T>::tMapped>::DeserializeMap(stream, map);
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#endif
