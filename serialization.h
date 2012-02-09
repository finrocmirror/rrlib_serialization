/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2010-2012 Max Reichardt,
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

#ifndef __rrlib__serialization__serialization_h__
#define __rrlib__serialization__serialization_h__

#include "rrlib/serialization/tSerializable.h"
#include "rrlib/serialization/tMemoryBuffer.h"
#include "rrlib/serialization/tOutputStream.h"
#include "rrlib/serialization/tInputStream.h"
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
 * Deserializes binary CoreSerializable from hex string
 *
 * \param cs CoreSerializable
 * \param s Hex String to deserialize from
 */
void DeserializeFromHexString(tSerializable* cs, tStringInputStream& s);

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
  tOutputStream os(&(buf));

  os << src;

  os.Close();
  tInputStream ci(&(buf));
  ci.SetFactory(f);

  ci >> dest;

  ci.Close();
}

/*!
 * Serialization-based Equals()-method
 * (not very efficient/RT-capable - should therefore not be called regular loops)
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
  tOutputStream os1(&buf1);
  tOutputStream os2(&buf2);
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
 * Helper for below
 */
std::string ToString(tStringOutputStream& sos);

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
  return ToString(os);
}

/*!
 * Serializes binary CoreSerializable to hex string
 *
 * \param cs CoreSerializable
 * \param os String output stream
 */
void SerializeToHexString(const tSerializable* cs, tStringOutputStream& os);

} // namespace rrlib
} // namespace serialization

inline const rrlib::xml2::tXMLNode& operator>> (const rrlib::xml2::tXMLNode& node, std::string& s)
{
  s = node.GetTextContent();
  return node;
}

#include "rrlib/serialization/tStringOutputStream.h"
#include "rrlib/serialization/tStringInputStream.h"

#endif // __rrlib__serialization__serialization_h__
