/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2008-2011 Max Reichardt,
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

#ifndef __rrlib__serialization__tSerializable_h__
#define __rrlib__serialization__tSerializable_h__

#include "rrlib/xml/tNode.h"

#include "rrlib/serialization/tDataEncoding.h"

namespace rrlib
{
namespace serialization
{
class tInputStream;
class tStringInputStream;
class tOutputStream;
class tStringOutputStream;

/*!
 * \author Max Reichardt
 *
 * Default implementation of Serializable
 */
class tSerializable
{
public:

  tSerializable() {}

  /*!
   * Deserialize object. Object has to already exists.
   * Should be suited for reusing old objects.
   *
   * \param read_view Stream to deserialize from
   */
  virtual void Deserialize(tInputStream& is) = 0;

  /*!
   * Deserialize object. Object has to already exists.
   * Should be suited for reusing old objects.
   *
   * Parsing errors should throw an Exception - and set object to
   * sensible (default?) value
   *
   * \param s String to deserialize from
   */
  virtual void Deserialize(tStringInputStream& s);

  /*!
   * Deserialize from XML Node
   *
   * \param node Node to deserialize from
   */
  virtual void Deserialize(const xml::tNode& node);

  /*!
   * Deserialize data from binary input stream - possibly using non-binary encoding.
   *
   * \param is Binary input stream
   * \param enc Encoding to use
   */
  void Deserialize(serialization::tInputStream& is, serialization::tDataEncoding enc);

  /*!
   * \param os Stream to serialize object to
   */
  virtual void Serialize(tOutputStream& os) const = 0;

  /*!
   * Serialize object as string (e.g. for xml output)
   *
   * \param os String output stream
   */
  virtual void Serialize(tStringOutputStream& os) const;

  /*!
   * Serialize object to XML
   *
   * \param node Empty XML node (name shouldn't be changed)
   */
  virtual void Serialize(xml::tNode& node) const;

  /*!
   * Serialize data to binary output stream - possibly using non-binary encoding.
   *
   * \param os Binary output stream
   * \param enc Encoding to use
   */
  void Serialize(serialization::tOutputStream& os, serialization::tDataEncoding enc) const;

};

inline xml::tNode& operator << (xml::tNode& node, const tSerializable& ser)
{
  ser.Serialize(node);
  return node;
}

inline const xml::tNode& operator >> (const xml::tNode& node, tSerializable& ser)
{
  ser.Deserialize(node);
  return node;
}

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tSerializable_h__
