/*
 * example.cc -- example program using clusterlib (currently used
 * to ensure that clusterlib can be linked into an application).
 */

#include "clusterlib.h"

using namespace std;
using namespace boost;
using namespace clusterlib;

class MyTimerHandler
    : public TimerEventHandler
{
  public:
    /*
     * Constructor.
     */
    MyTimerHandler(Client *cp)
        : mp_cp(cp)
    {
    }

    /*
     * Handler.
     */
    virtual void handleTimerEvent(TimerId id,
                                  ClientData data)
    {
        cerr << "Called handler for timer id: "
             << id
             << ", client data: "
             << data
             << ", Client: "
             << mp_cp
             << ", now: "
             << TimerService::getCurrentTimeMsecs()
             << endl;
    }

  private:
    /*
     * The client.
     */
    Client *mp_cp;
};

int
main(int ac, char **av)
{
    try {
        Factory *f = new Factory("localhost:2221");
        cerr << "factory = " << f << endl;
        Client *c = f->createClient();
        cerr << "client = " << c << endl;

        MyTimerHandler *tp = new MyTimerHandler(c);
        TimerId id =
            c->registerTimer(tp, 3000, (ClientData) f);
        cerr << "Registered timer id: "
             << id
             << " with handler: "
             << tp
             << " and client data: "
             << f
             << " for 3000msec from now, now: "
             << TimerService::getCurrentTimeMsecs()
             << endl;        
        TimerId id1 =
            c->registerTimer(tp, 2000, (ClientData) c);
        cerr << "Registered timer id: "
             << id1
             << " with handler: "
             << tp
             << " and client data: "
             << c
             << " for 2000msec from now, now: "
             << TimerService::getCurrentTimeMsecs()
             << endl;        
        
        sleep(3);

        cerr << "After sleep in main thread." << endl;

        shared_ptr<Application> applicationSP = 
            c->getRoot()->getApplication("app", CREATE_IF_NOT_FOUND);
        cerr << "app = " << applicationSP << endl;
        shared_ptr<Group> groupSP = 
            applicationSP->getGroup("grp", CREATE_IF_NOT_FOUND);

        applicationSP = 
            c->getRoot()->getApplication("foo", CREATE_IF_NOT_FOUND);
        cerr << "app = " << applicationSP << endl;

        groupSP = applicationSP->getGroup("bar", CREATE_IF_NOT_FOUND);
        shared_ptr<Node> nodeSP = groupSP->getNode("zop", CREATE_IF_NOT_FOUND);
        shared_ptr<DataDistribution> dst = applicationSP->getDataDistribution(
            "dist",
            CREATE_IF_NOT_FOUND);

        shared_ptr<Application> application1SP = dst->getMyApplication();
        if (applicationSP != application1SP) {
            throw
                Exception("app->dist->app non-equivalence");
        }
        shared_ptr<Group> group1SP = nodeSP->getMyGroup();
        if (groupSP != group1SP) {
            throw
                Exception(
		    "group->node->group non-equivalence");
        }
        application1SP = groupSP->getMyApplication();
        if (applicationSP != application1SP) {
            throw
               Exception("app->group->app non-equivalence");
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
