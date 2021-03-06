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

#include <sys/utsname.h>
#include <string.h>
#include <sys/wait.h>
#include "clusterlibinternal.h"
#include "activenodeperiodiccheck.h"

using namespace std;
using namespace boost;
using namespace clusterlib;
using namespace json;

namespace activenode {

ActiveNodePeriodicCheck::ActiveNodePeriodicCheck(
    int64_t msecsFrequency,
    const shared_ptr<Node> &nodeSP,
    PredMutexCond &predMutexCond)
    : Periodic(msecsFrequency, nodeSP),
      m_predMutexCond(predMutexCond)
{
}

ActiveNodePeriodicCheck::~ActiveNodePeriodicCheck()
{
}

void
ActiveNodePeriodicCheck::run()
{
    TRACE(CL_LOG, "run");

    if (getNotifyable() == NULL) {
        throw InconsistentInternalStateException(
            "run: getNotifyable() is NULL");
    }

    bool shutdownFound = false;
    JSONValue jsonShutdown;
    
    NotifyableLocker l(getNotifyable(),
                       CLString::NOTIFYABLE_LOCK,
                       DIST_LOCK_EXCL);

    getNotifyable()->cachedCurrentState().set(
        Notifyable::PID_KEY, ProcessThreadService::getPid());
    getNotifyable()->cachedCurrentState().set(
        Node::HEALTH_KEY, Node::HEALTH_GOOD_VALUE);
    shutdownFound = getNotifyable()->cachedDesiredState().get(
        Node::ACTIVENODE_SHUTDOWN, jsonShutdown);
    int64_t msecs = TimerService::getCurrentTimeMsecs();
    getNotifyable()->cachedCurrentState().set(
        Node::HEALTH_KEY, Node::HEALTH_GOOD_VALUE);
    getNotifyable()->cachedCurrentState().set(
        Node::HEALTH_SET_MSECS_KEY, msecs);
    getNotifyable()->cachedCurrentState().set(
        Node::HEALTH_SET_MSECS_AS_DATE_KEY, 
        TimerService::getMsecsTimeString(msecs));
    getNotifyable()->cachedCurrentState().publish();

    LOG_DEBUG(CL_LOG, 
              "run: pid=%" PRId32 ", shutdown=%s", 
              static_cast<int32_t>(ProcessThreadService::getPid()),
              JSONCodec::encode(jsonShutdown).c_str());

    if (shutdownFound) {
        if (jsonShutdown.get<JSONValue::JSONBoolean>() == true) {
            /* Let the ActiveNode know to shutdown. */
            m_predMutexCond.predSignal();
        }
    }
} 

}
