/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#include <boost/test/auto_unit_test.hpp>
#include <clusterlib.h>
#include "util.h"

using namespace std;
using namespace boost;
using namespace json;
using namespace json::rpc;

class MockMethod : public JSONRPCMethod
{
  public:
    MockMethod() : name("MockMethod") {}

    virtual const string &getName() const
    {
        return name;
    }
    
    virtual void checkParams(const ::json::JSONValue::JSONArray &paramArr)
    {
        JSONCodec::encode(paramArr);
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

    string name;
};

class MockErrorMethod : public JSONRPCMethod
{
  public:
    MockErrorMethod() : name("MockErrorMethod") {}

    virtual const string &getName() const
    {
        return name;
    }
    
    virtual void checkParams(const ::json::JSONValue::JSONArray &paramArr)
    {
        JSONCodec::encode(paramArr);
    }

    virtual JSONValue invoke(const string &name, 
                             const JSONValue::JSONArray &params, 
                             StatePersistence *persistence) 
    {
        throw JSONRPCInvocationException("Something goes wrong");
    }

    string name;
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
