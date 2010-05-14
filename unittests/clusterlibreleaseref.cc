#include "testparams.h"
#include "MPITestFixture.h"
#include "clusterlib.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;

const string appName = "unittests-releaseRef-app";

class ClusterlibReleaseRef : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibReleaseRef);
    CPPUNIT_TEST(testReleaseRef1);
    CPPUNIT_TEST(testReleaseRef2);
    CPPUNIT_TEST(testReleaseRef3);
    CPPUNIT_TEST(testReleaseRef4);
    CPPUNIT_TEST_SUITE_END();

  public:
    ClusterlibReleaseRef() 
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
     * Simple test to try release a Node
     */
    void testReleaseRef1()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testReleaseRef1");
        _node0->releaseRef();
    }

    /** 
     * Simple test to try and get another pointer to a node and release 2x.
     */
    void testReleaseRef2()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testReleaseRef2");
        Node *node0 = _group0->getNode("server-0");
        MPI_CPPUNIT_ASSERT(node0 != NULL);
        node0->releaseRef();
        _node0->releaseRef();
    }

    /**                                                                        
     * Simple test to try releaseRef() then remove()
     */
    void testReleaseRef3()
    {
        initializeAndBarrierMPITest(1,
                                    true,
                                    _factory,
                                    true,
                                    "testReleaseRef3");
        if (isMyRank(0)) {
            _node0->releaseRef();
            MPI_CPPUNIT_ASSERT(_node0);
            _node0->remove();
        }
    }
        
    /**                                                                        
     * Simple test to try remove() then releaseRef() 
     */
    void testReleaseRef4()
    {
        initializeAndBarrierMPITest(1,
                                    true,
                                    _factory,
                                    true,
                                    "testReleaseRef3");
        if (isMyRank(0)) {
            _node0->remove();
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::REMOVED);
            _node0->releaseRef();
        }
    }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Group *_group0;
    Node *_node0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibReleaseRef);
