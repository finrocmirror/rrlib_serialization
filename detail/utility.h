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
/*!\file    rrlib/serialization/detail/utility.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-18
 *
 * Serialization helper structs.
 * They offer serialization methods for all types and will always compile.
 * If a type does not support a certain type of serialization, an error
 * is printed at runtime.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__detail__utility_h__
#define __rrlib__serialization__detail__utility_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/serialization/type_traits.h"

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace rrlib
{
namespace serialization
{
namespace detail
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------------

template <typename T, bool BINARY_SERIALIZABLE = tIsBinarySerializable<T>::value>
struct tBinarySerialization
{
  static void Deserialize(T& t, serialization::tInputStream& sis)
  {
    sis >> t;
  }

  static void Serialize(const T& t, serialization::tOutputStream& sos)
  {
    sos << t;
  }
};

template <typename T>
struct tBinarySerialization<T, false>
{
  static void Deserialize(T& t, serialization::tInputStream& sis)
  {
    RRLIB_LOG_PRINT(ERROR, "Type ", typeid(T).name(), " is not serializable to binary data.");
  }

  static void Serialize(const T& t, serialization::tOutputStream& sos)
  {
    RRLIB_LOG_PRINT(ERROR, "Type ", typeid(T).name(), " is not serializable to binary data.");
  }
};

template <typename T, bool STRING_SERIALIZABLE = tIsStringSerializable<T>::value>
struct tStringSerialization
{
  static void Deserialize(T& t, serialization::tStringInputStream& sis)
  {
    sis >> t;
  }

  static void Serialize(const T& t, serialization::tStringOutputStream& sos)
  {
    sos << t;
  }
};

template <typename T>
struct tStringSerialization<T, false>
{
  static void Deserialize(T& t, serialization::tStringInputStream& sis)
  {
    RRLIB_LOG_PRINT(ERROR, "Type ", typeid(T).name(), " is not serializable to string.");
    std::string tmp; // read string so that stream is not corrupted
    sis >> tmp;
  }

  static void Serialize(const T& t, serialization::tStringOutputStream& sos)
  {
    std::string tmp(std::string("Type ") + typeid(T).name() + " is not serializable to string.");
    RRLIB_LOG_PRINT(ERROR, tmp);
    sos << tmp; // write empty string so that stream is not corrupted
  }
};

template <typename T, bool XML_SERIALIZABLE = tIsXMLSerializable<T>::value>
struct tXMLSerialization
{
  static void Deserialize(T& t, const xml::tNode& x)
  {
    x >> t;
  }

  static void Serialize(const T& t, xml::tNode& x)
  {
    x << t;
  }
};

template <typename T>
struct tXMLSerialization<T, false>
{
  static void Deserialize(T& t, const xml::tNode& x)
  {
    RRLIB_LOG_PRINT(ERROR, "Type ", typeid(T).name(), " is not serializable to XML.");
  }

  static void Serialize(const T& t, xml::tNode& x)
  {
    std::string tmp(std::string("Type ") + typeid(T).name() + " is not serializable to XML.");
    RRLIB_LOG_PRINT(ERROR, tmp);
    x.SetContent(tmp);
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
