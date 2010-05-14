#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;
using namespace json;

const string appName = "unittests-endevent-app";

class ClusterlibEndEvent : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibEndEvent);
    CPPUNIT_TEST(testEndEvent1);
    CPPUNIT_TEST_SUITE_END();

  public:
    class HealthUpdater : public Periodic {
      public:
        HealthUpdater(int64_t msecsFrequency, 
                      Notifyable *notifyable, 
                      ClientData *clientData)
            : Periodic(msecsFrequency, notifyable, clientData) {}
        
        virtual void run() 
        {
            Notifyable *notifyable = getNotifyable();
            
            assert(notifyable != NULL);
            
            JSONValue jsonHealth;
            notifyable->cachedCurrentState().get(Node::HEALTH_KEY, jsonHealth);
        }    
    };

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
	_app = _client->getRoot()->getApplication(appName, CREATE_IF_NOT_FOUND);
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
        
        Node *myNode = _app->getNode(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(myNode != NULL);
        myNode->acquireOwnership();
        HealthUpdater updater(1, myNode, NULL);
        _factory->registerPeriodicThread(updater);

        /*
         * Shouldn't be used this way, but will be in this case to
         * test for the end event 
         */
        myNode->releaseOwnership();
        barrier(_factory, true);
        finishedClTest(_factory);
	delete _factory;
    }

  private:
    Factory *_factory;
    Client *_client;
    Application *_app;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibEndEvent);
