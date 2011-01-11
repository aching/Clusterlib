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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "util.h"

using namespace std;
using namespace boost;
using namespace json;

void checkEqual(const JSONValue &v1, const JSONValue &v2) {
    // The type must be the same
    BOOST_CHECK(v1.type() == v2.type());

    if (v1.type() == typeid(JSONValue::JSONNull)) {
        BOOST_CHECK(v1.get<JSONValue::JSONNull>() == 
                    v2.get<JSONValue::JSONNull>());
    } else if (v1.type() == typeid(JSONValue::JSONInteger)) {
        BOOST_CHECK_EQUAL(v1.get<JSONValue::JSONInteger>(), 
                          v2.get<JSONValue::JSONInteger>());
    } else if (v1.type() == typeid(JSONValue::JSONFloat)) {
        BOOST_CHECK_CLOSE(v1.get<JSONValue::JSONFloat>(), 
                          v2.get<JSONValue::JSONFloat>(), 1e-9);
    } else if (v1.type() == typeid(JSONValue::JSONBoolean)) {
        BOOST_CHECK_EQUAL(v1.get<JSONValue::JSONBoolean>(), 
                          v2.get<JSONValue::JSONBoolean>());
    } else if (v1.type() == typeid(JSONValue::JSONString)) {
        BOOST_CHECK_EQUAL(v1.get<JSONValue::JSONString>(), 
                          v2.get<JSONValue::JSONString>());
    } else if (v1.type() == typeid(JSONValue::JSONArray)) {
        JSONValue::JSONArray a1 = v1.get<JSONValue::JSONArray>();
        JSONValue::JSONArray a2 = v2.get<JSONValue::JSONArray>();
        BOOST_CHECK_EQUAL(a1.size(), a2.size());
        for (uint64_t i = 0; i < a1.size(); ++i) {
            checkEqual(a1[i], a2[i]);
        }
    } else if (v1.type() == typeid(JSONValue::JSONObject)) {
        JSONValue::JSONObject a1 = v1.get<JSONValue::JSONObject>();
        JSONValue::JSONObject a2 = v2.get<JSONValue::JSONObject>();
        BOOST_CHECK_EQUAL(a1.size(), a2.size());
        for (JSONValue::JSONObject::const_iterator iter = a1.begin(); 
             iter != a1.end(); 
             ++iter) {
            if (a2.find(iter->first) == a2.end()) {
                BOOST_FAIL("Key not found.");
            }
            checkEqual(iter->second, a2[iter->first]);
        }
    }
}
