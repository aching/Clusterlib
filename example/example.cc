/*
 * example.cc -- example program using clusterlib (currently used
 * to ensure that clusterlib can be linked into an application).
 */

#include "clusterlibinternal.h"

using namespace std;

class MyTimerHandler
    : public clusterlib::TimerEventHandler
{
  public:
    /*
     * Constructor.
     */
    MyTimerHandler(clusterlib::Client *cp)
        : mp_cp(cp)
    {
    }

    /*
     * Handler.
     */
    virtual void handleTimerEvent(clusterlib::TimerId id,
                                  clusterlib::ClientData data)
    {
        cerr << "Called handler for timer id: "
             << id
             << ", client data: "
             << data
             << ", Client: "
             << mp_cp
             << ", now: "
             << clusterlib::FactoryOps::getCurrentTimeMillis()
             << endl;
    }

  private:
    /*
     * The client.
     */
    clusterlib::Client *mp_cp;
};

int
main(int ac, char **av)
{
    try {
        clusterlib::Factory *f = new clusterlib::Factory("localhost:2221");
        cerr << "factory = " << f << endl;
        clusterlib::Client *c = f->createClient();
        cerr << "client = " << c << endl;

        MyTimerHandler *tp = new MyTimerHandler(c);
        clusterlib::TimerId id =
            c->registerTimer(tp, 3000, (clusterlib::ClientData) f);
        cerr << "Registered timer id: "
             << id
             << " with handler: "
             << tp
             << " and client data: "
             << f
             << " for 3000msec from now, now: "
             << clusterlib::FactoryOps::getCurrentTimeMillis()
             << endl;        
        clusterlib::TimerId id1 =
            c->registerTimer(tp, 2000, (clusterlib::ClientData) c);
        cerr << "Registered timer id: "
             << id1
             << " with handler: "
             << tp
             << " and client data: "
             << c
             << " for 2000msec from now, now: "
             << clusterlib::FactoryOps::getCurrentTimeMillis()
             << endl;        
        
        sleep(3);

        cerr << "After sleep in main thread." << endl;

        clusterlib::Application *app = 
            c->getRoot()->getApplication("app", true);
        cerr << "app = " << app << endl;
        clusterlib::Group *grp = app->getGroup("grp", true);
        clusterlib::Server *s =
            f->createServer(grp, "node", NULL, SF_NONE);
        cerr << "server = " << s << endl;

        if (s == NULL) {
            cerr << "BLEH, couldn't create server!" << endl;
            exit(99);
        }

        app = s->getRoot()->getApplication("foo");
        cerr << "app = " << app << endl;

        grp = app->getGroup("bar");
        clusterlib::Node *node = grp->getNode("zop");
        clusterlib::DataDistribution *dst = app->getDataDistribution("dist");

        clusterlib::Application *app1 = dst->getMyApplication();
        if (app != app1) {
            throw
                clusterlib::Exception("app->dist->app non-equivalence");
        }
        clusterlib::Group *grp1 = node->getMyGroup();
        if (grp != grp1) {
            throw
                clusterlib::Exception(
		    "group->node->group non-equivalence");
        }
        app1 = grp->getMyApplication();
        if (app != app1) {
            throw
               clusterlib::Exception("app->group->app non-equivalence");
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
