#include "MPITestFixture.h"
#include "testparams.h"

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;

class ClusterlibProperties : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibProperties);
    CPPUNIT_TEST(testGetProperties1);
    CPPUNIT_TEST(testGetProperties2);
    CPPUNIT_TEST(testGetProperties3);
    CPPUNIT_TEST(testGetProperties4);
    CPPUNIT_TEST(testGetProperties5);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibProperties() : _factory(NULL),
                             _client0(NULL),
                             _app0(NULL),
                             _group0(NULL),
                             _node0(NULL),
                             _properties0(NULL) {}

    /* Runs prior to each test */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	CPPUNIT_ASSERT(_client0 != NULL);
	_app0 = _client0->getRoot()->getApplication("properties-app", true);
	CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup("properties-group-servers", true);
	CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", true);
	CPPUNIT_ASSERT(_node0 != NULL);
    }

    /* Runs after each test */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(_factory, true);
	delete _factory;
        _factory = NULL;
    }

    /* 
     * Simple test to see if properties set by one process are read by
     * another (when 2 processes or more are available).  If only one
     * process is available, runs as a single process test. 
     */
    void testGetProperties1()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetProperties1");

        if (isMyRank(0)) {
            _properties0 = _node0->getProperties(true);
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            _properties0->setProperty("test", "v1");
            _properties0->publish();
            _properties0->releaseLock();
        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            _properties0 = _node0->getProperties();
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            string val = _properties0->getProperty("test");
            CPPUNIT_ASSERT(val == "v1");
            cerr << "Got correct test = v1" << endl;
            _properties0->setProperty("test", "v2");
            _properties0->publish();
            _properties0->releaseLock();
        }

        waitsForOrder(1, 0, _factory, true);

        if (isMyRank(0)) {
            _properties0->acquireLock();
            string val = _properties0->getProperty("test");
            _properties0->releaseLock();
            cerr << "Got value " << val << " (should be v2)" << endl;
            CPPUNIT_ASSERT(val == "v2");
            cerr << "Got correct test = v2" << endl;
        }
    }

    /* 
     * Try to set properties for a group and its node.  Make sure that
     * recursive properties works.  Tries to use 2 processes or more
     * are available.  If only one process is available, runs as a
     * single process test.
     */
    void testGetProperties2()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetProperties2");

        if (isMyRank(0)) { 
            _properties0 = _group0->getProperties(true);
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            _properties0->setProperty("test", "v3");
            _properties0->publish();
            _properties0->releaseLock();
            _properties0 = _node0->getProperties(true);
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            _properties0->setProperty("test", "v4");
            _properties0->publish();
            _properties0->releaseLock();
        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            string val;
            _properties0 = _group0->getProperties();
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            val = _properties0->getProperty("test");
            CPPUNIT_ASSERT(val == "v3");
            cerr << "Got correct test = v3" << endl;
            _properties0->releaseLock();
            
            _properties0 = _node0->getProperties();
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            val = _properties0->getProperty("test");
            CPPUNIT_ASSERT(val == "v4");
            cerr << "Got correct test = v4" << endl;
            _properties0->deleteProperty("test");
            _properties0->publish();
            _properties0->releaseLock();
            
            _properties0 = _app0->getProperties(true);
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            val = _properties0->getProperty("test");
            _properties0->setProperty("test", "v5");
            _properties0->publish();
            _properties0->releaseLock();
        }

        waitsForOrder(1, 0, _factory, true);
        
        if (isMyRank(0)) {
            string val;
            _properties0 = _group0->getProperties();
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            val = _properties0->getProperty("test");
            _properties0->releaseLock();
            
            cerr << "Got value " << val << " (should be v3)" << endl;
            CPPUNIT_ASSERT(val == "v3");
            cerr << "Got correct test = v3" << endl;
            
            _properties0 = _node0->getProperties();
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            val = _properties0->getProperty("test", false);
            _properties0->releaseLock();
            
            cerr << "Got value " << val << " (should be empty)" << endl;
            CPPUNIT_ASSERT(val == "");
            cerr << "Got correct test = empty" << endl;
            
            _properties0->acquireLock();
            val = _properties0->getProperty("test", true);
            _properties0->releaseLock();
            
            cerr << "Got value " << val << " (should be v3)" << endl;
            CPPUNIT_ASSERT(val == "v3");
            cerr << "Got correct test = v3" << endl;
            
            _properties0 = _group0->getProperties();
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            _properties0->deleteProperty("test");
            _properties0->publish();
            _properties0->releaseLock();
        }
            
        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            string val;
            _properties0 = _node0->getProperties();
            CPPUNIT_ASSERT(_properties0);
            _properties0->acquireLock();
            val = _properties0->getProperty("test", true);
            _properties0->releaseLock();
            
            cerr << "Got value " << val << " (should be v5)" << endl;
            CPPUNIT_ASSERT(val == "v5");
            cerr << "Got correct test = v5" << endl;
        }
    }

    /* 
     * Ordering and lock protection test.  No process should see the
     * value of a property change while it is holding the distributed
     * lock. Also, the new property value should be evident to every
     * process at the end.
     */
    void testGetProperties3()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetProperties3");
        string prop = "prop3";
        if (isMyRank(0)) {
            _properties0 = _node0->getProperties(true);
            _properties0->deleteProperty(prop);
        }

        barrier(_factory, true);

        _properties0 = _node0->getProperties();
        if (isMyRank(0)) {
            _properties0->acquireLock();
            _properties0->setProperty(prop, "new value");
            CPPUNIT_ASSERT(_properties0->getProperty(prop) == "new value");
            _properties0->publish();
            CPPUNIT_ASSERT(_properties0->getProperty(prop) == "new value");
            _properties0->releaseLock();
            CPPUNIT_ASSERT(_properties0->getProperty(prop) == "new value");
        }
        else {
            _properties0->acquireLock();
            string value = _properties0->getProperty(prop);
            usleep(500000);
            string value2 = _properties0->getProperty(prop);
            _properties0->releaseLock();
            CPPUNIT_ASSERT(value.compare(value2) == 0);
        }

        barrier(_factory, true);
        CPPUNIT_ASSERT(_properties0->getProperty(prop) == "new value");
    }

    /* 
     * Ordering.  If another process sets a property, another process
     * shouldn't see the results of that operation while it has the
     * distributed lock.
    */
    void testGetProperties4()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetProperties4");
        bool matchedVal = false;
        string prop = "prop4";
        /* Clean up the old properties */
        if (isMyRank(0)) {
            _properties0 = _node0->getProperties(true);
            _properties0->acquireLock();
            _properties0->deleteProperty(prop);
            _properties0->publish();
            _properties0->releaseLock();
        }

        barrier(_factory, true);
        _properties0 = _node0->getProperties(true);

        if (isMyRank(0)) {
            usleep(200000);
            _properties0->acquireLock();
            _properties0->setProperty(prop, "new value");
            CPPUNIT_ASSERT(_properties0->getProperty(prop) == "new value");
            _properties0->publish();
            if (_properties0->getProperty(prop) == "new value") {
                matchedVal = true;
            }
            _properties0->releaseLock();
        }
        else if (isMyRank(1) || isMyRank(2) || isMyRank(3)) {
            /* 
             * Only let up to 3 processes do the test to prevent a
             * really long test time.
             */
            _properties0->acquireLock();
            string value = _properties0->getProperty(prop);
            usleep(500000);
            string value2 = _properties0->getProperty(prop);
            _properties0->releaseLock();
            if (value.compare(value2) == 0) {
                matchedVal = true;
            }
        }
        else {
            /* Everyone else is successful by default. */
            matchedVal = true;
        }

        barrier(_factory, true);
        CPPUNIT_ASSERT(matchedVal == true);
        CPPUNIT_ASSERT(_properties0->getProperty(prop) == "new value");
    }

    /*
     * Shouldn't cause deadlock even if clients hold more than one
     * distributed lock.
     */
    void testGetProperties5()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetProperties5");
        string prop = "prop5";
        /* Clean up the old properties */
        if (isMyRank(0)) {
            _properties0 = _node0->getProperties(true);
            _properties0->acquireLock();
            _properties0->deleteProperty(prop);
            _properties0->publish();
            _properties0->releaseLock();
        }

        barrier(_factory, true);
        _properties0 = _node0->getProperties(true);

        if (isMyRank(0)) {
            _properties0->acquireLock();
        }

        barrier(_factory, true);

        if (!isMyRank(0)) {
            _properties0->getProperty(prop);
            _properties0->setProperty(prop, "no");
            try {
                _properties0->publish();
                cerr << "testGetProperties5: Published successfully" << endl;
            }
            catch (Exception e) {
                /* 
                 * Might not always succeed if lots of processes are
                 * trying (of course a lock would fix this =) )
                 */
                cerr << "testGetProperties5: Safe failure to publish" << endl;
            }
        }
        
        barrier(_factory, true);
        
        if (isMyRank(0)) {
            _group0->acquireLock();
            _group0->releaseLock();
            _properties0->releaseLock();
        }
    }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Group *_group0;
    Node *_node0;
    Properties *_properties0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibProperties);

