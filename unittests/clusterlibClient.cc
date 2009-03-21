#include "MPITestFixture.h"
#include "testparams.h"

extern TestParams globalTestParams;

using namespace std;

/*
 * Forward decl needed for the timer & user
 * event handlers.
 */
class ClusterlibClient;

class ClientTimerEventHandler 
    : public clusterlib::TimerEventHandler
{
  public:
    void handleTimerEvent(clusterlib::TimerId id,
                          clusterlib::ClientData data);
};

class ClientClusterEventHandler
    : public clusterlib::ClusterEventHandler
{
  public:
    ClientClusterEventHandler(clusterlib::Client *cp,
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
class ClusterlibClient
    : public MPITestFixture
{
    CPPUNIT_TEST_SUITE( ClusterlibClient );
    CPPUNIT_TEST( testClient1 );
    CPPUNIT_TEST( testClient2 );
    CPPUNIT_TEST( testClient3 );
    CPPUNIT_TEST( testClient4 );
    CPPUNIT_TEST( testClient5 );

    CPPUNIT_TEST( testClient20 );
    CPPUNIT_TEST( testClient21 );
    CPPUNIT_TEST( testClient22 );
    CPPUNIT_TEST( testClient23 );
    CPPUNIT_TEST( testClient24 );
    CPPUNIT_TEST( testClient25 );
    CPPUNIT_TEST( testClient26 );
    CPPUNIT_TEST( testClient27 );

    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibClient()
        : _factory(NULL),
          _client0(NULL),
          _app0(NULL),
          _id0(0),
          _fired0(false),
          _cancelled0(false)
    {
    }

    /* Called from the timer handler. */
    void setFired0(bool v) { _fired0 = v; }

    /* Runs prior to all tests */
    virtual void setUp() 
    {
	_factory =
            new clusterlib::Factory(globalTestParams.getZkServerPortList());

	CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	CPPUNIT_ASSERT(_client0 != NULL);
        _handler0 = new ClientTimerEventHandler();
        CPPUNIT_ASSERT(_handler0 != NULL);
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

        /*
         * Delete my own data.
         */
        delete _handler0;
        _handler0 = NULL;
    }

    void testClient1()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 1" << endl;
            
        /*
         * Create applications with different names and
         * observe allowed creation and exceptions.
         */
        _app0 = _client0->getApplication("foo-app", true);
        CPPUNIT_ASSERT(_app0 != NULL);
        try {
            _app0 = _client0->getApplication("", false);
            CPPUNIT_ASSERT("UNREACHABLE BECAUSE OF EXCEPTION" == NULL);
        } catch (clusterlib::ClusterException &e) {
        }
        try {
            _app0 = _client0->getApplication("/frob", true);
            CPPUNIT_ASSERT("UNREACHABLE BECAUSE OF EXCEPTION" == NULL);
        } catch (clusterlib::ClusterException &e) {
        }
        cerr << "Test 1 end" << endl;        
    }
    void testClient2()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 2, cp == " << this << endl;

        /*
         * Set up a timer, cancel and observe that it did not
         * fire.
         */
        _fired0 = _cancelled0 = false;

        _id0 = _client0->registerTimer(_handler0, 500, this);
        CPPUNIT_ASSERT(_id0 > -1);
        CPPUNIT_ASSERT(_fired0 == false);
        CPPUNIT_ASSERT(_cancelled0 == false);

        _cancelled0 = _client0->cancelTimer(_id0);
        CPPUNIT_ASSERT(_cancelled0 == true);
        CPPUNIT_ASSERT(_fired0 == false);

        cerr << "Before sleep" << endl;
        sleep(1);
        cerr << "After sleep" << endl;

        CPPUNIT_ASSERT(_cancelled0 == true);
        CPPUNIT_ASSERT(_fired0 == false);
        cerr << "Test 2 end" << endl;
    }
    void testClient3()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 3, cp == " << this << endl;

        /*
         * Set up a timer, allow it to fire.
         */
        _fired0 = _cancelled0 = false;

        _id0 = _client0->registerTimer(_handler0, 500, this);
        CPPUNIT_ASSERT(_id0 > -1);
        CPPUNIT_ASSERT(_fired0 == false);
        CPPUNIT_ASSERT(_cancelled0 == false);

        cerr << "Before sleep" << endl;
        sleep(1);
        cerr << "After sleep" << endl;

        CPPUNIT_ASSERT(_fired0 == true);
        CPPUNIT_ASSERT(_cancelled0 == false);
        cerr << "Test 3 end" << endl;
    }
    void testClient4()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 4" << endl;

        /*
         * Set up a timer, cancel twice and observe that the 2nd
         * cancel returns false.
         */
        _fired0 = _cancelled0 = false;

        _id0 = _client0->registerTimer(_handler0, 1000, this);
        CPPUNIT_ASSERT(_fired0 == false);
        CPPUNIT_ASSERT(_cancelled0 == false);

        _cancelled0 = _client0->cancelTimer(_id0);
        CPPUNIT_ASSERT(_fired0 == false);
        CPPUNIT_ASSERT(_cancelled0 == true);
        cerr << "After first cancel" << endl;

        _cancelled1 = _client0->cancelTimer(_id0);
        CPPUNIT_ASSERT(_fired0 == false);
        CPPUNIT_ASSERT(_cancelled1 == false);
        cerr << "After second cancel" << endl;

        cerr << "Test 4 end" << endl;
    }
    void testClient5()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 5" << endl;

        /*
         * Cancel a non-existent timer, see that it returns false.
         */
        _fired0 = _cancelled0 = false;

        _cancelled0 = _client0->cancelTimer((clusterlib::TimerId) 10001);
        CPPUNIT_ASSERT(_cancelled0 == false);
        cerr << "Test 5 end" << endl;
    }

    void testClient20()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 20" << endl;
        /*
         * Register a user event handler.
         */
        cerr << "Test 20 end" << endl;
    }
    void testClient21()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 21" << endl;
        /*
         * Register a user event handler twice,
         * see that second registration is silently
         * ignored.
         */
        cerr << "Test 21 end" << endl;
    }
    void testClient22()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);
        
        cerr << "Test 22" << endl;
        /*
         * Register and cancel a user event handler,
         * see that cancellation is successful.
         */
        cerr << "Test 22 end" << endl;
    }
    void testClient23()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 23" << endl;
        /*
         * Register a user event handler, then
         * cause the event and see that it fired
         * and the handler was called.
         */
        cerr << "Test 23 end" << endl;
    }
    void testClient24()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 24" << endl;
        /*
         * Register a user event handler, then
         * cause the event 10 times, see that it
         * fires 10 times and that the handler was
         * called 10 times.
         */
        cerr << "Test 24 end" << endl;
    }
    void testClient25()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 25" << endl;
        /*
         * Register a user event handler, then
         * cause the event 5 times, see that it
         * fired 5 times, cancel the handler and see
         * that it no longer fires for subsequent
         * events.
         */
        cerr << "Test 25 end" << endl;
    }
    void testClient26()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 26" << endl;
        /*
         * Register two user event handlers, then
         * cause the event, see that both handlers
         * are called.
         */
        cerr << "Test 26 end" << endl;
    }
    void testClient27()
    {
        INIT_BARRIER_MPI_TEST_OR_DONE(-1, true, _factory);

        cerr << "Test 27" << endl;
        /*
         * Register two user event handlers, then
         * cause the event, see that both handlers
         * are called. Then cancel one, cause the
         * event and see that the other one is still
         * being called.
         */
        cerr << "Test 27 end" << endl;
    }

  private:
    clusterlib::Factory *_factory;
    clusterlib::Client *_client0;
    clusterlib::Application *_app0;

    clusterlib::TimerId _id0;
    clusterlib::TimerId _id1;

    bool _fired0;
    bool _cancelled0;
    bool _fired1;
    bool _cancelled1;

    ClientTimerEventHandler *_handler0;
};

void
ClientTimerEventHandler::handleTimerEvent(clusterlib::TimerId id,
                                          clusterlib::ClientData data)
{
    ClusterlibClient *cp = (ClusterlibClient*) data;

    cerr << "cp == " << cp << ", id == " << id << endl;

    cp->setFired0(true);
}

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION( ClusterlibClient );

