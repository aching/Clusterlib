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

#include "clusterlibinternal.h"
#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace std;
using namespace boost;
using namespace clusterlib;

const string appName = "unittests-remove-app";

class ClusterlibRemove : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibRemove);
    CPPUNIT_TEST(testRemove1);
    CPPUNIT_TEST(testRemove2);
    CPPUNIT_TEST(testRemove3);
    CPPUNIT_TEST(testRemove4);
    CPPUNIT_TEST(testRemove5);
    CPPUNIT_TEST(testRemove6);
    CPPUNIT_TEST(testRemove10);
    CPPUNIT_TEST(testRemove11);
    CPPUNIT_TEST(testRemove12);
    CPPUNIT_TEST(testRemove20);
    CPPUNIT_TEST(testRemove30);
    CPPUNIT_TEST(testRemove31);
    CPPUNIT_TEST(testRemove40);
    CPPUNIT_TEST(testRemove99);
    CPPUNIT_TEST_SUITE_END();

  public:
    ClusterlibRemove() 
        : MPITestFixture(globalTestParams),
          _factory(NULL) {}
    
    /**
     * Runs prior to each test 
     */
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
	_group0 = _app0->getGroup("servers", CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_node0 != NULL);
    }

    /** 
     * Runs after each test 
     */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(_factory, true);
	delete _factory;
        _factory = NULL;
    }

    /** 
     * Simple test to try and delete a Node
     */
    void testRemove1()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove1");
        
        if (isMyRank(0)) {
            shared_ptr<Node> node = _group0->getNode(
                "node-to-be-deleted", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(node);
            node->remove();
            MPI_CPPUNIT_ASSERT(node->getState() == Notifyable::REMOVED);
        }
    }

    /** 
     * Simple test to try and delete a PropertyList
     */
    void testRemove2()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove2");

        if (isMyRank(0)) {
            shared_ptr<PropertyList> prop = _group0->getPropertyList(
                CLString::DEFAULT_PROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(prop);
            prop->remove();
            MPI_CPPUNIT_ASSERT(prop->getState() == Notifyable::REMOVED);
        }
   }

    /**                                                                        
     * Simple test to try and delete a Group
     */
    void testRemove3()
    {
        initializeAndBarrierMPITest(-1,
                                    true,
                                    _factory,
                                    true,
                                    "testRemove3");

        if (isMyRank(0)) {
            shared_ptr<Group> group = _group0->getGroup(
                "group-to-be-deleted", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(group);
            group->remove();
            MPI_CPPUNIT_ASSERT(group->getState() == Notifyable::REMOVED);
        }
    }

    /** 
     * Simple test to try and delete an Application
     */
    void testRemove4()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove4");

        if (isMyRank(0)) {
            shared_ptr<Application> app = _client0->getRoot()->getApplication(
                "app-to-be-deleted", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(app);
            app->remove();
            MPI_CPPUNIT_ASSERT(app->getState() == Notifyable::REMOVED);
        }
    }

    /** 
     * Simple test to try and delete a DataDistribution
     */
    void testRemove5()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove5");

        if (isMyRank(0)) {
            shared_ptr<DataDistribution> dist = _group0->getDataDistribution(
                "dist-to-be-deleted", 
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(dist);
            dist->remove();
            MPI_CPPUNIT_ASSERT(dist->getState() == Notifyable::REMOVED);
        }
    }

    /** 
     * Simple test to try and delete the Root
     */
    void testRemove6()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove6");
        
        if (isMyRank(0)) {
            shared_ptr<Root> root = _client0->getRoot();
            try {
                root->remove();
                MPI_CPPUNIT_ASSERT("testRemove6: Shouldn't be able "
                               "to remove root" == NULL);
            } catch (InvalidMethodException &e) {
                cerr << "testRemove6: Exception caught as expected" << endl;
            }
        }
    }

    /**
     * Multi-level removal of a Node and its PropertyList
     */
    void testRemove10()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove10");

        if (isMyRank(0)) {
            shared_ptr<PropertyList> prop = _node0->getPropertyList(
                CLString::DEFAULT_PROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(prop);
            try {
                _node0->remove(false);
                MPI_CPPUNIT_ASSERT(
                    "testRemove10: Can't remove a Notifyable with "
                    "children" == 0);
            } catch (Exception &e) {
                cerr << "testRemove10: Exception caught as expected" << endl;
            }
            
            _node0->remove(true);
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::REMOVED);
            MPI_CPPUNIT_ASSERT(prop->getState() == Notifyable::REMOVED);
        }
    }

    /**
     * Multi-level removal of a Group and its Node 
     */
    void testRemove11()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove11");

        if (isMyRank(0)) {
            _group0->remove(true);
            MPI_CPPUNIT_ASSERT(_group0->getState() == Notifyable::REMOVED);
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::REMOVED);
        }
    }

    /**
     * Multi-level removal of an Application, its Group and its Node 
     */
    void testRemove12()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove12");

        if (isMyRank(0)) {
            _app0->remove(true);
            MPI_CPPUNIT_ASSERT(_app0->getState() == Notifyable::REMOVED);
            MPI_CPPUNIT_ASSERT(_group0->getState() == Notifyable::REMOVED);
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::REMOVED);
        }
    }

    /** 
     * Try to delete a node that all processes know about. 
     */
    void testRemove20()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove20");

        cerr << "testRemove20: state of _node0 (" << _node0->getKey() 
             << ") is (" 
             << _node0->getState() << ")" << endl;
        MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::READY);

        barrier(_factory, true);

        if (isMyRank(0)) {
            _node0->remove();
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::REMOVED);
        }

        barrier(_factory, true);
        
        MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::REMOVED);
    }

    /**
     * Try to create and delete 2x in a row.
     */
    void testRemove30()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove30");
        
        if (isMyRank(0)) {
            shared_ptr<Node> node = _group0->getNode(
                "node-to-be-deleted", CREATE_IF_NOT_FOUND);
            node->remove();
        }
        barrier(_factory, true);
        if (isMyRank(1)) {
            shared_ptr<Node> node = _group0->getNode(
                "node-to-be-deleted", CREATE_IF_NOT_FOUND);
            node->remove();
        }
    }

    /**
     * Try to have one thread remove and then another thread remove.
     */
    void testRemove31()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove31");
        
        if (isMyRank(0)) {
            shared_ptr<Node> node = _group0->getNode(
                "node-to-be-deleted", CREATE_IF_NOT_FOUND);
            node->remove();
        }
        barrier(_factory, true);
        if (isMyRank(1)) {
            shared_ptr<Node> node = _group0->getNode(
                "node-to-be-deleted", CREATE_IF_NOT_FOUND);
            node->remove();
        }
    }

    /**
     * Make sure that the creation after deletion works properly!
     */
    void testRemove40()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove40");
        if (isMyRank(0)) {
            _app0->remove(true);
            MPI_CPPUNIT_ASSERT(_app0->getState() == Notifyable::REMOVED);
            _app0 = _client0->getRoot()->getApplication(
                appName, CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_app0->getState() == Notifyable::READY);
        }

        barrier(_factory, true);
        if (!isMyRank(0)) {
            _app0 = _client0->getRoot()->getApplication(appName, 
                                                        LOAD_FROM_REPOSITORY);
        }

        _app0->getState();
        MPI_CPPUNIT_ASSERT(_app0->getState() == Notifyable::READY);
        MPI_CPPUNIT_ASSERT(_group0->getState() == Notifyable::REMOVED);
        MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::REMOVED);
    }

    /**
     * Make sure that removing all the unittest applications is
     * successful.  As long this is the last test, it will clean up
     * the unittest applications in the clusterlib object hierarchy.
     */
    void testRemove99()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testRemove99");
        if (isMyRank(0)) {
            NameList nl =  _client0->getRoot()->getApplicationNames();
            NameList::iterator it;
            const string unittestPrefix = "unittests-";
            for (it = nl.begin(); it != nl.end(); ++it) {
                if ((it->size() > unittestPrefix.size()) &&
                    (it->compare(0,
                                 unittestPrefix.size(),
                                 unittestPrefix) == 0)) {
                    _client0->getRoot()->getApplication(
                        *it, LOAD_FROM_REPOSITORY)->remove(true);
                    cerr << "Removed the application " << *it << endl;
                }
            }
        }
    }

  private:
    Factory *_factory;
    Client *_client0;
    shared_ptr<Application> _app0;
    shared_ptr<Group> _group0;
    shared_ptr<Node> _node0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibRemove);
