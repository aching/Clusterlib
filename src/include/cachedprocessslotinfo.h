/*
 * cachedprocessslotinfo.h --
 *
 * Definition of class CachedProcessSlotInfo; it represents cached
 * data of a node's process slot information.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDPROCESSSLOTINFO_H_
#define _CL_CACHEDPROCESSSLOTINFO_H_

namespace clusterlib {

/**
 * Definition of class CachedProcessSlotInfo
 */
class CachedProcessSlotInfo
    : public virtual CachedData
{
  public:
    /**
     * Are the process slots enabled?
     * 
     * @return true Return true if the process slots are enabled, 
     *         false otherwise.
     */
    virtual bool getEnable() = 0;
        
    /**
     * Set whether the procecess slots are enabled.
     *
     * @param enable If true, enable the process slots.  Otherwise, disable 
     *        the process slots.
     */
    virtual void setEnable(bool enable) = 0;

    /**
     * How many process slots are allocated?
     *
     * @return Returns the number of maximum process slots .
     */
    virtual int32_t getMaxProcessSlots() = 0;
    
    /**
     *  Set the number of maximum process slots.
     *
     * @param maxProcessSlots The number of maximum process slots allowed for
     *        this notifyable.
     */
    virtual void setMaxProcessSlots(int32_t maxProcessSlots) = 0;

    /**
     * Destructor.
     */
    virtual ~CachedProcessSlotInfo() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDPROCESSSLOTINFO_H_ */
