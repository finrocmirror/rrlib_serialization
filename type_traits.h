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
/*!\file    rrlib/serialization/type_traits.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-18
 *
 * Type traits to determine which kinds of serialization a type supports.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__type_traits_h__
#define __rrlib__serialization__type_traits_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
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
// Function declarations
//----------------------------------------------------------------------

/*!
 * This type-trait is used to determine whether a type is binary serializable
 */
template <typename T>
class tIsBinarySerializable
{
  static tOutputStream &MakeOutputStream();

  static tInputStream &MakeInputStream();

  template <typename U>
  static U &Make();

  template <typename U = T>
  static int16_t TestOutput(decltype(MakeOutputStream() << Make<U>()));

  template <typename U = T>
  static int16_t TestInput(decltype(MakeInputStream() >> Make<U>()));

  static int32_t TestOutput(...);

  static int32_t TestInput(...);

public:

  enum { value = sizeof(TestOutput(MakeOutputStream())) == sizeof(int16_t) && sizeof(TestInput(MakeInputStream())) == sizeof(int16_t) };
};

/*!
 * This type-trait is used to determine whether a type is string serializable
 */
template <typename T>
struct tIsStringSerializable
{
  enum { value = detail::tIsStringOutputSerializable<T>::value && detail::tIsStringInputSerializable<T>::value };
};

/*!
 * This type-trait is used to determine whether a type is serializable to XML
 */
template <typename T>
class tIsXMLSerializable
{
  static xml::tNode &MakeXMLNode();

  template <typename U>
  static U &Make();

  template <typename U = T>
  static int16_t TestOutput(decltype(MakeXMLNode() << Make<U>()));

  template <typename U = T>
  static int16_t TestInput(decltype(MakeXMLNode() >> Make<U>()));

  static int32_t TestOutput(...);

  static int32_t TestInput(...);

public:

  enum { value = sizeof(TestOutput(MakeXMLNode())) == sizeof(int16_t) && sizeof(TestInput(MakeXMLNode())) == sizeof(int16_t) };
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
