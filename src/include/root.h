/*
 * root.h --
 *
 * Interface class Root; it represents a set of applications within
 * clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_ROOT_H_
#define _CL_ROOT_H_

namespace clusterlib {

/**
 * Definition of class Root.
 */
class Root
    : public virtual Notifyable
{
  public:
    /**
     * Get a list of names of all applications.
     * 
     * @return a copy of the list of all applications
     */
    virtual NameList getApplicationNames() = 0;

    /**
     * Get the name Application.
     *
     * @param name Name of the Application to create
     * @param accessType Mode of access
     * @param msecTimeout Amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param pApplicationSP Pointer to that pApplicationSP if it exists, 
     *                  otherwise NULL.
     * @return True if the operation finished before the timeout
     * @throw Exception if Notifyable is the root or application
     */
    virtual bool getApplicationWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Application> *pApplicationSP) = 0;

    /**
     * Get the named Application (no timeout).
     * 
     * @param name Name of the Application to get
     * @param accessType Mode of access
     * @return NULL if the named Application does not exist
     * @throw Exception only if tried to create and couldn't create
     */
    virtual boost::shared_ptr<Application> getApplication(
        const std::string &name,
        AccessType accessType) = 0;

    /*
     * Destructor.
     */
    virtual ~Root() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_ROOT_H_ */
