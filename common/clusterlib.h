/*
 * clusterlib.h --
 *
 * The main include file for users of clusterlib.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CLUSTERLIB_H_
#define	_CLUSTERLIB_H_

#include "forwarddecls.h"
#include "blockingqueue.h"
#include "mutex.h"
#include "thread.h"
#include "event.h"
#include "zkadapter.h"
#include "healthchecker.h"
#include "clusterexception.h"
#include "clusterclient.h"
#include "clusterserver.h"

namespace clusterlib
{

class Factory
{
  public:
    /*
     * Create a factory instance, connect it to
     * the specified cluster registry.
     */
    Factory(const string &registry);

    /*
     * Destructor.
     */
    ~Factory();

    /*
     * Create a cluster client object.
     */
    ClusterClient *createClient();

    /*
     * Create a cluster server object. Also
     * create the needed registration if createReg
     * is set to true.
     */
    ClusterServer *createServer(const string &app,
                                const string &group,
                                const string &node,
                                HealthChecker *checker,
                                bool createReg = false);
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERLIB_H_ */
