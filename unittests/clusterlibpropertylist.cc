#include "testparams.h"
#include "MPITestFixture.h"
#include <algorithm>
#include <sstream>

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;
using namespace json;

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
	_app0 = _client0->getRoot()->getApplication(
            appName, CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
	_group0 = _app0->getGroup(
            "propertyList-group-servers", CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_group0 != NULL);
	_node0 = _group0->getNode("server-0", CREATE_IF_NOT_FOUND);
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
        JSONValue jsonValue;
        bool found;

        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().set("test", "v1");
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            _propertyList0 = _node0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            found = _propertyList0->cachedKeyValues().get("test", jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() == "v1");
            cerr << "Got correct test = v1" << endl;
            _propertyList0->cachedKeyValues().set("test", "v2");
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
        }

        waitsForOrder(1, 0, _factory, true);

        if (isMyRank(0)) {
            _propertyList0->acquireLock();
            found = _propertyList0->cachedKeyValues().get("test", jsonValue);
            _propertyList0->releaseLock();
            cerr << "Got value " << jsonValue.get<JSONValue::JSONString>() 
                 << " (should be v2)" << endl;
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() == "v2");
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
        JSONValue jsonValue;
        bool found;

        if (isMyRank(0)) { 
            _propertyList0 = _group0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().set("test", "v3");
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().set("test", "v4");
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
        }

        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            _propertyList0 = _group0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            found = _propertyList0->cachedKeyValues().get("test", jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() == "v3");
            cerr << "Got correct test = v3" << endl;
            _propertyList0->releaseLock();
            
            _propertyList0 = _node0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            found = _propertyList0->cachedKeyValues().get("test", jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() == "v4");
            cerr << "Got correct test = v4" << endl;
            _propertyList0->cachedKeyValues().erase("test");
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
            
            _propertyList0 = _app0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            found =_propertyList0->cachedKeyValues().get("test", jsonValue);
            _propertyList0->cachedKeyValues().set("test", "v5");
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
        }

        waitsForOrder(1, 0, _factory, true);
        
        if (isMyRank(0)) {
            _propertyList0 = _group0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            found =_propertyList0->cachedKeyValues().get("test", jsonValue);
            _propertyList0->releaseLock();
            
            cerr << "Got value " << jsonValue.get<JSONValue::JSONString>() 
                 << " (should be v3)" << endl;
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() == "v3");
            cerr << "Got correct test = v3" << endl;
            
            _propertyList0 = _node0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            found =_propertyList0->cachedKeyValues().get(
                "test", jsonValue, false);
            _propertyList0->releaseLock();
            
            cerr << "Found " << found
                 << " (should be false)" << endl;
            MPI_CPPUNIT_ASSERT(found == false);
            cerr << "Got correct test = empty" << endl;
            
            _propertyList0->acquireLock();
            found =_propertyList0->cachedKeyValues().get(
                "test", jsonValue, true);
            _propertyList0->releaseLock();
            
            cerr << "Got value " << jsonValue.get<JSONValue::JSONString>() 
                 << " (should be v3)" << endl;
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() == "v3");
            cerr << "Got correct test = v3" << endl;
            
            _propertyList0 = _group0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().erase("test");
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
        }
            
        waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            string val;
            _propertyList0 = _node0->getPropertyList();
            MPI_CPPUNIT_ASSERT(_propertyList0);
            _propertyList0->acquireLock();
            found =_propertyList0->cachedKeyValues().get(
                "test", jsonValue, true);
            _propertyList0->releaseLock();
            
            cerr << "Got value " << jsonValue.get<JSONValue::JSONString>() 
                 << " (should be v5)" << endl;
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() == "v5");
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
        JSONValue jsonValue, jsonValue2;
        bool found, found2;
        string prop = "prop3";
        string newValue = "prop3-newvalue";
        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().erase(prop);
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
        }

        barrier(_factory, true);

        _propertyList0 = _node0->getPropertyList();
        if (isMyRank(0)) {
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().set(prop, newValue);
            _propertyList0->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                               == newValue);
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                               == newValue);
            _propertyList0->cachedKeyValues().get(prop, jsonValue);
            _propertyList0->releaseLock();
            _propertyList0->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                               == newValue);
        }
        else {
            _propertyList0->acquireLock();
            found =_propertyList0->cachedKeyValues().get(prop, jsonValue);
            usleep(500000);
            found2 = _propertyList0->cachedKeyValues().get(prop, jsonValue2);
            _propertyList0->releaseLock();
            if (found) {
                cerr << "testGetPropertyList3: value (" 
                     << jsonValue.get<JSONValue::JSONString>() << endl;
            }
            if (found2) {
                cerr << "testGetPropertyList3: value (" 
                     << jsonValue2.get<JSONValue::JSONString>() << endl;
            }
            if (found == true && found2 == true) {
                MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() == 
                                   jsonValue2.get<JSONValue::JSONString>());
            }
            MPI_CPPUNIT_ASSERT(found == found2);
        }

        barrier(_factory, true);

        _propertyList0->cachedKeyValues().get(prop, jsonValue);
        MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                           == newValue);
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
        JSONValue jsonValue, jsonValue2;
        bool found, found2;
        bool matchedVal = false;
        string prop = "prop4";
        /* Clean up the old propertyList */
        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().erase(prop);
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
        }

        barrier(_factory, true);
        _propertyList0 = _node0->getPropertyList(
            ClusterlibStrings::DEFAULTPROPERTYLIST,
            CREATE_IF_NOT_FOUND);

        if (isMyRank(0)) {
            usleep(200000);
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().set(prop, "new value");
            found = _propertyList0->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                               == "new value");
            _propertyList0->cachedKeyValues().publish();
            found = _propertyList0->cachedKeyValues().get(prop, jsonValue);
            if (jsonValue.get<JSONValue::JSONString>() == "new value") {
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
            found =_propertyList0->cachedKeyValues().get(prop, jsonValue);
            usleep(500000);
            found2 = _propertyList0->cachedKeyValues().get(prop, jsonValue2);
            _propertyList0->releaseLock();
            if (found) {
                cerr << "testGetPropertyList4: value (" 
                     << jsonValue.get<JSONValue::JSONString>() << endl;
            }
            if (found2) {
                cerr << "testGetPropertyList4: value (" 
                     << jsonValue2.get<JSONValue::JSONString>() << endl;
            }
            if (found == true && found2 == true) {
                if (jsonValue.get<JSONValue::JSONString>() == 
                    jsonValue2.get<JSONValue::JSONString>()) {
                    matchedVal = true;
                }
            }
            else if (found == false && found == false) {
                matchedVal = true;
            }
        }
        else {
            /* Everyone else is successful by default. */
            matchedVal = true;
        }

        barrier(_factory, true);
        MPI_CPPUNIT_ASSERT(matchedVal == true);
        found = _propertyList0->cachedKeyValues().get(prop, jsonValue);
        MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                           == "new value");
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
        JSONValue jsonValue;
        bool found;
        string prop = "prop5";
        /* Clean up the old propertyList */
        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().erase(prop);
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
        }

        barrier(_factory, true);

        _propertyList0 = _node0->getPropertyList(
            ClusterlibStrings::DEFAULTPROPERTYLIST,
            CREATE_IF_NOT_FOUND);

        barrier(_factory, true);

        if (isMyRank(0)) {
            _propertyList0->acquireLock();
        }

        barrier(_factory, true);

        if (!isMyRank(0)) {
            found = _propertyList0->cachedKeyValues().get(prop, jsonValue);
            _propertyList0->cachedKeyValues().set(prop, "no");
            try {
                _propertyList0->cachedKeyValues().publish();
                cerr << "testGetPropertyList5: Published successfully" << endl;
            }
            catch (PublishVersionException e) {
                /* 
                 * Might not always succeed if lots of processes are
                 * trying (of course a lock would fix this =) )
                 */
                cerr << "testGetPropertyList5: Safe failure to publish"
                     << endl;
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
        JSONValue jsonValue;

        if (isMyRank(0)) {
            PropertyList *toolkitProp = 
                _app0->getPropertyList("toolkit", CREATE_IF_NOT_FOUND);
            PropertyList *kernelProp = 
                _app0->getPropertyList("kernel", CREATE_IF_NOT_FOUND);

            toolkitProp->acquireLock();
            toolkitProp->cachedKeyValues().set("clusterlib", "1.1");
            toolkitProp->cachedKeyValues().publish();
            toolkitProp->releaseLock();
            
            kernelProp->acquireLock();
            kernelProp->cachedKeyValues().set("linux", "2.4");
            kernelProp->cachedKeyValues().publish();
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
        JSONValue jsonValue, jsonValue2;
        bool found, found2;
        string prop = "prop7";
        if (isMyRank(0)) {
            _propertyList0 = _node0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                CREATE_IF_NOT_FOUND);
            _propertyList0->acquireLock();
            _propertyList0->cachedKeyValues().erase(prop);
            _propertyList0->cachedKeyValues().publish();
            _propertyList0->releaseLock();
        }

        barrier(_factory, true);

        _propertyList0 = _node0->getPropertyList();

        stringstream ss;
        ss << getRank();
        _propertyList0->acquireLock();
        _propertyList0->cachedKeyValues().set(prop, ss.str());
        usleep(100000);
        found = _propertyList0->cachedKeyValues().get(prop, jsonValue);
        MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                           == ss.str());
        _propertyList0->cachedKeyValues().publish();
        found = _propertyList0->cachedKeyValues().get(prop, jsonValue);
        MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                           == ss.str());
        _propertyList0->releaseLock();

        _propertyList0->acquireLock();
        found = _propertyList0->cachedKeyValues().get(prop, jsonValue);
        usleep(300000);
        found2 = _propertyList0->cachedKeyValues().get(prop, jsonValue2);
        _propertyList0->releaseLock();

        if (found) {
            cerr << "testGetPropertyList7: value (" 
                 << jsonValue.get<JSONValue::JSONString>() << endl;
        }
        if (found2) {
            cerr << "testGetPropertyList7: value (" 
                 << jsonValue2.get<JSONValue::JSONString>() << endl;
        }
        if (found == true && found2 == true) {
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() ==
                               jsonValue2.get<JSONValue::JSONString>());
        }
        MPI_CPPUNIT_ASSERT(found == found2);
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
        JSONValue jsonValue;
        bool found;
        string prop = "prop8";
        string newValue = "prop8-newvalue";
        string propList1Name = "propList1";
        string propList2Name = "propList2";
        string propList3Name = "propList3";
        PropertyList *propList1 = NULL;
        PropertyList *propList2 = NULL;
        PropertyList *propList3 = NULL;

        if (isMyRank(0)) {
            propList1 = _node0->getPropertyList(
                propList1Name, CREATE_IF_NOT_FOUND);
            propList2 = _node0->getPropertyList(
                propList2Name, CREATE_IF_NOT_FOUND);
            propList3 = _node0->getPropertyList(
                propList2Name, CREATE_IF_NOT_FOUND);
            _node0->acquireLock();
            propList1->cachedKeyValues().erase(prop);
            propList1->cachedKeyValues().publish();
            propList2->cachedKeyValues().erase(prop);
            propList2->cachedKeyValues().publish();
            propList3->cachedKeyValues().erase(prop);
            propList3->cachedKeyValues().publish();
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
            propList1->cachedKeyValues().set(prop, newValue);
            found = propList1->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() ==
                               newValue);
            propList2->cachedKeyValues().set(prop, newValue);
            found = propList2->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() ==
                               newValue);
            propList3->cachedKeyValues().set(prop, newValue);
            found = propList3->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() ==
                               newValue);
            /* Publish backward to make the error case the most likely */
            propList3->cachedKeyValues().publish();    
            propList2->cachedKeyValues().publish();
            propList1->cachedKeyValues().publish();     
            _node0->releaseLock();
            barrier(NULL, false);
            found = propList1->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() ==
                               newValue);
            found = propList2->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() ==
                               newValue);
            found = propList3->cachedKeyValues().get(prop, jsonValue);
            MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>() ==
                               newValue);
        }
        else {
            bool found1_1, found2_1, found3_1;
            bool found1_2, found2_2, found3_2;
            JSONValue jsonValue1_1, jsonValue2_1, jsonValue3_1;
            JSONValue jsonValue1_2, jsonValue2_2, jsonValue3_2;
            barrier(NULL, false);

            _node0->acquireLock();

            found1_1 = propList1->cachedKeyValues().get(prop, jsonValue1_1);
            found2_1 = propList2->cachedKeyValues().get(prop, jsonValue2_1);
            found3_1 = propList3->cachedKeyValues().get(prop, jsonValue3_1);
            usleep(500000);
            found1_2 = propList1->cachedKeyValues().get(prop, jsonValue1_2);
            found2_2 = propList2->cachedKeyValues().get(prop, jsonValue2_2);
            found3_2 = propList3->cachedKeyValues().get(prop, jsonValue3_2);

            _node0->releaseLock();

            if (found1_1) {
                cerr << "testGetPropertyList8: value1_1 (" 
                     << jsonValue1_1.get<JSONValue::JSONString>() << endl;
            }
            if (found2_1) {
                cerr << "testGetPropertyList8: value2_1 (" 
                     << jsonValue2_1.get<JSONValue::JSONString>() << endl;
            }
            if (found3_1) {
                cerr << "testGetPropertyList8: value3_1 (" 
                     << jsonValue3_1.get<JSONValue::JSONString>() << endl;
            }
            if (found1_1 == true && found2_1 == true && found3_1 == true) {
                MPI_CPPUNIT_ASSERT(
                    (jsonValue1_1.get<JSONValue::JSONString>() ==
                    jsonValue2_1.get<JSONValue::JSONString>()) &&
                    (jsonValue2_1.get<JSONValue::JSONString>()== 
                     jsonValue3_1.get<JSONValue::JSONString>()));
            }
            MPI_CPPUNIT_ASSERT(found1_1 == found2_1);
            MPI_CPPUNIT_ASSERT(found2_1 == found3_1);

            if (found1_2) {
                cerr << "testGetPropertyList8: value1_2 (" 
                     << jsonValue1_2.get<JSONValue::JSONString>() << endl;
            }
            if (found2_2) {
                cerr << "testGetPropertyList8: value2_2 (" 
                     << jsonValue2_2.get<JSONValue::JSONString>() << endl;
            }
            if (found3_2) {
                cerr << "testGetPropertyList8: value3_2 (" 
                     << jsonValue3_2.get<JSONValue::JSONString>() << endl;
            }
            if (found1_2 == true && found2_2 == true && found3_2 == true) {
                MPI_CPPUNIT_ASSERT(
                    (jsonValue1_2.get<JSONValue::JSONString>() ==
                    jsonValue2_2.get<JSONValue::JSONString>()) &&
                    (jsonValue2_2.get<JSONValue::JSONString>()== 
                     jsonValue3_2.get<JSONValue::JSONString>()));
            }
            MPI_CPPUNIT_ASSERT(found1_2 == found2_2);
            MPI_CPPUNIT_ASSERT(found2_2 == found3_2);

            if (found1_1 == true && found1_2 == true) {
                MPI_CPPUNIT_ASSERT(jsonValue1_1.get<JSONValue::JSONString>() ==
                                   jsonValue1_2.get<JSONValue::JSONString>());
            }
            MPI_CPPUNIT_ASSERT(found1_1 == found1_2);
        }

        barrier(_factory, true);

        found = propList1->cachedKeyValues().get(prop, jsonValue);
        MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                           == newValue);
        found = propList2->cachedKeyValues().get(prop, jsonValue);
        MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                           == newValue);
        found = propList3->cachedKeyValues().get(prop, jsonValue);
        MPI_CPPUNIT_ASSERT(jsonValue.get<JSONValue::JSONString>()
                           == newValue);
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

