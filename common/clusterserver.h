/*
 * clusterserver.h --
 *
 * Include file for server side types. Include this file if you are writing
 * an implementation of an application that is managed by clusterlib.
 *
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CLUSTERSERVER_H_
#define	_CLUSTERSERVER_H_

namespace clusterlib
{

class ClusterServer
    : public ClusterClient
{
};

};

#endif	/* !_CLUSTERSERVER_H_ */
