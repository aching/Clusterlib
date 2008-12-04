/*
 * example2.cc -- example program using clusterlib (currently used
 * to ensure that clusterlib can be linked into an application).
 */

#include "clusterlib.h"

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
        clusterlib::Factory *f = new clusterlib::Factory("wmdev1003:2181");
        cerr << "factory = " << f << endl;
        clusterlib::Client *c = f->createClient();
        cerr << "client = " << c << endl;
        clusterlib::Application *app0 = c->getApplication("foo", true);
        cerr << "app0 = " << app0 << endl;
	clusterlib::Group *grp0 = app0->getGroup("bar-clients", true);
	cerr << "grp0 = " << grp0 << endl;
	clusterlib::Group *grp1 = app0->getGroup("bar-clients");
	cerr << "grp1 = " << grp1 << endl;
	
	clusterlib::Node *node0 = grp0->getNode("zopc", true);
	cerr << "node0 = " << node0 << endl;

        clusterlib::Node *node1 = grp0->getNode("zopc");
	cerr << "node1 = " << node1 << endl;

#if 0
	clusterlib::DataDistribution *dst = 
	    app0->getDistribution("dist", true);


        clusterlib::Application *app1 = dst->getApplication();
        if (app0 != app1) {
            throw
                clusterlib::ClusterException("app->dist->app non-equivalence");
        }
#else
	clusterlib::Application *app1 ;
#endif
        clusterlib::Group *grp2 = node0->getGroup();
        if (grp1 != grp2) {
            throw
                clusterlib::ClusterException(
		    "group->node->group non-equivalence");
        }
        app1 = grp0->getApplication();
        if (app0 != app1) {
            throw
               clusterlib::ClusterException("app->group->app non-equivalence");
        }

	MyHealthChecker check;
	
	clusterlib::Server *s = f->createServer("foo",
						"bar-servers",
						"zops",
						&check,
						SF_CREATEREG | SF_MANAGED);
        cerr << "server = " << s << endl;

	clusterlib::Properties *prop0 = app1->getProperties();
	
	string test = prop0->getProperty("test");
	cerr << "test = " << test << " and should be [blank] " << endl;

	vector<string> keys;
	keys.push_back("test");
	keys.push_back("weird");
	vector<string> values;
	values.push_back("passed");
	values.push_back("yessir");
	prop0->setProperties(keys, values);

	string test2 = prop0->getProperty("test");
	cerr << "test2 = " << test2 << " and should be passed " << endl;

	clusterlib::Properties *prop1 = app0->getProperties();
	string test3 = prop1->getProperty("test");
	cerr << "test3 = " << test3 << " and should be passed " << endl;
	
	keys.clear();
	keys.push_back("avery");
	keys.push_back("test");
	values.clear();
	values.push_back("ching");
	values.push_back("good");

	prop0->setProperties(keys, values);

	test3 = prop1->getProperty("test");
	cerr << "(app) test3 (test) = " << test3 
	     << " and should be good " << endl;
	test3 = prop1->getProperty("avery");
	cerr << "(app) test3 (avery) = " << test3 
	     << " and should be ching " << endl;

	clusterlib::Properties *prop2 = node0->getProperties();
	test3 = prop2->getProperty("test");
	cerr << "(node) test3 (test) = " << test3 
	     << " and should be good " << endl;

	keys.clear();
	keys.push_back("test");
	values.clear();
	values.push_back("node");

	prop2->setProperties(keys, values);

	prop2->getProperty("test");
	cerr << "(node) test3 (test) = " << test3 
	     << " and should be node " << endl;

	test3 = prop1->getProperty("test");
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
