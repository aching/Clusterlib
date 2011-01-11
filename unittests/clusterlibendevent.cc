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

extern TestParams globalTestParams;

using namespace std;
using namespace boost;
using namespace clusterlib;
using namespace json;

const string appName = "unittests-endevent-app";

class ClusterlibEndEvent : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibEndEvent);
    CPPUNIT_TEST(testEndEvent1);
    CPPUNIT_TEST_SUITE_END();

  public:
    class HealthUpdater : public Periodic {
      public:
        HealthUpdater(int64_t msecsFrequency, 
                      const shared_ptr<Notifyable> &notifyableSP, 
                      ClientData *clientData)
            : Periodic(msecsFrequency, notifyableSP, clientData) {}
        
        virtual void run() 
        {
            shared_ptr<Notifyable> notifyableSP = getNotifyable();
            
            assert(notifyableSP != NULL);
            
            JSONValue jsonHealth;
            notifyableSP->cachedCurrentState().get(
                Node::HEALTH_KEY, jsonHealth);
        }    
    };

    ClusterlibEndEvent() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client(NULL) {}

    /**
     * Runs prior to each test 
     */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client != NULL);
        if (isMyRank(0)) {
            _app = _client->getRoot()->getApplication(appName,
                                                      LOAD_FROM_REPOSITORY);
            if (_app != NULL) {
                _app->remove(true);
            }
        }
        barrier(_factory, true);
	_app = _client->getRoot()->getApplication(
            appName, CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_app != NULL);
    }

    /** 
     * Runs after each test
     */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(NULL, false);
    }

    /**
     * One problem that we have observed is that if the health checker
     * or any other event causing operation is run after the
     * endEvent() is passed around (called by the destructor of
     * FactoryOps), failure is caused because of Zookeeper being not
     * available.  This should not happen, even if the user did not
     * remember to unregister their health checkers.
     */
    void testEndEvent1()
    {
        initializeAndBarrierMPITest(-1,
                                    true,
                                    _factory,
                                    true,
                                    "testEndEvent1");
        stringstream ss;
        ss << "node" << getRank();
        
        shared_ptr<Node> myNode = _app->getNode(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(myNode != NULL);
        myNode->acquireLock(CLString::OWNERSHIP_LOCK,
                            DIST_LOCK_EXCL);
        HealthUpdater updater(1, myNode, NULL);
        _factory->registerPeriodicThread(updater);

        /*
         * Shouldn't be used this way, but will be in this case to
         * test for the end event 
         */
        myNode->releaseLock(CLString::OWNERSHIP_LOCK);
        barrier(_factory, true);
        finishedClTest(_factory);
	delete _factory;
    }

  private:
    Factory *_factory;
    Client *_client;
    shared_ptr<Application> _app;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibEndEvent);
