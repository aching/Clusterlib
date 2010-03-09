#include "testparams.h"
#include "MPITestFixture.h"
#include "clusterlibinternal.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;

const string appName = "unittests-datadistribution-app";

class ClusterlibDataDistribution : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibDataDistribution);
    CPPUNIT_TEST(testDataDistribution1);
    CPPUNIT_TEST(testDataDistribution2);
    CPPUNIT_TEST(testDataDistribution3);
    CPPUNIT_TEST(testDataDistribution4);
    CPPUNIT_TEST(testDataDistribution5);
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
	_app0 = _client0->getRoot()->getApplication(appName, true);
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
                "dd0", true);
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
                "dd0", true);
            MPI_CPPUNIT_ASSERT(dist);
            
            dist->acquireLock();
            dist->insertShard(0, 999, NULL);
            dist->insertShard(1000, 1999, NULL);
            int32_t shardCount = dist->getShardCount();
            cerr << "testDataDistribution2: shardCount = " << shardCount
                 << endl;
            MPI_CPPUNIT_ASSERT(shardCount == 2);
            dist->clear();
            dist->insertShard(2000, 2999, NULL);
            MPI_CPPUNIT_ASSERT(dist->getShardCount() == 1);
            dist->publish();
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
                "dd0", true);
            MPI_CPPUNIT_ASSERT(dist);
            
            dist->acquireLock();
            dist->insertShard(0, 999, NULL);
            dist->insertShard(1000, 1999, NULL);
            dist->insertShard(2000, 2999, NULL);
            dist->publish();
            MPI_CPPUNIT_ASSERT(dist->isCovered() == false);
            dist->insertShard(3000, numeric_limits<uint64_t>::max(), NULL);
            dist->publish();
            MPI_CPPUNIT_ASSERT(dist->getShardCount() == 4);
            MPI_CPPUNIT_ASSERT(dist->isCovered() == true);
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
                "dd0", true);
            MPI_CPPUNIT_ASSERT(dist);
            
            dist->acquireLock();
            dist->insertShard(0, 999, NULL);
            dist->insertShard(1000, 1999, NULL);
            dist->insertShard(2000, 2999, NULL);
            dist->insertShard(3000, 3999, NULL);
            dist->insertShard(4000, 4999, NULL);

            vector<Shard> shardVec = dist->getAllShards();
            MPI_CPPUNIT_ASSERT(dist->getShardCount() == 5);
            MPI_CPPUNIT_ASSERT(shardVec.size() == 5);

            MPI_CPPUNIT_ASSERT(dist->removeShard(shardVec[2]) == true);
            MPI_CPPUNIT_ASSERT(shardVec[2].getStartRange() == 2000);
            MPI_CPPUNIT_ASSERT(shardVec[2].getEndRange() == 2999);
            MPI_CPPUNIT_ASSERT(shardVec[2].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->getShardCount() == 4);

            MPI_CPPUNIT_ASSERT(dist->removeShard(shardVec[1]) == true);
            MPI_CPPUNIT_ASSERT(shardVec[1].getStartRange() == 1000);
            MPI_CPPUNIT_ASSERT(shardVec[1].getEndRange() == 1999);
            MPI_CPPUNIT_ASSERT(shardVec[1].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->getShardCount() == 3);

            MPI_CPPUNIT_ASSERT(dist->removeShard(shardVec[4]) == true);
            MPI_CPPUNIT_ASSERT(shardVec[4].getStartRange() == 4000);
            MPI_CPPUNIT_ASSERT(shardVec[4].getEndRange() == 4999);
            MPI_CPPUNIT_ASSERT(shardVec[4].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->getShardCount() == 2);

            MPI_CPPUNIT_ASSERT(dist->removeShard(shardVec[3]) == true);
            MPI_CPPUNIT_ASSERT(shardVec[3].getStartRange() == 3000);
            MPI_CPPUNIT_ASSERT(shardVec[3].getEndRange() == 3999);
            MPI_CPPUNIT_ASSERT(shardVec[3].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->getShardCount() == 1);

            MPI_CPPUNIT_ASSERT(dist->removeShard(shardVec[0]) == true);
            MPI_CPPUNIT_ASSERT(shardVec[0].getStartRange() == 0);
            MPI_CPPUNIT_ASSERT(shardVec[0].getEndRange() == 999);
            MPI_CPPUNIT_ASSERT(shardVec[0].getNotifyable() == NULL);
            MPI_CPPUNIT_ASSERT(dist->getShardCount() == 0);
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
                "dd0", true);
            dist->acquireLock();

            MPI_CPPUNIT_ASSERT(dist);
            Node *n0 = _app0->getNode("n0", true);
            MPI_CPPUNIT_ASSERT(n0);
            Node *n1 = _app0->getNode("n1", true);
            MPI_CPPUNIT_ASSERT(n1);
            Node *n2 = _app0->getNode("n2", true);
            MPI_CPPUNIT_ASSERT(n2);

            Md5Key key("hello");
            cerr << "hashrange of hello is " << key.hashKey() << endl;
            MPI_CPPUNIT_ASSERT(key.hashKey() == 6719722671305337462LL);
            vector<const Notifyable *> ntpVec = dist->getNotifyables(key);
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 0);

            dist->insertShard(0, 6719722671305337462LL, n0);
            ntpVec = dist->getNotifyables(key);
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 1);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n0") == 0);

            dist->insertShard(6719722671305337462LL, 6719722671305399999LL, 
                              n1);
            ntpVec = dist->getNotifyables(key);
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 2);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n0") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[1]->getName().compare("n1") == 0);
            ntpVec = dist->getNotifyables(key.hashKey());
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 2);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n0") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[1]->getName().compare("n1") == 0);

            dist->insertShard(6719722671305337450LL, 6719722671305399999LL,
                              n2);
            ntpVec = dist->getNotifyables(key.hashKey());
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 3);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n0") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[1]->getName().compare("n2") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[2]->getName().compare("n1") == 0);
            vector<Shard> shardVec = dist->getAllShards();
            
            dist->removeShard(shardVec[0]);
            ntpVec = dist->getNotifyables(key);
            MPI_CPPUNIT_ASSERT(ntpVec.size() == 2);
            MPI_CPPUNIT_ASSERT(ntpVec[0]->getName().compare("n2") == 0);
            MPI_CPPUNIT_ASSERT(ntpVec[1]->getName().compare("n1") == 0);

            dist->publish();
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

