/*
 * example2.cc -- example program using clusterlib (currently used
 * to ensure that clusterlib can be linked into an application).
 */

#include "clusterlib.h"

class MyHealthChecker : public clusterlib::HealthChecker {
  public:
    virtual bool checkHealth() {
	return true;
    }
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

	clusterlib::DataDistribution *dst = 
	    app0->getDistribution("dist", true);

        clusterlib::Application *app1 = dst->getApplication();
        if (app0 != app1) {
            throw
                clusterlib::ClusterException("app->dist->app non-equivalence");
        }
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

        delete f;
        cerr << "Clean exit, bye!" << endl;
    } catch (std::exception &e) {
        cerr << "Exception: "
             << e.what()
             << endl;
        cerr << "Aborting, bye!" << endl;
    }
}
