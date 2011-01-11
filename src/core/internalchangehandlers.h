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

#ifndef	_CL_INTERNALCHANGEHANDLERS_H_
#define	_CL_INTERNALCHANGEHANDLERS_H_

namespace clusterlib {

/*
 * Any change handlers that are internal, not external to Clusterlib.
 */
class InternalChangeHandlers
{
  public:

    /**
     * Handle existence change for preceding lock node.
     */
    Event handlePrecLockNodeExistsChange(int32_t etype,
                                         const std::string &key);

    /*
     * Get the CachedObjectEventHandler for the appropriate change event
     */
    InternalEventHandler *getPrecLockNodeExistsHandler()
    {
        return &m_precLockNodeExistsHandler;
    }

    /*
     * Constructor used by FactoryOps.
     */
    InternalChangeHandlers(FactoryOps *factoryOps) 
        : mp_ops(factoryOps),
          m_precLockNodeExistsHandler(
              this,
              &InternalChangeHandlers::handlePrecLockNodeExistsChange) {}

  private:
    /**
     * Private access to mp_ops
     */
    FactoryOps *getOps() { return mp_ops; }
    
  private:
    /**
     * Does all the factory operations
     */
    FactoryOps *mp_ops;

    /*
     * Handlers for event delivery.
     */
    InternalEventHandler m_precLockNodeExistsHandler;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_INTERNALCHANGEHANDLERS_H_ */
