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
/*!\file    rrlib/serialization/serialization.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-17
 *
 * Various utility functions related to (de)serialization.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__serialization_h__
#define __rrlib__serialization__serialization_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#ifdef _LIB_RRLIB_XML_PRESENT_
#include "rrlib/xml/tDocument.h"
#endif

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/tOutputStream.h"
#include "rrlib/serialization/tStackMemoryBuffer.h"
#include "rrlib/serialization/tStringInputStream.h"
#include "rrlib/serialization/tStringOutputStream.h"
#include "rrlib/serialization/detail/utility.h"
#include "rrlib/serialization/ContainerSerialization.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace rrlib
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------
namespace rtti
{
class tGenericObject;
}

namespace serialization
{

//----------------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------------

/*!
 * In SerializationBasedDeepCopy and SerializationEquals memory buffers are created on the stack.
 * This specifies the size, that they should have (all together).
 */
const int cSTACK_BUFFERS_SIZE = 65536;

/*!
 * Converts binary to hex string
 *
 * \param binary_input_stream Input stream that contains binary data
 * \param string_output_stream Output stream to write hex string to
 */
void ConvertBinaryToHexString(tInputStream& binary_input_stream, tStringOutputStream& string_output_stream);

/*!
 * Converts hex string from StringInputStream to binary
 *
 * \param string_input_stream Input stream that contains hex string
 * \param binary_output_stream Output stream to write binary data to
 */
void ConvertHexStringToBinary(tStringInputStream& string_input_stream, tOutputStream& binary_output_stream);

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
#ifdef _LIB_RRLIB_XML_PRESENT_
template <typename T>
void Deserialize(const xml::tNode& node, T& t)
{
  detail::tXMLSerialization<T>::Deserialize(t, node);
}
#endif

/*!
 * Deserialize data from binary input stream - possibly using non-binary encoding.
 * If selected type of serialization is not supported, an error will be printed at runtime.
 *
 * \param stream Binary input stream
 * \param t Object to deserialize
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
    Deserialize(sis, t);
  }
  else
  {
#ifdef _LIB_RRLIB_XML_PRESENT_
    std::string s = stream.ReadString();
    xml::tDocument d(s.c_str(), s.length(), false);
    xml::tNode& n = d.RootNode();
    Deserialize(n, t);
#else
    throw std::invalid_argument("XML support not available");
#endif
  }
}

/*!
 * Creates deep copy of serializable object using serialization to and from memory buffer
 *
 * \param src Object to be copied
 * \param dest Object to copy to
 */
template <typename T>
void SerializationBasedDeepCopy(const T& src, T& dest)
{
  tStackMemoryBuffer<cSTACK_BUFFERS_SIZE> buf;
  SerializationBasedDeepCopy(src, dest, buf);
}

/*!
 * Creates deep copy of serializable object using serialization to and from specified memory buffer
 *
 * \param src Object to be copied
 * \param dest Object to copy to
 * \param buf Memory buffer to use
 */
template <typename T>
void SerializationBasedDeepCopy(const T& src, T& dest, tMemoryBuffer& buf)
{
  buf.Clear();
  tOutputStream os(buf);

  os << src;

  os.Close();
  tInputStream ci(buf);

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
 * \param t Object to serialize
 * \return String
 */
template < typename T, bool ENABLE = (!std::is_base_of<rtti::tGenericObject, T>::value) >
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
#ifdef _LIB_RRLIB_XML_PRESENT_
template <typename T>
void Serialize(xml::tNode& node, const T& t)
{
  detail::tXMLSerialization<T>::Serialize(t, node);
}
#endif

/**
 * Serialize data to binary output stream - possibly using non-binary encoding.
 * If selected type of serialization is not supported, an error will be printed at runtime.
 *
 * \param stream Binary output stream
 * \param t Object to serialize
 * \param enc Encoding to use
 */
template <typename T>
void Serialize(tOutputStream& stream, const T& t, tDataEncoding enc)
{
  static_assert(!std::is_same<T, rrlib::rtti::tGenericObject>::value, "Use Serialize method from tGenericObject.");
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
#ifdef _LIB_RRLIB_XML_PRESENT_
    xml::tDocument d;
    xml::tNode& n = d.AddRootNode("value");
    Serialize(n, t);
    stream.WriteString(n.GetXMLDump(true));
#else
    throw std::invalid_argument("XML support not available");
#endif
  }
}

#ifdef _LIB_RRLIB_XML_PRESENT_

namespace internal
{
template <int ELEMENT, typename ... TArgs>
struct tTupleXmlSerializer
{
  static void SerializeTuple(xml::tNode &node, const std::tuple<TArgs...>& tuple)
  {
    tTupleXmlSerializer < ELEMENT - 1, TArgs... >::SerializeTuple(node, tuple);
    rrlib::xml::tNode &child = node.AddChildNode("tuple_element");
    child << std::get<ELEMENT>(tuple);
  }
};

template <typename ... TArgs>
struct tTupleXmlSerializer < -1, TArgs... >
{
  static void SerializeTuple(const xml::tNode &node, const std::tuple<TArgs...>& tuple)
  {
  }
};

template <int ELEMENT, typename ... TArgs>
struct tTupleXmlDeserializer
{
  static void DeserializeTuple(const xml::tNode &node, std::tuple<TArgs...>& tuple)
  {
    node >> std::get < static_cast<int>(std::tuple_size<std::tuple<TArgs...>>::value) - ELEMENT - 1 > (tuple);
    if (!node.HasNextSibling())
    {
      RRLIB_LOG_PRINTF(ERROR, "Not enough XML siblings to de-serialize std::tuple of size %d completely! There are %d elements left to be de-serialized.\n",
                       static_cast<int>(std::tuple_size<std::tuple<TArgs...>>::value), ELEMENT);
      throw std::exception();
    }
    tTupleXmlDeserializer < ELEMENT - 1, TArgs... >::DeserializeTuple(node.NextSibling(), tuple);
  }
};

template <typename ... TArgs>
struct tTupleXmlDeserializer <0, TArgs...>
{
  static void DeserializeTuple(const xml::tNode &node, std::tuple<TArgs...>& tuple)
  {
    node >> std::get < static_cast<int>(std::tuple_size<std::tuple<TArgs...>>::value) - 1 > (tuple);
  }
};

} // namespace internal

} // namespace serialization

namespace xml
{

inline const xml::tNode& operator>> (const xml::tNode& node, std::string& s)
{
  s = node.GetTextContent();
  return node;
}

inline const xml::tNode& operator>> (const xml::tNode& node, typename std::vector<bool>::reference bool_reference)
{
  serialization::tStringInputStream stream(node.GetTextContent());
  stream >> bool_reference;
  return node;
}

template <typename ... TArgs>
inline xml::tNode& operator<< (xml::tNode &node, const std::tuple<TArgs...> &tuple)
{
  serialization::internal::tTupleXmlSerializer < static_cast<int>(std::tuple_size<std::tuple<TArgs...>>::value) - 1, TArgs... >::SerializeTuple(node, tuple);
  return node;
}

template <typename ... TArgs>
inline const xml::tNode& operator>> (const xml::tNode &node, std::tuple<TArgs...> &tuple)
{
  serialization::internal::tTupleXmlDeserializer < static_cast<int>(std::tuple_size<std::tuple<TArgs...>>::value) - 1, TArgs... >::DeserializeTuple(node.FirstChild(), tuple);
  return node;
}

template <typename T>
inline typename std::enable_if < serialization::IsSerializableContainer<T>::value && (!serialization::IsSerializableMap<T>::value) &&
(!std::is_same<T, std::string>::value) &&
serialization::ContainerSerialization<typename serialization::IsSerializableContainer<T>::tValue>::cXML_SERIALIZABLE, tNode >::type&
operator<< (tNode& node, const T& t)
{
  static_assert(!std::is_same<std::string, T>::value, "This is not supposed to be used for std::string");
  serialization::ContainerSerialization<typename serialization::IsSerializableContainer<T>::tValue>::Serialize(node, t);
  return node;
}

template <typename T>
inline const typename std::enable_if < serialization::IsSerializableContainer<T>::value && (!serialization::IsSerializableMap<T>::value) &&
(!std::is_same<T, std::string>::value) &&
serialization::ContainerSerialization<typename serialization::IsSerializableContainer<T>::tValue>::cXML_SERIALIZABLE, tNode >::type&
operator>> (const tNode& node, T& t)
{
  static_assert(!std::is_same<std::string, T>::value, "This is not supposed to be used for std::string");
  serialization::ContainerSerialization<typename serialization::IsSerializableContainer<T>::tValue>::Deserialize(node, t);
  return node;
}

template <typename T>
inline typename std::enable_if < serialization::IsSerializableMap<T>::value &&
serialization::ContainerSerialization<typename serialization::IsSerializableMap<T>::tMapped>::cXML_SERIALIZABLE, tNode >::type&
operator<< (tNode &node, const T& map)
{
  serialization::ContainerSerialization<typename serialization::IsSerializableMap<T>::tMapped>::SerializeMap(node, map);
  return node;
}

template <typename T>
inline const typename std::enable_if < serialization::IsSerializableMap<T>::value &&
serialization::ContainerSerialization<typename serialization::IsSerializableMap<T>::tMapped>::cXML_SERIALIZABLE, tNode >::type&
operator>> (const tNode &node, T& map)
{
  serialization::ContainerSerialization<typename serialization::IsSerializableMap<T>::tMapped>::DeserializeMap(node, map);
  return node;
}

template < typename TFirst, typename TSecond, bool Tenable = serialization::IsXMLSerializable<TFirst>::value && serialization::IsXMLSerializable<TSecond>::value >
inline typename std::enable_if <Tenable, xml::tNode>::type& operator<< (xml::tNode &node, const std::pair<TFirst, TSecond>& pair)
{
  node.AddChildNode("first") << pair.first;
  node.AddChildNode("second") << pair.second;
  return node;
}

template < typename TFirst, typename TSecond, bool Tenable = serialization::IsXMLSerializable<TFirst>::value && serialization::IsXMLSerializable<TSecond>::value >
inline const typename std::enable_if <Tenable, xml::tNode>::type& operator>> (const xml::tNode &node, std::pair<TFirst, TSecond>& pair)
{
  bool first = false, second = false;
  for (auto it = node.ChildrenBegin(); it != node.ChildrenEnd(); ++it)
  {
    if (it->Name() == "first")
    {
      (*it) >> pair.first;
      first = true;
    }
    if (it->Name() == "second")
    {
      (*it) >> pair.second;
      second = true;
    }
  }
  if (first && second)
  {
    return node;
  }
  throw std::runtime_error("Node 'first' and/or 'second' missing");
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
#endif
}
}


#endif
