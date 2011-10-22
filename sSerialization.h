/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2010-2011 Max Reichardt,
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

#ifndef __rrlib__serialization__tSerialization_h__
#define __rrlib__serialization__tSerialization_h__

#include "rrlib/serialization/tSerializable.h"
#include "rrlib/serialization/tMemoryBuffer.h"
#include "rrlib/serialization/tOutputStream.h"
#include "rrlib/serialization/tInputStream.h"
#include <boost/utility.hpp>
#include <string>
#include <vector>

namespace rrlib
{
namespace serialization
{
class tStringOutputStream;
class tStringInputStream;
class tFactory;
class tGenericObject;

/*!
 * \author Max Reichardt
 *
 * Helper class:
 * Serializes binary CoreSerializables to hex string - and vice versa.
 */
class sSerialization : public boost::noncopyable
{
private:

  /*! int -> hex char */
  static char cTO_HEX[16];

  /*! hex char -> int */
  static int cTO_INT[256];

  /*! Helper variable to trigger static initialization in C++ */
  static int cINIT_HELPER;

  static std::string ToString(tStringOutputStream& sos);

public:

  sSerialization() {}

  /*!
   * Converts binary to hex string
   *
   * \param src Input stream that contains binary data
   * \param co Output stream to write hex string to
   */
  static void ConvertBinaryToHexString(tInputStream& src, tStringOutputStream& os);

  /*!
   * Converts hex string from StringInputStream to binary
   *
   * \param src Input stream that contains hex string
   * \param co Output stream to write binary data to
   */
  static void ConvertHexStringToBinary(tStringInputStream& src, tOutputStream& co);

  /*!
   * Creates deep copy of serializable object
   *
   * \param src Object to be copied
   * \param dest Object to copy to
   */
  template <typename T>
  static void DeepCopy(const T& src, T& dest, tFactory* f = NULL);

  /*!
   * Creates deep copy of serializable object using serialization to and from specified memory buffer
   *
   * \param src Object to be copied
   * \param dest Object to copy to
   * \param buf Memory buffer to use
   */
  template <typename T>
  inline static void DeepCopyImpl(const T& src, T& dest, tFactory* f, tMemoryBuffer& buf)
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
   * Deserializes string stream serializable from string
   * (convenience function)
   *
   * \param s String to deserialize from
   * \return String
   */
  template <typename T>
  static T Deserialize(const std::string& s)
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
  static void DeserializeFromHexString(tSerializable* cs, tStringInputStream& s);

  /*!
   * Serialization-based equals()-method
   * (not very efficient/RT-capable - should therefore not be called regular loops)
   *
   * \param obj1 Object1
   * \param obj2 Object2
   * @returns true if both objects are serialized to the same binary data (usually they are equal then)
   */
  static bool Equals(const tGenericObject& obj1, const tGenericObject& obj2);

  /*!
   * \return If a .so-file currently performs static initializations - returns name of .so file
   */
  static std::string GetBinaryCurrentlyPerformingStaticInitialization();

  /*!
   * Resize vector (also works for vectors with noncopyable types)
   *
   * \param vector Vector to resize
   * \param new_size New Size
   */
  template <typename T>
  static void ResizeVector(std::vector<T>& vector, size_t new_size);

  /*!
   * Serializes string stream serializable object to string
   * (convenience function)
   *
   * \param cs Serializable
   * \return String
   */
  static std::string Serialize(const tSerializable& rs);

  /*!
   * Serializes generic object to string
   * (convenience function)
   *
   * \param cs Serializable
   * \return String
   */
  static std::string Serialize(const tGenericObject& go);

  /*!
   * Serializes string stream serializable object to string
   * (convenience function)
   *
   * \param cs Serializable
   * \return String
   */
  template < typename T, bool ENABLE = (!std::is_base_of<tSerializable, T>::value) && (!std::is_base_of<tGenericObject, T>::value) >
  static typename std::enable_if<ENABLE, std::string>::type Serialize(const T& t)
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
  static void SerializeToHexString(const tSerializable* cs, tStringOutputStream& os);

  static int StaticInit();

  // demangle mangled type name
  static std::string Demangle(const char* mangled);

};

} // namespace rrlib
} // namespace serialization

#include "rrlib/serialization/deepcopy.h"

namespace rrlib
{
namespace serialization
{
template <typename T>
void sSerialization::DeepCopy(const T& src, T& dest, tFactory* f)
{
  tDefaultFactory df;
  detail::DeepCopy(src, dest, f != NULL ? f : (tFactory*)&df);
}

template <typename T>
void sSerialization::ResizeVector(std::vector<T>& vector, size_t new_size)
{
  detail::tResize < std::vector<T>, T, !std::is_base_of<boost::noncopyable, T>::value >::Resize(vector, new_size);
}

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tSerialization_h__
