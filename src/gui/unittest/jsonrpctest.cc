#include <boost/test/auto_unit_test.hpp>
#include <clusterlib.h>
#include "util.h"

using namespace std;
using namespace boost;
using namespace json;
using namespace json::rpc;

class MockMethod : public JSONRPCMethod {
  public:
    virtual std::string getName()
    {
        return "MockMethod"; 
    }
    
    virtual bool checkInitParams(const ::json::JSONValue::JSONArray &paramArr,
                                 bool initialize)
    {
        try {
            string encodedString = JSONCodec::encode(paramArr);
            return true;
        }
        catch (const JSONRPCInvocationException &ex) {
            return false;
        }
    }

    virtual JSONValue invoke(const string &name, 
                             const JSONValue::JSONArray &params, 
                             StatePersistence *persistence) 
    {
        JSONValue::JSONArray array = params;
        array.push_back(string("ADDITIONAL DATA"));
        array.push_back(name);
        return array;
    }
};

class MockErrorMethod : public JSONRPCMethod {
  public:
    virtual std::string getName()
    {
        return "MockErrorMethod"; 
    }
    
    virtual bool checkInitParams(const ::json::JSONValue::JSONArray &paramArr,
                                 bool initialize)
    {
        try {
            string encodedString = JSONCodec::encode(paramArr);
            return true;
        }
        catch (const JSONRPCInvocationException &ex) {
            return false;
        }
    }

    virtual JSONValue invoke(const string &name, 
                             const JSONValue::JSONArray &params, 
                             StatePersistence *persistence) 
    {
        throw JSONRPCInvocationException("Something goes wrong");
    }
};
    
BOOST_AUTO_TEST_CASE(testInvokeNonObject) {
    JSONValue invoke = JSONValue::JSONArray();
    JSONValue::JSONObject result;
    JSONRPCManager rpcManager;
    rpcManager.invoke(invoke).get(&result);
    BOOST_CHECK(result["result"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(result["error"].type() != typeid(JSONValue::JSONNull));
}

BOOST_AUTO_TEST_CASE(testInvokeObjectInvalid) {
    JSONValue::JSONObject request;
    request["why"] = "whatsoever";
    request["nothingisvalid"] = "blah";
    JSONValue invoke = request;
    JSONValue::JSONObject result;
    JSONRPCManager rpcManager;
    rpcManager.invoke(invoke).get(&result);
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
    JSONRPCManager rpcManager;
    rpcManager.invoke(invoke).get(&result);
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
    JSONRPCManager rpcManager;
    rpcManager.invoke(invoke).get(&result);
    BOOST_CHECK(result["result"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(result["error"].type() != typeid(JSONValue::JSONNull));
    BOOST_CHECK_EQUAL(result["id"].get<JSONValue::JSONString>(), "ok");
}

BOOST_AUTO_TEST_CASE(testInvokeObjectNormal) {
    MockMethod mockMethod;
    JSONRPCManager rpcManager;
    BOOST_CHECK(rpcManager.registerMethod("mock", &mockMethod));

    JSONValue::JSONObject request;
    JSONValue::JSONArray params;
    params.push_back("param1");
    params.push_back("param2");
    request["method"] = "mock";
    request["params"] = params;
    request["id"] = "ok";
    JSONValue invoke = request;
    JSONValue::JSONObject result;
    rpcManager.invoke(invoke).get(&result);
    
    JSONValue::JSONArray expected;
    expected.push_back("param1");
    expected.push_back("param2");
    expected.push_back("ADDITIONAL DATA");
    expected.push_back("mock");
    checkEqual(expected, result["result"]);
    BOOST_CHECK(result["error"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK_EQUAL(result["id"].get<JSONValue::JSONString>(), "ok");

    // Clean up
    BOOST_CHECK(rpcManager.unregisterMethod("mock"));
}

BOOST_AUTO_TEST_CASE(testInvokeObjectError) {
    MockErrorMethod mockErrorMethod;
    JSONRPCManager rpcManager;
    BOOST_CHECK(rpcManager.registerMethod("mock", &mockErrorMethod));

    JSONValue::JSONObject request;
    JSONValue::JSONArray params;
    params.push_back("param1");
    params.push_back("param2");
    request["method"] = "mock";
    request["params"] = params;
    request["id"] = "ok";
    JSONValue invoke = request;
    JSONValue::JSONObject result;
    rpcManager.invoke(invoke).get(&result);
    
    BOOST_CHECK(result["result"].type() == typeid(JSONValue::JSONNull));
    BOOST_CHECK(result["error"].type() != typeid(JSONValue::JSONNull));
    BOOST_CHECK_EQUAL(result["id"].get<JSONValue::JSONString>(), "ok");

    // Clean up
    BOOST_CHECK(rpcManager.unregisterMethod("mock"));
}

BOOST_AUTO_TEST_CASE(testUnregisterUnknown) {
    JSONRPCManager rpcManager;
    BOOST_CHECK(!rpcManager.unregisterMethod("mock"));
}

BOOST_AUTO_TEST_CASE(testDoubleRegister) {
    MockErrorMethod mockErrorMethod;
    JSONRPCManager rpcManager;
    BOOST_CHECK(rpcManager.registerMethod("mock", &mockErrorMethod));
    BOOST_CHECK(!rpcManager.registerMethod("mock", &mockErrorMethod));

    // Clean up
    BOOST_CHECK(rpcManager.unregisterMethod("mock"));
}
