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
/*!\file    serialization.cpp
 *
 * \author  Michael Arndt
 *
 * \date    2014-03-17
 *
 */
//----------------------------------------------------------------------
#include "rrlib/serialization/tests/serialization.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <cstdlib>
#include <iostream>
#include <set>

#include "rrlib/util/tUnitTestSuite.h"

#include "rrlib/serialization/serialization.h"
#include "rrlib/serialization/tInputStream.h"
#include "rrlib/serialization/tOutputStream.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>


//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace rrlib
{
namespace serialization
{

// We check this here so that not all programs using rrlib_serialization include <set> by default
static_assert(IsSerializableContainer<std::set<std::string>>::value == true, "Incorrect trait implementation");
static_assert(IsConstElementContainer<std::set<std::string>>::value == true, "Incorrect trait implementation");

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

class TestSerialization : public util::tUnitTestSuite
{
  RRLIB_UNIT_TESTS_BEGIN_SUITE(TestSerialization);
  RRLIB_UNIT_TESTS_ADD_TEST(TestXMLMap);
  RRLIB_UNIT_TESTS_ADD_TEST(TestBinaryMap);
  RRLIB_UNIT_TESTS_ADD_TEST(TestEnumsBinary);
  RRLIB_UNIT_TESTS_ADD_TEST(TestEnumsString);
  RRLIB_UNIT_TESTS_ADD_TEST(TestFloatingPointStrings);
  RRLIB_UNIT_TESTS_END_SUITE;

private:

  void TestXMLMap()
  {
#ifdef _LIB_RRLIB_XML_PRESENT_
    rrlib::xml::tDocument doc;
    rrlib::xml::tNode &root = doc.AddRootNode("root");
    std::map<size_t, std::string> map;
    map[0] = "Zero";
    map[1] = "One";
    map[2] = "Two";
    root << map;
    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Pretty print of XML serialization must be correct", std::string("<root>\n  <element>\n    <key>0</key>\n    <value>Zero</value>\n  </element>\n  <element>\n    <key>1</key>\n    <value>One</value>\n  </element>\n  <element>\n    <key>2</key>\n    <value>Two</value>\n  </element>\n</root>"), root.GetXMLDump(true));

    std::map<size_t, std::string> other_map;
    root >> other_map;

    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("There must be the correct number of elements in the map", (size_t) 3, other_map.size());
    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Value to key must be correct", std::string("Zero"), other_map[0]);
    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Value to key must be correct", std::string("One"), other_map[1]);
    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Value to key must be correct", std::string("Two"), other_map[2]);
#endif
  }

  void TestBinaryMap()
  {
    std::map<size_t, std::string> map;
    map[0] = "Zero";
    map[1] = "One";
    map[2] = "Two";

    // serialize to memory
    rrlib::serialization::tMemoryBuffer mb;
    rrlib::serialization::tOutputStream os(mb);

    os << map;
    os.Flush();

    // de-serialize
    rrlib::serialization::tInputStream is(mb);

    std::map<size_t, std::string> other_map;
    is >> other_map;

    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("There must be the correct number of elements in the map", (size_t) 3, other_map.size());
    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Value to key must be correct", std::string("Zero"), other_map[0]);
    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Value to key must be correct", std::string("One"), other_map[1]);
    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Value to key must be correct", std::string("Two"), other_map[2]);
  }

  void TestEnumsBinary()
  {
    // serialize to memory
    rrlib::serialization::tMemoryBuffer mb;
    rrlib::serialization::tOutputStream os(mb);
    os << tEnumSigned::MIN_VALUE << tEnumSigned::MAX_VALUE;
    os << tEnumUnsigned::MIN_VALUE << tEnumUnsigned::MAX_VALUE;
    os.Close();

    // deserialize
    tEnumSigned min_signed, max_signed;
    tEnumUnsigned min_unsigned, max_unsigned;
    rrlib::serialization::tInputStream is(mb);
    is >> min_signed >> max_signed >> min_unsigned >> max_unsigned;
    RRLIB_UNIT_TESTS_ASSERT(min_signed == tEnumSigned::MIN_VALUE);
    RRLIB_UNIT_TESTS_ASSERT(max_signed == tEnumSigned::MAX_VALUE);
    RRLIB_UNIT_TESTS_ASSERT(min_unsigned == tEnumUnsigned::MIN_VALUE);
    RRLIB_UNIT_TESTS_ASSERT(max_unsigned == tEnumUnsigned::MAX_VALUE);
  }

  void TestEnumsString()
  {
    RRLIB_UNIT_TESTS_ASSERT(tEnumSigned::MIN_VALUE == Deserialize<tEnumSigned>(Serialize(tEnumSigned::MIN_VALUE)));
    RRLIB_UNIT_TESTS_ASSERT(tEnumSigned::MAX_VALUE == Deserialize<tEnumSigned>(Serialize(tEnumSigned::MAX_VALUE)));
    RRLIB_UNIT_TESTS_ASSERT(tEnumUnsigned::MIN_VALUE == Deserialize<tEnumUnsigned>(Serialize(tEnumUnsigned::MIN_VALUE)));
    RRLIB_UNIT_TESTS_ASSERT(tEnumUnsigned::MAX_VALUE == Deserialize<tEnumUnsigned>(Serialize(tEnumUnsigned::MAX_VALUE)));

    // produce warning messages
    RRLIB_UNIT_TESTS_ASSERT(tEnumSigned::MIN_VALUE == Deserialize<tEnumSigned>("Invalid string for testing ignore warning (" + std::to_string(std::numeric_limits<int64_t>::min()) + ")"));
    RRLIB_UNIT_TESTS_ASSERT(tEnumUnsigned::MAX_VALUE == Deserialize<tEnumUnsigned>("Invalid string for testing ignore warning (" + std::to_string(std::numeric_limits<uint64_t>::max()) + ")"));

    // Enum-based-Flags
    typedef rrlib::util::tEnumBasedFlags<tStandardEnum> tFlags;
    tFlags none, one, many, all;
    one.Set(tStandardEnum::VALUE_3);
    many.Set(tStandardEnum::ANOTHER_VALUE);
    many.Set(tStandardEnum::A_VALUE);
    many.Set(tStandardEnum::WORD);
    all = many;
    all.Set(tStandardEnum::VALUE_3);

    RRLIB_UNIT_TESTS_ASSERT(none == Deserialize<tFlags>(Serialize(none)));
    RRLIB_UNIT_TESTS_ASSERT(one == Deserialize<tFlags>(Serialize(one)));
    RRLIB_UNIT_TESTS_ASSERT(many == Deserialize<tFlags>(Serialize(many)));
    RRLIB_UNIT_TESTS_ASSERT(all == Deserialize<tFlags>(Serialize(all)));
  }

  template <typename T>
  void TestFloatingPointString(T t)
  {
    tStringOutputStream output_stream;
    output_stream << t;
    T read;
    //std::cout << std::endl << output_stream.ToString() << std::endl;
    tStringInputStream input_stream(output_stream.ToString());
    input_stream >> read;
    if (std::isnan(t))
    {
      RRLIB_UNIT_TESTS_ASSERT(std::isnan(read));
    }
    else
    {
      RRLIB_UNIT_TESTS_EQUALITY(t, read);
    }
  }

  void TestFloatingPointStringPair(double value)
  {
    TestFloatingPointString(static_cast<float>(value));
    TestFloatingPointString(value);
  }

  void TestFloatingPointStrings()
  {
    TestFloatingPointStringPair(4.52);
    TestFloatingPointStringPair(0.000000000000582956);
    TestFloatingPointStringPair(0.0000000000005829562352435643);
    TestFloatingPointStringPair(6091367925921349623646693463463469.0005);
    TestFloatingPointStringPair(std::numeric_limits<double>::min());
    TestFloatingPointStringPair(std::numeric_limits<double>::max());
    TestFloatingPointStringPair(std::numeric_limits<double>::lowest());
    TestFloatingPointStringPair(std::numeric_limits<double>::infinity());
    TestFloatingPointStringPair(-std::numeric_limits<double>::infinity());
    TestFloatingPointStringPair(std::numeric_limits<double>::quiet_NaN());
    TestFloatingPointStringPair(-std::numeric_limits<double>::quiet_NaN());
    TestFloatingPointStringPair(1234.000000000000000000555555);
    TestFloatingPointStringPair(-34.500000000000000000555555);
    TestFloatingPointStringPair(-550000000000055.00000);
  }
};

RRLIB_UNIT_TESTS_REGISTER_SUITE(TestSerialization);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
