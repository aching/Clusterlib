#include "clusterlibinternal.h"
#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace std;
using namespace boost;
using namespace clusterlib;

const string appName = "unittests-leader-app";

class ClusterlibLeader : public MPITestFixture
{
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
            _group0->acquireLock(CLString::OWNERSHIP_LOCK,
                                 DIST_LOCK_EXCL);
            MPI_CPPUNIT_ASSERT(_group0->hasLock(
                CLString::OWNERSHIP_LOCK) == true);
            _group0->releaseLock(CLString::OWNERSHIP_LOCK);
            MPI_CPPUNIT_ASSERT(_group0->hasLock(
                CLString::OWNERSHIP_LOCK) == false);
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
            _group0->acquireLock(CLString::OWNERSHIP_LOCK,
                                 DIST_LOCK_EXCL);
            MPI_CPPUNIT_ASSERT(_group0->hasLock(
                CLString::OWNERSHIP_LOCK) == true);
            _group0->releaseLock(CLString::OWNERSHIP_LOCK);
            MPI_CPPUNIT_ASSERT(_group0->hasLock(
                CLString::OWNERSHIP_LOCK) == false);
        }
        waitsForOrder(0, 1, _factory, true);
        if (isMyRank(1)) {
            _group0->acquireLock(CLString::OWNERSHIP_LOCK,
                                 DIST_LOCK_EXCL);
            MPI_CPPUNIT_ASSERT(_group0->hasLock(
                CLString::OWNERSHIP_LOCK) == true);
            _group0->releaseLock(CLString::OWNERSHIP_LOCK);
            MPI_CPPUNIT_ASSERT(_group0->hasLock(
                CLString::OWNERSHIP_LOCK) == false);
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

        _group0->acquireLock(CLString::OWNERSHIP_LOCK,
                             DIST_LOCK_EXCL);
        MPI_CPPUNIT_ASSERT(_group0->hasLock(
            CLString::OWNERSHIP_LOCK) == true);
        _group0->releaseLock(CLString::OWNERSHIP_LOCK);
        MPI_CPPUNIT_ASSERT(_group0->hasLock(
            CLString::OWNERSHIP_LOCK) == false);
    }

  private:
    Factory *_factory;
    Client *_client0;
    shared_ptr<Application> _app0;
    shared_ptr<Group> _group0;
    shared_ptr<Node> _node0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibLeader);

