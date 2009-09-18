#include "MPITestFixture.h"
#include "testparams.h"

extern TestParams globalTestParams;

using namespace std;
using namespace clusterlib;

class ClusterlibHierarchy : public MPITestFixture {
    CPPUNIT_TEST_SUITE(ClusterlibHierarchy);
    CPPUNIT_TEST(testHierarchy1);
    CPPUNIT_TEST(testHierarchy2);
    CPPUNIT_TEST(testHierarchy3);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibHierarchy() : _factory(NULL),
                            _client(NULL),
                            _app(NULL) {}
    
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
	_app = _client->getRoot()->getApplication("hierarchy-app", true);
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
     * properties works.  Prefers 2 nodes, but if only one process is
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
            Group *group = _app->getGroup("hierarchy-group", true);
	    MPI_CPPUNIT_ASSERT(group);
	    DataDistribution *dist = 
		group->getDataDistribution("hierarchy-dist",
					    true);
	    MPI_CPPUNIT_ASSERT(dist);
	    Node *node = group->getNode("hierarchy-node", true);
	    MPI_CPPUNIT_ASSERT(node);
	    Properties *prop = node->getProperties(
                ClusterlibStrings::DEFAULTPROPERTIES, 
                true);
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
	    
	    Group *group = _app->getGroup("hierarchy-group");
	    MPI_CPPUNIT_ASSERT(group);
	    MPI_CPPUNIT_ASSERT(group->getMyParent() == _app);
	    MPI_CPPUNIT_ASSERT(group->getMyGroup() == _app);
	    MPI_CPPUNIT_ASSERT(group->getMyApplication() == _app);
	    
	    DataDistribution *dist = 
		group->getDataDistribution("hierarchy-dist");
	    MPI_CPPUNIT_ASSERT(dist);
	    MPI_CPPUNIT_ASSERT(dist->getMyParent() == group);
	    MPI_CPPUNIT_ASSERT(dist->getMyGroup() == group);
	    MPI_CPPUNIT_ASSERT(dist->getMyApplication() == _app);

	    Node *node = group->getNode("hierarchy-node");
	    MPI_CPPUNIT_ASSERT(node);
	    MPI_CPPUNIT_ASSERT(node->getMyParent() == group);
	    MPI_CPPUNIT_ASSERT(node->getMyGroup() == group);
	    MPI_CPPUNIT_ASSERT(node->getMyApplication() == _app);
	    
	    Properties *prop = node->getProperties();
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
	    Node *node1 = _app->getNode("node1", true);
	    MPI_CPPUNIT_ASSERT(node1);

	    Group *group1 = _app->getGroup("hier-group1", true);
	    MPI_CPPUNIT_ASSERT(group1);
	    Node *node2 = group1->getNode("node2", true);
	    MPI_CPPUNIT_ASSERT(node2);

	    Group *group2 = group1->getGroup("hier-group2", true);
	    MPI_CPPUNIT_ASSERT(group2);
	    Node *node3 = group2->getNode("node3", true);
	    MPI_CPPUNIT_ASSERT(node3);

            Properties *prop = node3->getProperties(
                ClusterlibStrings::DEFAULTPROPERTIES, 
                true);
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
                cerr << "Caught app->getMyGroup() exception incorrectly" 
                     << endl;
            }

	    MPI_CPPUNIT_ASSERT(_app->getMyApplication() == _app);
	    
	    Node *node1 = _app->getNode("node1");
            MPI_CPPUNIT_ASSERT(node1->getMyParent() == _app);
            MPI_CPPUNIT_ASSERT(node1->getMyGroup() == _app);
            MPI_CPPUNIT_ASSERT(node1->getMyApplication() == _app);

            Group *group1 = _app->getGroup("hier-group1");
            MPI_CPPUNIT_ASSERT(group1->getMyParent() == _app);
            MPI_CPPUNIT_ASSERT(group1->getMyGroup() == _app);
            MPI_CPPUNIT_ASSERT(group1->getMyApplication() == _app);

	    Node *node2 = group1->getNode("node2", true);
            MPI_CPPUNIT_ASSERT(node2->getMyParent() == group1);
            MPI_CPPUNIT_ASSERT(node2->getMyGroup() == group1);
            MPI_CPPUNIT_ASSERT(node2->getMyApplication() == _app);

            Group *group2 = group1->getGroup("hier-group2");
            MPI_CPPUNIT_ASSERT(group2->getMyParent() == group1);
            MPI_CPPUNIT_ASSERT(group2->getMyGroup() == group1);
            MPI_CPPUNIT_ASSERT(group2->getMyApplication() == _app);

	    Node *node3 = group2->getNode("node3");
            MPI_CPPUNIT_ASSERT(node3->getMyParent() == group2);
            MPI_CPPUNIT_ASSERT(node3->getMyGroup() == group2);
            MPI_CPPUNIT_ASSERT(node3->getMyApplication() == _app);

	    Properties *prop = node3->getProperties();
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

            string newkey = nl[index] + "next";
            Application *app0 = 
                _client->getRoot()->getApplication(newkey, true);
            MPI_CPPUNIT_ASSERT(app0);
        }        
        
	waitsForOrder(0, 1, _factory, true);

        if (isMyRank(1)) {
            NameList nl =  _client->getRoot()->getApplicationNames();
            cerr << "Orignally had " << appCount << " applications, now has "
                 << nl.size() << " ." << endl; 
            MPI_CPPUNIT_ASSERT(appCount + 1 == nl.size());
            /* Clean up all applications, since we are always
             * increasing the size of it. */
            NameList::iterator it;
            for (it = nl.begin(); it != nl.end(); it++) {
                _client->getRoot()->getApplication(*it)->remove(true);
            }
        }
    }

  private:
    Factory *_factory;
    Client *_client;
    Application *_app;
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibHierarchy);

