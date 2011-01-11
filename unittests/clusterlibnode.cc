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
#include <sstream>

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;
using namespace boost;

const string appName = "unittests-node-app";

class ClusterlibNode : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibNode);
    CPPUNIT_TEST(testNode1);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibNode() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client0(NULL) 
    {
    }
    
    /* Runs prior to each test */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client0 != NULL);
        _root = _client0->getRoot();
        MPI_CPPUNIT_ASSERT(_root != NULL);
	_app0 = _root->getApplication(
            appName, CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
    }

    /* Runs after each test */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(_factory, true);
	delete _factory;
        _factory = NULL;
    }

    /* 
     * Simple test to create a node and set some cached data.
     */
    void testNode1()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testNode1");
        const string name = "node1";
        if (isMyRank(0)) {
            _node0 = _app0->getNode(name, CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_node0);
            
            NotifyableLocker l(_node0,
                               CLString::NOTIFYABLE_LOCK,
                               DIST_LOCK_EXCL);

            _node0->cachedProcessSlotInfo().setEnable(true);
            _node0->cachedProcessSlotInfo().setMaxProcessSlots(3);
            _node0->cachedProcessSlotInfo().publish();
        }
    }

  private:
    Factory *_factory;
    Client *_client0;
    shared_ptr<Root> _root;
    shared_ptr<Application> _app0;
    shared_ptr<Node> _node0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibNode);

