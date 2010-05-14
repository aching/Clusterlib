/*
 * cachedprocessslotinfoimpl.h --
 *
 * Implementation of class CachedProcessSlotInfoImpl; it represents cached
 * data of a node's process slot information.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDPROCESSSLOTINFOIMPL_H_
#define _CL_CACHEDPROCESSSLOTINFOIMPl_H_

namespace clusterlib
{

/**
 * Definition of class CachedProcessSlotInfoImpl
 */
class CachedProcessSlotInfoImpl
    : public virtual CachedDataImpl,
      public virtual CachedProcessSlotInfo
{
  public:
    virtual int32_t publish(bool unconditional = false);

    virtual void loadDataFromRepository(bool setWatchesOnly);

  public:
    enum ProcessSlotInfoArrEnum {
        ENABLE = 0,
        MAX_PROCESSES,
        MAX_SIZE
    };

    virtual bool getEnable();
        
    virtual void setEnable(bool enable);

    virtual int32_t getMaxProcessSlots();
    
    virtual void setMaxProcessSlots(int32_t maxProcessSlots);

    /**
     * Constructor.
     */
    explicit CachedProcessSlotInfoImpl(NotifyableImpl *ntp);

    /**
     * Destructor.
     */
    virtual ~CachedProcessSlotInfoImpl() {}
    
  private:
    /**
     * The cached process slot information
     * format is [bool enable, integer maxProcesses].
     */
    ::json::JSONValue::JSONArray m_processSlotInfoArr;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDPROCESSSLOTINFOIMPL_H_ */
