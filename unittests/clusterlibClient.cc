#include "MPITestFixture.h"

/*
 * Forward decl needed for the timer & user
 * event handlers.
 */
class ClusterlibClient;

class MyTimerEventHandler 
    : public clusterlib::TimerEventHandler
{
  public:
    void handleTimerEvent(clusterlib::TimerId id,
                          clusterlib::ClientData data);
};

class MyClusterEventHandler
    : public clusterlib::ClusterEventHandler
{
  public:
    MyClusterEventHandler(clusterlib::Client *cp,
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
	_factory = new clusterlib::Factory("localhost:2181");
	CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	CPPUNIT_ASSERT(_client0 != NULL);
        _handler0 = new MyTimerEventHandler();
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
    }
    void testClient2()
    {
        /*
         * Set up a timer, cancel and observe that it did not
         * fire.
         */
        _fired0 = _cancelled0 = false;

        _id0 = _client0->registerTimer(_handler0, 1000, this);
        CPPUNIT_ASSERT(_id0 > -1);
        CPPUNIT_ASSERT(_fired0 == false);
        CPPUNIT_ASSERT(_cancelled0 == false);

        _cancelled0 = _client0->cancelTimer(_id0);
        CPPUNIT_ASSERT(_cancelled0 == true);
        CPPUNIT_ASSERT(_fired0 == false);

        sleep(2);

        CPPUNIT_ASSERT(_cancelled0 == true);
        CPPUNIT_ASSERT(_fired0 == false);
    }
    void testClient3()
    {
        /*
         * Set up a timer, allow it to fire.
         */
        _fired0 = _cancelled0 = false;

        _id0 = _client0->registerTimer(_handler0, 1000, this);
        CPPUNIT_ASSERT(_id0 > -1);
        CPPUNIT_ASSERT(_fired0 == false);
        CPPUNIT_ASSERT(_cancelled0 == false);

        sleep(2);

        CPPUNIT_ASSERT(_fired0 == true);
        CPPUNIT_ASSERT(_cancelled0 == false);
    }
    void testClient4()
    {
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

        _cancelled1 = _client0->cancelTimer(_id0);
        CPPUNIT_ASSERT(_fired0 == false);
        CPPUNIT_ASSERT(_cancelled0 == false);
    }
    void testClient5()
    {
        /*
         * Cancel a non-existent timer, see that it returns false.
         */
        _fired0 = _cancelled0 = false;

        _cancelled0 = _client0->cancelTimer((clusterlib::TimerId) 10001);
        CPPUNIT_ASSERT(_cancelled0 == false);
    }

    void testClient20()
    {
        /*
         * Register a user event handler.
         */
    }
    void testClient21()
    {
        /*
         * Register a user event handler twice,
         * see that second registration is silently
         * ignored.
         */
    }
    void testClient22()
    {
        /*
         * Register and cancel a user event handler,
         * see that cancellation is successful.
         */
    }
    void testClient23()
    {
        /*
         * Register a user event handler, then
         * cause the event and see that it fired
         * and the handler was called.
         */
    }
    void testClient24()
    {
        /*
         * Register a user event handler, then
         * cause the event 10 times, see that it
         * fires 10 times and that the handler was
         * called 10 times.
         */
    }
    void testClient25()
    {
        /*
         * Register a user event handler, then
         * cause the event 5 times, see that it
         * fired 5 times, cancel the handler and see
         * that it no longer fires for subsequent
         * events.
         */
    }
    void testClient26()
    {
        /*
         * Register two user event handlers, then
         * cause the event, see that both handlers
         * are called.
         */
    }
    void testClient27()
    {
        /*
         * Register two user event handlers, then
         * cause the event, see that both handlers
         * are called. Then cancel one, cause the
         * event and see that the other one is still
         * being called.
         */
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

    MyTimerEventHandler *_handler0;
};

void
MyTimerEventHandler::handleTimerEvent(clusterlib::TimerId id,
                                      clusterlib::ClientData data)
{
    ClusterlibClient *cp = (ClusterlibClient*) data;

    cp->setFired0(true);
}

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION( ClusterlibClient );

