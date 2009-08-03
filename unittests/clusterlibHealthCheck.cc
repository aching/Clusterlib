#include "MPITestFixture.h"
#include "testparams.h"
#include "clusterlibinternal.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;

const string appName = "healthCheck-app";

class MyHealthChecker : public HealthChecker {
  public:
    MyHealthChecker() 
        : m_healthy(true) {}
    virtual clusterlib::HealthReport checkHealth() {
        if (m_healthy) {
            return clusterlib::HealthReport(
                clusterlib::HealthReport::HS_HEALTHY, 
                "set by member function");
        }
        else {
            return clusterlib::HealthReport(
                clusterlib::HealthReport::HS_UNHEALTHY,
                "set by member function");
        }
    }
    void setHealth(bool healthy) {
        m_healthy = healthy;
    }
  private:
    bool m_healthy;
};

class ClusterlibHealthCheck : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibHealthCheck);
    CPPUNIT_TEST(testHealthCheck1);
    CPPUNIT_TEST(testHealthCheck2);
    CPPUNIT_TEST(testHealthCheck3);
    CPPUNIT_TEST_SUITE_END();

  public:
    ClusterlibHealthCheck() : _factory(NULL) {}
    
    /**
     * Runs prior to each test 
     */
    virtual void setUp() 
    {
        _checker = new MyHealthChecker();
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client0 != NULL);
	_app0 = _client0->getRoot()->getApplication(appName, true);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup("servers", true);
	MPI_CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", true);
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
        delete _checker;
        _checker = NULL;
    }

    /** 
     * Simple test to try register a health checker on a node.
     */
    void testHealthCheck1()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testHealthCheck1");
        
        if (isMyRank(0)) {
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::READY);
            _node0->remove(true);
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::REMOVED);
            /*
             * This synchronization is required since it is currently
             * possible to have a removed event handler destroy a new
             * Notifyable by accident.  This won't be possible when we
             * fix the clusterlib event system to get the Notifyable *
             * each zk event is linked to.
             */
            _factory->synchronize();
            _node0 = _group0->getNode("server-0", true);
            MPI_CPPUNIT_ASSERT(_node0);
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::READY);
            _checker->setMsecsPerCheckIfHealthy(10);
            _checker->setMsecsPerCheckIfUnhealthy(10);
            MPI_CPPUNIT_ASSERT(_node0->isHealthy() == false);
            _node0->registerHealthChecker(_checker);
            sleep(1);
            /*
             * Since the time to check is 10 ms, 1 second should be
             * enough time to get the health updated in the cache.
             */
            MPI_CPPUNIT_ASSERT(_node0->isHealthy() == true);
            _node0->unregisterHealthChecker();
        }
    }

    /** 
     * Simple test for to make sure that error cases are handled.
     */
    void testHealthCheck2()
    {
        initializeAndBarrierMPITest(-1, 
                                    true,
                                    _factory, 
                                    true, 
                                    "testHealthCheck2");
        if (isMyRank(0)) {
            try {
                _node0->unregisterHealthChecker();
                MPI_CPPUNIT_ASSERT("Shouldn't have been successful" == 0);
            }
            catch (InvalidMethodException &e) {
            }
            _node0->registerHealthChecker(_checker);
            /* Even though unregisterHealthChecker() is not called,
             * things should be cleaned up wihtout any exceptions or
             * memory leaks */
        }
        
    }

    /** 
     * Test for two processes. One process gets the health checker on
     * the node, then the other will monitor health to see the
     * changes.
     */
    void testHealthCheck3()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testHealthCheck3");

        
        if (isMyRank(0)) {
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::READY);
            _node0->remove(true);
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::REMOVED);
            /*
             * This synchronization is required since it is currently
             * possible to have a removed event handler destroy a new
             * Notifyable by accident.  This won't be possible when we
             * fix the clusterlib event system to get the Notifyable *
             * each zk event is linked to.
             */
            _factory->synchronize();
            _node0 = _group0->getNode("server-0", true);
            MPI_CPPUNIT_ASSERT(_node0);
            MPI_CPPUNIT_ASSERT(_node0->getState() == Notifyable::READY);
            _checker->setMsecsPerCheckIfHealthy(10);
            _checker->setMsecsPerCheckIfUnhealthy(10);
            MPI_CPPUNIT_ASSERT(_node0->isHealthy() == false);
            _node0->registerHealthChecker(_checker);
            sleep(1);
            /*
             * Since the time to check is 10 ms, 1 second should be
             * enough time to get the health updated in the cache.
             */
            MPI_CPPUNIT_ASSERT(_node0->isHealthy() == true);

        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            _node0 = _group0->getNode("server-0");
            MPI_CPPUNIT_ASSERT(_node0);
            MPI_CPPUNIT_ASSERT(_node0->isHealthy() == true);
        }
            
        waitsForOrder(1, 0, _factory, true);
            
        if (isMyRank(0)) {
            _checker->setHealth(false);
            sleep(1);
            MPI_CPPUNIT_ASSERT(_node0->isHealthy() == false);
        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            MPI_CPPUNIT_ASSERT(_node0->isHealthy() == false);
        }

        if (isMyRank(0)) {
            _node0->unregisterHealthChecker();
        }
   }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Group *_group0;
    Node *_node0;
    MyHealthChecker *_checker;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibHealthCheck);

