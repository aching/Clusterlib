#include "clusterlib.h"
#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace std;
using namespace boost;
using namespace clusterlib;

const string appName = "unittests-client-app";

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

/*
 * The test class itself.
 */
class ClusterlibClient
    : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibClient);
    CPPUNIT_TEST(testClient1);
    CPPUNIT_TEST(testClient2);
    CPPUNIT_TEST(testClient3);
    CPPUNIT_TEST(testClient4);
    CPPUNIT_TEST(testClient5);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibClient()
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client0(NULL),
                    _id0(0),
          _fired0(false),
          _cancelled0(false) {}

    /* Called from the timer handler. */
    void setFired0(bool v) 
    { 
        clusterlib::Locker l(&_fired0Lock);
        _fired0 = v; 
    }

    bool getFired0()
    {
        clusterlib::Locker l(&_fired0Lock);
        return _fired0;
    }

    /* Runs prior to each test */
    virtual void setUp() 
    {
	_factory =
            new clusterlib::Factory(globalTestParams.getZkServerPortList());

	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client0 != NULL);
        _handler0 = new ClientTimerEventHandler();
        MPI_CPPUNIT_ASSERT(_handler0 != NULL);
    }

    /* Runs after each test */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(_factory, true);
        /*
         * Delete only the factory, that automatically deletes
         * all the other objects.
         */
	delete _factory;
        _factory = NULL;
        _client0 = NULL;
        
        /*
         * Delete my own data.
         */
        delete _handler0;
        _handler0 = NULL;
    }

    void testClient1()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient1");
       
        /*
         * Create applications with different names and
         * observe allowed creation and exceptions.
         */
        _app0 = _client0->getRoot()->getApplication(appName, 
                                                    CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(_app0 != NULL);
        /* Should not exist, didn't try to create, so return NULL */
        _app0 = _client0->getRoot()->getApplication("a", LOAD_FROM_REPOSITORY);
        MPI_CPPUNIT_ASSERT(_app0 == NULL);
        try {
            _app0 = _client0->getRoot()->getApplication("/frob", 
                                                        CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT("UNREACHABLE BECAUSE OF EXCEPTION" == NULL);
        } catch (clusterlib::InvalidArgumentsException &e) {
        }
    }
    void testClient2()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient2");
        cerr << "Test 2, cp == " << this << endl;

        /*
         * Set up a timer, cancel and observe that it did not
         * fire.
         */
        _fired0 = _cancelled0 = false;

        _id0 = _client0->registerTimer(_handler0, 500, this);
        MPI_CPPUNIT_ASSERT(_id0 > -1);
        MPI_CPPUNIT_ASSERT(getFired0() == false);
        MPI_CPPUNIT_ASSERT(_cancelled0 == false);

        _cancelled0 = _client0->cancelTimer(_id0);
        MPI_CPPUNIT_ASSERT(_cancelled0 == true);
        MPI_CPPUNIT_ASSERT(getFired0() == false);

        cerr << "Before sleep" << endl;
        sleep(1);
        cerr << "After sleep" << endl;

        MPI_CPPUNIT_ASSERT(_cancelled0 == true);
        MPI_CPPUNIT_ASSERT(getFired0() == false);
    }
    void testClient3()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient3");

        /*
         * Set up a timer, allow it to fire.
         */
        _fired0 = _cancelled0 = false;

        _id0 = _client0->registerTimer(_handler0, 500, this);
        MPI_CPPUNIT_ASSERT(_id0 > -1);
        MPI_CPPUNIT_ASSERT(getFired0() == false);
        MPI_CPPUNIT_ASSERT(_cancelled0 == false);

        cerr << "Before sleep" << endl;
        sleep(1);
        cerr << "After sleep" << endl;

        MPI_CPPUNIT_ASSERT(getFired0() == true);
        MPI_CPPUNIT_ASSERT(_cancelled0 == false);
    }
    void testClient4()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient4");

        /*
         * Set up a timer, cancel twice and observe that the 2nd
         * cancel returns false.
         */
        _fired0 = _cancelled0 = false;

        _id0 = _client0->registerTimer(_handler0, 1000, this);
        MPI_CPPUNIT_ASSERT(getFired0() == false);
        MPI_CPPUNIT_ASSERT(_cancelled0 == false);

        _cancelled0 = _client0->cancelTimer(_id0);
        MPI_CPPUNIT_ASSERT(getFired0() == false);
        MPI_CPPUNIT_ASSERT(_cancelled0 == true);
        cerr << "After first cancel" << endl;

        _cancelled1 = _client0->cancelTimer(_id0);
        MPI_CPPUNIT_ASSERT(getFired0() == false);
        MPI_CPPUNIT_ASSERT(_cancelled1 == false);
        cerr << "After second cancel" << endl;
    }
    void testClient5()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient5");

        /*
         * Cancel a non-existent timer, see that it returns false.
         */
        _fired0 = _cancelled0 = false;

        _cancelled0 = _client0->cancelTimer((clusterlib::TimerId) 10001);
        MPI_CPPUNIT_ASSERT(_cancelled0 == false);
    }

    void testClient20()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient20");

        /*
         * Register a user event handler.
         */
    }
    void testClient21()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient21");

        /*
         * Register a user event handler twice,
         * see that second registration is silently
         * ignored.
         */
    }
    void testClient22()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient22");

        /*
         * Register and cancel a user event handler,
         * see that cancellation is successful.
         */
    }
    void testClient23()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient23");

        /*
         * Register a user event handler, then
         * cause the event and see that it fired
         * and the handler was called.
         */
    }
    void testClient24()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient24");
        /*
         * Register a user event handler, then
         * cause the event 10 times, see that it
         * fires 10 times and that the handler was
         * called 10 times.
         */
    }
    void testClient25()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient25");

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
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient26");
        /*
         * Register two user event handlers, then
         * cause the event, see that both handlers
         * are called.
         */
    }
    void testClient27()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testClient27");

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
    shared_ptr<Application> _app0;

    clusterlib::TimerId _id0;
    clusterlib::TimerId _id1;

    bool _fired0;
    /**
     * Makes _fired0 thread-safe;
     */
    clusterlib::Mutex _fired0Lock;

    bool _cancelled0;
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
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibClient);

