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

#ifndef	_ROOT_H_
#define _ROOT_H_

namespace clusterlib
{

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
     * Get the named application.
     * 
     * @param create create the application if doesn't exist
     * @return NULL if the named application does not exist and create
     * == false
     * @throw Exception only if tried to create and couldn't create
     */
    virtual Application *getApplication(const std::string &appName,
                                        bool create = false) = 0;

    /**
     * Get a notifyable from a key. 
     *
     * @param key the key that represents a notifyable.
     * @return a pointer to that notifyable if it exists, otherwise NULL.
     */
    virtual Notifyable *getNotifyableFromKey(const std::string &key) = 0;

    /*
     * Destructor.
     */
    virtual ~Root() {};
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_ROOT_H_ */
