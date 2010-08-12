#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace std;
using namespace boost;
using namespace clusterlib;

const string appName = "unittests-hierarchy-app";

class ClusterlibHierarchy : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibHierarchy);
    CPPUNIT_TEST(testHierarchy1);
    CPPUNIT_TEST(testHierarchy2);
    CPPUNIT_TEST(testHierarchy3);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibHierarchy() 
        : MPITestFixture(globalTestParams),
          _factory(NULL),
          _client(NULL) {}
    
    /**
     * Runs prior to each test 
     */
    virtual void setUp() 
    {
	_factory = new Factory(
            globalTestParams.getZkServerPortList());
	MPI_CPPUNIT_ASSERT(_factory != NULL);
	_client = _factory->createClient();
	MPI_CPPUNIT_ASSERT(_client != NULL);
	_app = _client->getRoot()->getApplication(appName, 
                                                  CREATE_IF_NOT_FOUND);
	MPI_CPPUNIT_ASSERT(_app != NULL);
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
     * Simple test to see if basic group, data distribution, and
     * propertyList works.  Prefers 2 nodes, but if only one process is
     * available, runs as a single process test.
     */
    void testHierarchy1()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testHierarchy1");

        if (isMyRank(0)) {
            shared_ptr<Group> group = _app->getGroup("hierarchy-group", 
                                          CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(group);
	    shared_ptr<DataDistribution> dist = 
		group->getDataDistribution("hierarchy-dist",
                                           CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(dist);
	    shared_ptr<Node> node = group->getNode("hierarchy-node", 
                                                   CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(node);
	    shared_ptr<PropertyList> prop = node->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST, 
                CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(prop);
        }
	
	waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
	    MPI_CPPUNIT_ASSERT(_app);

            /* Applications now have parents! */
            try {
                MPI_CPPUNIT_ASSERT(_app->getMyParent() == _client->getRoot());
            } catch (Exception &e) { 
                MPI_CPPUNIT_ASSERT("UNREACHABLE BECAUSE OF EXCEPTION" == NULL);
                cerr << "Caught app->getMyParent() exception correctly" 
                     << endl;
            }
            
            try {
                _app->getMyGroup();
                MPI_CPPUNIT_ASSERT("UNREACHABLE BECAUSE OF EXCEPTION" == NULL);
            } catch (InvalidMethodException &e) {                
                cerr << "Caught app->getMyGroup() exception correctly" 
                     << endl;
            }

	    MPI_CPPUNIT_ASSERT(_app->getMyApplication() == _app);
	    
	    shared_ptr<Group> group = _app->getGroup("hierarchy-group", 
                                                     LOAD_FROM_REPOSITORY);
	    MPI_CPPUNIT_ASSERT(group);
	    MPI_CPPUNIT_ASSERT(group->getMyParent() == _app);
	    MPI_CPPUNIT_ASSERT(group->getMyGroup() == _app);
	    MPI_CPPUNIT_ASSERT(group->getMyApplication() == _app);
	    
	    shared_ptr<DataDistribution> dist = 
		group->getDataDistribution("hierarchy-dist",
                                           LOAD_FROM_REPOSITORY);
	    MPI_CPPUNIT_ASSERT(dist);
	    MPI_CPPUNIT_ASSERT(dist->getMyParent() == group);
	    MPI_CPPUNIT_ASSERT(dist->getMyGroup() == group);
	    MPI_CPPUNIT_ASSERT(dist->getMyApplication() == _app);

	    shared_ptr<Node> node = group->getNode("hierarchy-node",
                                                   LOAD_FROM_REPOSITORY);
	    MPI_CPPUNIT_ASSERT(node);
	    MPI_CPPUNIT_ASSERT(node->getMyParent() == group);
	    MPI_CPPUNIT_ASSERT(node->getMyGroup() == group);
	    MPI_CPPUNIT_ASSERT(node->getMyApplication() == _app);
	    
	    shared_ptr<PropertyList> prop = node->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                LOAD_FROM_REPOSITORY);
	    MPI_CPPUNIT_ASSERT(prop);
	    MPI_CPPUNIT_ASSERT(prop->getMyParent() == node);
	    MPI_CPPUNIT_ASSERT(prop->getMyGroup() == group);
	    MPI_CPPUNIT_ASSERT(prop->getMyApplication() == _app);
        }
    }

    /** 
     * Try to create a large hierarchy of groups with nodes at every
     * level.  Prefers 2 nodes, but if only one process is available,
     * runs as a single process test.
     */
    void testHierarchy2()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testHierarchy2");

        if (isMyRank(0)) {
	    shared_ptr<Node> node1 = _app->getNode("node1", 
                                                   CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(node1);

	    shared_ptr<Group> group1 = _app->getGroup("hier-group1", 
                                                      CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(group1);
	    shared_ptr<Node> node2 = group1->getNode("node2", 
                                                     CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(node2);

	    shared_ptr<Group> group2 = group1->getGroup("hier-group2", 
                                                        CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(group2);
	    shared_ptr<Node> node3 = group2->getNode("node3", 
                                                     CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(node3);

            shared_ptr<PropertyList> prop = node3->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST, 
                CREATE_IF_NOT_FOUND);
	    MPI_CPPUNIT_ASSERT(prop);
        }
	
	waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
	    MPI_CPPUNIT_ASSERT(_app);

            /* Applications now have parents! */
            try {
                MPI_CPPUNIT_ASSERT(_app->getMyParent() == _client->getRoot());
            } catch (Exception &e) { 
                MPI_CPPUNIT_ASSERT("UNREACHABLE BECAUSE OF EXCEPTION" == NULL);
                cerr << "Caught app->getMyParent() exception incorrectly" 
                     << endl;
            }
            
            try {
                _app->getMyGroup();
                MPI_CPPUNIT_ASSERT("UNREACHABLE BECAUSE OF EXCEPTION" == NULL);
            } catch (InvalidMethodException &e) {                
                cerr << "Caught app->getMyGroup() exception correctly" 
                     << endl;
            }

	    MPI_CPPUNIT_ASSERT(_app->getMyApplication() == _app);
	    
	    shared_ptr<Node> node1 = _app->getNode("node1",
                                                   LOAD_FROM_REPOSITORY);
            MPI_CPPUNIT_ASSERT(node1->getMyParent() == _app);
            MPI_CPPUNIT_ASSERT(node1->getMyGroup() == _app);
            MPI_CPPUNIT_ASSERT(node1->getMyApplication() == _app);

            shared_ptr<Group> group1 = _app->getGroup("hier-group1",
                                                      LOAD_FROM_REPOSITORY);
            MPI_CPPUNIT_ASSERT(group1->getMyParent() == _app);
            MPI_CPPUNIT_ASSERT(group1->getMyGroup() == _app);
            MPI_CPPUNIT_ASSERT(group1->getMyApplication() == _app);

	    shared_ptr<Node> node2 = group1->getNode("node2", 
                                                     CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(node2->getMyParent() == group1);
            MPI_CPPUNIT_ASSERT(node2->getMyGroup() == group1);
            MPI_CPPUNIT_ASSERT(node2->getMyApplication() == _app);

            shared_ptr<Group> group2 = group1->getGroup("hier-group2",
                                                        LOAD_FROM_REPOSITORY);
            MPI_CPPUNIT_ASSERT(group2->getMyParent() == group1);
            MPI_CPPUNIT_ASSERT(group2->getMyGroup() == group1);
            MPI_CPPUNIT_ASSERT(group2->getMyApplication() == _app);

	    shared_ptr<Node> node3 = group2->getNode("node3",
                                                     LOAD_FROM_REPOSITORY);
            MPI_CPPUNIT_ASSERT(node3->getMyParent() == group2);
            MPI_CPPUNIT_ASSERT(node3->getMyGroup() == group2);
            MPI_CPPUNIT_ASSERT(node3->getMyApplication() == _app);

	    shared_ptr<PropertyList> prop = node3->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST,
                LOAD_FROM_REPOSITORY);
            MPI_CPPUNIT_ASSERT(prop->getMyParent() == node3);
            MPI_CPPUNIT_ASSERT(prop->getMyGroup() == group2);
            MPI_CPPUNIT_ASSERT(prop->getMyApplication() == _app);
        }
    }

    /** 
     * Try to create a new application and see if the other process
     * picks it up.  Prefers 2 nodes, but if only one process is
     * available, runs as a single process test.
     */
    void testHierarchy3()
    {
        initializeAndBarrierMPITest(2, 
                                    true, 
                                    _factory, 
                                    true, 
                                    "testHierarchy3");

        const string postfix = "next-hierarchy";
        uint32_t appCount = 0;
        if (isMyRank(1)) {
            NameList nl =  _client->getRoot()->getApplicationNames();
            appCount = nl.size();
        }
        
	waitsForOrder(1, 0, _factory, true);

        if (isMyRank(0)) {
            NameList nl =  _client->getRoot()->getApplicationNames();
            /* Get the longest name and then add a character to
             * guarantee it's a new one. */
            uint32_t biggestSize = 0, index = 0;
            for (uint32_t i = 0; i < nl.size(); i++) {
                if (i == 0) {
                    biggestSize = nl[i].size();
                    index = i;
                }
                else {
                    if (nl[i].size() > biggestSize) {
                        biggestSize = nl[i].size();
                        index = i;
                    }
                }
            }

            string newkey = nl[index] + postfix;
            shared_ptr<Application> app0 = 
                _client->getRoot()->getApplication(newkey, 
                                                   CREATE_IF_NOT_FOUND);
            MPI_CPPUNIT_ASSERT(app0);
        }        
        
	waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            NameList nl =  _client->getRoot()->getApplicationNames();
            cerr << "Orignally had " << appCount << " applications, now has "
                 << nl.size() << " ." << endl; 
            MPI_CPPUNIT_ASSERT(appCount + 1 == nl.size());
            /* Clean up last application we added. */
            NameList::iterator it;
            for (it = nl.begin(); it != nl.end(); it++) {
                if ((it->size() > postfix.size()) &&
                    (it->compare(it->size() - postfix.size(), 
                                 postfix.size(), 
                                 postfix) == 0)) {
                    _client->getRoot()->getApplication(
                        *it, LOAD_FROM_REPOSITORY)->remove(true);
                    cerr << "Removed the application " << *it << endl;
                    break;
                }
            }
        }
    }

  private:
    Factory *_factory;
    Client *_client;
    shared_ptr<Application> _app;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibHierarchy);

