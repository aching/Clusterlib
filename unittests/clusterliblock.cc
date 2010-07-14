#include "testparams.h"
#include "MPITestFixture.h"
#include "clusterlibinternal.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;

const string appName = "unittests-lock-app";
const string groupName = "lock-group";

class ClusterlibLock : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibLock);
    CPPUNIT_TEST(testLock1);
    CPPUNIT_TEST(testLock2);
    CPPUNIT_TEST(testLock20);
    CPPUNIT_TEST(testLock21);
    CPPUNIT_TEST(testLock22);
    CPPUNIT_TEST(testLock23);
    CPPUNIT_TEST(testLock24);
    CPPUNIT_TEST_SUITE_END();

  public:
    ClusterlibLock() 
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
	_app0 = _client0->getRoot()->getApplication(appName, 
                                                    CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup(groupName, CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_group0 != NULL);
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
     * Simple test for a process to lock/unlock the group
     */
    void testLock1()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock1");
        
        if (isMyRank(0)) {
            MPI_CPPUNIT_ASSERT(_group0);
            _group0->acquireLock();
            _group0->releaseLock();
        }
    }

    /** 
     * Test a process locking/unlocking the group 3x.
     */
    void testLock2()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock2");
        if (isMyRank(0)) {
            MPI_CPPUNIT_ASSERT(_group0);
            _group0->acquireLock();
            _group0->acquireLock();
            _group0->acquireLock();
            _group0->releaseLock();
            _group0->releaseLock();
            _group0->releaseLock();
        }
    }

    /** 
     * Test many processes to lock/unlock the group
     */
    void testLock20()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock20");
        
        MPI_CPPUNIT_ASSERT(_group0);
        _group0->acquireLock();
        _group0->releaseLock();
    }

    /** 
     * Test many processes to lock/unlock the group
     */
    void testLock21()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock21");
        
        MPI_CPPUNIT_ASSERT(_group0);
        _group0->acquireLock();
        _group0->acquireLock();
        _group0->acquireLock();
        _group0->releaseLock();
        _group0->releaseLock();
        _group0->releaseLock();
    }

    /** 
     * Test many processes where one has the lock then the others try
     * and fail to get it.
     */
    void testLock22()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock22");
        
        MPI_CPPUNIT_ASSERT(_group0);
        if (isMyRank(0)) {
            _group0->acquireLock();
            barrier(_factory, true);
            barrier(_factory, true);
            _group0->releaseLock();
        }
        else {
            barrier(_factory, true);
            MPI_CPPUNIT_ASSERT(_group0->acquireLockWaitMsecs(10LL) == false);
            barrier(_factory, true);
        }
    }

    /** 
     * Test many processes all try to get the lock, but only 1 and
     * exactly one gets it.
     */
    void testLock23()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock23");
        
        MPI_CPPUNIT_ASSERT(_group0);
        bool gotLock = _group0->acquireLockWaitMsecs(0LL);
        if (gotLock) {
            cerr << getRank() << ": I got the lock" << endl;
            barrier(_factory, true);            
            _group0->releaseLock();
        }
        else {
            MPI_CPPUNIT_ASSERT(_group0->acquireLockWaitMsecs(0LL) == false);
            barrier(_factory, true);
        }
    }

    /** 
     * Test many processes to lock/unlock the group with the
     * NotifyableLocker.
     */
    void testLock24()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock24");
        
        MPI_CPPUNIT_ASSERT(_group0);
        NotifyableLocker l(_group0);
        MPI_CPPUNIT_ASSERT(l.hasLock() == true);
    }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Group *_group0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibLock);

