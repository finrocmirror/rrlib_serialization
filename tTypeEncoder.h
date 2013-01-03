/**
 * You received this file as part of an advanced experimental
 * robotics framework prototype ('finroc')
 *
 * Copyright (C) 2011 Max Reichardt,
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

#ifndef __rrlib__serialization__tTypeEncoder_h__
#define __rrlib__serialization__tTypeEncoder_h__

namespace rrlib
{
namespace rtti
{
class tType;
}
namespace serialization
{
class tInputStream;
class tOutputStream;

/*!
 * Type encoding for streams
 */
enum class tTypeEncoding
{
  LOCAL_UIDS, NAMES, CUSTOM
};

/*!
 * \author Max Reichardt
 *
 * Class to encode and decode types in InputStream and OutputStream.
 */
class tTypeEncoder
{
public:

  /*!
   * \param is Input Stream
   * \return Type decoded from input stream
   */
  virtual rtti::tType ReadType(tInputStream& is) = 0;

  /*!
   * \param os Output stream
   * \param type Type to encode to output stream
   */
  virtual void WriteType(tOutputStream& os, rtti::tType type) = 0;

};

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tTypeEncoder_h__
