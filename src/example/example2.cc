/*
 * example2.cc -- example program using clusterlib (currently used to
 * ensure that clusterlib can be linked into an application).  Does
 * some simple property list checking and data distribution
 * instantiation.
 */

#include "clusterlib.h"

using namespace std;
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
        Application *
            app0 = c->getRoot()->getApplication("app-foo", 
                                                CREATE_IF_NOT_FOUND);
        cerr << "app0 = " << app0 << endl;
        c->getRoot()->getApplication("app-bar", CREATE_IF_NOT_FOUND);

        NameList appNames = c->getRoot()->getApplicationNames();
        NameList::iterator appIt;
        for (appIt = appNames.begin(); appIt != appNames.end(); appIt++) {
            cerr << "app name: " << *appIt << endl;
                
        }

	Group *grp0 = app0->getGroup("bar-servers", 
                                                 CREATE_IF_NOT_FOUND);
	cerr << "grp0 = " << grp0 << endl;
	grp0 = app0->getGroup("bar-clients", CREATE_IF_NOT_FOUND);
	cerr << "grp0 = " << grp0 << endl;
	Group *grp1 = app0->getGroup("bar-clients");
	cerr << "grp1 = " << grp1 << endl;
	
	Node *node0 = grp0->getNode("zopc-0", 
                                                CREATE_IF_NOT_FOUND);
	cerr << "node0 = " << node0 << endl;

        Node *node1 = grp0->getNode("zopc-0");
	cerr << "node1 = " << node1 << endl;

	Node *s0 = grp0->getNode("zops-0",
                                 CREATE_IF_NOT_FOUND);
        cerr << "server = " << s0 << endl;

	Node *s1 = grp0->getNode("zops-1",
                                 CREATE_IF_NOT_FOUND);
        cerr << "server = " << s1 << endl;
        
	Node *s2 = grp0->getNode("zops-2",
                                 CREATE_IF_NOT_FOUND);
        cerr << "server = " << s2 << endl;
        
	DataDistribution *dst = 
	    app0->getDataDistribution("dist", CREATE_IF_NOT_FOUND);
	cerr << "dist name = " << dst->getName() << endl;
	cerr << "dist key = " << dst->getKey() << endl;

	vector<HashRange> shards;
	shards.push_back(100);
	shards.push_back(1000);
	shards.push_back(10000);

	dst->acquireLock();
        dst->cachedShards().insert(0, 99, s0);
        dst->cachedShards().insert(100, 199, s1);        
        dst->cachedShards().insert(200, 299, s2);
	dst->cachedShards().publish();
	dst->releaseLock();

        Application *app1 = dst->getMyApplication();
        if (app0 != app1) {
            throw clusterlib::Exception(
                "app->dist->app non-equivalence");
        }

        Group *grp2 = node0->getMyGroup();
        if (grp1 != grp2) {
            throw clusterlib::Exception(
                "group->node->group non-equivalence");
        }
        app1 = grp0->getMyApplication();
        if (app0 != app1) {
            throw clusterlib::Exception(
                "app->group->app non-equivalence");
        }

	PropertyList *propList0 = app1->getPropertyList(
            ClusterlibStrings::DEFAULTPROPERTYLIST, 
            CREATE_IF_NOT_FOUND);
	propList0->acquireLock();
        
        found = propList0->cachedKeyValues().get(
            "test", jsonValue);
	cerr << "(app1) test (test) = " 
             << jsonValue.get<JSONValue::JSONString>()
             << " and should be empty (if this is the first time running) "
	     << endl;

	propList0->cachedKeyValues().set("test", "passed");
	propList0->cachedKeyValues().set("weird", "yessir");
	propList0->cachedKeyValues().publish();
	propList0->releaseLock();

        found = propList0->cachedKeyValues().get("test", jsonValue);
	cerr << "(app1) test2 (test) = " 
             << jsonValue.get<JSONValue::JSONString>()
             << " and should be passed " << endl;

	PropertyList *propList1 = app0->getPropertyList();
	propList1->cachedKeyValues().get("test", jsonValue);
        string test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(app0) test3 (test) = " << test3
	     << " and should be passed " << endl;
	
	propList0->acquireLock();
	propList0->cachedKeyValues().set("avery", "ching");
	propList0->cachedKeyValues().set("test", "good");
	propList0->cachedKeyValues().publish();
	propList0->releaseLock();

	propList1->cachedKeyValues().get("test", jsonValue);
        test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(app0) test3 (test) = " << test3 
	     << " and should be good " << endl;
	propList1->cachedKeyValues().get("avery", jsonValue);
        test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(app0) test3 (avery) = " << test3 
	     << " and should be ching " << endl;

	PropertyList *propList2 = node0->getPropertyList(
            ClusterlibStrings::DEFAULTPROPERTYLIST,
            CREATE_IF_NOT_FOUND);
	propList2->cachedKeyValues().get("test", jsonValue);
        test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(node) test3 (test) = " << test3 
	     << " and should be good " << endl;

	propList2->acquireLock();
	propList2->cachedKeyValues().set("test", "node");
	propList2->cachedKeyValues().publish();
	propList2->releaseLock();

	propList2->cachedKeyValues().get("test", jsonValue);
        test3 = jsonValue.get<JSONValue::JSONString>();
	cerr << "(node) test3 (test) = " << test3 
	     << " and should be node " << endl;

	propList1->cachedKeyValues().get("test", jsonValue);
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
