#include <boost/test/auto_unit_test.hpp>
#include "jsonrpc.h"
#include "util.h"

using namespace std;
using namespace boost;
using namespace json;
using namespace json::rpc;

class MockMethod : public JSONRPCMethod {
public:
    JSONValue invoke(const string &name, const JSONValue::JSONArray &params, StatePersistence *persistence) {
        JSONValue::JSONArray array = params;
        array.push_back(string("ADDITIONAL DATA"));
        array.push_back(name);
        return array;
    }
};

class MockErrorMethod : public JSONRPCMethod {
public:
    JSONValue invoke(const string &name, const JSONValue::JSONArray &params, StatePersistence *persistence) {
        throw JSONRPCInvocationException("Something goes wrong");
    }
};
    
BOOST_AUTO_TEST_CASE(testJSONRPCManagerGetInstance) {
    // Test the singleton
    BOOST_CHECK_EQUAL(JSONRPCManager::getInstance(), JSONRPCManager::getInstance());
}

BOOST_AUTO_TEST_CASE(testInvokeNonObject) {
    JSONValue invoke = JSONValue::JSONArray();
    JSONValue::JSONObject result;
    JSONRPCManager::getInstance()->invoke(invoke).get(&result);
    BOOST_CHECK(result["result"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(result["error"].type() != typeid(JSONValue::JSONNull));
}

BOOST_AUTO_TEST_CASE(testInvokeObjectInvalid) {
    JSONValue::JSONObject request;
    request["why"] = "whatsoever";
    request["nothingisvalid"] = "blah";
    JSONValue invoke = request;
    JSONValue::JSONObject result;
    JSONRPCManager::getInstance()->invoke(invoke).get(&result);
    BOOST_CHECK(result["result"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(result["error"].type() != typeid(JSONValue::JSONNull));
}

BOOST_AUTO_TEST_CASE(testInvokeObjectParamsInvalid) {
    JSONValue::JSONObject request;
    request["method"] = "whatsoever";
    request["params"] = "blah";
    request["id"] = "ok";
    JSONValue invoke = request;
    JSONValue::JSONObject result;
    JSONRPCManager::getInstance()->invoke(invoke).get(&result);
    BOOST_CHECK(result["result"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(result["error"].type() != typeid(JSONValue::JSONNull));
    BOOST_CHECK_EQUAL(result["id"].get<JSONValue::JSONString>(), "ok");
}

BOOST_AUTO_TEST_CASE(testInvokeObjectUnknown) {
    JSONValue::JSONObject request;
    JSONValue::JSONArray params;
    params.push_back("param1");
    params.push_back("param2");
    request["method"] = "unknown";
    request["params"] = params;
    request["id"] = "ok";
    JSONValue invoke = request;
    JSONValue::JSONObject result;
    JSONRPCManager::getInstance()->invoke(invoke).get(&result);
    BOOST_CHECK(result["result"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(result["error"].type() != typeid(JSONValue::JSONNull));
    BOOST_CHECK_EQUAL(result["id"].get<JSONValue::JSONString>(), "ok");
}

BOOST_AUTO_TEST_CASE(testInvokeObjectNormal) {
    MockMethod mockMethod;
    BOOST_CHECK(JSONRPCManager::getInstance()->registerMethod("mock", &mockMethod));

    JSONValue::JSONObject request;
    JSONValue::JSONArray params;
    params.push_back("param1");
    params.push_back("param2");
    request["method"] = "mock";
    request["params"] = params;
    request["id"] = "ok";
    JSONValue invoke = request;
    JSONValue::JSONObject result;
    JSONRPCManager::getInstance()->invoke(invoke).get(&result);
    
    JSONValue::JSONArray expected;
    expected.push_back("param1");
    expected.push_back("param2");
    expected.push_back("ADDITIONAL DATA");
    expected.push_back("mock");
    checkEqual(expected, result["result"]);
    BOOST_CHECK(result["error"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK_EQUAL(result["id"].get<JSONValue::JSONString>(), "ok");

    // Clean up
    BOOST_CHECK(JSONRPCManager::getInstance()->unregisterMethod("mock"));
}

BOOST_AUTO_TEST_CASE(testInvokeObjectError) {
    MockErrorMethod mockErrorMethod;
    BOOST_CHECK(JSONRPCManager::getInstance()->registerMethod("mock", &mockErrorMethod));

    JSONValue::JSONObject request;
    JSONValue::JSONArray params;
    params.push_back("param1");
    params.push_back("param2");
    request["method"] = "mock";
    request["params"] = params;
    request["id"] = "ok";
    JSONValue invoke = request;
    JSONValue::JSONObject result;
    JSONRPCManager::getInstance()->invoke(invoke).get(&result);
    
    BOOST_CHECK(result["result"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(result["error"].type() != typeid(JSONValue::JSONNull));
    BOOST_CHECK_EQUAL(result["id"].get<JSONValue::JSONString>(), "ok");

    // Clean up
    BOOST_CHECK(JSONRPCManager::getInstance()->unregisterMethod("mock"));
}

BOOST_AUTO_TEST_CASE(testUnregisterUnknown) {
    BOOST_CHECK(!JSONRPCManager::getInstance()->unregisterMethod("mock"));
}

BOOST_AUTO_TEST_CASE(testDoubleRegister) {
    MockErrorMethod mockErrorMethod;
    BOOST_CHECK(JSONRPCManager::getInstance()->registerMethod("mock", &mockErrorMethod));
    BOOST_CHECK(!JSONRPCManager::getInstance()->registerMethod("mock", &mockErrorMethod));

    // Clean up
    BOOST_CHECK(JSONRPCManager::getInstance()->unregisterMethod("mock"));
}
