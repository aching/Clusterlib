/*
 * example2.cc -- example program using clusterlib (currently used to
 * ensure that clusterlib can be linked into an application).  Does
 * some simple property list checking and data distribution
 * instantiation.
 */

#include "clusterlib.h"

using namespace std;
using namespace boost;
using namespace clusterlib;
using namespace json;

int
main(int ac, char **av)
{
    try {
        bool found;
        JSONValue jsonValue;
        Factory *f = new Factory("localhost:2221");
        cerr << "factory = " << f << endl;
        Client *c = f->createClient();
        cerr << "client = " << c << endl;
        shared_ptr<Application> application0SP =
            c->getRoot()->getApplication("app-foo", CREATE_IF_NOT_FOUND);
        cerr << "application0SP = " << application0SP << endl;
        c->getRoot()->getApplication("app-bar", CREATE_IF_NOT_FOUND);

        NameList appNames = c->getRoot()->getApplicationNames();
        NameList::iterator appIt;
        for (appIt = appNames.begin(); appIt != appNames.end(); appIt++) {
            cerr << "app name: " << *appIt << endl;
                
        }

	shared_ptr<Group> group0SP = application0SP->getGroup(
            "bar-servers", CREATE_IF_NOT_FOUND);
	cerr << "group0SP = " << group0SP << endl;
	group0SP = application0SP->getGroup(
            "bar-clients", CREATE_IF_NOT_FOUND);
	cerr << "group0SP = " << group0SP << endl;
	shared_ptr<Group> group1SP = application0SP->getGroup(
            "bar-clients", LOAD_FROM_REPOSITORY);
	cerr << "group1SP = " << group1SP << endl;
	
	shared_ptr<Node> node0SP = group0SP->getNode("zopc-0", 
                                                     CREATE_IF_NOT_FOUND);
	cerr << "node0 = " << node0SP << endl;

        shared_ptr<Node> node1SP = group0SP->getNode(
            "zopc-0", LOAD_FROM_REPOSITORY);
	cerr << "node1 = " << node1SP << endl;

	shared_ptr<Node> s0SP = group0SP->getNode("zops-0",
                                                  CREATE_IF_NOT_FOUND);
        cerr << "server = " << s0SP << endl;

	shared_ptr<Node> s1SP = group0SP->getNode("zops-1",
                                                  CREATE_IF_NOT_FOUND);
        cerr << "server = " << s1SP << endl;
        
	shared_ptr<Node> s2SP = group0SP->getNode("zops-2",
                                                  CREATE_IF_NOT_FOUND);
        cerr << "server = " << s2SP << endl;
        
	shared_ptr<DataDistribution> dst = 
	    application0SP->getDataDistribution("dist", CREATE_IF_NOT_FOUND);
	cerr << "dist name = " << dst->getName() << endl;
	cerr << "dist key = " << dst->getKey() << endl;

	dst->acquireLock(ClusterlibStrings::NOTIFYABLE_LOCK, DIST_LOCK_EXCL);
        dst->cachedShards().insert(Uint64HashRange(0),
                                   Uint64HashRange(99), 
                                   s0SP);
        dst->cachedShards().insert(Uint64HashRange(100),
                                   Uint64HashRange(199),
                                   s1SP);        
        dst->cachedShards().insert(Uint64HashRange(200),
                                   Uint64HashRange(299),
                                   s2SP);
	dst->cachedShards().publish();
	dst->releaseLock(ClusterlibStrings::NOTIFYABLE_LOCK);

        shared_ptr<Application> application1SP = dst->getMyApplication();
        if (application0SP != application1SP) {
            throw clusterlib::Exception(
                "app->dist->app non-equivalence");
        }

        shared_ptr<Group> group2SP = node0SP->getMyGroup();
        if (group1SP != group2SP) {
            throw clusterlib::Exception(
                "group->node->group non-equivalence");
        }
        application1SP = group0SP->getMyApplication();
        if (application0SP != application1SP) {
            throw clusterlib::Exception(
                "app->group->app non-equivalence");
        }

	shared_ptr<PropertyList> properyList0SP = 
            application1SP->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST, 
                CREATE_IF_NOT_FOUND);
	properyList0SP->acquireLock(ClusterlibStrings::NOTIFYABLE_LOCK,
                                    DIST_LOCK_EXCL);
        
        found = properyList0SP->cachedKeyValues().get(
            "test", jsonValue);
	cerr << "(application1SP) test (test) = " 
             << jsonValue.get<JSONValue::JSONString>()
             << " and should be empty (if this is the first time running) "
	     << endl;

	properyList0SP->cachedKeyValues().set("test", "passed");
	properyList0SP->cachedKeyValues().set("weird", "yessir");
	properyList0SP->cachedKeyValues().publish();
	properyList0SP->releaseLock(ClusterlibStrings::NOTIFYABLE_LOCK);

        found = properyList0SP->cachedKeyValues().get("test", jsonValue);
	cerr << "(application1SP) test2 (test) = " 
             << jsonValue.get<JSONValue::JSONString>()
             << " and should be passed " << endl;

	shared_ptr<PropertyList> properyList1SP = 
            application0SP->getPropertyList(
                ClusterlibStrings::DEFAULTPROPERTYLIST, CREATE_IF_NOT_FOUND);
	properyList1SP->cachedKeyValues().get("test", jsonValue);
        string test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(application0SP) test3 (test) = " << test3
	     << " and should be passed " << endl;
	
	properyList0SP->acquireLock(ClusterlibStrings::NOTIFYABLE_LOCK,
                                    DIST_LOCK_EXCL);
	properyList0SP->cachedKeyValues().set("avery", "ching");
	properyList0SP->cachedKeyValues().set("test", "good");
	properyList0SP->cachedKeyValues().publish();
	properyList0SP->releaseLock(ClusterlibStrings::NOTIFYABLE_LOCK);

	properyList1SP->cachedKeyValues().get("test", jsonValue);
        test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(application0SP) test3 (test) = " << test3 
	     << " and should be good " << endl;
	properyList1SP->cachedKeyValues().get("avery", jsonValue);
        test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(application0SP) test3 (avery) = " << test3 
	     << " and should be ching " << endl;

	shared_ptr<PropertyList> propertyList2SP = node0SP->getPropertyList(
            ClusterlibStrings::DEFAULTPROPERTYLIST,
            CREATE_IF_NOT_FOUND);
	propertyList2SP->cachedKeyValues().get("test", jsonValue);
        test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(node) test3 (test) = " << test3 
	     << " and should be good " << endl;

	propertyList2SP->acquireLock(ClusterlibStrings::NOTIFYABLE_LOCK,
                                     DIST_LOCK_EXCL);
	propertyList2SP->cachedKeyValues().set("test", "node");
	propertyList2SP->cachedKeyValues().publish();
	propertyList2SP->releaseLock(ClusterlibStrings::NOTIFYABLE_LOCK);

	propertyList2SP->cachedKeyValues().get("test", jsonValue);
        test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(node) test3 (test) = " << test3 
	     << " and should be node " << endl;

	properyList1SP->cachedKeyValues().get("test", jsonValue);
        test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(app) test3 (test) = " << test3 
	     << " and should be good " << endl;

	for (int i = 5; i > 0; i--) {
	    cerr << "Sleeping for " << i << " more seconds..." << endl;
	    sleep(1);
	}

        delete f;
        cerr << "Clean exit, bye!" << endl;
    } catch (std::exception &e) {
        cerr << "Exception: "
             << e.what()
             << endl;
        cerr << "Aborting, bye!" << endl;
    }
}
