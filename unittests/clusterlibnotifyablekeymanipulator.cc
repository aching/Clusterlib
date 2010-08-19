#include "testparams.h"
#include "MPITestFixture.h"
#include "clusterlibinternal.h"

extern TestParams globalTestParams;

using namespace std;
using namespace boost;
using namespace clusterlib;

class ClusterlibNotifyableKeyManipulator : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibNotifyableKeyManipulator);
    CPPUNIT_TEST(testNotifyableKeyManipulator1);
    CPPUNIT_TEST(testNotifyableKeyManipulator2);
    CPPUNIT_TEST(testNotifyableKeyManipulator3);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibNotifyableKeyManipulator() 
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
     * Simple test to see if root is found.
     */
    void testNotifyableKeyManipulator1()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testNotifyableKeyManipulator1");

        string expectedres =         
            CLStringInternal::ROOT_ZNODE +
            CLStringInternal::CLUSTERLIB +
            CLString::KEY_SEPARATOR +
            CLStringInternal::CLUSTERLIB_VERSION +
            CLString::KEY_SEPARATOR +
            CLString::ROOT_DIR;
        
        string res =         
            expectedres +
            CLString::KEY_SEPARATOR +
            CLString::APPLICATION_DIR;
        
        string final = NotifyableKeyManipulator::removeObjectFromKey(res);
        cerr << "initial key = " << res
             << " final key = " << final << endl;

        MPI_CPPUNIT_ASSERT(final == expectedres);
    }

    /** 
     * Try to run on root, should return empty string.
     */
    void testNotifyableKeyManipulator2()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testNotifyableKeyManipulator2");

        string res =         
            CLStringInternal::ROOT_ZNODE +
            CLStringInternal::CLUSTERLIB +
            CLString::KEY_SEPARATOR +
            CLStringInternal::CLUSTERLIB_VERSION +
            CLString::KEY_SEPARATOR +
            CLString::ROOT_DIR;
                
        string final = NotifyableKeyManipulator::removeObjectFromKey(res);
        cerr << "initial key = " << res
             << " final key = " << final << endl;

        MPI_CPPUNIT_ASSERT(final.empty());
    }

    /** 
     * Try to create a new application and see if the other process
     * picks it up.  Prefers 2 nodes, but if only one process is
     * available, runs as a single process test.
     */
    void testNotifyableKeyManipulator3()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testNotifyableKeyManipulator3");

        string expectedres =         
            CLStringInternal::ROOT_ZNODE +
            CLStringInternal::CLUSTERLIB +
            CLString::KEY_SEPARATOR +
            CLStringInternal::CLUSTERLIB_VERSION +
            CLString::KEY_SEPARATOR +
            CLString::ROOT_DIR +
            CLString::KEY_SEPARATOR +
            CLString::APPLICATION_DIR +
            CLString::KEY_SEPARATOR +
            string("test-app");
        
        string res =         
            expectedres +
            CLString::KEY_SEPARATOR +
            CLString::GROUP_DIR +
            CLString::KEY_SEPARATOR +
            string("test-group");
        
        string final = NotifyableKeyManipulator::removeObjectFromKey(res);
        cerr << "initial key = " << res
             << " final key = " << final << endl;

        MPI_CPPUNIT_ASSERT(final == expectedres);
    }

  private:
    Factory *_factory;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibNotifyableKeyManipulator);

