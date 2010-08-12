/*
 * internalchangehandlers.h --
 *
 * The definition of InternalChangeHandlers
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_INTERNALCHANGEHANDLERS_H_
#define	_CL_INTERNALCHANGEHANDLERS_H_

namespace clusterlib {

/*
 * The actual factory class.
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
