#include "MPITestFixture.h"
#include "testparams.h"

extern TestParams globalTestParams;

class MyHealthChecker : public clusterlib::HealthChecker {
  public:
    virtual clusterlib::HealthReport checkHealth() {
        return clusterlib::HealthReport(clusterlib::HealthReport::HS_HEALTHY,
                                        "No real check");
    }
  private:
};

class ClusterlibProperties : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibProperties);
    CPPUNIT_TEST(testGetProperties1);
    CPPUNIT_TEST(testGetProperties2);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibProperties() : _factory(NULL),
                             _client0(NULL),
                             _app0(NULL),
                             _group0(NULL),
                             _node0(NULL),
                             _properties0(NULL) {}

    /* Runs prior to all tests */
    virtual void setUp() 
    {
	_factory = new clusterlib::Factory(
            globalTestParams.getZkServerPortList());
	CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	CPPUNIT_ASSERT(_client0 != NULL);
	_app0 = _client0->getApplication("properties-app", true);
	CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup("properties-group-servers", true);
	CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", true);
	CPPUNIT_ASSERT(_node0 != NULL);
    }

    /* Runs after all tests */
    virtual void tearDown() 
    {
	cerr << "delete called " << endl;
	delete _factory;
        _factory = NULL;
    }

    /* Simple test to see if properties set by one process are read by
     * another. */
    void testGetProperties1()
    {
	barrier();
	/* Do not run if there less than 2 processes */
	if (getSize() >= 2) {
	    if (getRank() == 0) { 
		_properties0 = _node0->getProperties(true);
		CPPUNIT_ASSERT(_properties0);
		_properties0->acquireLock();
		_properties0->setProperty("test", "v1");
		_properties0->publish();
		_properties0->releaseLock();

	    }
	    waitsForOrder(_factory, 0, 1);
	    if (getRank() == 1) {
		_properties0 = _node0->getProperties();
		_properties0->acquireLock();
                string val = _properties0->getProperty("test");
		CPPUNIT_ASSERT(val == "v1");
		cerr << "Got correct test = v1" << endl;
                _properties0->setProperty("test", "v2");
		_properties0->publish();
                _properties0->releaseLock();
	    }
	    waitsForOrder(_factory, 1, 0);
	    if (getRank() == 0) {
		_properties0->acquireLock();
                string val = _properties0->getProperty("test");
                _properties0->releaseLock();
		cerr << "Got value " << val << " (should be v2)" << endl;
                CPPUNIT_ASSERT(val == "v2");
                cerr << "Got correct test = v2" << endl;
	    }
	}
    }

    /* Try to set properties for a group and its node.  Make sure that
     * recursive properties works. */
    void testGetProperties2()
    {
	barrier();
	/* Do not run if there less than 2 processes */
	if (getSize() >= 2) {
	    if (getRank() == 0) { 
		_properties0 = _group0->getProperties();
		CPPUNIT_ASSERT(_properties0);
		_properties0->acquireLock();
		_properties0->setProperty("test", "v3");
		_properties0->publish();
		_properties0->releaseLock();
		_properties0 = _node0->getProperties();
		CPPUNIT_ASSERT(_properties0);
		_properties0->acquireLock();
		_properties0->setProperty("test", "v4");
		_properties0->publish();
		_properties0->releaseLock();
	    }
	    waitsForOrder(_factory, 0, 1);
	    if (getRank() == 1) {
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

		_properties0 = _app0->getProperties();
		CPPUNIT_ASSERT(_properties0);
		_properties0->acquireLock();
                val = _properties0->getProperty("test");
                _properties0->setProperty("test", "v5");
                _properties0->publish();
                _properties0->releaseLock();
	    }
	    waitsForOrder(_factory, 1, 0);
	    if (getRank() == 0) {
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
	    waitsForOrder(_factory, 0, 1);
	    if (getRank() == 1) {
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
    }

  private:
    clusterlib::Factory *_factory;
    clusterlib::Client *_client0;
    clusterlib::Application *_app0;
    clusterlib::Group *_group0;
    clusterlib::Node *_node0;
    clusterlib::Properties *_properties0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibProperties);

