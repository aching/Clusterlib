/*
 * application.h --
 *
 * Definition of class Application; it represents a set of groups of nodes that
 * together form a clusterlib application.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_APPLICATION_H_
#define _APPLICATION_H_

namespace clusterlib
{

/*
 * Definition of class Application.
 */
class Application
    : public virtual Group
{
  public:

    virtual Group *getMyGroup() 
    {
        throw ClusterException("Application cannot be a part of a group!");
    }

  protected:
    /*
     * Friend declaration for factory so that it can call
     * the protected constructor.
     */
    friend class Factory;

    /*
     * Constructor used by Factory.
     */
    Application(const string &name, const string &key, FactoryOps *fp)
        : Notifyable(fp, key, name, NULL),
          Group(name, key, fp, NULL)
    {
    }

    /*
     * Initialize the cached representation.
     */
    virtual void initializeCachedRepresentation();

  private:
    /*
     * The default constructor is private so no one can call it.
     */
    Application()
        : Notifyable(NULL, "", "", NULL),
          Group("", "", NULL, NULL)
    {
        throw ClusterException("Someone called the Application "
                               "default constructor!");
    }

    /*
     * Make the destructor private also.
     */
    virtual ~Application() {};

  private:


};

};	/* End of 'namespace clusterlib' */

#endif	/* !_APPLICATION_H_ */
