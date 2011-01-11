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

#include "clusterlibinternal.h"
#include <sys/socket.h>
#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif
#include <netdb.h>
#include <fcntl.h>

using namespace json;
using namespace clusterlib;
using namespace std;
using namespace boost;

namespace clusterlib {

void
ZookeeperPeriodicCheck::run()
{
    TRACE(CL_LOG, "run");

    Locker l(&m_lock);

    ostringstream oss;
    if (getNotifyable() && 
        (m_hostPortVec.size() != m_nodeSPVec.size())) {
        oss.str("");
        oss << "run: m_hostPortVec.size=" << m_hostPortVec.size()
            << ", m_nodeSPVec.size=" << m_nodeSPVec.size();
        LOG_FATAL(CL_LOG, "%s", oss.str().c_str());
        throw InconsistentInternalStateException(oss.str());
    }

    JSONValue::JSONArray goodNodeArr;
    JSONValue::JSONArray badNodeArr;
    for (size_t i = 0; i < m_hostPortVec.size(); ++i) {
        string ruokRes;
        string enviRes;
        string reqsRes;
        string statRes;
        try {
            ruokRes = telnetCommand(
                m_hostPortVec[i].first, m_hostPortVec[i].second, "ruok", 1000);
            enviRes = telnetCommand(
                m_hostPortVec[i].first, m_hostPortVec[i].second, "envi", 1000);
            reqsRes = telnetCommand(
                m_hostPortVec[i].first, m_hostPortVec[i].second, "reqs", 1000);
            statRes = telnetCommand(
                m_hostPortVec[i].first, m_hostPortVec[i].second, "stat", 1000);
        }
        catch (const clusterlib::Exception &e) {
            LOG_ERROR(CL_LOG, "run: %s", e.what());
        }

        oss.str("");
        oss << m_hostPortVec[i].first << ":" << m_hostPortVec[i].second;
        if (ruokRes == "imok") {
            goodNodeArr.push_back(oss.str());
        }
        else {
            badNodeArr.push_back(oss.str()); 
        }

        if (getNotifyable() != NULL) {
            NotifyableLocker l(m_nodeSPVec[i],
                               CLString::NOTIFYABLE_LOCK,
                               DIST_LOCK_EXCL);

            m_nodeSPVec[i]->cachedCurrentState().set(
                Node::HEALTH_KEY, 
                (ruokRes == "imok") ? Node::HEALTH_GOOD_VALUE : 
                Node::HEALTH_BAD_VALUE);
            m_nodeSPVec[i]->cachedCurrentState().set(
                CLString::ZK_RUOK_STATE_KEY, ruokRes);
            m_nodeSPVec[i]->cachedCurrentState().set(
                CLString::ZK_ENVI_STATE_KEY, enviRes);
            m_nodeSPVec[i]->cachedCurrentState().set(
                CLString::ZK_REQS_STATE_KEY, reqsRes);
            m_nodeSPVec[i]->cachedCurrentState().set(
                CLString::ZK_STAT_STATE_KEY, statRes);
            m_nodeSPVec[i]->cachedCurrentState().publish();
        }
    }

    m_aggNodeStateObj.clear();
    m_aggNodeStateObj["Good Nodes"] = goodNodeArr;
    m_aggNodeStateObj["Good Node Count"] = goodNodeArr.size();
    m_aggNodeStateObj["Bad Nodes"] = badNodeArr;
    m_aggNodeStateObj["Bad Node Count"] = badNodeArr.size();
    m_aggNodeStateObj["Total Node Count"] = m_hostPortVec.size();

    if (getNotifyable() != NULL) {
        NotifyableLocker l(m_applicationSP,
                           CLString::NOTIFYABLE_LOCK,
                           DIST_LOCK_EXCL);

        m_applicationSP->cachedCurrentState().set(
            CLString::ZK_AGG_NODES_STATE_KEY, 
            m_aggNodeStateObj);
        int64_t currentMsecs = TimerService::getCurrentTimeMsecs();
        m_applicationSP->cachedCurrentState().set(
            CLString::STATE_SET_MSECS, currentMsecs);
        m_applicationSP->cachedCurrentState().set(
            CLString::STATE_SET_MSECS_AS_DATE, 
            TimerService::getMsecsTimeString(currentMsecs));
        m_applicationSP->cachedCurrentState().publish();
    }
}

ZookeeperPeriodicCheck::ZookeeperPeriodicCheck(
    int64_t msecsFrequency,
    const string &registry,
    const shared_ptr<Root> &rootSP)
    : Periodic(msecsFrequency, dynamic_pointer_cast<Notifyable>(rootSP))
{
    TRACE(CL_LOG, "ZookeeperPeriodicCheck");

    ostringstream oss;

    Locker l(&m_lock);

    if (getNotifyable() != NULL) {
        m_applicationSP = 
            rootSP->getApplication(string("Zookeeper (") + registry + ")",
                                   CREATE_IF_NOT_FOUND);
    }

    vector<string> componentVec;
    split(componentVec, registry, is_any_of(","));
    vector<string> hostPortVec;
    vector<string>::const_iterator componentVecIt;
    shared_ptr<Node> nodeSP;
    for (componentVecIt = componentVec.begin();
         componentVecIt != componentVec.end();
         ++componentVecIt) {
        if (componentVecIt->empty()) {
            continue;
        }

        hostPortVec.clear();
        split(hostPortVec, *componentVecIt, is_any_of(":"));
        if (hostPortVec.size() != 2) {
            oss.str("");
            oss << "ZookeeperPeriodicCheck: Invalid registry part - " 
                << *componentVecIt;
            throw clusterlib::InvalidArgumentsException(oss.str());
        }

        if (getNotifyable() != NULL) {
            nodeSP = 
                m_applicationSP->getNode(*componentVecIt, CREATE_IF_NOT_FOUND);
            
            nodeSP->acquireLock(CLString::OWNERSHIP_LOCK,
                                DIST_LOCK_EXCL);
            m_nodeSPVec.push_back(nodeSP);
        }

        m_hostPortVec.push_back(make_pair<string, int32_t>(
                                    hostPortVec[0], 
                                    ::atoi(hostPortVec[1].c_str())));
    }
}

ZookeeperPeriodicCheck::~ZookeeperPeriodicCheck()
{
    TRACE(CL_LOG, "~ZookeeperPeriodicCheck");

    Locker l(&m_lock);

    vector<shared_ptr<Node> >::const_iterator nodeVecIt;
    for (nodeVecIt = m_nodeSPVec.begin(); 
         nodeVecIt != m_nodeSPVec.end(); 
         ++nodeVecIt) {
        (*nodeVecIt)->releaseLock(CLString::OWNERSHIP_LOCK);
    }
}

JSONValue::JSONObject
ZookeeperPeriodicCheck::getAggNodeState()
{
    TRACE(CL_LOG, "getAggNodeState");

    Locker l(&m_lock);

    return m_aggNodeStateObj;
}


string
ZookeeperPeriodicCheck::telnetCommand(const string &host, 
                                      int32_t port, 
                                      const string &command, 
                                      int64_t maxMsecs) const
{
#ifdef HAVE_SYS_EPOLL_H
    const int32_t telnetEpollMsecsTimeout = 100;
    ostringstream oss;
    string resp;
    int64_t upperLimitMsecs = TimerService::getCurrentTimeMsecs() + maxMsecs;

    LOG_DEBUG(CL_LOG, 
              "telnetCommand: Trying host=%s,port=%" PRId32 ",command=%s",
              host.c_str(),
              port,
              command.c_str());

    Sock sock(socket(AF_INET, SOCK_STREAM, 0));
    if (sock.getFd() == -1) {
        oss.str("");
        oss << "telnetCommand: sock=-1, errno=" << errno << ", strerror="
            << strerror(errno);
        throw clusterlib::SystemFailureException(oss.str());
    }
    
    /* Get IP address */
    addrinfo hints;
    addrinfo *res;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int32_t ret = getaddrinfo(host.c_str(), NULL, &hints, &res);
    if (ret != 0) {
        oss.str("");
        oss << "telnetCommand: Error - getaddrinfo returned "
            << ret << " with error " << strerror(ret);
        throw clusterlib::SystemFailureException(oss.str());
    }
    
    /* Set the socket as nonblocking */
    int32_t flags = fcntl(sock.getFd(), F_GETFL, 0);
    if (flags < 0) {
        oss.str("");
        oss << "telnetCommand: Error - get socket flag failed with error " 
            << strerror(errno);
        throw clusterlib::SystemFailureException(oss.str());
    }
    ret = fcntl(sock.getFd(), F_SETFL, flags | O_NONBLOCK);
    if (ret < 0) {
        oss.str("");
        oss << "telnetCommand: Error - set socket flag failed with error " 
            << strerror(errno);
        throw clusterlib::SystemFailureException(oss.str());
    }

    /* Make connection (nonblocking) */
    sockaddr sockAddr = *(res->ai_addr);
    sockaddr_in &addrServer = reinterpret_cast<sockaddr_in &>(sockAddr);
    addrServer.sin_port = htons(port);
    ret = connect(sock.getFd(), &sockAddr, sizeof(sockAddr));

    if (ret != 0 && errno != EINPROGRESS) {
        oss.str("");
        oss << "telnetCommand: Error - nonblocking connect "
            << "returned " << ret << " with error: "
            << strerror(errno);
        freeaddrinfo(res);
        throw clusterlib::SystemFailureException(oss.str());
    }
    freeaddrinfo(res);

    /* 
     * Setup epoll to ensure the connection was made, send the data,
     * and get a response within the allotted time.
     */
    Sock epollSock(epoll_create(1));
    if (epollSock.getFd() == -1) {
        oss.str("");
        oss << "telnetCommand: epoll_create returned " << epollSock.getFd()
            << " errno=" << errno << ", strerror=" << strerror(errno);
        throw clusterlib::SystemFailureException(oss.str());
    }

    epoll_event event;
    bzero(&event, sizeof(event));
    event.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR;
    event.data.fd = sock.getFd();
    ret = epoll_ctl(epollSock.getFd(), EPOLL_CTL_ADD, sock.getFd(), &event);
    if (ret != 0) {
        oss.str("");
        oss << "telnetCommand: epoll_ctl returned " << ret 
            << " errno=" << errno << ", strerror=" << strerror(errno);
        throw clusterlib::SystemFailureException(oss.str());
    }

    const int32_t bufSize = 4096;
    char buf[bufSize];
    size_t commandIndex = 0;
    do {
        bzero(&event, sizeof(event));
        ret = epoll_wait(
            epollSock.getFd(), &event, 1, telnetEpollMsecsTimeout);
        if (ret == -1) {
            oss.str("");
            oss << "telnetCommand: epoll_wait returned " << ret
                 << " errno=" << errno << ", strerror=" << strerror(errno);
            throw clusterlib::SystemFailureException(oss.str());
        }
        else if (ret == 1) {
            if (event.data.fd != sock.getFd()) {
                oss.str("");
                oss << "telnetCommand: event.data.fd=" << event.data.fd 
                    << ", sock.getFd="  << sock.getFd();
                throw SystemFailureException(oss.str());
            }

            if (event.events & EPOLLIN) {
                ret = read(event.data.fd, buf, sizeof(buf));
                if (ret == -1 && errno == EAGAIN) {
                    continue;
                }
                else if (ret < 0) {
                    oss.str("");
                    oss << "telnetCommand: read ret=" << ret << ", errno=" 
                        << errno << ", strerror=" << strerror(errno);
                    throw SystemFailureException(oss.str());
                }
                else {
                    resp.append(buf, ret);
                }
            }
            if ((event.events & EPOLLOUT) && 
                (commandIndex != command.size())) {
                ret = write(event.data.fd, 
                            &command[commandIndex], 
                            command.size() - commandIndex);
                if (ret == -1 && errno == EAGAIN) {
                    continue;
                }
                else if (ret < 0) {
                    oss.str("");
                    oss << "telnetCommand: write ret=" << ret << ", errno=" 
                        << errno << ", strerror=" << strerror(errno);
                    throw SystemFailureException(oss.str());
                }
                else {
                    commandIndex += ret;
                }
            }
            if (event.events & EPOLLERR) {
                oss.str("");
                oss << "telnetCommand: epoll_wait EPOLLERR, errno=" 
                    << errno << ", strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            if (event.events & EPOLLHUP) {
                if (commandIndex != command.size()) {
                    oss.str("");
                    oss << "telnetCommand: epoll_wait EPOLLHUP prior to "
                        << "command being sent, commandIndex=" << commandIndex;
                    throw SystemFailureException(oss.str());
                }
                break;
            }
        }
        else if (ret == 0) {
            continue;
        }
        else {
            oss.str("");
            oss << "telnetCommand: Impossible that epoll_wait returns " << ret;
            throw InconsistentInternalStateException(oss.str());
        }
    } while (TimerService::getCurrentTimeMsecs() < upperLimitMsecs);

    return resp;
#else
    return "Not implemented (epoll not supported on this system)";
#endif
}

}
