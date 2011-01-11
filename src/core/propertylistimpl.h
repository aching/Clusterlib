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

#ifndef _CL_PROPERTYLISTIMPL_H_
#define _CL_PROPERTYLISTIMPL_H_

namespace clusterlib {

/**
 * Implementation of PropertyList.
 */
class PropertyListImpl
    : public virtual PropertyList,
      public virtual NotifyableImpl
{
  public:
    virtual CachedKeyValues &cachedKeyValues();    

    virtual bool getPropertyListWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<PropertyList> *pPropertyListSP);

    /*
     * Internal functions not used by outside clients
     */    
  public:
    /**
     * Constructor.
     */
    PropertyListImpl(FactoryOps *fp,
                     const std::string &key,
                     const std::string &name,
                     const boost::shared_ptr<NotifyableImpl> &parent);

    virtual NotifyableList getChildrenNotifyables();
    
    virtual void initializeCachedRepresentation();

    /**
     * Create the key-value JSONObject key
     *
     * @param propertyListKey the property list key
     * @return the generated key-value JSONObject key
     */
    static std::string createKeyValJsonObjectKey(
        const std::string &propertyListKey);

  private:
    /**
     * Do not call the default constructor.
     */
    PropertyListImpl();

  private:
    /**
     * The cached key-values
     */
    CachedKeyValuesImpl m_cachedKeyValues;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_PROPERTYLISTIMPL_H_ */

