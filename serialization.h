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
#ifndef __rrlib__serialization__serialization_h__
#define __rrlib__serialization__serialization_h__

#include "rrlib/xml/tDocument.h"

#include "rrlib/serialization/tSerializable.h"
#include "rrlib/serialization/tMemoryBuffer.h"
#include "rrlib/serialization/tOutputStream.h"
#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/tStringOutputStream.h"
#include "rrlib/serialization/tStringInputStream.h"
#include "rrlib/serialization/detail/utility.h"
#include <boost/utility.hpp>
#include <string>
#include <vector>

namespace rrlib
{
namespace rtti
{
class tFactory;
class tGenericObject;
}
namespace serialization
{
class tStringOutputStream;
class tStringInputStream;

/*!
 * In SerializationBasedDeepCopy and SerializationEquals memory buffers are created on the stack.
 * This specifies the size, that they should have (all together).
 */
const int cSTACK_BUFFERS_SIZE = 65536;

/*!
 * Converts binary to hex string
 *
 * \param src Input stream that contains binary data
 * \param co Output stream to write hex string to
 */
void ConvertBinaryToHexString(tInputStream& src, tStringOutputStream& os);

/*!
 * Converts hex string from StringInputStream to binary
 *
 * \param src Input stream that contains hex string
 * \param co Output stream to write binary data to
 */
void ConvertHexStringToBinary(tStringInputStream& src, tOutputStream& co);

/*!
 * Deserializes string stream serializable from string
 * (convenience function)
 *
 * \param s String to deserialize from
 * \return String
 */
template <typename T>
T Deserialize(const std::string& s)
{
  tStringInputStream is(s);
  T t;
  is >> t;
  return t;
}

/*!
 * Deserialize objects of type T from specified stream.
 * Unlike the stream-operator, calling this method with an unserializable type T
 * won't cause a compilation error.
 * Instead, an error message will be printed when it is actually called with such an object at runtime.
 */
template <typename T>
void Deserialize(tInputStream& stream, T& t)
{
  detail::tBinarySerialization<T>::Deserialize(t, stream);
}
template <typename T>
void Deserialize(tStringInputStream& stream, T& t)
{
  detail::tStringSerialization<T>::Deserialize(t, stream);
}
template <typename T>
void Deserialize(const xml::tNode& node, T& t)
{
  detail::tXMLSerialization<T>::Deserialize(t, node);
}

/*!
 * Deserialize data from binary input stream - possibly using non-binary encoding.
 * If selected type of serialization is not supported, an error will be printed at runtime.
 *
 * \param is Binary input stream
 * \param s Object to deserialize
 * \param enc Encoding to use
 */
template <typename T>
void Deserialize(tInputStream& stream, T& t, tDataEncoding enc)
{
  if (enc == tDataEncoding::BINARY)
  {
    Deserialize(stream, t);
  }
  else if (enc == tDataEncoding::STRING)
  {
    tStringInputStream sis(stream.ReadString());
    try
    {
      Deserialize(sis, t);
    }
    catch (const std::exception& e)
    {
      RRLIB_LOG_PRINT(ERROR, e);
    }
  }
  else
  {
    try
    {
      std::string s = stream.ReadString();
      xml::tDocument d(s.c_str(), s.length(), false);
      xml::tNode& n = d.RootNode();
      Deserialize(n, t);
    }
    catch (const std::exception& e)
    {
      RRLIB_LOG_PRINT(ERROR, e);
    }
  }
}

/*!
 * Deserializes binary CoreSerializable from hex string
 *
 * \param cs serializable
 * \param s Hex String to deserialize from
 */
void DeserializeFromHexString(tSerializable& cs, tStringInputStream& s);

/*!
 * Creates deep copy of serializable object using serialization to and from memory buffer
 *
 * \param src Object to be copied
 * \param dest Object to copy to
 * \param f Factory for creation of new objects (e.g. in pointer vectors)
 */
template <typename T>
void SerializationBasedDeepCopy(const T& src, T& dest, rtti::tFactory* f = NULL)
{
  tStackMemoryBuffer<cSTACK_BUFFERS_SIZE> buf;
  SerializationBasedDeepCopy(src, dest, buf, f);
}

/*!
 * Creates deep copy of serializable object using serialization to and from specified memory buffer
 *
 * \param src Object to be copied
 * \param dest Object to copy to
 * \param f Factory for creation of new objects (e.g. in pointer vectors)
 * \param buf Memory buffer to use
 */
template <typename T>
void SerializationBasedDeepCopy(const T& src, T& dest, tMemoryBuffer& buf, rtti::tFactory* f = NULL)
{
  buf.Clear();
  tOutputStream os(buf);

  os << src;

  os.Close();
  tInputStream ci(buf);
  ci.SetFactory(f);

  ci >> dest;

  ci.Close();
}

/*!
 * Serialization-based Equals()-method
 * (not very efficient/RT-capable - should therefore not be called in regular loops)
 * (types are not checked)
 *
 * \param o1 First object
 * \param o2 Second object
 *
 * \return True, if both objects are serialized to the same binary data (usually they are equal then)
 */
template <typename T>
bool SerializationEquals(const T& o1, const T& o2)
{
  tStackMemoryBuffer < cSTACK_BUFFERS_SIZE / 2 > buf1;
  tStackMemoryBuffer < cSTACK_BUFFERS_SIZE / 2 > buf2;
  tOutputStream os1(buf1);
  tOutputStream os2(buf2);
  os1 << o1;
  os2 << o2;
  os1.Close();
  os2.Close();

  return buf1.Equals(buf2);
}

/*!
 * Serializes string stream serializable object to string
 * (convenience function)
 *
 * \param cs Serializable
 * \return String
 */
std::string Serialize(const tSerializable& rs);

/*!
 * Serializes string stream serializable object to string
 * (convenience function)
 *
 * \param cs Serializable
 * \return String
 */
template < typename T, bool ENABLE = (!std::is_base_of<tSerializable, T>::value) && (!std::is_base_of<rtti::tGenericObject, T>::value) >
typename std::enable_if<ENABLE, std::string>::type Serialize(const T& t)
{
  tStringOutputStream os;
  os << t;
  return os.ToString();
}

/*!
 * Serialize objects of type T to specified stream.
 * Unlike the stream-operator, calling this method with an unserializable type T
 * won't cause a compilation error.
 * Instead, an error message will be printed when it is actually called with such an object at runtime.
 */
template <typename T>
void Serialize(tOutputStream& stream, const T& t)
{
  detail::tBinarySerialization<T>::Serialize(t, stream);
}
template <typename T>
void Serialize(tStringOutputStream& stream, const T& t)
{
  detail::tStringSerialization<T>::Serialize(t, stream);
}
template <typename T>
void Serialize(xml::tNode& node, const T& t)
{
  detail::tXMLSerialization<T>::Serialize(t, node);
}

/**
 * Serialize data to binary output stream - possibly using non-binary encoding.
 * If selected type of serialization is not supported, an error will be printed at runtime.
 *
 * \param os Binary output stream
 * \param s Object to serialize
 * \param enc Encoding to use
 */
template <typename T>
void Serialize(tOutputStream& stream, const T& t, tDataEncoding enc)
{
  if (enc == tDataEncoding::BINARY)
  {
    Serialize(stream, t);
  }
  else if (enc == tDataEncoding::STRING)
  {
    tStringOutputStream sos;
    Serialize(sos, t);
    stream.WriteString(sos.ToString());
  }
  else
  {
    xml::tDocument d;
    try
    {
      xml::tNode& n = d.AddRootNode("value");
      Serialize(n, t);
      stream.WriteString(n.GetXMLDump(true));
    }
    catch (const std::exception& e)
    {
      RRLIB_LOG_PRINT(ERROR, "Error generating XML code: ", e);
    }
  }
}

/*!
 * Serializes binary serializable to hex string
 *
 * \param cs Serializable
 * \param os String output stream
 */
void SerializeToHexString(const tSerializable& cs, tStringOutputStream& os);

} // namespace serialization

namespace xml
{

inline const xml::tNode& operator>> (const xml::tNode& node, std::string& s)
{
  s = node.GetTextContent();
  return node;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}

#include "rrlib/serialization/tStringOutputStream.h"
#include "rrlib/serialization/tStringInputStream.h"

#endif // __rrlib__serialization__serialization_h__
