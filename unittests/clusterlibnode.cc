#include "testparams.h"
#include "MPITestFixture.h"
#include <algorithm>
#include <sstream>

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;

const string appName = "unittests-node-app";

class ClusterlibNode : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibNode);
    CPPUNIT_TEST(testNode1);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibNode() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client0(NULL),
          _app0(NULL),
          _node0(NULL) {}
    
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
            _node0->acquireLock();
            _node0->cachedProcessSlotInfo().setEnable(true);
            _node0->cachedProcessSlotInfo().setMaxProcessSlots(3);
            _node0->cachedProcessSlotInfo().publish();
            _node0->releaseLock();
        }
    }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Node *_node0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibNode);

