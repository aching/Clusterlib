#include "testparams.h"
#include "MPITestFixture.h"
#include "clusterlibinternal.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;

const string appName = "leader-app";

class ClusterlibLeader : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibLeader);
    CPPUNIT_TEST(testLeader1);
    CPPUNIT_TEST(testLeader2);
    CPPUNIT_TEST(testLeader3);
    CPPUNIT_TEST_SUITE_END();

  public:
    ClusterlibLeader() 
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
	_app0 = _client0->getRoot()->getApplication(appName, true);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup("servers", true);
	MPI_CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", true);
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
     * Simple test to try and become the leader of a group 
     */
    void testLeader1()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLeader1");
        
        if (isMyRank(0)) {
            _group0->becomeLeader();
            MPI_CPPUNIT_ASSERT(_group0->isLeader() == true);
            _group0->abdicateLeader();
            MPI_CPPUNIT_ASSERT(_group0->isLeader() == false);
        }
    }

    /** 
     * Simple test for two processes to try become leader - one then another
     */
    void testLeader2()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLeader2");

        if (isMyRank(0)) {
            _group0->becomeLeader();
            MPI_CPPUNIT_ASSERT(_group0->isLeader() == true);
            _group0->abdicateLeader();
            MPI_CPPUNIT_ASSERT(_group0->isLeader() == false);
        }
        waitsForOrder(0, 1, _factory, true);
        if (isMyRank(1)) {
            _group0->becomeLeader();
            MPI_CPPUNIT_ASSERT(_group0->isLeader() == true);
            _group0->abdicateLeader();
            MPI_CPPUNIT_ASSERT(_group0->isLeader() == false);
        }        
   }

    /** 
     * Simple test for all processes to try and become the leader at
     * the same time.  This is fun with more than 1 process.
     */
    void testLeader3()
    {
        initializeAndBarrierMPITest(-1, 
                                    true,
                                    _factory, 
                                    true, 
                                    "testLeader3");

        _group0->becomeLeader();
        MPI_CPPUNIT_ASSERT(_group0->isLeader() == true);
        _group0->abdicateLeader();
        MPI_CPPUNIT_ASSERT(_group0->isLeader() == false);
    }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Group *_group0;
    Node *_node0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibLeader);

