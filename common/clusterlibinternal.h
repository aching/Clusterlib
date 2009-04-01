/*
 * clusterlibinternal.h --
 *
 * The internal full include file for clusterlib.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CLUSTERLIBINTERNAL_H_
#define	_CLUSTERLIBINTERNAL_H_

#include "clusterlib.h"

#include "notifyableimpl.h"
#include "groupimpl.h"
#include "applicationimpl.h"
#include "nodeimpl.h"
#include "datadistributionimpl.h"
#include "propertiesimpl.h"
#include "rootimpl.h"
#include "clientimpl.h"
#include "serverimpl.h"

#include "notifyablekeymanipulator.h"
#include "cachedobjectchangehandlers.h"
#include "factoryops.h"

DEFINE_LOGGER(CL_LOG, "clusterlib")

#endif	/* !_CLUSTERLIBINTERNAL_H_ */
