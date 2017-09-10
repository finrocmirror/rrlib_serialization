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
class IsBinarySerializable
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
struct IsStringSerializable
{
  enum { value = detail::IsStringOutputSerializable<T>::value && detail::IsStringInputSerializable<T>::value };
};

/*!
 * This type-trait is used to determine whether a type is serializable to XML
 */
template <typename T>
class IsXMLSerializable
{
#ifdef _LIB_RRLIB_XML_PRESENT_
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
#else
public:
  enum { value = 0 };
#endif
};

/*! Struct to tag default implementations; used as base class */
struct DefaultImplementation
{};

/*! Standard implementation of DefaultInstantiation type trait below */
template <typename T, bool ENUM>
struct StandardDefaultInstantiation;

/*!
 * Type trait that defines how an object of type T can be instantiated
 * This type trait can be specialized for classes that do not have a default constructor
 * so that e.g. STL containers can be resized during deserialization or
 * rrlib_rtti can perform generic instantation.
 */
template <typename T>
struct DefaultInstantiation : public StandardDefaultInstantiation<T, std::is_enum<T>::value>
{};

template <typename T, bool ENUM>
struct StandardDefaultInstantiation : public DefaultImplementation
{
  /*!
   * Creates a new object of type T.
   * (More precisely: Returns first constructor argument for creating an object of type T)
   */
  static T Create()
  {
    return T();
  }
};

template <typename T>
struct StandardDefaultInstantiation<T, true>
{
  /*!
   * Creates a new object of type T.
   * (More precisely: Returns first constructor argument for creating an object of type T)
   */
  static T Create()
  {
    const make_builder::internal::tEnumStrings& enum_strings = make_builder::internal::GetEnumStrings<T>();
    if (enum_strings.non_standard_values)
    {
      return static_cast<const T*>(enum_strings.non_standard_values)[0];
    }
    return T();
  }
};


/*!
 * Type trait that defines whether type T is a serializable container type.
 *
 * Therefore, it needs begin() and end(), which dereferences have 'value_type'.
 * Furthermore, ContainerResize<T>::Resize(container, size_t) must be valid.
 *
 * False for std::string
 */
template <typename T>
class IsSerializableContainer
{

  template <typename U>
  static U &Make();

  template <typename U = T>
  static typename std::remove_const<typename std::remove_reference<decltype(*Make<U>().begin())>::type>::type TestBegin(void*);
  static void TestBegin(...);

  template <typename U = T>
  static typename std::remove_const<typename std::remove_reference<decltype(*Make<U>().end())>::type>::type TestEnd(void*);
  static void TestEnd(...);

  template <typename U = T>
  static typename U::value_type TestValueType(void*);
  static void TestValueType(...);

  typedef decltype(TestBegin(nullptr)) tBegin;
  typedef decltype(TestEnd(nullptr)) tEnd;

public:

  /*! Value type of container if T is a container; void otherwise */
  typedef decltype(TestValueType(nullptr)) tValue;

  enum { value = std::is_same<tBegin, tEnd>::value &&
         std::is_same<tBegin, tValue>::value &&
         (!std::is_same<tBegin, void>::value)
       };
};

template <typename TAlloc>
class IsSerializableContainer<std::vector<bool, TAlloc>>
{
public:
  typedef bool tValue;
  enum { value = 1 };
};

template <>
class IsSerializableContainer<std::string>
{
public:
  typedef std::string::value_type tValue;
  enum { value = 0 };
};

/*!
 * Type trait that defines whether type T is a serializable map container type.
 */
template <typename T>
class IsSerializableMap
{
  template <typename U = T>
  static typename U::mapped_type TestMappedType(void*);
  static void TestMappedType(...);

  template <typename U = T>
  static typename U::key_type TestKeyType(void*);
  static void TestKeyType(...);

public:

  /*! Key type of map if T is a map; void otherwise */
  typedef decltype(TestKeyType(nullptr)) tKey;

  /*! Mapped type of map if T is a map; void otherwise */
  typedef decltype(TestMappedType(nullptr)) tMapped;

  enum { value = (!std::is_same<tKey, void>::value) && (!std::is_same<tMapped, void>::value) && IsSerializableContainer<T>::value };
};

/*!
 * Type trait that checks whether T is a container with constant elements (such as std::set)
 */
template <typename T>
class IsConstElementContainer
{
  template <typename U>
  static U &Make();

  template <typename U = T>
  static typename std::remove_reference<decltype(*Make<U>().begin())>::type TestBegin(void*);
  static void TestBegin(...);

public:

  enum { value = std::is_const<decltype(TestBegin(nullptr))>::value };
};

/*! Type trait that returns whether type T is a std::array */
template <typename T>
struct IsStdArray
{
  enum { value = false };
};
template <typename T, size_t N>
struct IsStdArray<std::array<T, N>>
{
  enum { value = true };
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
