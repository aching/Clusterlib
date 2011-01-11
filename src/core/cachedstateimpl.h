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

#ifndef	_CL_CACHEDSTATEIMPL_H_
#define _CL_CACHEDSTATEIMPL_H_

namespace clusterlib {

/**
 * Implementation of class CachedState.
 */
class CachedStateImpl
    : public virtual CachedDataImpl,
      public virtual CachedState
{
  public:
    virtual int32_t publish(bool unconditional = false);

    virtual void loadDataFromRepository(bool setWatchesOnly);

  public:
    virtual int32_t getMaxHistorySizePublished();

    virtual void setMaxHistorySizePublished(int32_t maxHistorySize);

    virtual int32_t getHistorySize();

    virtual std::vector<json::JSONValue::JSONString> getHistoryKeys(
        int32_t historyIndex);

    virtual bool getHistory(int32_t historyIndex,
                            const std::string &key, 
                            json::JSONValue &jsonValue);

    virtual bool get(const std::string &key, 
                     json::JSONValue &jsonValue);

    virtual void set(const ::json::JSONValue::JSONString &key, 
                     const ::json::JSONValue &value);

    virtual bool erase(const ::json::JSONValue::JSONString &key);

    virtual void clear();

    virtual json::JSONValue::JSONArray getHistoryArray();

    enum StateType {
        CURRENT_STATE = 0,
        DESIRED_STATE
    };

    /**
     * Constructor.
     */
    explicit CachedStateImpl(NotifyableImpl *notifyable, 
                             StateType stateType);

    /**
     * Destructor.
     */
    virtual ~CachedStateImpl() {}
    
  private:
    /**
     * Maximum number of states to keep when publishing.
     */
    int32_t m_maxHistorySize;

    /**
     * Type of state.
     */
    StateType m_stateType;

    /**
     * The historical state array in user-defined format.
     */
    ::json::JSONValue::JSONArray m_historyArr;

    /**
     * The current staete that will be added to the history state
     * array upon publishing.
     */
    ::json::JSONValue::JSONObject m_state;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDSTATEIMPL_H_ */
