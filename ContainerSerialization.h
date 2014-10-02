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
/*!\file    rrlib/serialization/ContainerSerialization.h
 *
 * \author  Max Reichardt
 *
 * \date    2014-03-24
 *
 * \brief   Contains ContainerSerialization
 *
 * \b ContainerSerialization
 *
 * Type trait that defines how STL containers containing type T are serialized.
 * May be specialized for data types with special requirements.
 * May also be partly-specialized for e.g. smart pointer classes.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__serialization__ContainerSerialization_h__
#define __rrlib__serialization__ContainerSerialization_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#ifdef _LIB_RRLIB_XML_PRESENT_
#include "rrlib/xml/tNode.h"
#endif

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

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Container Serialization
/*!
 * Type trait that defines how STL containers containing type T are serialized.
 * May be specialized for data types with special requirements.
 * May also be partly-specialized for e.g. smart pointer classes.
 */
template <typename T>
struct ContainerSerialization;

/*!
 * Type trait defining how to resize containers during deserialization - such as std::vector or std::list
 */
template <typename T, bool NO_ARG_CONSTRUCTOR = std::is_base_of<DefaultImplementation, DefaultInstantiation<T>>::value>
struct ContainerResize
{
  template <typename TContainer>
  static void Resize(TContainer& container, size_t new_size)
  {
    container.resize(new_size);
  }
};

template <typename T>
struct ContainerResize<T, false>
{
  template <typename TContainer>
  static void Resize(TContainer& container, size_t new_size)
  {
    if (container.size() < new_size)
    {
      container.reserve(new_size);
    }
    while (container.size() < new_size)
    {
      container.emplace_back(DefaultInstantiation<T>::Create());
    }
    while (container.size() > new_size)
    {
      container.pop_back();
    }
  }
};

/*!
 * Helper class for ContainerSerializationDefault
 * (allows different deserialization implementation for sets)
 */
template <typename T, bool CONTAINER_HAS_CONST_ELEMENTS>
struct ContainerDeserializationDefaulImplementation
{
  template <typename TContainer>
  static void Deserialize(tInputStream& stream, TContainer& container)
  {
    size_t size = stream.ReadInt();
    bool const_type = stream.ReadBoolean();
    if (!const_type)
    {
      throw std::runtime_error("Only const type deserialization is supported");
    }
    ContainerResize<T>::Resize(container, size);
    for (auto it = container.begin(); it != container.end(); it++)
    {
      stream >> *it;
    }
  }

#ifdef _LIB_RRLIB_XML_PRESENT_
  template <typename TContainer>
  static void Deserialize(const xml::tNode& node, TContainer& container)
  {
    // Count elements first and resize (=> possibly less memory allocations)
    size_t count = 0;
    for (auto it = node.ChildrenBegin(); it != node.ChildrenEnd(); ++it)
    {
      count++;
    }
    ContainerResize<T>::Resize(container, count);

    // Deserialize
    count = 0;
    for (auto it = node.ChildrenBegin(); it != node.ChildrenEnd(); ++it)
    {
      (*it) >> container[count];
      count++;
    }
  }
#endif
};

template <typename T>
struct ContainerDeserializationDefaulImplementation<T, true>
{
  template <typename TContainer>
  static void Deserialize(tInputStream& stream, TContainer& container)
  {
    size_t size = stream.ReadInt();
    bool const_type = stream.ReadBoolean();
    if (!const_type)
    {
      throw std::runtime_error("Only const type deserialization is supported");
    }
    container.clear();
    for (size_t i = 0; i < size; i++)
    {
      T next_element(DefaultInstantiation<T>::Create());
      stream >> next_element;
      container.emplace(std::move(next_element));
    }
  }

#ifdef _LIB_RRLIB_XML_PRESENT_
  template <typename TContainer>
  static void Deserialize(const xml::tNode& node, TContainer& container)
  {
    container.clear();

    // Deserialize
    for (auto it = node.ChildrenBegin(); it != node.ChildrenEnd(); ++it)
    {
      T next_element(DefaultInstantiation<T>::Create());
      (*it) >> next_element;
      container.emplace(std::move(next_element));
    }
  }
#endif
};


/*!
 * Base class with default implementation.
 * Is a base class so that specializations may conveniently reuse parts of it.
 */
template <typename T>
struct ContainerSerializationDefault
{
  enum
  {
    cBINARY_SERIALIZABLE = IsBinarySerializable<T>::value,
    cMAP_BINARY_SERIALIZABLE = cBINARY_SERIALIZABLE,
    cXML_SERIALIZABLE = IsXMLSerializable<T>::value,
    cMAP_XML_SERIALIZABLE = cXML_SERIALIZABLE
  };

  // Binary serialization

  template <typename TContainer>
  static void Serialize(tOutputStream& stream, const TContainer& container)
  {
    stream.WriteInt(container.size());
    stream.WriteBoolean(true); // const type?  (possibly unnecessary; if we remove it, this will break binary compatibility to 13.10 though)
    for (auto it = container.begin(); it != container.end(); it++)
    {
      stream << *it;
    }
  }

  template <typename TContainer>
  static void Deserialize(tInputStream& stream, TContainer& container)
  {
    ContainerDeserializationDefaulImplementation<T, IsConstElementContainer<TContainer>::value>::Deserialize(stream, container);
  }

  template <typename TMap>
  static void SerializeMap(tOutputStream& stream, const TMap& map)
  {
    Serialize(stream, map);
  }

  template <typename TMap>
  static void DeserializeMap(tInputStream& stream, TMap& map)
  {
    typename TMap::size_type size = stream.ReadInt();
    bool const_type = stream.ReadBoolean(); // possibly unnecessary (see above)
    if (!const_type)
    {
      throw std::runtime_error("Only const type deserialization is supported");
    }
    map.clear();
    for (typename TMap::size_type i = 0; i < size; ++i)
    {
      typedef typename TMap::key_type tKey;
      typedef typename TMap::mapped_type tMapped;
      std::pair<tKey, tMapped> entry(tKey(), DefaultInstantiation<tMapped>::Create());
      stream >> entry;
      map.insert(std::move(entry));
    }
  }

  // XML serialization
#ifdef _LIB_RRLIB_XML_PRESENT_
  template <typename TContainer>
  static void Serialize(xml::tNode& node, const TContainer& container)
  {
    for (auto it = container.begin(); it != container.end(); it++)
    {
      node.AddChildNode("element") << *it;
    }
  }

  template <typename TContainer>
  static void Deserialize(const xml::tNode& node, TContainer& container)
  {
    ContainerDeserializationDefaulImplementation<T, IsConstElementContainer<TContainer>::value>::Deserialize(node, container);
  }

  template <typename TMap>
  static void SerializeMap(xml::tNode& node, const TMap& map)
  {
    for (auto const & it : map)
    {
      xml::tNode &element_node = node.AddChildNode("element");
      element_node.AddChildNode("key") << it.first;
      element_node.AddChildNode("value") << it.second;
    }
  }

  template <typename TMap>
  static void DeserializeMap(const xml::tNode& node, TMap& map)
  {
    map.clear();
    for (auto it = node.ChildrenBegin(); it != node.ChildrenEnd(); ++it)
    {
      if (it->Name() == "element")
      {
        const xml::tNode *key_node = nullptr;
        const xml::tNode *value_node = nullptr;

        for (auto element_it = it->ChildrenBegin(); element_it != it->ChildrenEnd(); ++element_it)
        {
          if (element_it->Name() == "key")
          {
            key_node = &*element_it;
          }
          if (element_it->Name() == "value")
          {
            value_node = &*element_it;
          }
        }

        if (key_node == nullptr)
        {
          throw std::runtime_error("No key node found");
        }
        if (value_node == nullptr)
        {
          throw std::runtime_error("No value node found");
        }

        typedef typename TMap::key_type tKey;
        typedef typename TMap::mapped_type tMapped;
        std::pair<tKey, tMapped> entry(tKey(), DefaultInstantiation<tMapped>::Create());
        *key_node >> entry.first;
        *value_node >> entry.second;
        map.insert(std::move(entry));
      }
    }
  }
#endif
};

template <typename T>
struct ContainerSerialization : public ContainerSerializationDefault<T>, public DefaultImplementation
{};


//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
