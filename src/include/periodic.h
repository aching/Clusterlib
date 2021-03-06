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

#ifndef	_CL_PERIODIC_H_
#define	_CL_PERIODIC_H_

namespace clusterlib {

/**
 * Interface for objects that run periodically.  Users should subclass
 * this for anything that needs to be periodically done by
 * implementing run() i.e. health checking, updating the current
 * state, etc.
 */
class Periodic
{
  public:
    /**
     * Constructor.
     */
    explicit Periodic(
        int64_t msecsFrequency, 
        const boost::shared_ptr<Notifyable> &notifyableSP = 
        boost::shared_ptr<Notifyable>(), 
        ClientData clientData = NULL)
        : m_runMsecsFrequency(msecsFrequency),
          m_notifyableSP(notifyableSP),
          m_clientData(clientData) {}

    /**
     * Virtual destructor.
     */
    virtual ~Periodic() {}

    /**
     * Subclasses define this function to be run periodically.
     */
    virtual void run() = 0;

    /**
     * Get the amount of msecs to periodic do run().
     * 
     * @return The amount of time in milliseconds.
     */
    int64_t getMsecsFrequency() 
    {
        Locker l(getMutex());

        return m_runMsecsFrequency;
    }

    /**
     * Get the amount of msecs to periodic do run().
     * 
     * @return The amount of time in milliseconds.
     */
    void setMsecsFrequency(int64_t msecsFrequency) 
    {
        Locker l(getMutex());

        if (msecsFrequency > 0) {
            m_runMsecsFrequency = msecsFrequency;
        }
        else {
            throw InvalidArgumentsException(
                "setMsecsFrequency: msecsAllowedPerHealthCheck <= 0");
        }
    }

    const boost::shared_ptr<Notifyable> &getNotifyable() 
    { 
        Locker l(getMutex());

        return m_notifyableSP; 
    }
    
    void setNotifyable(const boost::shared_ptr<Notifyable> &notifyableSP) 
    {
        Locker l(getMutex());

        m_notifyableSP = notifyableSP;
    }

    ClientData getClientData() 
    {
        Locker l(getMutex());

        return m_clientData; 
    }

    void setClientData(ClientData clientData) 
    {
        Locker l(getMutex());

        m_clientData = clientData;
    }

  private:
    Mutex *getMutex() { return &m_mutex; }

  private:
    /**
     * Lock to guarantee consistent internal state
     */
    Mutex m_mutex;

    /**
     * Milliseconds to wait in between run().
     */
    int64_t m_runMsecsFrequency;

    /**
     * Saved Notifyable.
     */
    boost::shared_ptr<Notifyable> m_notifyableSP;

    /**
     * Client data 
     */
    ClientData m_clientData;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_PERIODIC_H_ */
