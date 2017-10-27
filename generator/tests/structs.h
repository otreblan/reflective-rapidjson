#ifndef REFLECTIVE_RAPIDJSON_TESTS_STRUCTS_H
#define REFLECTIVE_RAPIDJSON_TESTS_STRUCTS_H

#include "../../lib/jsonserializable.h"

#include <string>
#include <vector>

using namespace std;
using namespace ReflectiveRapidJSON;

/*!
 * \brief The TestStruct struct inherits from JSONSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testIncludingGeneratedHeader();
 */
struct TestStruct : public JSONSerializable<TestStruct> {
    int someInt = 0;
    string someString = "foo";
    string yetAnotherString = "bar";
};

/*!
 * \brief The AnotherTestStruct struct inherits from JSONSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testInheritence();
 */
struct AnotherTestStruct : public JSONSerializable<AnotherTestStruct> {
    vector<string> arrayOfStrings{ "a", "b", "cd" };
};

/*!
 * \brief The DerivedTestStruct struct inherits from JSONSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testInheritence();
 */
struct DerivedTestStruct : public TestStruct, public JSONSerializable<DerivedTestStruct> {
    bool someBool = true;
};

/*!
 * \brief The NonSerializable struct should be ignored when used as base class because it isn't serializable.
 */
struct NonSerializable {
    int ignored = 25;
};

/*!
 * \brief The MultipleDerivedTestStruct struct inherits from JSONSerializable and should hence have functional fromJson()
 *        and toJson() methods. This is asserted in OverallTests::testInheritence();
 */
struct MultipleDerivedTestStruct : public TestStruct,
                                   public AnotherTestStruct,
                                   public NonSerializable,
                                   public JSONSerializable<MultipleDerivedTestStruct> {
    bool someBool = false;
};

#endif // REFLECTIVE_RAPIDJSON_TESTS_STRUCTS_H