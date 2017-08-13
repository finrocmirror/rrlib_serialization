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
/*!\file    rrlib/serialization/tests/serialization.cpp
 *
 * \author  Michael Arndt
 * \author  Max Reichardt
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
  RRLIB_UNIT_TESTS_ADD_TEST(TestStringSerialization);
  RRLIB_UNIT_TESTS_ADD_TEST(TestXMLMap);
  RRLIB_UNIT_TESTS_ADD_TEST(TestBinaryMap);
  RRLIB_UNIT_TESTS_ADD_TEST(TestBinarySet);
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

  void TestBinarySet()
  {
    std::set<std::string> set;
    set.emplace("Zero");
    set.emplace("One");
    set.emplace("Two");

    std::set<std::string> other_set = TestBinarySerialization(set);

    RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("There must be the correct number of elements in the set", (size_t) 3, other_set.size());
    RRLIB_UNIT_TESTS_ASSERT_MESSAGE("Set must contain the element", other_set.find("Zero") != other_set.end());
    RRLIB_UNIT_TESTS_ASSERT_MESSAGE("Set must contain the element", other_set.find("One") != other_set.end());
    RRLIB_UNIT_TESTS_ASSERT_MESSAGE("Set must contain the element", other_set.find("Two") != other_set.end());
  }

  void TestEnumsBinary()
  {
    if (!make_builder::internal::GetEnumStrings<tEnumSigned>().non_standard_values)
    {
      RRLIB_LOG_PRINT(WARNING, "No enum values are available. They are only available when using the clang plugin for generating enum strings - instead of doxygen (which is deprecated). Skipping test.");
      return;
    }

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
    if (!make_builder::internal::GetEnumStrings<tEnumSigned>().non_standard_values)
    {
      RRLIB_LOG_PRINT(WARNING, "No enum values are available. They are only available when using the clang plugin for generating enum strings - instead of doxygen (which is deprecated). Skipping test.");
      return;
    }

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

  /*!
   * Helper method for testing binary serialization for an object of type T
   *
   * \param value Object of type T
   * \return Object created from deserializing the serialized object passed to this function
   */
  template <typename T, size_t Texpected_size = 0>
  T TestBinarySerialization(const T &value)
  {
    // serialize to memory
    rrlib::serialization::tMemoryBuffer mb;
    rrlib::serialization::tOutputStream os(mb);

    os << value;
    os.Close();

    if (Texpected_size > 0)
    {
      RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("Serialized size must be correct", Texpected_size, mb.GetSize());
    }

    // deserialize to original type
    rrlib::serialization::tInputStream is(mb);
    T val;
    is >> val;
    RRLIB_UNIT_TESTS_ASSERT_MESSAGE("After de-serializing to original type, value must be correct", value == val);
    //RRLIB_UNIT_TESTS_EQUALITY_MESSAGE("After de-serializing to original type, value must be correct", value, val);
    return val;
  }

  template <size_t MAX_TEST_STRING_LENGTH>
  void TestStringSerialization(const std::vector<std::string>& test_strings, size_t serialization_count, size_t max_length)
  {
    // serialize to memory
    rrlib::serialization::tMemoryBuffer mb;
    rrlib::serialization::tOutputStream os(mb);
    for (size_t i = 0; i < serialization_count; i++)
    {
      for (const std::string & string : test_strings)
      {
        os << string;
      }
    }
    os.Close();

    // Deserialize in different ways
    rrlib::serialization::tInputStream input1(mb), input2(mb), input3(mb), input4(mb), input5(mb), input6(mb), input7(mb), input8(mb);
    std::string string_buffer;
    std::stringstream string_stream;
    char char_buffer[MAX_TEST_STRING_LENGTH];
    for (size_t i = 0; i < serialization_count; i++)
    {
      for (const std::string & string : test_strings)
      {
        // without max_length
        input1 >> string_buffer;
        RRLIB_UNIT_TESTS_EQUALITY(string, string_buffer);
        string_buffer = input2.ReadString();
        RRLIB_UNIT_TESTS_EQUALITY(string, string_buffer);
        string_stream.str(std::string());
        string_stream.clear(); // reset stream
        input3.ReadString(string_stream);
        RRLIB_UNIT_TESTS_EQUALITY(string, string_stream.str());
        input4.ReadString(char_buffer, true);
        RRLIB_UNIT_TESTS_EQUALITY(string, std::string(char_buffer));
        //RRLIB_UNIT_TESTS_ASSERT(string == char_buffer);

        // with max_length < string_length
        input5.ReadString(string_buffer, max_length);
        RRLIB_UNIT_TESTS_EQUALITY(string.substr(0, max_length), string_buffer);
        input5.ReadString(string_buffer);
        RRLIB_UNIT_TESTS_EQUALITY(string.substr(max_length), string_buffer);

        string_buffer = input6.ReadString(max_length);
        RRLIB_UNIT_TESTS_EQUALITY(string.substr(0, max_length), string_buffer);
        string_buffer = input6.ReadString();
        RRLIB_UNIT_TESTS_EQUALITY(string.substr(max_length), string_buffer);

        string_stream.str(std::string());
        string_stream.clear(); // reset stream
        input7.ReadString(string_stream, max_length);
        RRLIB_UNIT_TESTS_EQUALITY(string.substr(0, max_length), string_stream.str());
        string_stream.str(std::string());
        string_stream.clear(); // reset stream
        input7.ReadString(string_stream);
        RRLIB_UNIT_TESTS_EQUALITY(string.substr(max_length), string_stream.str());

        char small_char_buffer[max_length + 1];
        input8.ReadString(small_char_buffer, max_length + 1, true);
        RRLIB_UNIT_TESTS_ASSERT(string.substr(0, max_length) == small_char_buffer);
        input8.ReadString(char_buffer, true);
        RRLIB_UNIT_TESTS_ASSERT(string.substr(max_length) == char_buffer);
      }
    }
  }

  void TestStringSerialization()
  {
    TestStringSerialization<7>({ "string", "123456", "qwertz" }, 8192, 4);
    TestStringSerialization<8>({ "str", "123", "qwe" }, 8192, 2);

    std::ostringstream string_buffer;
    for (size_t i = 0; i < 8400; i++)
    {
      string_buffer << static_cast<char>('a' + (i % 26));
    }
    TestStringSerialization<10000>({ string_buffer.str() }, 10, 1070);
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
