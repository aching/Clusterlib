#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <clusterlib.h>
#include "util.h"
#include <utility>
#include <boost/random.hpp>

using namespace std;
using namespace boost;
using namespace json;


BOOST_AUTO_TEST_CASE(testJSONValueEmpty) {
    JSONValue value;
    BOOST_CHECK(value.type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(value.get<JSONValue::JSONNull>() == JSONValue::Null);
}

BOOST_AUTO_TEST_CASE(testJSONValueInteger) {
    JSONValue value(1);
    BOOST_CHECK(value.type() == typeid(JSONValue::JSONInteger));
    BOOST_CHECK_EQUAL(value.get<JSONValue::JSONInteger>(), 1);
}

BOOST_AUTO_TEST_CASE(testJSONValueBoolean) {
    JSONValue value(true);
    BOOST_CHECK(value.type() == typeid(JSONValue::JSONBoolean));
    BOOST_CHECK_EQUAL(value.get<JSONValue::JSONBoolean>(), true);
}

BOOST_AUTO_TEST_CASE(testJSONValueFloat) {
    JSONValue value(1.0);
    BOOST_CHECK(value.type() == typeid(JSONValue::JSONFloat));
    BOOST_CHECK_CLOSE(value.get<JSONValue::JSONFloat>(), (JSONValue::JSONFloat)(1.0), 1e-9);
}

BOOST_AUTO_TEST_CASE(testJSONValueString) {
    JSONValue value(string("abc"));
    BOOST_CHECK(value.type() == typeid(JSONValue::JSONString));
    BOOST_CHECK_EQUAL(value.get<JSONValue::JSONString>(), "abc");
}

BOOST_AUTO_TEST_CASE(testJSONValueNull) {
    JSONValue value(JSONValue::Null);
    BOOST_CHECK(value.type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(value.get<JSONValue::JSONNull>() == JSONValue::Null);
}

BOOST_AUTO_TEST_CASE(testJSONValueArray) {
    JSONValue::JSONArray array;
    array.push_back(JSONValue(1));
    JSONValue value(array);
    BOOST_CHECK(value.type() == typeid(JSONValue::JSONArray));
    BOOST_CHECK_EQUAL(value.get<JSONValue::JSONArray>().size(), 1u);
    BOOST_CHECK_EQUAL(value.get<JSONValue::JSONArray>()[0].get<JSONValue::JSONInteger>(), 1);
}

BOOST_AUTO_TEST_CASE(testJSONValueObject) {
    JSONValue::JSONObject obj; 
    obj.insert(make_pair("test", JSONValue(1)));
    JSONValue value(obj);
    BOOST_CHECK(value.type() == typeid(JSONValue::JSONObject));
    BOOST_CHECK_EQUAL(value.get<JSONValue::JSONObject>().size(), 1u);
    BOOST_CHECK_EQUAL(value.get<JSONValue::JSONObject>()["test"].get<JSONValue::JSONInteger>(), 1);
}

BOOST_AUTO_TEST_CASE(testJSONValueGetWrongType) {
    JSONValue value(100);
    BOOST_CHECK_THROW(value.get<JSONValue::JSONArray>(), JSONValueException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeUnknown) {
    BOOST_CHECK_THROW(JSONCodec::decode("TRUE"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("   something meaningless? "), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeInteger) {
    JSONValue value = JSONCodec::decode("   123456789    ");
    JSONValue golden = 123456789;
    checkEqual(value, golden);

    value = JSONCodec::decode("   -123456789    ");
    golden = -123456789;
    checkEqual(value, golden);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeFloat) {
    JSONValue value;
    value = 36.578165187;
    checkEqual(JSONCodec::decode(JSONCodec::encode(value)), value);

    value = -36.578165187;
    checkEqual(JSONCodec::decode(JSONCodec::encode(value)), value);

    value = -36e100;
    checkEqual(JSONCodec::decode(JSONCodec::encode(value)), value);

    value = -36e-100;
    checkEqual(JSONCodec::decode(JSONCodec::encode(value)), value);
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeFloat) {
    JSONValue value = JSONCodec::decode("   0.125    ");
    JSONValue golden = 0.125;
    checkEqual(value, golden);

    value = JSONCodec::decode("   1.125e-10    ");
    golden = 1.125e-10;
    checkEqual(value, golden);

    value = JSONCodec::decode("   1.125e10    ");
    golden = 1.125e10;
    checkEqual(value, golden);

    value = JSONCodec::decode("   -123.125e+10    ");
    golden = -123.125e10;
    checkEqual(value, golden);

    value = JSONCodec::decode("   1E10    ");
    golden = 1e10;
    checkEqual(value, golden);

    value = JSONCodec::decode("   112321.23183    ");
    golden = 112321.23183;
    checkEqual(value, golden);
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeFloatError) {
    BOOST_CHECK_THROW(JSONCodec::decode("0."), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("0.a"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("-"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("-a"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("-0e"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("-0Ea"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("-0e-"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("-0E+a"), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeInteger) {
    JSONValue value;
    value = 123456789;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "123456789");

    value = -123456789;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "-123456789");
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeStringNormal) {
    JSONValue value;
    value = string("abcdef");
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "\"abcdef\"");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeStringNormal) {
    JSONValue value = JSONCodec::decode("\"abc  def\"");
    JSONValue golden = string("abc  def");
    checkEqual(value, golden);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeStringEscapeQuote) {
    JSONValue value;
    value = string("ab\"def");
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "\"ab\\\"def\"");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeStringEscapeQuote) {
    JSONValue value = JSONCodec::decode("   \"abc \\\"  def\"");
    JSONValue golden = string("abc \"  def");
    checkEqual(value, golden);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeStringEscapeBackslash) {
    JSONValue value;
    value = string("ab\\def");
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "\"ab\\\\def\"");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeStringEscapeBackslash) {
    JSONValue value = JSONCodec::decode("\t\r\n \"abc \\\\  def\"  ");
    JSONValue golden = string("abc \\  def");
    checkEqual(value, golden);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeStringEscapeSlash) {
    JSONValue value;
    value = string("ab/def");
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "\"ab\\/def\"");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeStringEscapeSlash) {
    JSONValue value = JSONCodec::decode("\"abc \\/  def\"  ");
    JSONValue golden = string("abc /  def");
    checkEqual(value, golden);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeStringEscapeSpecialControl) {
    JSONValue value;
    value = string("a\b\n\r\t\fz");
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "\"a\\b\\n\\r\\t\\fz\"");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeStringEscapeSpecialControl) {
    JSONValue value = JSONCodec::decode("\"\\b\\n\\r\\t\\f\"");
    JSONValue golden = string("\b\n\r\t\f");
    checkEqual(value, golden);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeStringEscapeUnicode) {
    JSONValue value;
    value = string("a\x01\x02\x03\x04\x1f\x1e\x1d\x1c\x1bz");
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "\"a\\u0001\\u0002\\u0003\\u0004\\u001f\\u001e\\u001d\\u001c\\u001bz\"");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeStringEscapeUnicode) {
    JSONValue value = JSONCodec::decode("\"\\u001F\\u001a\\u001A\\u000f\\u0010\"");
    JSONValue golden = string("\x1f\x1a\x1a\x0f\x10");
    checkEqual(value, golden);
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeStringFailure) {
    BOOST_CHECK_THROW(JSONCodec::decode("\"abcdef"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("\"\\a\\b\\c\\d\\e\\f\""), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("\"\\u00\""), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("\"\\u001z\""), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeBoolean) {
    JSONValue value;
    value = true;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "true");
    value = false;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "false");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeBoolean) {
    checkEqual(JSONCodec::decode("true"), JSONValue(true));
    checkEqual(JSONCodec::decode("false"), JSONValue(false));
    BOOST_CHECK_THROW(JSONCodec::decode("tr"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("fjks"), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeNull) {
    JSONValue value;
    value = JSONValue::Null;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "null");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeNull) {
    checkEqual(JSONCodec::decode("null"), JSONValue());
    BOOST_CHECK_THROW(JSONCodec::decode("NULL"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("nu"), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeArrayEmpty) {
    JSONValue value;
    value = JSONValue::JSONArray();
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "[]");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeArrayEmpty) {
    JSONValue value;
    value = JSONValue::JSONArray();
    checkEqual(JSONCodec::decode("   [                  ]"), value);
    checkEqual(JSONCodec::decode("[]"), value);
    
    BOOST_CHECK_THROW(JSONCodec::decode("["), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeArrayOneElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    array.push_back(string("abc"));
    value = array;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "[\"abc\"]");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeArrayOneElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    array.push_back(string("abc"));
    value = array;
    checkEqual(JSONCodec::decode(" [      \"abc\"    ]  "), value);
    checkEqual(JSONCodec::decode("[\"abc\"]"), value);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeArrayMixedElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    array.push_back(string("abc"));
    array.push_back(1);
    array.push_back(0.5);
    array.push_back(true);
    array.push_back(JSONValue::Null);
    value = array;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "[\"abc\",1,0.5,true,null]");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeArrayMixedElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    array.push_back(string("abc"));
    array.push_back(1);
    array.push_back(0.5);
    array.push_back(true);
    array.push_back(JSONValue::Null);
    value = array;
    checkEqual(JSONCodec::decode("   [  \"abc\"     ,    1,    0.5,true     ,  null   ] "), value);
    checkEqual(JSONCodec::decode("[\"abc\",1,0.5,true,null]"), value);

    BOOST_CHECK_THROW(JSONCodec::decode("   [  \"abc\"         1,    0.5,true     ,  null   ] "), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("   [  \"abc\"  ,       1,    0.5,true     ,  null    "), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeArrayArrayElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    JSONValue::JSONArray array1;
    array1.push_back(false);
    array1.push_back(0.25);
    array.push_back(string("abc"));
    array.push_back(1);
    array.push_back(array1);
    array.push_back(0.5);
    array.push_back(true);
    array.push_back(JSONValue::Null);
    value = array;
    checkEqual(JSONCodec::decode("   [  \"abc\"  ,  1  ,    [   false  ,  0.25     ],   0.5,true,null]"), value);
    checkEqual(JSONCodec::decode("[\"abc\",1,[false,0.25],0.5,true,null]"), value);

    BOOST_CHECK_THROW(JSONCodec::decode("[\"abc\",1,[false,0.25,0.5,true,null]"), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeArrayArrayElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    JSONValue::JSONArray array1;
    array1.push_back(false);
    array1.push_back(0.25);
    array.push_back(string("abc"));
    array.push_back(1);
    array.push_back(array1);
    array.push_back(0.5);
    array.push_back(true);
    array.push_back(JSONValue::Null);
    value = array;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "[\"abc\",1,[false,0.25],0.5,true,null]");
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeArrayObjectElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    JSONValue::JSONObject obj;
    obj["key1"] = false;
    array.push_back(string("abc"));
    array.push_back(1);
    array.push_back(obj);
    array.push_back(0.5);
    array.push_back(true);
    array.push_back(JSONValue::Null);
    value = array;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "[\"abc\",1,{\"key1\":false},0.5,true,null]");
}


BOOST_AUTO_TEST_CASE(testJSONCodecDecodeArrayObjectElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    JSONValue::JSONObject obj;
    obj["key1"] = false;
    array.push_back(string("abc"));
    array.push_back(1);
    array.push_back(obj);
    array.push_back(0.5);
    array.push_back(true);
    array.push_back(JSONValue::Null);
    value = array;
    checkEqual(JSONCodec::decode("   [  \"abc\"  ,  1  ,    {   \"key1\"  :  false     },   0.5,true,null]"), value);
    checkEqual(JSONCodec::decode("[\"abc\",1,{\"key1\":false},0.5,true,null]"), value);

    BOOST_CHECK_THROW(JSONCodec::decode("[\"abc\",1,{\"key1\":false,0.25,0.5,true,null]"), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeArrayDeepArrayElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    JSONValue::JSONArray array1;
    JSONValue::JSONArray array2;
    array1.push_back(false);
    array1.push_back(0.25);
    array2.push_back(JSONValue::JSONArray());
    array1.push_back(array2);
    array.push_back(string("abc"));
    array.push_back(1);
    array.push_back(array1);
    array.push_back(0.5);
    array.push_back(true);
    array.push_back(JSONValue::Null);
    value = array;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "[\"abc\",1,[false,0.25,[[]]],0.5,true,null]");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeArrayDeepArrayElement) {
    JSONValue value;
    JSONValue::JSONArray array;
    JSONValue::JSONArray array1;
    JSONValue::JSONArray array2;
    array1.push_back(false);
    array1.push_back(0.25);
    array2.push_back(JSONValue::JSONArray());
    array1.push_back(array2);
    array.push_back(string("abc"));
    array.push_back(1);
    array.push_back(array1);
    array.push_back(0.5);
    array.push_back(true);
    array.push_back(JSONValue::Null);
    value = array;
    checkEqual(JSONCodec::decode("[  \"abc\"  , 1 , [false ,0.25 ,[[   ] ]],0.5,true,null]"), value);
    checkEqual(JSONCodec::decode("[\"abc\",1,[false,0.25,[[]]],0.5,true,null]"), value);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeObjectEmpty) {
    JSONValue value;
    value = JSONValue::JSONObject();
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "{}");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeObjectEmpty) {
    JSONValue value;
    value = JSONValue::JSONObject();
    checkEqual(JSONCodec::decode("  {     } "), value);
    checkEqual(JSONCodec::decode("{}"), value);

    BOOST_CHECK_THROW(JSONCodec::decode("   {   "), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("{,}"), JSONParseException);
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeObjectOneKey) {
    JSONValue value;
    JSONValue::JSONObject object;
    object["key1"] = 123;
    value = object;
    BOOST_CHECK_EQUAL(JSONCodec::encode(value), "{\"key1\":123}");
}

BOOST_AUTO_TEST_CASE(testJSONCodecDecodeObjectOneKey) {
    JSONValue value;
    JSONValue::JSONObject object;
    object["key1"] = 123;
    value = object;
    checkEqual(JSONCodec::decode("{    \"key1\":123}"), value);
    checkEqual(JSONCodec::decode("{\"key1\"  :123}"), value);
    checkEqual(JSONCodec::decode("{\"key1\":  123}"), value);
    checkEqual(JSONCodec::decode("{\"key1\":123  }"), value);

    BOOST_CHECK_THROW(JSONCodec::decode("{123:123}"), JSONParseException);
    BOOST_CHECK_THROW(JSONCodec::decode("{\"key1\",123}"), JSONParseException);
}

mt19937 seed(8347u);

JSONValue generateRandomJSONValue(int depth = 0) {
    variate_generator<mt19937&, uniform_int<> > typeRand(seed, uniform_int<>(0,6));
    int type = typeRand();
    // At most 5 nested levels for array and object
    while (depth == 5 && (type == 1 || type == 2))
        type = typeRand();
    switch(type) {
    case 0:
    {
        // Generate random string
        ostringstream oss;
        variate_generator<mt19937&, uniform_int<> > lengthRand(seed, uniform_int<>(0,20));
        int len = lengthRand();
        variate_generator<mt19937&, uniform_int<> > charRand(seed, uniform_int<>(0,255));
        while (len > 0) {
            oss << (char)charRand();
            --len;
        }
        return JSONValue(oss.str());
    }
    case 1:
    {
        // Generate random array
        variate_generator<mt19937&, uniform_int<> > lengthRand(seed, uniform_int<>(0,20));
        int len = lengthRand();
        JSONValue::JSONArray array;
        while (len > 0) {
            array.push_back(generateRandomJSONValue(depth + 1));
            --len;
        }
        return JSONValue(array);
    }
    case 2:
    {
        // Generate random object
        variate_generator<mt19937&, uniform_int<> > lengthRand(seed, uniform_int<>(0,20));
        variate_generator<mt19937&, uniform_int<> > charRand(seed, uniform_int<>(0,255));
        int len = lengthRand();
        JSONValue::JSONObject object;
        while (len > 0) {
            // Generate random string
            ostringstream oss;
            int lenKey = lengthRand();
            while (lenKey > 0) {
                oss << (char)charRand();
                --lenKey;
            }
            object[oss.str()] = generateRandomJSONValue(depth + 1);
            --len;
        }
        return JSONValue(object);
    }
    case 3:
    {
        // Generate random integer
        variate_generator<mt19937&, uniform_int<JSONValue::JSONInteger> > integerRand(seed, uniform_int<JSONValue::JSONInteger>(-12784612,12783612));
        return JSONValue(integerRand());
    }
    case 4:
    {
        // Generate random float
        variate_generator<mt19937&, uniform_real<JSONValue::JSONFloat> > floatRand(seed, uniform_real<JSONValue::JSONFloat>(0,1));
        return JSONValue(floatRand());
    }
    case 5:
    {
        // Generate random boolean
        variate_generator<mt19937&, uniform_int<> > boolRand(seed, uniform_int<>(0,1));
        return JSONValue(boolRand() == 1);
    }
    default:
        return JSONValue();
    }
}

BOOST_AUTO_TEST_CASE(testJSONCodecEncodeDecodeRandom) {
    for (int i = 0; i < 100; ++i) {
        JSONValue value = generateRandomJSONValue();
        checkEqual(JSONCodec::decode(JSONCodec::encode(value)), value);
    }
}
