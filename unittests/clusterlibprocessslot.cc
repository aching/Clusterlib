#include "testparams.h"
#include "MPITestFixture.h"
#include <algorithm>

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;

const string appName = "unittests-processSlot-app";

class ClusterlibProcessSlot : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibProcessSlot);
    CPPUNIT_TEST(testProcessSlot1);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibProcessSlot() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client0(NULL),
          _app0(NULL),
          _group0(NULL),
          _node0(NULL),
          _processSlot0(NULL) {}
    
    /* Runs prior to each test */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client0 != NULL);
	_app0 = _client0->getRoot()->getApplication(appName, true);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup("processSlot-group-servers", true);
	MPI_CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", true);
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
                true);
            MPI_CPPUNIT_ASSERT(_processSlot0);
            _processSlot0->acquireLock();
            vector<int32_t> portVec;
            portVec.push_back(1234);
            portVec.push_back(5678);
            _processSlot0->setPortVec(portVec);
            _processSlot0->releaseLock();
        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            _processSlot0 = _node0->getProcessSlot(processSlotName);
            MPI_CPPUNIT_ASSERT(_processSlot0);
            _processSlot0->acquireLock();
            vector<int32_t> portVec = _processSlot0->getPortVec();
            MPI_CPPUNIT_ASSERT(portVec.size() == 2);
            if (portVec.size() == 2) {
                MPI_CPPUNIT_ASSERT(portVec[0] == 1234);
                MPI_CPPUNIT_ASSERT(portVec[1] == 5678);
            }
            portVec[0] = 12;
            portVec[1] = 56;
            _processSlot0->setPortVec(portVec);
            _processSlot0->releaseLock();
        }

        waitsForOrder(1, 0, _factory, true);

        if (isMyRank(0)) {
            _processSlot0->acquireLock();
            vector<int32_t> portVec = _processSlot0->getPortVec();
            _processSlot0->releaseLock();

            MPI_CPPUNIT_ASSERT(portVec.size() == 2);
            if (portVec.size() == 2) {
                MPI_CPPUNIT_ASSERT(portVec[0] == 12);
                MPI_CPPUNIT_ASSERT(portVec[1] == 56);
            }
        }
    }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Group *_group0;
    Node *_node0;
    ProcessSlot *_processSlot0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibProcessSlot);

