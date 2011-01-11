/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef	_CL_CLUSTERLIBINTERNAL_H_
#define	_CL_CLUSTERLIBINTERNAL_H_

/*
 * The internal full include file for clusterlib.
 */

#include "clusterlib.h"

#include "log.h"
#include "clstringinternal.h"
#include "clnumericinternal.h"
#include "intervaltree.h"
#include "event.h"
#include "signalmap.h"
#include "clusterlibrpc.h"
#include "callbackandcontext.h"
#include "zkexceptions.h"
#include "zkadapter.h"

#include "cacheddataimpl.h"
#include "cachedkeyvaluesimpl.h"
#include "cachedprocessinfoimpl.h"
#include "cachedprocessslotinfoimpl.h"
#include "cachedshardsimpl.h"
#include "cachedstateimpl.h"
#include "notifyableimpl.h"
#include "groupimpl.h"
#include "applicationimpl.h"
#include "nodeimpl.h"
#include "processslotimpl.h"
#include "datadistributionimpl.h"
#include "propertylistimpl.h"
#include "rootimpl.h"
#include "queueimpl.h"
#include "unknownhashrange.h"
#include "safenotifyablemap.h"
#include "clientimpl.h"
#include "registerednotifyable.h"
#include "registerednotifyableimpl.h"
#include "registeredrootimpl.h"
#include "registeredapplicationimpl.h"
#include "registeredgroupimpl.h"
#include "registereddatadistributionimpl.h"
#include "registerednodeimpl.h"
#include "registeredqueueimpl.h"
#include "registeredprocessslotimpl.h"
#include "registeredpropertylistimpl.h"

#include "notifyablekeymanipulator.h"
#include "cachedobjectchangehandlers.h"
#include "internalchangehandlers.h"
#include "distributedlocks.h"
#include "factoryops.h"
#include "jsonrpcresponsehandler.h"
#include "jsonrpcmethodhandler.h"

DEFINE_LOGGER(CL_LOG, "clusterlib")

#endif	/* !_CL_CLUSTERLIBINTERNAL_H_ */
