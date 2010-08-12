#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace std;
using namespace boost;
using namespace clusterlib;

const string appName = "unittests-timer-app";

/*
 * Helper class that counts how many times a timer fired.
 */
class MyTimerEventHandler
    : public TimerEventHandler
{
  public:
    /*
     * Constructor.
     */
    MyTimerEventHandler() : counter(0) {}

    /*
     * Utility method to reset the counter.
     */
    void resetCounter()
    {
        Locker l(&counterLock);
        counter = 0;
    }

    /*
     * Utility method to retrieve the counter.
     */
    int32_t getCounter()
    {
        Locker l(&counterLock);
        return counter;
    }

    /*
     * Actually handle the timer event, just increment
     * our counter.
     */
    virtual void handleTimerEvent(TimerId id, ClientData data)
    {
        Locker l(&counterLock);
        ++counter;
    }
  private:
    /**
     * Makes counter thread-safe.
     */ 
    Mutex counterLock;

    /*
     * Count how many times a timer event was fired.
     */
    int32_t counter;
};

/*
 * The test class itself.
 */
class ClusterlibTimer
    : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibTimer);
    CPPUNIT_TEST(testTimer1);
    CPPUNIT_TEST(testTimer2);
    CPPUNIT_TEST(testTimer3);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibTimer()
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client0(NULL),
          _zk(NULL),
          _timer0(NULL)
    {
    }

    /* Runs prior to each test */
    virtual void setUp() 
    {
	_factory =
            new Factory(globalTestParams.getZkServerPortList());

	MPI_CPPUNIT_ASSERT(_factory != NULL);
        _zk = _factory->getRepository();
        MPI_CPPUNIT_ASSERT(_zk != NULL);
	_client0 = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client0 != NULL);
        _app0 = _client0->getRoot()->getApplication(
            appName, CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(_app0 != NULL);
        _grp0 = _app0->getGroup("bar-group", CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(_grp0 != NULL);
        _nod0 = _grp0->getNode("nod3", CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(_nod0 != NULL);
        _dist0 = _grp0->getDataDistribution("dist1", CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(_dist0 != NULL);
        _timer0 = new MyTimerEventHandler();
        MPI_CPPUNIT_ASSERT(_timer0 != NULL);
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
        _zk = NULL;
        delete _timer0;
        _timer0 = NULL;
    }

    void testTimer1()
    {
        initializeAndBarrierMPITest(-1,
                                    true,
                                    _factory,
                                    true,
                                    "testTimer1");
        MPI_CPPUNIT_ASSERT(_timer0->getCounter() == 0);
        TimerId id1 = _client0->registerTimer(_timer0, 5000, (ClientData) NULL);
        bool cancelled = _client0->cancelTimer(id1);
        MPI_CPPUNIT_ASSERT(cancelled == true);
        cancelled = _client0->cancelTimer(id1);
        MPI_CPPUNIT_ASSERT(cancelled == false);
        cerr << "counter=" << _timer0->getCounter() << endl;
        MPI_CPPUNIT_ASSERT(_timer0->getCounter() == 0);
    }

    void testTimer2()
    {
        initializeAndBarrierMPITest(-1,
                                    true,
                                    _factory,
                                    true,
                                    "testTimer2");
        (void) _client0->registerTimer(_timer0, 200, (ClientData) NULL);
        sleep(1);
        MPI_CPPUNIT_ASSERT(_timer0->getCounter() == 1);
        (void) _client0->registerTimer(_timer0, 200, (ClientData) NULL);
        (void) _client0->registerTimer(_timer0, 200, (ClientData) NULL);
        (void) _client0->registerTimer(_timer0, 200, (ClientData) NULL);
        sleep(1);
        cerr << "counter=" << _timer0->getCounter() << endl;
        MPI_CPPUNIT_ASSERT(_timer0->getCounter() == 4);
    }

    void testTimer3()
    {
        initializeAndBarrierMPITest(-1,
                                    true,
                                    _factory,
                                    true,
                                    "testTimer3");
        (void) _client0->registerTimer(_timer0, 200, (ClientData) NULL);
        TimerId id2 = 
            _client0->registerTimer(_timer0, 5000, (ClientData) NULL);
        (void) _client0->registerTimer(_timer0, 200, (ClientData) NULL);
        bool cancelled = _client0->cancelTimer(id2);
        MPI_CPPUNIT_ASSERT(cancelled == true);
        sleep(1);
        cerr << "counter=" << _timer0->getCounter() << endl;
        MPI_CPPUNIT_ASSERT(_timer0->getCounter() == 2);
    }

  private:
    Factory *_factory;
    Client *_client0;
    shared_ptr<Application> _app0;
    shared_ptr<Group> _grp0;
    shared_ptr<Node> _nod0;
    shared_ptr<DataDistribution> _dist0;
    zk::ZooKeeperAdapter *_zk;
    MyTimerEventHandler *_timer0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibTimer);
