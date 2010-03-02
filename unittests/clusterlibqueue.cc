#include "testparams.h"
#include "MPITestFixture.h"
#include <algorithm>
#include <sstream>

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;

const string appName = "queue-app";

class ClusterlibQueue : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibQueue);
    CPPUNIT_TEST(testQueue1);
    CPPUNIT_TEST(testQueue2);
    CPPUNIT_TEST(testQueue3);
    CPPUNIT_TEST(testQueue4);
    CPPUNIT_TEST(testQueue5);
    CPPUNIT_TEST(testQueue6);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibQueue() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client0(NULL),
          _app0(NULL),
          _group0(NULL),
          _queue0(NULL) {}
    
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
	_group0 = _app0->getGroup("queue-group", true);
	MPI_CPPUNIT_ASSERT(_group0 != NULL);
    }

    /* Runs after each test */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(_factory, true);
	delete _factory;
        _factory = NULL;
    }

    /* 
     * Simple test to create a queue and put and take one element from it.
     */
    void testQueue1()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testQueue1");
        const string name = "queue1";
        if (isMyRank(0)) {
            _queue0 = _group0->getQueue(name, true);
            _queue0->remove();
            _queue0 = _group0->getQueue(name, true);
            const string data = "element1";
            MPI_CPPUNIT_ASSERT(_queue0);
            _queue0->put(data);
            MPI_CPPUNIT_ASSERT(_queue0->size() == 1);
            string value;
            _queue0->take(value);
            MPI_CPPUNIT_ASSERT(value == data);
            MPI_CPPUNIT_ASSERT(_queue0->size() == 0);
        }
    }

    /* 
     * Simple test to create a queue and put and remove one element from it.
     */
    void testQueue2()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testQueue2");
        const string name = "queue2";
        if (isMyRank(0)) {
            _queue0 = _group0->getQueue(name, true);
            _queue0->remove();
            _queue0 = _group0->getQueue(name, true);
            const string data = "element1";
            MPI_CPPUNIT_ASSERT(_queue0);
            MPI_CPPUNIT_ASSERT(_queue0->empty() == true);
            int64_t id1 = _queue0->put(data);
            _queue0->put(data + "1");
            MPI_CPPUNIT_ASSERT(_queue0->size() == 2);
            MPI_CPPUNIT_ASSERT(_queue0->removeElement(id1) == true);
            string element2;
            _queue0->front(element2);
            MPI_CPPUNIT_ASSERT(element2.compare(data + "1") == 0);
            MPI_CPPUNIT_ASSERT(_queue0->size() == 1);
        }
    }

    /* 
     * Simple test to add a whole bunch of elements, clear and add more.
     */
    void testQueue3()
    {
        initializeAndBarrierMPITest(1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testQueue3");
        const string name = "queue3";
        if (isMyRank(0)) {
            _queue0 = _group0->getQueue(name, true);
            _queue0->remove();
            _queue0 = _group0->getQueue(name, true);
            const string data = "el";
            MPI_CPPUNIT_ASSERT(_queue0);
            MPI_CPPUNIT_ASSERT(_queue0->empty() == true);
            stringstream ss;
            for (int i = 0; i < 7; i++) {
                ss.str("");
                ss << data << i;
                _queue0->put(ss.str());
            }
            MPI_CPPUNIT_ASSERT(_queue0->size() == 7);
            _queue0->clear();
            MPI_CPPUNIT_ASSERT(_queue0->size() == 0);
            for (int i = 0; i < 4; i++) {
                ss.str("");
                ss << data << i;
                _queue0->put(ss.str());
            }
            map<int64_t, string> elMap = _queue0->getAllElements();
            MPI_CPPUNIT_ASSERT(elMap.size() == 4);
        }
    }

    /* 
     * This test is designed to test whether the queue handles waiting
     * for a new element correctly.
     */
    void testQueue4()
    {
        initializeAndBarrierMPITest(2, 
                                    true,
                                    _factory, 
                                    true, 
                                    "testQueue4");
        const string name = "queue4";
        const string data = "el";
        if (isMyRank(0)) {
            _queue0 = _group0->getQueue(name, true);
            _queue0->remove();
            _queue0 = _group0->getQueue(name, true);
        }

        barrier(_factory, true);

        if (isMyRank(0)) {
            _queue0 = _group0->getQueue(name);
            MPI_CPPUNIT_ASSERT(_queue0);
            /*
             * Wait a half a second first - in most case, put() will
             * happen after take(). 
             */
            usleep(500000);
            _queue0->put(data);
        }
        else if (isMyRank(1)) {
            _queue0 = _group0->getQueue(name);
            MPI_CPPUNIT_ASSERT(_queue0);
            string newData;
            _queue0->take(newData);
            MPI_CPPUNIT_ASSERT(newData.compare(data) == 0);
        }
    }

    /*
     * As long as their are even numbers of processes, just have half
     * put() and half take().
     */
    void testQueue5()
    {
        initializeAndBarrierMPITest(-1, 
                                    false,
                                    _factory, 
                                    true, 
                                    "testQueue5");
        const string name = "queue5";
        const string data = "el";

        if (isMyRank(0)) {
            _queue0 = _group0->getQueue(name, true);
            _queue0->remove();
            _queue0 = _group0->getQueue(name, true);
        }

        barrier(_factory, true);

        /* Only use an even number of processes */
        if (getRank() >= ((getSize() / 2) * 2)) {
            return;
        } 

        /* Half put() and half take() */
        if ((getRank() % 2) == 0) {
            _queue0 = _group0->getQueue(name);
            MPI_CPPUNIT_ASSERT(_queue0);
            string newData;
            _queue0->take(newData);
            MPI_CPPUNIT_ASSERT(newData.compare(data) == 0);
        }
        else {
            _queue0 = _group0->getQueue(name);
            MPI_CPPUNIT_ASSERT(_queue0);
            usleep(500000);
            _queue0->put(data);
        }
    }

    /* 
     * This test is designed to test whether a waiting take will work
     * properly
     */
    void testQueue6()
    {
        initializeAndBarrierMPITest(2, 
                                    true,
                                    _factory, 
                                    true, 
                                    "testQueue6");
        const string name = "queue6";
        const string data = "el";
        if (isMyRank(0)) {
            _queue0 = _group0->getQueue(name, true);
            _queue0->remove();
            _queue0 = _group0->getQueue(name, true);
        }

        barrier(_factory, true);
        
        bool found = false;
        if (isMyRank(0)) {
            _queue0 = _group0->getQueue(name);
            MPI_CPPUNIT_ASSERT(_queue0);
            // Shouldn't be found in 1 msecs
            string newData;
            found = _queue0->takeWaitMsecs(1, newData);
            MPI_CPPUNIT_ASSERT(found == false);
             cerr << "newData should be empty (" << newData 
                  << ") data (" << data << ")" << endl;
        }
        if (isMyRank(1)) {
            _queue0 = _group0->getQueue(name);
            MPI_CPPUNIT_ASSERT(_queue0);
            /*
             * Wait a half a second first - in most case, put() will
             * happen after take(). 
             */
            usleep(500000);
            _queue0->put(data);
        }
        if (isMyRank(0)) {
            // Should be found in 5 secs
            string newData;
            found = _queue0->takeWaitMsecs(5000, newData);
            MPI_CPPUNIT_ASSERT(found == true);
            cerr << "newData (" << newData 
                 << ") should be data (" << data << ")" << endl;
            MPI_CPPUNIT_ASSERT(newData == data);
            cerr << "testQueue6 finished for proc 0" << endl;
        }
    }


  private:
    Factory *_factory;
    Client *_client0;
    Application *_app0;
    Group *_group0;
    Queue *_queue0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibQueue);

