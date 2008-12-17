#include "MPITestFixture.h"

class MyHealthChecker : public clusterlib::HealthChecker {
  public:
    virtual clusterlib::HealthReport checkHealth() {
        return clusterlib::HealthReport(clusterlib::HealthReport::HS_HEALTHY,
                                        "No real check");
    }
  private:
};

class ClusterlibProperties : public MPITestFixture {
    CPPUNIT_TEST_SUITE( ClusterlibProperties );
    CPPUNIT_TEST( testGetProperties1 );
    CPPUNIT_TEST_SUITE_END();

  public:
    /* Runs prior to all tests */
    virtual void setUp() 
    {
	_factory = new clusterlib::Factory("localhost:2181");
	CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	CPPUNIT_ASSERT(_client0 != NULL);
	_app0 = _client0->getApplication("foo-app", true);
	CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup("bar-servers", true);
	CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", true);
	CPPUNIT_ASSERT(_group0 != NULL);
    }

    /* Runs after all tests */
    virtual void tearDown() 
    {
	cout << "delete called " << endl;
	delete _factory;
    }

    void testGetProperties1()
    {
	barrier();
	/* Do not run if there less than 2 processes */
	if (getSize() >= 2) {
	    if (getRank() == 0) { 
		_properties0 = _node0->getProperties();
		CPPUNIT_ASSERT(_properties0);
		_properties0->acquireLock();
		_properties0->setProperty("test", "v1");
		_properties0->publish();
		_properties0->releaseLock();

	    }
	    waitsForOrder(0, 1);
	    if (getRank() == 1) {
		_properties0 = _node0->getProperties();
                string val = _properties0->getProperty("test");
		CPPUNIT_ASSERT(val == "v1");
		cout << "Got correct test = v1" << endl;
		_properties0->acquireLock();
                _properties0->setProperty("test", "v2");
                _properties0->publish();
                _properties0->releaseLock();
	    }
	    waitsForOrder(1, 0);
	    if (getRank() == 0) {
                string val = _properties0->getProperty("test");
                CPPUNIT_ASSERT(val == "v2");
                cout << "Got correct test = v2" << endl;
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
CPPUNIT_TEST_SUITE_REGISTRATION( ClusterlibProperties );

