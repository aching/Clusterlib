/*
 * example2.cc -- example program using clusterlib (currently used to
 * ensure that clusterlib can be linked into an application).  Does
 * some simply properties checking and data distribution
 * instantiation.
 */

#include "clusterlib.h"

using namespace std;

class MyHealthChecker : public clusterlib::HealthChecker {
  public:
    virtual clusterlib::HealthReport checkHealth() {
	return clusterlib::HealthReport(clusterlib::HealthReport::HS_HEALTHY, 
					"No real check");
    }
  private:
};

int
main(int ac, char **av)
{
    try {
        clusterlib::Factory *f = new clusterlib::Factory("localhost:2221");
        cerr << "factory = " << f << endl;
        clusterlib::Client *c = f->createClient();
        cerr << "client = " << c << endl;
        clusterlib::Application *
            app0 = c->getRoot()->getApplication("app-foo", true);
        cerr << "app0 = " << app0 << endl;
        c->getRoot()->getApplication("app-bar", true);

        clusterlib::NameList appNames = c->getRoot()->getApplicationNames();
        clusterlib::NameList::iterator appIt;
        for (appIt = appNames.begin(); appIt != appNames.end(); appIt++) {
            cerr << "app name: " << *appIt << endl;
                
        }

	clusterlib::Group *grp0 = app0->getGroup("bar-servers", true);
	cerr << "grp0 = " << grp0 << endl;
	grp0 = app0->getGroup("bar-clients", true);
	cerr << "grp0 = " << grp0 << endl;
	clusterlib::Group *grp1 = app0->getGroup("bar-clients");
	cerr << "grp1 = " << grp1 << endl;
	
	clusterlib::Node *node0 = grp0->getNode("zopc-0", true);
	cerr << "node0 = " << node0 << endl;

        clusterlib::Node *node1 = grp0->getNode("zopc-0");
	cerr << "node1 = " << node1 << endl;

	MyHealthChecker check;
	
	clusterlib::Server *s0 = f->createServer(grp0,
						 "zops-0",
						 &check,
						 SF_CREATEREG | SF_MANAGED);
        cerr << "server = " << s0 << endl;

	clusterlib::Server *s1 = f->createServer(grp0,
						 "zops-1",
						 &check,
						 SF_CREATEREG | SF_MANAGED);
        cerr << "server = " << s1 << endl;

	clusterlib::Server *s2 = f->createServer(grp0,
                                                 "zops-2",
                                                 &check,
                                                 SF_CREATEREG | SF_MANAGED);
        cerr << "server = " << s2 << endl;

	clusterlib::DataDistribution *dst = 
	    app0->getDataDistribution("dist", true);
	cerr << "dist name = " << dst->getName() << endl;
	cerr << "dist key = " << dst->getKey() << endl;

	vector<clusterlib::HashRange> shards;
	shards.push_back(100);
	shards.push_back(1000);
	shards.push_back(10000);

	dst->acquireLock();
	dst->setShards(shards);
	dst->reassignShard(
            0, 
            dynamic_cast<clusterlib::Notifyable *>(s0->getMyNode()));
	dst->reassignShard(
            1, 
            dynamic_cast<clusterlib::Notifyable *>(s1->getMyNode()));
	dst->reassignShard(
            2, 
            dynamic_cast<clusterlib::Notifyable *>(s2->getMyNode()));
	dst->publish();
	dst->releaseLock();

        clusterlib::Application *app1 = dst->getMyApplication();
        if (app0 != app1) {
            throw
                clusterlib::Exception("app->dist->app non-equivalence");
        }

        clusterlib::Group *grp2 = node0->getMyGroup();
        if (grp1 != grp2) {
            throw
                clusterlib::Exception(
		    "group->node->group non-equivalence");
        }
        app1 = grp0->getMyApplication();
        if (app0 != app1) {
            throw
               clusterlib::Exception("app->group->app non-equivalence");
        }

	clusterlib::Properties *prop0 = app1->getProperties();
	prop0->acquireLock();

	string test = prop0->getProperty("test", true);
	cerr << "(app1) test (test) = " << test
	     << " and should be empty (if this is the first time running) "
	     << endl;

	prop0->setProperty("test", "passed");
	prop0->setProperty("weird", "yessir");
	prop0->publish();
	prop0->releaseLock();

	string test2 = prop0->getProperty("test", true);
	cerr << "(app1) test2 (test) = " << test2
	     << " and should be passed " << endl;

	clusterlib::Properties *prop1 = app0->getProperties();
	string test3 = prop1->getProperty("test", true);
	cerr << "(app0) test3 (test) = " << test3
	     << " and should be passed " << endl;
	
	prop0->acquireLock();
	prop0->setProperty("avery", "ching");
	prop0->setProperty("test", "good");
	prop0->publish();
	prop0->releaseLock();

	test3 = prop1->getProperty("test", true);
	cerr << "(app0) test3 (test) = " << test3 
	     << " and should be good " << endl;
	test3 = prop1->getProperty("avery", true);
	cerr << "(app0) test3 (avery) = " << test3 
	     << " and should be ching " << endl;

	clusterlib::Properties *prop2 = node0->getProperties();
	test3 = prop2->getProperty("test", true);
	cerr << "(node) test3 (test) = " << test3 
	     << " and should be good " << endl;

	prop2->acquireLock();
	prop2->setProperty("test", "node");
	prop2->publish();
	prop2->releaseLock();

	test3 = prop2->getProperty("test", true);
	cerr << "(node) test3 (test) = " << test3 
	     << " and should be node " << endl;

	test3 = prop1->getProperty("test", true);
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
