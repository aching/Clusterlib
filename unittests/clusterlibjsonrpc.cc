#include "testparams.h"
#include "MPITestFixture.h"
#include <algorithm>
#include <sstream>

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;
using namespace boost;
using namespace json;
using namespace json::rpc;

const string appName = "unittests-jsonrpc-app";
const string testMsgSuccess = "testMsgSuccess";

class TestMsgRPC 
    : public virtual JSONRPC 
{
  public:
    TestMsgRPC() : name("testMsg") {}

    virtual const string &getName() const
    {
        return name;
    }

    virtual void checkParams(const JSONValue::JSONArray &paramArr)
    {
        if (paramArr.size() != 1) {
            throw clusterlib::Exception("checkParams failed");
        }
    }

    string name;
};

class TestMsgMethod 
    : public virtual ClusterlibRPCMethod,
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

    virtual void unmarshalParams(const JSONValue::JSONArray &paramArr) {}
};

class TestMsgRequest 
    : public virtual ClusterlibRPCRequest,
      public virtual TestMsgRPC
{
  public:
    TestMsgRequest(Client *client) : ClusterlibRPCRequest(client) {}

    virtual JSONValue::JSONArray marshalParams() 
    { 
        JSONValue::JSONArray jsonArr;
        JSONValue::JSONObject jsonObj;
        jsonArr.push_back(jsonObj);
        return jsonArr;
    }
};

const string respQueuePrefix = "respQueue";
const string recvQueuePrefix = "recvQueue";
const string compQueuePrefix = "compQueue";

class ClusterlibJSONRPC : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibJSONRPC);
    CPPUNIT_TEST(testJSONRPC1);
    CPPUNIT_TEST(testJSONRPC2);
    CPPUNIT_TEST_SUITE_END();

  public:    
    ClusterlibJSONRPC() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client0(NULL) {}
    
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
            _app0 = _root->getApplication(appName, CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(_app0 != NULL);
            _app0->remove(true);
        }
        barrier(_factory, true);
	_app0 = _root->getApplication(appName, CREATE_IF_NOT_FOUND);
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
        shared_ptr<Queue> respQueue = _app0->getQueue(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(respQueue);
        ss.str("");

        ss << compQueuePrefix << "_resp_id_" 
           << getRank();
        shared_ptr<Queue> compRespQueue = _app0->getQueue(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(compRespQueue);
        ss.str("");

        Client *respClient = _factory->createJSONRPCResponseClient(
            respQueue,
            compRespQueue);
        
        ss << recvQueuePrefix << "_id_" << getRank();
        shared_ptr<Queue> recvQueue = _app0->getQueue(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(recvQueue);
        ss.str("");

        ss << compQueuePrefix << "_recv_id_" 
           << getRank();
        shared_ptr<Queue> compRecvQueue = 
            _app0->getQueue(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(compRecvQueue);
        ss.str("");

        ClusterlibRPCManager rpcManager(_root,
                                        recvQueue,
                                        compRecvQueue,
                                        -1,
                                        shared_ptr<PropertyList>());

        Client *methodClient = 
            _factory->createJSONRPCMethodClient(&rpcManager);
        MPI_CPPUNIT_ASSERT(_factory->removeClient(respClient));
        MPI_CPPUNIT_ASSERT(_factory->removeClient(methodClient));
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
        shared_ptr<Queue> respQueue = _app0->getQueue(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(respQueue);
        ss.str("");

        ss << compQueuePrefix << "_resp_id_" 
           << getRank();
        shared_ptr<Queue> compRespQueue = _app0->getQueue(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(compRespQueue);
        ss.str("");

        Client *rpcResponseClient  =
            _factory->createJSONRPCResponseClient(respQueue,
                                                  compRespQueue);
        
        ss << recvQueuePrefix << "_id_" << getRank();
        shared_ptr<Queue> recvQueue = _app0->getQueue(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(recvQueue);
        ss.str("");

        ss << compQueuePrefix << "_recv_id_" 
           << getRank();
        shared_ptr<Queue> compRecvQueue = _app0->getQueue(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(compRecvQueue);
        ss.str("");

        auto_ptr<ClusterlibRPCManager> rpcManager(
            new ClusterlibRPCManager(_root,
                                     recvQueue,
                                     compRecvQueue,
                                     -1,
                                     shared_ptr<PropertyList>()));

        auto_ptr<TestMsgMethod> testMsgMethod(new TestMsgMethod());
        rpcManager->registerMethod(testMsgMethod->getName(),
                                  testMsgMethod.get());
        Client *rpcMethodClient = 
            _factory->createJSONRPCMethodClient(rpcManager.get());

        /* Find who to send the message to */
        int myDestRank = getRank() + 1;
        if (myDestRank == getSize()) {
            myDestRank = 0;
        }
        ss << recvQueuePrefix << "_id_" << myDestRank;
        shared_ptr<Queue> destQueue = _app0->getQueue(ss.str(), CREATE_IF_NOT_FOUND);
        MPI_CPPUNIT_ASSERT(destQueue);

        /* 
         * Send the message with my response queue and wait for the
         * response.
         */
        TestMsgRequest testMsgRequest(_client0);
        testMsgRequest.setRespQueueKey(respQueue->getKey());
        cerr << "JSONRPC2: Process " << getRank() << " sending "
             << JSONCodec::encode(testMsgRequest.marshalParams()) << " to "
             << destQueue->getKey() << endl;
        testMsgRequest.setDestination(destQueue->getKey().c_str());
        testMsgRequest.sendRequest();
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
        MPI_CPPUNIT_ASSERT(_factory->removeClient(rpcResponseClient));
        MPI_CPPUNIT_ASSERT(_factory->removeClient(rpcMethodClient));
    }

  private:
    Factory *_factory;
    Client *_client0;
    shared_ptr<Root> _root;
    shared_ptr<Application> _app0;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibJSONRPC);

