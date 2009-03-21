#include "MPITestFixture.h"
#include "testparams.h"

extern TestParams globalTestParams;

using namespace std;

/*
 * Forward decl needed for the timer & user
 * event handlers.
 */
class ClusterlibCache;

class CacheClusterEventHandler
    : public clusterlib::ClusterEventHandler
{
  public:
    CacheClusterEventHandler(clusterlib::Client *cp,
                             clusterlib::Event mask,
                             clusterlib::Notifyable *np)
        : clusterlib::ClusterEventHandler(np, mask, cp),
          counter(0),
          lastEvent(clusterlib::EN_NOEVENT)
    {
    }

    int32_t getCounter() { return counter; }
    void setCounter(int32_t newval) { counter = newval; }

    clusterlib::Event getLastEvent() { return lastEvent; }

    void handleClusterEvent(clusterlib::Event e)
    {
        counter++;
        lastEvent = e;
    }

  private:
    /*
     * Count how many times this handler was called.
     */
    int32_t counter;

    /*
     * Last event fired.
     */
    clusterlib::Event lastEvent;
};

/*
 * The test class itself.
 */
class ClusterlibCache
    : public MPITestFixture
{
    CPPUNIT_TEST_SUITE( ClusterlibCache );
    CPPUNIT_TEST( testCache1 );
    CPPUNIT_TEST( testCache2 );
    CPPUNIT_TEST( testCache3 );
    CPPUNIT_TEST( testCache4 );
    CPPUNIT_TEST( testCache5 );

    CPPUNIT_TEST( testCache20 );
    CPPUNIT_TEST( testCache21 );
    CPPUNIT_TEST( testCache22 );
    CPPUNIT_TEST( testCache23 );
    CPPUNIT_TEST( testCache24 );
    CPPUNIT_TEST( testCache25 );
    CPPUNIT_TEST( testCache26 );
    CPPUNIT_TEST( testCache27 );

    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibCache()
        : _factory(NULL),
          _client0(NULL),
          _app0(NULL),
          _grp0(NULL),
          _nod0(NULL),
          _zk(NULL)
    {
    }

    /* Runs prior to all tests */
    virtual void setUp() 
    {
	_factory =
            new clusterlib::Factory(globalTestParams.getZkServerPortList());

	CPPUNIT_ASSERT(_factory != NULL);
        _zk = _factory->getRepository();
        CPPUNIT_ASSERT(_zk != NULL);
	_client0 = _factory->createClient();
	CPPUNIT_ASSERT(_client0 != NULL);
        _app0 = _client0->getApplication("foo-app", true);
        CPPUNIT_ASSERT(_app0 != NULL);
        _grp0 = _app0->getGroup("bar-group", true);
        CPPUNIT_ASSERT(_grp0 != NULL);
        _nod0 = _grp0->getNode("nod3", true);
        CPPUNIT_ASSERT(_nod0 != NULL);
    }

    /* Runs after all tests */
    virtual void tearDown() 
    {
	cerr << "delete called " << endl;

        /*
         * Delete only the factory, that automatically deletes
         * all the other objects.
         */
	delete _factory;
        _factory = NULL;
        _client0 = NULL;
        _app0 = NULL;
        _grp0 = NULL;
        _nod0 = NULL;
    }

    void testCache1()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 1" << endl;

        cerr << "Nod0 = " << _nod0 << ", " << _nod0->getKey() << endl;
        /*
         * Set the health report of a node and see if the cache
         * is updated.
         */
        string hrpath =
            _nod0->getKey() +
            "/" +
            "clientState";
        cerr << "Before setting \"" << hrpath << "\"" << endl;
        _zk->setNodeData(hrpath, "healthy");
        cerr << "After setting \"" << hrpath << "\"" << endl;
        _factory->synchronize();
        cerr << "client state: \""
             << _nod0->getClientState()
             << "\""
             << endl;
        CPPUNIT_ASSERT(string("healthy") == _nod0->getClientState());

        cerr << "Test 1 end" << endl;
    }
    void testCache2()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 2, cp == " << this << endl;

        cerr << "Test 2 end" << endl;
    }
    void testCache3()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 3, cp == " << this << endl;

        cerr << "Test 3 end" << endl;
    }
    void testCache4()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 4" << endl;

        cerr << "Test 4 end" << endl;
    }
    void testCache5()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 5" << endl;

        cerr << "Test 5 end" << endl;
    }

    void testCache20()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 20" << endl;

        cerr << "Test 20 end" << endl;
    }
    void testCache21()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 21" << endl;

        cerr << "Test 21 end" << endl;
    }
    void testCache22()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 22" << endl;

        cerr << "Test 22 end" << endl;
    }
    void testCache23()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 23" << endl;

        cerr << "Test 23 end" << endl;
    }
    void testCache24()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 24" << endl;

        cerr << "Test 24 end" << endl;
    }
    void testCache25()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 25" << endl;

        cerr << "Test 25 end" << endl;
    }
    void testCache26()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 26" << endl;

        cerr << "Test 26 end" << endl;
    }
    void testCache27()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        cerr << "Test 27" << endl;

        cerr << "Test 27 end" << endl;
    }

  private:
    clusterlib::Factory *_factory;
    clusterlib::Client *_client0;
    clusterlib::Application *_app0;
    clusterlib::Group *_grp0;
    clusterlib::Node *_nod0;
    zk::ZooKeeperAdapter *_zk;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION( ClusterlibCache );

