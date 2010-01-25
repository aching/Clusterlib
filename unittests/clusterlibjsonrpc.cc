#include "MPITestFixture.h"
#include "testparams.h"
#include <algorithm>
#include <sstream>

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;
using namespace json;
using namespace json::rpc;

const string testMsgSuccess = "testMsgSuccess";

class TestMsgRPC 
    : public virtual JSONRPC 
{
  public:
    virtual string getName() { return "testMsg"; }

    virtual bool checkParams(const JSONValue::JSONArray &paramArr)
    {
        if (paramArr.size() == 1) {
            return true;
        }
        return false;
    }
};

class TestMsgMethod 
    : public virtual JSONRPCMethod,
      public virtual TestMsgRPC 
{
  public:
    virtual JSONValue invoke(
        const string &name,
        const JSONValue::JSONArray &param,
        StatePersistence *persistance) 
    {
        return JSONValue::JSONString(testMsgSuccess);
    }
};

class TestMsgRequest 
    : public virtual ClusterlibRPCRequest,
      public virtual TestMsgRPC
{
  public:
    TestMsgRequest(Client *client) : ClusterlibRPCRequest(client) {}
};

const string appName = "JSONRPC-app";
const string respQueuePrefix = "respQueue";
const string recvQueuePrefix = "recvQueue";
const string compQueuePrefix = "compQueue";

class ClusterlibJSONRPC : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibJSONRPC);
    CPPUNIT_TEST(testJSONRPC1);
    CPPUNIT_TEST(testJSONRPC2);
    CPPUNIT_TEST_SUITE_END();

  public:    
    ClusterlibJSONRPC() : _factory(NULL),
                          _client0(NULL),
                          _app0(NULL) {}
    
    /* Runs prior to each test */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client0 = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client0 != NULL);
        _root = _client0->getRoot();
        MPI_CPPUNIT_ASSERT(_root != NULL);
        if (isMyRank(0)) {
            _app0 = _root->getApplication(appName, true);
            MPI_CPPUNIT_ASSERT(_app0 != NULL);
            _app0->remove(true);
        }
        barrier(_factory, true);
	_app0 = _root->getApplication(appName, true);
	MPI_CPPUNIT_ASSERT(_app0 != NULL);
    }

    /* Runs after each test */
    virtual void tearDown() 
    {
        cleanAndBarrierMPITest(_factory, true);
	if (_factory) {
            delete _factory;
            _factory = NULL;
        }
    }

    /* 
     * Simple test to create a JSONRPC sender and receiver on the same process.
     */
    void testJSONRPC1()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testJSONRPC1");
        stringstream ss;

        ss << respQueuePrefix << "_id_" << getRank();
        Queue *respQueue = _app0->getQueue(ss.str(), true);
        MPI_CPPUNIT_ASSERT(respQueue);
        ss.str("");

        ss << compQueuePrefix << "_resp_id_" 
           << getRank();
        Queue *compRespQueue = _app0->getQueue(ss.str(), true);
        MPI_CPPUNIT_ASSERT(compRespQueue);
        ss.str("");

        _factory->createJSONRPCResponseClient(respQueue,
                                              compRespQueue);
        
        ss << recvQueuePrefix << "_id_" << getRank();
        Queue *recvQueue = _app0->getQueue(ss.str(), true);
        MPI_CPPUNIT_ASSERT(recvQueue);
        ss.str("");

        ss << compQueuePrefix << "_recv_id_" 
           << getRank();
        Queue *compRecvQueue = _app0->getQueue(ss.str(), true);
        MPI_CPPUNIT_ASSERT(compRecvQueue);
        ss.str("");

        JSONRPCManager rpcManager;

        _factory->createJSONRPCMethodClient(recvQueue,
                                            compRecvQueue,
                                            &rpcManager);
    }

    /* 
     * Simple test to create a JSONRPC sender and receiver on the same
     * process and send one message to each other in a circle.
     */
    void testJSONRPC2()
    {
        initializeAndBarrierMPITest(-1, 
                                    true,
                                    _factory, 
                                    true, 
                                    "testJSONRPC2");
        stringstream ss;

        ss << respQueuePrefix << "_id_" << getRank();
        Queue *respQueue = _app0->getQueue(ss.str(), true);
        MPI_CPPUNIT_ASSERT(respQueue);
        ss.str("");

        ss << compQueuePrefix << "_resp_id_" 
           << getRank();
        Queue *compRespQueue = _app0->getQueue(ss.str(), true);
        MPI_CPPUNIT_ASSERT(compRespQueue);
        ss.str("");

        _factory->createJSONRPCResponseClient(respQueue,
                                              compRespQueue);
        
        ss << recvQueuePrefix << "_id_" << getRank();
        Queue *recvQueue = _app0->getQueue(ss.str(), true);
        MPI_CPPUNIT_ASSERT(recvQueue);
        ss.str("");

        ss << compQueuePrefix << "_recv_id_" 
           << getRank();
        Queue *compRecvQueue = _app0->getQueue(ss.str(), true);
        MPI_CPPUNIT_ASSERT(compRecvQueue);
        ss.str("");

        auto_ptr<JSONRPCManager> rpcManager(new JSONRPCManager());
        auto_ptr<TestMsgMethod> testMsgMethod(new TestMsgMethod());
        rpcManager->registerMethod(testMsgMethod->getName(),
                                  testMsgMethod.get());
        _factory->createJSONRPCMethodClient(recvQueue,
                                            compRecvQueue,
                                            rpcManager.get());

        /* Find who to send the message to */
        int myDestRank = getRank() + 1;
        if (myDestRank == getSize()) {
            myDestRank = 0;
        }
        ss << recvQueuePrefix << "_id_" << myDestRank;
        Queue *destQueue = _app0->getQueue(ss.str());
        MPI_CPPUNIT_ASSERT(destQueue);

        /* 
         * Send the message with my response queue and wait for the
         * response.
         */
        TestMsgRequest testMsgRequest(_client0);
        JSONValue::JSONArray paramArr;
        JSONValue::JSONObject obj;
        obj[ClusterlibStrings::JSONOBJECTKEY_RESPQUEUEKEY] = 
            respQueue->getKey();
        paramArr.push_back(obj);
        cerr << "JSONRPC2: Process " << getRank() << " sending "
             << JSONCodec::encode(paramArr) << " to " << destQueue->getKey() 
             << endl;
        testMsgRequest.prepareRequest(paramArr);
        testMsgRequest.sendRequest(destQueue->getKey().c_str());
        testMsgRequest.waitResponse();
        JSONValue::JSONObject respObj = testMsgRequest.getResponse();
        cerr << "JSONRPC2: Process " << getRank() << " got response "
             << JSONCodec::encode(respObj) << endl;
        MPI_CPPUNIT_ASSERT(respObj["result"].type() == 
                           typeid(JSONValue::JSONString));
        MPI_CPPUNIT_ASSERT(respObj["result"].get<JSONValue::JSONString>() == 
                           testMsgSuccess);

        /*
         * If we don't barrier, some processes may never get around to
         * processing the other process's messages!
         */
        barrier(_factory, true);
        
        /* 
         * Have to delete the factory prior to the auto_ptrs going out
         * scope or else there could be segmentation faults.
         */
        delete _factory;
        _factory = NULL;
    }

  private:
    Factory *_factory;
    Client *_client0;
    Root *_root;
    Application *_app0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibJSONRPC);

