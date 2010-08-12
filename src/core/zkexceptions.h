/* 
 * ===========================================================================
 * $header$
 * $Revision$
 * $Date$
 * ===========================================================================
 */

#ifndef _CL_ZKEXEPTIONS_H_
#define _CL_ZKEXEPTIONS_H_

namespace zk {

/**
 * Generic exception for the Zookeeper namespace.
 */
class Exception 
    : public clusterlib::Exception
{
  public:
    /**
     * Constructor.
     */
    explicit Exception(const std::string &msg) throw()
        : clusterlib::Exception(msg),
          m_errorCode(0), 
          m_connected(true) {}
    
    /**
     * Constructor.
     */
    Exception(const std::string &msg, 
              int32_t errorCode, 
              bool connected) throw()
        : clusterlib::Exception(msg),
          m_errorCode(errorCode), 
          m_connected(connected) {}
    
    /**
     * Returns the error code.
     */
    int32_t getErrorCode() const 
    {
        return m_errorCode;
    }
    
    /**
     * \brief Returns whether the cause of the exception is that
     * the ZooKeeper connection is disconnected.
     */
    bool isConnected() 
    {
        return m_connected;
    }

  private:
    /**
     * Saved error code.
     */
    int32_t m_errorCode;

    /**
     * Whether the connection is open.
     */
    bool m_connected;
};

/**
 * Zookeeper internals are in an inconsistent state
 */
class InconsistentInternalStateException 
    : public Exception
{
  public:
    /**
    * Constructor.
    */
    explicit InconsistentInternalStateException(
        const std::string &msg) throw() 
        : Exception(msg) {}
};

/**
 * Invalid arguments given to a zookeeper member function.
 */
class InvalidArgumentsException 
    : public Exception
{
  public:
   /**
     * Constructor.
     */
    explicit InvalidArgumentsException(const std::string &msg) throw() 
        : Exception(msg) {}
};

/**
 * Invalid zookeeper method called (should not have called this method).
 */
class InvalidMethodException
    : public Exception
{
  public:
   /**
     * Constructor.
     */
    explicit InvalidMethodException(const std::string &msg) throw() 
        : Exception(msg) {}
};

/**
 * System failure (i.e. gethostname, pthread_self, etc)
 */
class SystemFailureException
    : public Exception
{
  public:
   /**
     * Constructor.
     */
    explicit SystemFailureException(const std::string &msg) throw() 
        : Exception(msg) {}
};

/**
 * Client doesn't have permission to this ZooKeeper node.
 */
class NoAuthException
    : public Exception 
{
  public:
    /**
     * Constructor
     */
    NoAuthException(const std::string &msg, 
                    int32_t errorCode, 
                    bool connected) 
        : Exception(msg, errorCode, connected) {}
};

/**
 * Node doesn't exist that the client is trying to access.
 */
class NoNodeExistsException
    : public Exception
{
  public:
    /**
     * Constructor.
     */
    NoNodeExistsException(const std::string &msg, 
                          int32_t errorCode, 
                          bool connected) 
        : Exception(msg, errorCode, connected) {}
};

/**
 * There was a version mismatch when trying to set a node.
 */
class BadVersionException
    : public Exception
{
 public:
    /**
     * Constructor.
     */
    BadVersionException(const std::string &msg, 
                        int32_t errorCode, 
                        bool connected) 
        : Exception(msg, errorCode, connected) {}
};

/**
 * Invalid state when trying to do the Zookeeper operation.
 */
class InvalidStateException
    : public Exception
{
 public:
    /**
     * Constructor.
     */
    InvalidStateException(const std::string &msg, 
                          int32_t errorCode, 
                          bool connected) 
        : Exception(msg, errorCode, connected) {}
};

/**
 * Unknown or unable to handle this error code.
 */
class UnknownErrorCodeException
    : public Exception
{
 public:
    /**
     * Constructor.
     */
    UnknownErrorCodeException(const std::string &msg, 
                              int32_t errorCode, 
                              bool connected) 
        : Exception(msg, errorCode, connected) {}
};

}	/* End of 'zk' */

#endif	/* _CL_ZKEXEPTIONS_H_ */

