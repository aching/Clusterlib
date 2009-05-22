#include "MPITestFixture.h"
#include "testparams.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;

/*
 * My user event handler.
 */
class MyUserEventHandler
    : public ClusterEventHandler
{
  public:
    /*
     * Constructor.
     */
    MyUserEventHandler(Notifyable *ntp,
                       Event mask,
                       ClientData cd)
        : ClusterEventHandler(ntp, mask, cd),
          m_counter(0)
    {
    }

    virtual void handleClusterEvent(Event e)
    {
        cerr << "Got event "
             << e
             << " on Notifyable "
             << getNotifyable()->getKey()
             << ", client data "
             << getClientData()
             << endl;
        m_counter++;
    }

    void reset() { m_counter = 0; }
    int32_t getCounter() { return m_counter; }

  private:
    /*
     * Counter for how many times the handler was
     * called.
     */
    int32_t m_counter;
};

/*
 * Health checker.
 */
class UEHealthChecker :
    public HealthChecker
{
  public:
    /*
     * Constructor.
     */
    UEHealthChecker(Node *np, Client *cp)
        : HealthChecker(),
          mp_np(np),
          mp_cp(cp) {}

    /*
     * Check health and return a health report.
     */
    virtual HealthReport checkHealth()
    {
        HealthReport hr(HealthReport::HS_HEALTHY,
                        mp_np->getName() + " is healthy!");

        return hr;
    }

  private:
    /*
     * The node for which we're reporting health.
     */
    Node *mp_np;

    /*
     * The client for which we're reporting health.
     */
    Client *mp_cp;    
};

/*
 * The test class.
 */
class ClusterlibUserEvents
    : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibUserEvents);

    CPPUNIT_TEST(testUserEvents1);
    CPPUNIT_TEST(testUserEvents2);
    CPPUNIT_TEST(testUserEvents3);
    CPPUNIT_TEST(testUserEvents4);
    CPPUNIT_TEST(testUserEvents5);
    CPPUNIT_TEST(testUserEvents6);

    CPPUNIT_TEST_SUITE_END();

  public:

    /*
     * Constructor.
     */
    ClusterlibUserEvents()
        : _factory(NULL),
          _client0(NULL),
          _app0(NULL),
          _grp0(NULL),
          _nod0(NULL),
          _dist0(NULL),
          _zk(NULL)
    {
    }

    /* Runs prior to each test */
    virtual void setUp() 
    {
	_factory =
            new Factory(globalTestParams.getZkServerPortList());

	CPPUNIT_ASSERT(_factory != NULL);
        _zk = _factory->getRepository();
        CPPUNIT_ASSERT(_zk != NULL);
	_client0 = _factory->createClient();
	CPPUNIT_ASSERT(_client0 != NULL);
        _app0 = _client0->getRoot()->getApplication("foo-app", true);
        CPPUNIT_ASSERT(_app0 != NULL);
        _grp0 = _app0->getGroup("bar-group", true);
        CPPUNIT_ASSERT(_grp0 != NULL);
        _nod0 = _grp0->getNode("nod3", true);
        CPPUNIT_ASSERT(_nod0 != NULL);
        _dist0 = _grp0->getDataDistribution("dist1", true);
        CPPUNIT_ASSERT(_dist0 != NULL);
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
        _app0 = NULL;
        _grp0 = NULL;
        _nod0 = NULL;
        _dist0 = NULL;
    }

    /*
     * The tests.
     */

    void testUserEvents1()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testUserEvents1");

        /*
         * Register & unregister once.
         */
        MyUserEventHandler *cehp = new MyUserEventHandler(_nod0,
                                                          EN_CONNECTED,
                                                          (void *) 0x3333);
        _client0->registerHandler(cehp);

        bool deregged = _client0->cancelHandler(cehp);
        CPPUNIT_ASSERT(deregged == true);

        deregged = _client0->cancelHandler(cehp);
        CPPUNIT_ASSERT(deregged == false);

        /*
         * Registering the same handler several times also works,
         * means it will be called several times!
         */
        _client0->registerHandler(cehp);
        _client0->registerHandler(cehp);
        _client0->registerHandler(cehp);

        /*
         * Cancelling several times also works...
         */
        deregged = _client0->cancelHandler(cehp);
        CPPUNIT_ASSERT(deregged == true);
        deregged = _client0->cancelHandler(cehp);
        CPPUNIT_ASSERT(deregged == true);
        deregged = _client0->cancelHandler(cehp);
        CPPUNIT_ASSERT(deregged == true);

        /*
         * Cancelling N+1 times does not work...
         */
        deregged = _client0->cancelHandler(cehp);
        CPPUNIT_ASSERT(deregged == false);
    }
    void testUserEvents2()
    {
        /*
         * This test checks that all handlers are
         * called if there are multiple handlers
         * registered for a user event.
         */
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testUserEvents2");

        /*
         * My handler.
         */
        MyUserEventHandler *cehp =
            new MyUserEventHandler(_nod0,
                                   EN_CONNECTED | EN_DISCONNECTED,
                                   (void *) 0x3333);
        UEHealthChecker *hcp = new UEHealthChecker(_nod0, _client0);

        /*
         * Register 3 times.
         */
        _client0->registerHandler(cehp);
        _client0->registerHandler(cehp);
        _client0->registerHandler(cehp);

        /*
         * Create a health checker, create the "connected" znode,
         * and in the process, cause the EN_CONNECTED event
         * to happen on the node.
         */
        _nod0->registerHealthChecker(hcp);

        /*
         * Wait for event propagation.
         */
        _factory->synchronize();
        sleep(1);

        /*
         * Event counter should be 3, since we registered the
         * handler 3 times.
         */
        CPPUNIT_ASSERT(cehp->getCounter() == 3);

        /*
         * Unregister the handler, we should get 3 more event
         * deliveries.
         */
        _nod0->unregisterHealthChecker();

        /*
         * Wait for event propagation.
         */
        _factory->synchronize();
        sleep(1);

        /*
         * Event counter should now be at 6.
         */
        CPPUNIT_ASSERT(cehp->getCounter() == 6);

        /*
         * Clean up.
         */
        delete cehp;
        delete hcp;
    }
    void testUserEvents3()
    {
        /*
         * This test checks that all handlers are
         * called if there are multiple handlers
         * registered for a user event.
         */
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testUserEvents3");

        /*
         * My handler.
         */
        MyUserEventHandler *cehp =
            new MyUserEventHandler(_nod0,
                                   EN_CONNECTED | EN_DISCONNECTED,
                                   (void *) 0x3333);
        UEHealthChecker *hcp = new UEHealthChecker(_nod0, _client0);

        /*
         * Register 3 times.
         */
        _client0->registerHandler(cehp);
        _client0->registerHandler(cehp);
        _client0->registerHandler(cehp);

        /*
         * Create a health checker, create the "connected" znode,
         * and in the process, cause the EN_CONNECTED event
         * to happen on the node.
         */
        _nod0->registerHealthChecker(hcp);

        /*
         * Wait for event propagation.
         */
        _factory->synchronize();
        sleep(1);

        /*
         * Event counter should be 3, since we registered the
         * handler 3 times.
         */
        CPPUNIT_ASSERT(cehp->getCounter() == 3);

        /*
         * Cancel *one* of the handlers. This means that the
         * connection events should be delivered to only 2 handlers.
         */
        bool cancelled = _client0->cancelHandler(cehp);
        CPPUNIT_ASSERT(cancelled == true);

        /*
         * Unregister the handler, we should get 2 more event
         * deliveries.
         */
        _nod0->unregisterHealthChecker();

        /*
         * Wait for event propagation.
         */
        _factory->synchronize();
        sleep(1);

        /*
         * Event counter should now be at 5.
         */
        CPPUNIT_ASSERT(cehp->getCounter() == 5);

        /*
         * Clean up.
         */
        delete cehp;
        delete hcp;
    }
    void testUserEvents4()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testUserEvents4");
    }
    void testUserEvents5()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testUserEvents5");
    }
    void testUserEvents6()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testUserEvents6");
    }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Group *_grp0;
    Node *_nod0;
    DataDistribution *_dist0;
    zk::ZooKeeperAdapter *_zk;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibUserEvents);

