#include "testparams.h"
#include "MPITestFixture.h"
#include <algorithm>
#include <sstream>

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;

const string appName = "unittests-propertylist-app";

class ClusterlibPropertyList : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibPropertyList);
    CPPUNIT_TEST(testGetPropertyList1);
    CPPUNIT_TEST(testGetPropertyList2);
    CPPUNIT_TEST(testGetPropertyList3);
    CPPUNIT_TEST(testGetPropertyList4);
    CPPUNIT_TEST(testGetPropertyList5);
    CPPUNIT_TEST(testGetPropertyList6);
    CPPUNIT_TEST(testGetPropertyList7);
    CPPUNIT_TEST(testGetPropertyList8);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibPropertyList() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client0(NULL),
          _app0(NULL),
          _group0(NULL),
          _node0(NULL),
          _propertyList0(NULL) {}

    /* Runs prior to each test */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client0 != NULL);
	_app0 = _client0->getRoot()->getApplication(appName, true);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup("propertyList-group-servers", true);
	MPI_CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", true);
	MPI_CPPUNIT_ASSERT(_node0 != NULL);
    }

    /* Runs after each test */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(_factory, true);
	delete _factory;
        _factory = NULL;
    }

    /* 
     * Simple test to see if propertyList set by one process are read by
     * another (when 2 processes or more are available).  If only one
     * process is available, runs as a single process test. 
     */
    void testGetPropertyList1()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetPropertyList1");

        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                true);
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            _propertyList0->setProperty("test", "v1");
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            _propertyList0 = _node0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            string val = _propertyList0->getProperty("test");
            MPI_CPPUNIT_ASSERT(val == "v1");
            cerr << "Got correct test = v1" << endl;
            _propertyList0->setProperty("test", "v2");
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
        }

        waitsForOrder(1, 0, _factory, true);

        if (isMyRank(0)) {
            _propertyList0->acquireLock();
            string val = _propertyList0->getProperty("test");
            _propertyList0->releaseLock();
            cerr << "Got value " << val << " (should be v2)" << endl;
            MPI_CPPUNIT_ASSERT(val == "v2");
            cerr << "Got correct test = v2" << endl;
        }
    }

    /* 
     * Try to set propertyList for a group and its node.  Make sure that
     * recursive propertyList works.  Tries to use 2 processes or more
     * are available.  If only one process is available, runs as a
     * single process test.
     */
    void testGetPropertyList2()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetPropertyList2");

        if (isMyRank(0)) { 
            _propertyList0 = _group0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                true);
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            _propertyList0->setProperty("test", "v3");
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                true);
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            _propertyList0->setProperty("test", "v4");
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            string val;
            _propertyList0 = _group0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            val = _propertyList0->getProperty("test");
            MPI_CPPUNIT_ASSERT(val == "v3");
            cerr << "Got correct test = v3" << endl;
            _propertyList0->releaseLock();
            
            _propertyList0 = _node0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            val = _propertyList0->getProperty("test");
            MPI_CPPUNIT_ASSERT(val == "v4");
            cerr << "Got correct test = v4" << endl;
            _propertyList0->deleteProperty("test");
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
            
            _propertyList0 = _app0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                true);
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            val = _propertyList0->getProperty("test");
            _propertyList0->setProperty("test", "v5");
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
        }

        waitsForOrder(1, 0, _factory, true);
        
        if (isMyRank(0)) {
            string val;
            _propertyList0 = _group0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            val = _propertyList0->getProperty("test");
            _propertyList0->releaseLock();
            
            cerr << "Got value " << val << " (should be v3)" << endl;
            MPI_CPPUNIT_ASSERT(val == "v3");
            cerr << "Got correct test = v3" << endl;
            
            _propertyList0 = _node0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            val = _propertyList0->getProperty("test", false);
            _propertyList0->releaseLock();
            
            cerr << "Got value " << val << " (should be empty)" << endl;
            MPI_CPPUNIT_ASSERT(val == "");
            cerr << "Got correct test = empty" << endl;
            
            _propertyList0->acquireLock();
            val = _propertyList0->getProperty("test", true);
            _propertyList0->releaseLock();
            
            cerr << "Got value " << val << " (should be v3)" << endl;
            MPI_CPPUNIT_ASSERT(val == "v3");
            cerr << "Got correct test = v3" << endl;
            
            _propertyList0 = _group0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            _propertyList0->deleteProperty("test");
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
        }
            
        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            string val;
            _propertyList0 = _node0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            val = _propertyList0->getProperty("test", true);
            _propertyList0->releaseLock();
            
            cerr << "Got value " << val << " (should be v5)" << endl;
            MPI_CPPUNIT_ASSERT(val == "v5");
            cerr << "Got correct test = v5" << endl;
        }
    }

    /* 
     * Ordering and lock protection test.  No process should see the
     * value of a property change while it is holding the distributed
     * lock. Also, the new property value should be evident to every
     * process at the end.
     */
    void testGetPropertyList3()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetPropertyList3");
        string prop = "prop3";
        string newValue = "prop3-newvalue";
        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                true);
            _propertyList0->acquireLock();
            _propertyList0->deleteProperty(prop);
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
        }

        barrier(_factory, true);

        _propertyList0 = _node0->getPropertyList();
        if (isMyRank(0)) {
            _propertyList0->acquireLock();
            _propertyList0->setProperty(prop, newValue);
            MPI_CPPUNIT_ASSERT(_propertyList0->getProperty(prop)
                               == newValue);
            _propertyList0->publishProperties();
            MPI_CPPUNIT_ASSERT(_propertyList0->getProperty(prop) 
                               == newValue);
            _propertyList0->releaseLock();
            MPI_CPPUNIT_ASSERT(_propertyList0->getProperty(prop)
                               == newValue);
        }
        else {
            _propertyList0->acquireLock();
            string value = _propertyList0->getProperty(prop);
            usleep(500000);
            string value2 = _propertyList0->getProperty(prop);
            _propertyList0->releaseLock();
            cerr << "testGetPropertyList3: value (" << value << "), value2 ("
                 << value2 << ")" << endl;
            MPI_CPPUNIT_ASSERT(value.compare(value2) == 0);
        }

        barrier(_factory, true);
        MPI_CPPUNIT_ASSERT(_propertyList0->getProperty(prop) == newValue);
    }

    /* 
     * Ordering.  If another process sets a property, another process
     * shouldn't see the results of that operation while it has the
     * distributed lock.
    */
    void testGetPropertyList4()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetPropertyList4");
        bool matchedVal = false;
        string prop = "prop4";
        /* Clean up the old propertyList */
        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                true);
            _propertyList0->acquireLock();
            _propertyList0->deleteProperty(prop);
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
        }

        barrier(_factory, true);
        _propertyList0 = _node0->getPropertyList(
            ClusterlibStrings::DEFAULTPROPERTYLIST,
            true);

        if (isMyRank(0)) {
            usleep(200000);
            _propertyList0->acquireLock();
            _propertyList0->setProperty(prop, "new value");
            MPI_CPPUNIT_ASSERT(_propertyList0->getProperty(prop) == "new value");
            _propertyList0->publishProperties();
            if (_propertyList0->getProperty(prop) == "new value") {
                matchedVal = true;
            }
            _propertyList0->releaseLock();
        }
        else if (isMyRank(1) || isMyRank(2) || isMyRank(3)) {
            /* 
             * Only let up to 3 processes do the test to prevent a
             * really long test time.
             */
            _propertyList0->acquireLock();
            string value = _propertyList0->getProperty(prop);
            usleep(500000);
            string value2 = _propertyList0->getProperty(prop);
            _propertyList0->releaseLock();
            if (value.compare(value2) == 0) {
                matchedVal = true;
            }
        }
        else {
            /* Everyone else is successful by default. */
            matchedVal = true;
        }

        barrier(_factory, true);
        MPI_CPPUNIT_ASSERT(matchedVal == true);
        MPI_CPPUNIT_ASSERT(_propertyList0->getProperty(prop) == "new value");
    }

    /*
     * Shouldn't cause deadlock even if clients hold more than one
     * distributed lock.
     */
    void testGetPropertyList5()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetPropertyList5");
        string prop = "prop5";
        /* Clean up the old propertyList */
        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                true);
            _propertyList0->acquireLock();
            _propertyList0->deleteProperty(prop);
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
        }

        barrier(_factory, true);
        _propertyList0 = _node0->getPropertyList(
            ClusterlibStrings::DEFAULTPROPERTYLIST,
            true);

        if (isMyRank(0)) {
            _propertyList0->acquireLock();
        }

        barrier(_factory, true);

        if (!isMyRank(0)) {
            _propertyList0->getProperty(prop);
            _propertyList0->setProperty(prop, "no");
            try {
                _propertyList0->publishProperties();
                cerr << "testGetPropertyList5: Published successfully" << endl;
            }
            catch (Exception e) {
                /* 
                 * Might not always succeed if lots of processes are
                 * trying (of course a lock would fix this =) )
                 */
                cerr << "testGetPropertyList5: Safe failure to publish" << endl;
            }
        }
        
        barrier(_factory, true);
        
        if (isMyRank(0)) {
            _group0->acquireLock();
            _group0->releaseLock();
            _propertyList0->releaseLock();
        }
    }

    /*
     * Simple test to create more than one propertyList object.
     */
    void testGetPropertyList6()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetPropertyList6");

        if (isMyRank(0)) {
            PropertyList *toolkitProp = _app0->getPropertyList("toolkit", true);
            PropertyList *kernelProp = _app0->getPropertyList("kernel", true);

            toolkitProp->acquireLock();
            toolkitProp->setProperty("clusterlib", "1.1");
            toolkitProp->publishProperties();
            toolkitProp->releaseLock();
            
            kernelProp->acquireLock();
            kernelProp->setProperty("linux", "2.4");
            kernelProp->publishProperties();
            kernelProp->releaseLock();
            
            kernelProp->acquireLock();
            NameList propList = _app0->getPropertyListNames();
            kernelProp->releaseLock();

            MPI_CPPUNIT_ASSERT(find(propList.begin(), propList.end(), 
                                    "toolkit") != propList.end());
            MPI_CPPUNIT_ASSERT(find(propList.begin(), propList.end(), 
                                    "kernel") != propList.end());
        }
    }

    /* 
     * Free for all set and get.  No one should see their value
     * change.
     */
    void testGetPropertyList7()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetPropertyList7");
        string prop = "prop7";
        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                true);
            _propertyList0->acquireLock();
            _propertyList0->deleteProperty(prop);
            _propertyList0->publishProperties();
            _propertyList0->releaseLock();
        }

        barrier(_factory, true);

        _propertyList0 = _node0->getPropertyList();

        stringstream ss;
        ss << getRank();
        _propertyList0->acquireLock();
        _propertyList0->setProperty(prop, ss.str());
        usleep(100000);
        MPI_CPPUNIT_ASSERT(_propertyList0->getProperty(prop)
                           == ss.str());
        _propertyList0->publishProperties();
        MPI_CPPUNIT_ASSERT(_propertyList0->getProperty(prop)
                           == ss.str());
        _propertyList0->releaseLock();

        _propertyList0->acquireLock();
        string value = _propertyList0->getProperty(prop);
        usleep(300000);
        string value2 = _propertyList0->getProperty(prop);
        _propertyList0->releaseLock();
        cerr << "testGetPropertyList7: value (" << value << "), value2 ("
             << value2 << ")" << endl;
        MPI_CPPUNIT_ASSERT(value.compare(value2) == 0);
    }

    /* 
     * No process should see the value of a property change while it
     * is holding the distributed lock (similar to
     * testGetPropertyList3). Also, the new property value should be
     * evident to every process at the end.  This should flush out any
     * issues with locking not getting all updates prior to lock by
     * updating several property lists and trying them all out.
     */
    void testGetPropertyList8()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testGetPropertyList8");
        string prop = "prop8";
        string newValue = "prop8-newvalue";
        string propList1Name = "propList1";
        string propList2Name = "propList2";
        string propList3Name = "propList3";
        PropertyList *propList1 = NULL;
        PropertyList *propList2 = NULL;
        PropertyList *propList3 = NULL;

        if (isMyRank(0)) {
            propList1 = _node0->getPropertyList(propList1Name, true);
            propList2 = _node0->getPropertyList(propList2Name, true);
            propList3 = _node0->getPropertyList(propList2Name, true);
            _node0->acquireLock();
            propList1->deleteProperty(prop);
            propList1->publishProperties();
            propList2->deleteProperty(prop);
            propList2->publishProperties();
            propList3->deleteProperty(prop);
            propList3->publishProperties();
            _node0->releaseLock();
        }

        barrier(_factory, true);

        propList1 = _node0->getPropertyList(propList1Name);
        propList2 = _node0->getPropertyList(propList2Name);
        propList3 = _node0->getPropertyList(propList2Name);
        MPI_CPPUNIT_ASSERT(propList1);
        MPI_CPPUNIT_ASSERT(propList2);
        MPI_CPPUNIT_ASSERT(propList3);
        if (isMyRank(0)) {
            _node0->acquireLock();
            propList1->setProperty(prop, newValue);
            MPI_CPPUNIT_ASSERT(propList1->getProperty(prop)
                               == newValue);
            propList2->setProperty(prop, newValue);
            MPI_CPPUNIT_ASSERT(propList2->getProperty(prop)
                               == newValue);
            propList3->setProperty(prop, newValue);
            MPI_CPPUNIT_ASSERT(propList3->getProperty(prop)
                               == newValue);
            /* Publish backward to make the error case the most likely */
            propList3->publishProperties();    
            propList2->publishProperties();
            propList1->publishProperties();     
            _node0->releaseLock();
            barrier(NULL, false);
            MPI_CPPUNIT_ASSERT(propList1->getProperty(prop)
                               == newValue);
            MPI_CPPUNIT_ASSERT(propList2->getProperty(prop)
                               == newValue);
            MPI_CPPUNIT_ASSERT(propList3->getProperty(prop)
                               == newValue);
        }
        else {
            barrier(NULL, false);
            _node0->acquireLock();
            string value1_1 = propList1->getProperty(prop);
            string value2_1 = propList2->getProperty(prop);
            string value3_1 = propList3->getProperty(prop);
            usleep(500000);
            string value1_2 = propList1->getProperty(prop);
            string value2_2 = propList2->getProperty(prop);
            string value3_2 = propList3->getProperty(prop);
            _node0->releaseLock();
            cerr << "testGetPropertyList8: value1_1 (" << value1_1
                 << "), value1_2 (" << value1_2 << ")" << endl;
            MPI_CPPUNIT_ASSERT(value1_1.compare(value1_2) == 0);
            cerr << "testGetPropertyList8: value2_1 (" << value2_1
                 << "), value2_2 (" << value2_2 << ")" << endl;
            MPI_CPPUNIT_ASSERT(value2_1.compare(value2_2) == 0);
            cerr << "testGetPropertyList8: value3_1 (" << value3_1
                 << "), value3_2 (" << value3_2 << ")" << endl;
            MPI_CPPUNIT_ASSERT(value3_1.compare(value3_2) == 0);
        }

        barrier(_factory, true);
        MPI_CPPUNIT_ASSERT(propList1->getProperty(prop) == newValue);
        MPI_CPPUNIT_ASSERT(propList2->getProperty(prop) == newValue);
        MPI_CPPUNIT_ASSERT(propList3->getProperty(prop) == newValue);
    }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Group *_group0;
    Node *_node0;
    PropertyList *_propertyList0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibPropertyList);

