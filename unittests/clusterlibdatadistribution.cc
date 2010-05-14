#include "testparams.h"
#include "MPITestFixture.h"
#include "clusterlibinternal.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;
using namespace json;

const string appName = "unittests-datadistribution-app";

class ClusterlibDataDistribution : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibDataDistribution);
    CPPUNIT_TEST(testDataDistribution1);
    CPPUNIT_TEST(testDataDistribution2);
    CPPUNIT_TEST(testDataDistribution3);
    CPPUNIT_TEST(testDataDistribution4);
    CPPUNIT_TEST(testDataDistribution5);
    CPPUNIT_TEST(testDataDistribution6);
    CPPUNIT_TEST(testDataDistribution7);
    CPPUNIT_TEST_SUITE_END();

  public:
    ClusterlibDataDistribution() 
        : MPITestFixture(globalTestParams),
          _factory(NULL) {}
    
    /**
     * Runs prior to each test 
     */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client0 != NULL);
	_app0 = _client0->getRoot()->getApplication(appName, 
                                                    CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
    }

    /** 
     * Runs after each test 
     */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(_factory, true);
	delete _factory;
        _factory = NULL;
    }

    /** 
     * Simple test to try and create a DataDistribution
     */
    void testDataDistribution1()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testDataDistribution1");
        
        if (isMyRank(0)) {
            DataDistribution *dist = _app0->getDataDistribution(
                "dd0", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(dist);
        }
    }

    /** 
     * Simple test to try and add a couple of shards with clearing in
     * between.
     */
    void testDataDistribution2()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testDataDistribution2");

        if (isMyRank(0)) {
            DataDistribution *dist = _app0->getDataDistribution(
                "dd0");
            if (dist != NULL) {
                dist->remove();
            }
            dist = _app0->getDataDistribution(
                "dd0", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(dist);
            
            dist->acquireLock();
            dist->cachedShards().insert(0, 999, NULL);
            dist->cachedShards().insert(1000, 1999, NULL);
            int32_t shardCount = dist->cachedShards().getCount();
            cerr << "testDataDistribution2: shardCount = " << shardCount
                 << endl;
            MPI_CPPUNIT_ASSERT(shardCount == 2);
            dist->cachedShards().clear();
            dist->cachedShards().insert(2000, 2999, NULL);
            MPI_CPPUNIT_ASSERT(dist->cachedShards().getCount() == 1);
            dist->cachedShards().publish();
            dist->releaseLock();
        }
    }
    
    /**
     * Try to see if the data distribution is covered
     */
    void testDataDistribution3()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testDataDistribution3");
        if (isMyRank(0)) {
            DataDistribution *dist = _app0->getDataDistribution(
                "dd0");
            if (dist != NULL) {
                dist->remove();
            }
            dist = _app0->getDataDistribution(
                "dd0", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(dist);
            
            dist->acquireLock();
            dist->cachedShards().insert(0, 999, NULL);
            dist->cachedShards().insert(1000, 1999, NULL);
            dist->cachedShards().insert(2000, 2999, NULL);
            dist->cachedShards().publish();
            MPI_CPPUNIT_ASSERT(dist->cachedShards().isCovered() == false);
            dist->cachedShards().insert(
                3000, numeric_limits<uint64_t>::max(), NULL);
            dist->cachedShards().publish();
            MPI_CPPUNIT_ASSERT(dist->cachedShards().getCount() == 4);
            MPI_CPPUNIT_ASSERT(dist->cachedShards().isCovered() == true);
            dist->releaseLock();
        }
    }

    /**
     * Remove some shards, and add shards
     */
    void testDataDistribution4()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testDataDistribution4");
        if (isMyRank(0)) {
            DataDistribution *dist = _app0->getDataDistribution(
                "dd0");
            if (dist != NULL) {
                dist->remove();
            }
            dist = _app0->getDataDistribution(
                "dd0", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(dist);
            
            dist->acquireLock();
            dist->cachedShards().insert(0, 999, NULL);
            dist->cachedShards().insert(1000, 1999, NULL);
            dist->cachedShards().insert(2000, 2999, NULL);
            dist->cachedShards().insert(3000, 3999, NULL);
            dist->cachedShards().insert(4000, 4999, NULL);

            vector<Shard> shardVec = dist->cachedShards().getAllShards();
            MPI_CPPUNIT_ASSERT(dist->cachedShards().getCount() == 5);
            MPI_CPPUNIT_ASSERT(shardVec.size() == 5);

            MPI_CPPUNIT_ASSERT(dist->cachedShards().remove(shardVec[2]) 
                               == true);
            MPI_CPPUNIT_ASSERT(shardVec[2].getStartRange() == 2000);
            MPI_CPPUNIT_ASSERT(shardVec[2].getEndRange() == 2999);
            MPI_CPPUNIT_ASSERT(shardVec[2].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->cachedShards().getCount() == 4);

            MPI_CPPUNIT_ASSERT(dist->cachedShards().remove(shardVec[1]) 
                               == true);
            MPI_CPPUNIT_ASSERT(shardVec[1].getStartRange() == 1000);
            MPI_CPPUNIT_ASSERT(shardVec[1].getEndRange() == 1999);
            MPI_CPPUNIT_ASSERT(shardVec[1].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->cachedShards().getCount() == 3);

            MPI_CPPUNIT_ASSERT(dist->cachedShards().remove(shardVec[4]) 
                               == true);
            MPI_CPPUNIT_ASSERT(shardVec[4].getStartRange() == 4000);
            MPI_CPPUNIT_ASSERT(shardVec[4].getEndRange() == 4999);
            MPI_CPPUNIT_ASSERT(shardVec[4].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->cachedShards().getCount() == 2);

            MPI_CPPUNIT_ASSERT(dist->cachedShards().remove(shardVec[3])
                               == true);
            MPI_CPPUNIT_ASSERT(shardVec[3].getStartRange() == 3000);
            MPI_CPPUNIT_ASSERT(shardVec[3].getEndRange() == 3999);
            MPI_CPPUNIT_ASSERT(shardVec[3].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->cachedShards().getCount() == 1);

            MPI_CPPUNIT_ASSERT(dist->cachedShards().remove(shardVec[0]) 
                               == true);
            MPI_CPPUNIT_ASSERT(shardVec[0].getStartRange() == 0);
            MPI_CPPUNIT_ASSERT(shardVec[0].getEndRange() == 999);
            MPI_CPPUNIT_ASSERT(shardVec[0].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->cachedShards().getCount() == 0);
            dist->releaseLock();
        }        
    }

    /** 
     * Simple test to try and create a DataDistribution and have it
     * point at some nodes.
     */
    void testDataDistribution5()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testDataDistribution5");
        
        if (isMyRank(0)) {
            DataDistribution *dist = _app0->getDataDistribution(
                "dd0");
            if (dist != NULL) {
                dist->remove();
            }
            dist = _app0->getDataDistribution(
                "dd0", CREATE_IF_NOT_FOUND);
            dist->acquireLock();

            MPI_CPPUNIT_ASSERT(dist);
            Node *n0 = _app0->getNode("n0", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(n0);
            Node *n1 = _app0->getNode("n1", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(n1);
            Node *n2 = _app0->getNode("n2", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(n2);

            Md5Key key("hello");
            cerr << "hashrange of hello is " << key.hashKey() << endl;
            MPI_CPPUNIT_ASSERT(key.hashKey() == 6719722671305337462LL);
            vector<Notifyable *> ntpVec = dist->cachedShards().getNotifyables(key);
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 0);

            dist->cachedShards().insert(0, 6719722671305337462LL, n0);
            ntpVec = dist->cachedShards().getNotifyables(key);
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 1);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n0") == 0);

            dist->cachedShards().insert(
                6719722671305337462LL, 6719722671305399999LL, n1);
            ntpVec = dist->cachedShards().getNotifyables(key);
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 2);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n0") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[1]->getName().compare("n1") == 0);
            ntpVec = dist->cachedShards().getNotifyables(key.hashKey());
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 2);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n0") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[1]->getName().compare("n1") == 0);

            dist->cachedShards().insert(
                6719722671305337450LL, 6719722671305399999LL, n2);
            ntpVec = dist->cachedShards().getNotifyables(key.hashKey());
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 3);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n0") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[1]->getName().compare("n2") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[2]->getName().compare("n1") == 0);
            vector<Shard> shardVec = dist->cachedShards().getAllShards();
            
            dist->cachedShards().remove(shardVec[0]);
            ntpVec = dist->cachedShards().getNotifyables(key);
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 2);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n2") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[1]->getName().compare("n1") == 0);

            dist->cachedShards().publish();
            dist->releaseLock();
        }
    }

    /** 
     * Create a DataDistribution in one node and check it on another.
    */
    void testDataDistribution6()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testDataDistribution6");
        
        if (isMyRank(0)) {
            DataDistribution *dist = _app0->getDataDistribution("dd0");
            if (dist != NULL) {
                dist->remove();
            }
            dist = _app0->getDataDistribution(
                "dd0", CREATE_IF_NOT_FOUND);
            dist->acquireLock();

            MPI_CPPUNIT_ASSERT(dist);
            Node *n0 = _app0->getNode("n0", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(n0);
            Node *n1 = _app0->getNode("n1", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(n1);
            Node *n2 = _app0->getNode("n2", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(n2);

            dist->cachedShards().clear();
            dist->cachedShards().insert(0, 6719722671305337462LL, n0);
            dist->cachedShards().insert(
                6719722671305337462LL, 6719722671305399999LL, n1);
            dist->cachedShards().insert(
                6719722671305337450LL, 6719722671305399999LL, n2);
            dist->cachedShards().publish();
            dist->releaseLock();
            barrier(_factory, true);
        }
        else {
            barrier(_factory, true);
            DataDistribution *dist = _app0->getDataDistribution(
                "dd0", CREATE_IF_NOT_FOUND);
            dist->acquireLock();
            vector<Shard> shardVec = dist->cachedShards().getAllShards();
            MPI_CPPUNIT_ASSERT(shardVec.size() == 3);
            for (vector<Shard>::iterator shardVecIt = shardVec.begin();
                 shardVecIt != shardVec.end(); ++shardVecIt) {
                Node *node = dynamic_cast<Node *>(shardVecIt->getNotifyable());
                MPI_CPPUNIT_ASSERT(node);
                cerr << "parent = " 
                     << node->getMyParent()->getName()
                     << ",appname = "
                     << appName << endl;
                MPI_CPPUNIT_ASSERT(node->getMyParent()->getName() == appName);
            }
            dist->releaseLock();
        }
    }

    /** 
     * Create a DataDistribution in one node and check it on another
     * with getNotifyableFromKey().
     */
    void testDataDistribution7()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testDataDistribution7");
        
        string plk = "distkey";

        if (isMyRank(0)) {
            DataDistribution *dist = _app0->getDataDistribution("dd0");
            if (dist != NULL) {
                dist->remove();
            }
            dist = _app0->getDataDistribution(
                "dd0", CREATE_IF_NOT_FOUND);
            dist->acquireLock();

            MPI_CPPUNIT_ASSERT(dist);
            Node *n0 = _app0->getNode("n0", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(n0);
            ProcessSlot *p0 = n0->getProcessSlot("s0", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(p0);
            Node *n1 = _app0->getNode("n1", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(n1);
            ProcessSlot *p1 = n1->getProcessSlot("s1", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(p1);
            Node *n2 = _app0->getNode("n2", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(n2);
            ProcessSlot *p2 = n2->getProcessSlot("s2", CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(p2);

            dist->cachedShards().clear();
            dist->cachedShards().insert(0, 6719722671305337462LL, p0);
            dist->cachedShards().insert(
                6719722671305337462LL, 6719722671305399999LL, p1);
            dist->cachedShards().insert(
                6719722671305337450LL, 6719722671305399999LL, p2);
            dist->cachedShards().publish();
            dist->releaseLock();
            PropertyList *pl = _app0->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST, CREATE_IF_NOT_FOUND);
            pl->acquireLock();
            pl->cachedKeyValues().set(plk, dist->getKey());
            pl->cachedKeyValues().publish();
            pl->releaseLock();
            barrier(_factory, true);
        }
        else {
            barrier(_factory, true);
            Root *root = _client0->getRoot();
            MPI_CPPUNIT_ASSERT(root);
            PropertyList *pl = _app0->getPropertyList();
            MPI_CPPUNIT_ASSERT(pl);
            pl->acquireLock();
            JSONValue jsonValuePlk;
            pl->cachedKeyValues().get(plk, jsonValuePlk);
            pl->releaseLock();
            DataDistribution *dist = dynamic_cast<DataDistribution *>(
                root->getNotifyableFromKey(
                    jsonValuePlk.get<JSONValue::JSONString>()));
            MPI_CPPUNIT_ASSERT(dist);
            dist->acquireLock();
            vector<Shard> shardVec = dist->cachedShards().getAllShards();
            MPI_CPPUNIT_ASSERT(shardVec.size() == 3);
            for (vector<Shard>::iterator shardVecIt = shardVec.begin();
                 shardVecIt != shardVec.end(); ++shardVecIt) {
                ProcessSlot *ps = dynamic_cast<ProcessSlot *>(
                    shardVecIt->getNotifyable());
                MPI_CPPUNIT_ASSERT(ps);
                cerr << "parent = " 
                     << ps->getMyParent()->getName() << endl;
                MPI_CPPUNIT_ASSERT(
                    ps->getMyParent()->getMyParent()->getName() == appName);
            }
            dist->releaseLock();
        }
    }

  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibDataDistribution);

