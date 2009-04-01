#include "MPITestFixture.h"
#include "testparams.h"
#include "clusterlibinternal.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;

class ClusterlibNotifyableKeyManipulator : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibNotifyableKeyManipulator);
    CPPUNIT_TEST(testNotifyableKeyManipulator1);
    CPPUNIT_TEST(testNotifyableKeyManipulator2);
    CPPUNIT_TEST(testNotifyableKeyManipulator3);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibNotifyableKeyManipulator() : _factoryP(NULL) {}
    
    /**
     * Runs prior to all tests 
     */
    virtual void setUp() 
    {
	_factoryP = new Factory(
            globalTestParams.getZkServerPortList());
	CPPUNIT_ASSERT(_factoryP != NULL);
    }

    /** 
     * Runs after all tests 
     */
    virtual void tearDown() 
    {
	cerr << "delete called " << endl;
	delete _factoryP;
        _factoryP = NULL;
    }

    /** 
     * Simple test to see if root is found.
     */
    void testNotifyableKeyManipulator1()
    {
        cerr << "testNotifyableKeyManipulator1: started" << endl;
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factoryP);

        string expectedres =         
            ClusterlibStrings::ROOTNODE +
            ClusterlibStrings::CLUSTERLIB +
            ClusterlibStrings::KEYSEPARATOR +
            ClusterlibStrings::CLUSTERLIBVERSION +
            ClusterlibStrings::KEYSEPARATOR +
            ClusterlibStrings::ROOT;
        
        string res =         
            expectedres +
            ClusterlibStrings::KEYSEPARATOR +
            ClusterlibStrings::APPLICATIONS;
        
        string final = NotifyableKeyManipulator::removeObjectFromKey(res);
        cerr << "initial key = " << res
             << " final key = " << final << endl;

        CPPUNIT_ASSERT(final == expectedres);

	cerr << "testGetNotifyableKeyManipulator1: done" << endl;
    }

    /** 
     * Try to run on root, should return empty string.
     */
    void testNotifyableKeyManipulator2()
    {
        cerr << "testNotifyableKeyManipulator2: started" << endl;
        INIT_BARRIER_MPI_TEST_OR_DONE(2, true, _factoryP);

        string res =         
            ClusterlibStrings::ROOTNODE +
            ClusterlibStrings::CLUSTERLIB +
            ClusterlibStrings::KEYSEPARATOR +
            ClusterlibStrings::CLUSTERLIBVERSION +
            ClusterlibStrings::KEYSEPARATOR +
            ClusterlibStrings::ROOT;
                
        string final = NotifyableKeyManipulator::removeObjectFromKey(res);
        cerr << "initial key = " << res
             << " final key = " << final << endl;

        CPPUNIT_ASSERT(final.empty());

	cerr << "testNotifyableKeyManipulator2 passed" << endl;
    }

    /** 
     * Try to create a new application and see if the other process
     * picks it up.  Prefers 2 nodes, but if only one process is
     * available, runs as a single process test.
     */
    void testNotifyableKeyManipulator3()
    {
        cerr << "testNotifyableKeyManipulator3: started" << endl;
        INIT_BARRIER_MPI_TEST_OR_DONE(2, true, _factoryP);

        string expectedres =         
            ClusterlibStrings::ROOTNODE +
            ClusterlibStrings::CLUSTERLIB +
            ClusterlibStrings::KEYSEPARATOR +
            ClusterlibStrings::CLUSTERLIBVERSION +
            ClusterlibStrings::KEYSEPARATOR +
            ClusterlibStrings::ROOT +
            ClusterlibStrings::KEYSEPARATOR +
            ClusterlibStrings::APPLICATIONS +
            ClusterlibStrings::KEYSEPARATOR +
            string("test-app");
        
        string res =         
            expectedres +
            ClusterlibStrings::KEYSEPARATOR +
            ClusterlibStrings::GROUPS +
            ClusterlibStrings::KEYSEPARATOR +
            string("test-group");
        
        string final = NotifyableKeyManipulator::removeObjectFromKey(res);
        cerr << "initial key = " << res
             << " final key = " << final << endl;

        CPPUNIT_ASSERT(final == expectedres);

	cerr << "testNotifyableKeyManipulator3 passed" << endl;
    }

  private:
    Factory *_factoryP;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibNotifyableKeyManipulator);

