#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;

const string appName = "unittests-endevent-app";

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

class ClusterlibEndEvent : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibEndEvent);
    CPPUNIT_TEST(testEndEvent1);
    CPPUNIT_TEST_SUITE_END();

  public:
    ClusterlibEndEvent() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client(NULL),
          _app(NULL) {}
    
    /**
     * Runs prior to each test 
     */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client != NULL);
        if (isMyRank(0)) {
            _app = _client->getRoot()->getApplication(appName);
            if (_app != NULL) {
                _app->remove(true);
            }
        }
        barrier(_factory, true);
	_app = _client->getRoot()->getApplication(appName, true);
	MPI_CPPUNIT_ASSERT(_app != NULL);
    }

    /** 
     * Runs after each test
     */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(NULL, false);
    }

    /**
     * One problem that we have observed is that if the health checker
     * or any other event causing operation is run after the
     * endEvent() is passed around (called by the destructor of
     * FactoryOps), failure is caused because of Zookeeper being not
     * available.  This should not happen, even if the user did not
     * remember to unregister their health checkers.
     */
    void testEndEvent1()
    {
        initializeAndBarrierMPITest(-1,
                                    true,
                                    _factory,
                                    true,
                                    "testEndEvent1");
        stringstream ss;
        ss << "node" << getRank();
        
        Node *myNode = _app->getNode(ss.str(), true);
        MPI_CPPUNIT_ASSERT(myNode != NULL);
        _checker = new MyHealthChecker();
        myNode->initializeConnection(true);
        myNode->registerHealthChecker(_checker);
        _checker->setMsecsPerCheckIfHealthy(1);
        _checker->setMsecsPerCheckIfUnhealthy(1);
        _checker->setMsecsAllowedPerHealthCheck(1);
        barrier(_factory, true);
        finishedClTest(_factory);
	delete _factory;
        delete _checker;
    }

  private:
    Factory *_factory;
    Client *_client;
    Application *_app;
    MyHealthChecker *_checker;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibEndEvent);

