#include "clusterlib.h"
#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace std;
using namespace boost;
using namespace clusterlib;

const string appName = "unittests-lock-app";
const string groupName = "lock-group";

class ClusterlibLock : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibLock);
    CPPUNIT_TEST(testLock1);
    CPPUNIT_TEST(testLock2);
    CPPUNIT_TEST(testLock3);
    CPPUNIT_TEST(testLock4);
    CPPUNIT_TEST(testLock5);
    CPPUNIT_TEST(testLock6);
    CPPUNIT_TEST(testLock20);
    CPPUNIT_TEST(testLock21);
    CPPUNIT_TEST(testLock22);
    CPPUNIT_TEST(testLock23);
    CPPUNIT_TEST(testLock24);
    CPPUNIT_TEST(testLock25);
    CPPUNIT_TEST(testLock26);
    CPPUNIT_TEST(testLock27);
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
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            _group0->releaseLock(
                CLString::NOTIFYABLE_LOCK);
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
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }
    }

    /** 
     * Test a process locking/unlocking the group with purposeful
     * double release.
     */
    void testLock3()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock3");
        if (isMyRank(0)) {
            MPI_CPPUNIT_ASSERT(_group0);
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
            try {
                _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
                MPI_CPPUNIT_ASSERT("SHOULD HAVE THROWN EXCEPTION" == 0);
            } catch (const InvalidMethodException &e) {
            }
        }
    }

    /** 
     * Test a process locking/unlocking the group with purposeful
     * double release.
     */
    void testLock4()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock4");
        if (isMyRank(0)) {
            MPI_CPPUNIT_ASSERT(_group0);
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            try {
                _group0->acquireLock(
                    CLString::NOTIFYABLE_LOCK, DIST_LOCK_SHARED);
                MPI_CPPUNIT_ASSERT("SHOULD HAVE THROWN EXCEPTION" == 0);
            } catch (const InvalidArgumentsException &e) {
            }
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }
    }

    /** 
     * Test a process getting a DIST_LOCK_SHARED lock.
     */
    void testLock5()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock5");
        if (isMyRank(0)) {
            MPI_CPPUNIT_ASSERT(_group0);
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_SHARED);
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }
    }

    /** 
     * Test a process getting a DIST_LOCK_SHARED lock with children.
     */
    void testLock6()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock6");
        if (isMyRank(0)) {
            MPI_CPPUNIT_ASSERT(_app0);
            _app0->acquireLock(
                CLString::NOTIFYABLE_LOCK, 
                DIST_LOCK_SHARED,
                true);
            NameList nl = _app0->getLockBids(
                CLString::NOTIFYABLE_LOCK, true);
            NameList::const_iterator nlIt;
            for (nlIt = nl.begin(); nlIt != nl.end(); ++nlIt) {
                cerr << "testLock6: Lock bid = " << *nlIt << endl;
            }
            MPI_CPPUNIT_ASSERT(nl.size() >= 2);
            _app0->releaseLock(CLString::NOTIFYABLE_LOCK,
                               true);
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
        _group0->acquireLock(
            CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
        _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
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
        _group0->acquireLock(
            CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
        _group0->acquireLock(
            CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
        _group0->acquireLock(
            CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
        _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
        _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
        _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
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
            NotifyableLocker l(_group0,
                               CLString::NOTIFYABLE_LOCK,
                               DIST_LOCK_EXCL);
            barrier(_factory, true);
            barrier(_factory, true);
        }
        else {
            barrier(_factory, true);
            MPI_CPPUNIT_ASSERT(_group0->acquireLockWaitMsecs(
                CLString::NOTIFYABLE_LOCK,
                DIST_LOCK_EXCL, 
                10LL) == false);
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
        bool gotLock = _group0->acquireLockWaitMsecs(
            CLString::NOTIFYABLE_LOCK,
            DIST_LOCK_EXCL,
            0LL);
        if (gotLock) {
            cerr << getRank() << ": I got the lock" << endl;
            barrier(_factory, true);            
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }
        else {
            MPI_CPPUNIT_ASSERT(_group0->acquireLockWaitMsecs(
                CLString::NOTIFYABLE_LOCK,
                DIST_LOCK_EXCL,
                0LL) == false);
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
        NotifyableLocker l(_group0,
                           CLString::NOTIFYABLE_LOCK,
                           DIST_LOCK_EXCL);
        MPI_CPPUNIT_ASSERT(l.hasLock() == true);
    }

    /** 
     * Test all processes to getting a DIST_LOCK_SHARED, verifying they
     * simultaneously got access and then releasing.
     */
    void testLock25()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock25");
        
        MPI_CPPUNIT_ASSERT(_group0);
        _group0->acquireLock(
            CLString::NOTIFYABLE_LOCK, DIST_LOCK_SHARED);
        barrier(_factory, true);
        _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
    }

    /**
     * DIST_LOCK_SHARED then DIST_LOCK_EXCL
     */
    void testLock26()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock26");
        
        MPI_CPPUNIT_ASSERT(_group0);
        if (isMyRank(0)) {
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_SHARED);
            barrier(_factory, true);
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }
        else {
            barrier(_factory, true);
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }
    }

    /**
     * DIST_LOCK_EXCL then DIST_LOCK_SHARED.
     */
    void testLock27()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testLock27");
        
        MPI_CPPUNIT_ASSERT(_group0);
        if (isMyRank(0)) {
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
            barrier(_factory, true);
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
            barrier(_factory, true);
        }
        else {
            barrier(_factory, true);
            _group0->acquireLock(
                CLString::NOTIFYABLE_LOCK, DIST_LOCK_SHARED);
            barrier(_factory, true);
            _group0->releaseLock(CLString::NOTIFYABLE_LOCK);
        }
    }


  private:
    Factory *_factory;
    Client *_client0;
    shared_ptr<Application> _app0;
    shared_ptr<Group> _group0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibLock);

