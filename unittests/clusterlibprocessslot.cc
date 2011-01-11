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

#include "clusterlib.h"
#include "testparams.h"
#include "MPITestFixture.h"
#include <algorithm>

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;
using namespace boost;
using namespace json;

const string appName = "unittests-processSlot-app";

class ClusterlibProcessSlot : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibProcessSlot);
    CPPUNIT_TEST(testProcessSlot1);
    CPPUNIT_TEST(testProcessSlot2);
    CPPUNIT_TEST(testProcessSlot3);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibProcessSlot() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client0(NULL) {}

    /* Runs prior to each test */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client0 != NULL);
	_app0 = _client0->getRoot()->getApplication(
            appName, CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup(
            "processSlot-group-servers", CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_node0 != NULL);
    }

    /* Runs after each test */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(_factory, true);
	delete _factory;
        _factory = NULL;
    }

    /* 
     * Simple test to see if processSlot set by one process are read by
     * another (when 2 processes or more are available).  If only one
     * process is available, runs as a single process test. 
     */
    void testProcessSlot1()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testProcessSlot1");

        string processSlotName = "slot0";

        if (isMyRank(0)) {
            _processSlot0 = _node0->getProcessSlot(
                processSlotName,
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_processSlot0);

            _processSlot0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            JSONValue::JSONArray portArr;
            portArr.push_back(1234);
            portArr.push_back(5678);
            _processSlot0->cachedProcessInfo().setPortArr(portArr);
            _processSlot0->cachedProcessInfo().publish();
            _processSlot0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            _processSlot0 = _node0->getProcessSlot(processSlotName, 
                                                   LOAD_FROM_REPOSITORY);
            MPI_CPPUNIT_ASSERT(_processSlot0);
            _processSlot0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            JSONValue::JSONArray portArr = 
                _processSlot0->cachedProcessInfo().getPortArr();
            MPI_CPPUNIT_ASSERT(portArr.size() == 2);
            if (portArr.size() == 2) {
                MPI_CPPUNIT_ASSERT(portArr[0].get<JSONValue::JSONInteger>() ==
                                   1234);
                MPI_CPPUNIT_ASSERT(portArr[1].get<JSONValue::JSONInteger>() == 
                                   5678);
            }
            portArr[0] = 12;
            portArr[1] = 56;
            _processSlot0->cachedProcessInfo().setPortArr(portArr);
            _processSlot0->cachedProcessInfo().publish();
            _processSlot0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }

        waitsForOrder(1, 0, _factory, true);

        if (isMyRank(0)) {
            _processSlot0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            JSONValue::JSONArray portArr = 
                _processSlot0->cachedProcessInfo().getPortArr();
            _processSlot0->releaseLock(CLString::NOTIFYABLE_LOCK);

            if (portArr.size() == 2) {
                MPI_CPPUNIT_ASSERT(portArr[0].get<JSONValue::JSONInteger>() ==
                                   12);
                MPI_CPPUNIT_ASSERT(portArr[1].get<JSONValue::JSONInteger>() == 
                                   56);
            }            
        }
    }


    /* 
     * Simple test to see if processSlot can be found with the
     * getNotifyableByKey() functions.
     */
    void testProcessSlot2()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testProcessSlot2");

        string processSlotName = "slot0";
        string propKey = "slotKey";

        if (isMyRank(0)) {
            _processSlot0 = _node0->getProcessSlot(
                processSlotName,
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_processSlot0);
            shared_ptr<PropertyList> propList = 
                _app0->getPropertyList(CLString::DEFAULT_PROPERTYLIST,
                                       CREATE_IF_NOT_FOUND);
            propList->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            propList->cachedKeyValues().set(propKey, _processSlot0->getKey());
            propList->cachedKeyValues().publish();
            propList->releaseLock(CLString::NOTIFYABLE_LOCK);
        }

        barrier(_factory, true);

        shared_ptr<Root> root = _client0->getRoot();
        shared_ptr<PropertyList> propList = 
            _app0->getPropertyList(CLString::DEFAULT_PROPERTYLIST,
                                   LOAD_FROM_REPOSITORY);
        MPI_CPPUNIT_ASSERT(propList);
        propList->acquireLock(
            CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
        JSONValue jsonValue;
        propList->cachedKeyValues().get(propKey, jsonValue);
        propList->releaseLock(CLString::NOTIFYABLE_LOCK);
        shared_ptr<ProcessSlot> processSlot = 
            dynamic_pointer_cast<ProcessSlot>(
                root->getNotifyableFromKey(
                    jsonValue.get<JSONValue::JSONString>()));
        MPI_CPPUNIT_ASSERT(processSlot);
    }

    /* 
     * Simple test to publish some of the commonly used state variables.
     */
    void testProcessSlot3()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testProcessSlot3");

        string processSlotName = "slot0";
        string propKey = "slotKey";

        if (isMyRank(0)) {
            _processSlot0 = _node0->getProcessSlot(
                processSlotName,
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_processSlot0);
            _processSlot0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            _processSlot0->cachedCurrentState().set(
                ProcessSlot::PID_KEY, 12341234);
            _processSlot0->cachedCurrentState().set(
                ProcessSlot::PROCESS_STATE_KEY, 
                ProcessSlot::PROCESS_STATE_RUN_CONTINUOUSLY_VALUE);
            _processSlot0->cachedCurrentState().publish();
            _processSlot0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }

        barrier(_factory, true);

        _processSlot0 = _node0->getProcessSlot(
            processSlotName,
            LOAD_FROM_REPOSITORY);
        MPI_CPPUNIT_ASSERT(_processSlot0);
        _processSlot0->acquireLock(
            CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
        JSONValue jsonValue, jsonValue2;
        bool found = _processSlot0->cachedCurrentState().get(
            ProcessSlot::PID_KEY, jsonValue);
        bool found2 = _processSlot0->cachedCurrentState().get(
            ProcessSlot::PROCESS_STATE_KEY, jsonValue2);
        _processSlot0->releaseLock(CLString::NOTIFYABLE_LOCK);
        
        MPI_CPPUNIT_ASSERT(found);
        MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONInteger>() == 
                           12341234);
        MPI_CPPUNIT_ASSERT(found2);
        MPI_CPPUNIT_ASSERT(jsonValue2.get<JSONValue::JSONString>() == 
                           ProcessSlot::PROCESS_STATE_RUN_CONTINUOUSLY_VALUE);
    }

  private:
    Factory *_factory;
    Client *_client0;
    shared_ptr<Application> _app0;
    shared_ptr<Group> _group0;
    shared_ptr<Node> _node0;
    shared_ptr<ProcessSlot> _processSlot0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibProcessSlot);

