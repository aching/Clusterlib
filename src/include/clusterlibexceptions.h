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

#ifndef _CL_CLUSTERLIBEXCEPTIONS_H_
#define _CL_CLUSTERLIBEXCEPTIONS_H_

namespace clusterlib  {

/**
 * \brief A general clusterlib related exception.
 */
class Exception 
    : public std::exception
{
  public:        
    /**
     * Constructor.
     */
    explicit Exception(const std::string &msg);

    /**
     * Demangle any name if possible to a string.
     * 
     * @param mangledName pointer to the mangled name
     * @param success true on success, false on failure
     * @return a string with the demangled name if successful, 
     *         original string on failure
     */ 
    static std::string demangleName(const char *mangledName, bool &success);

    /**
     * Destructor.
     */
    ~Exception() throw() {}
    
    /**
     * Returns detailed description of the exception.
     */
    virtual const char *what() const throw() 
    {
        return m_message.c_str();
    }
    
  private:
    /**
     * The detailed message associated with this exception.
     */
    std::string m_message;
};

/**
 * The node is already connected, cannot start another
 * health checker.
 */
class AlreadyConnectedException
    : public Exception
{
  public:
    /*
     * Constructor.
     */
    explicit AlreadyConnectedException(const std::string &msg) throw()
        : Exception(msg) {}
};

/**
 * Clusterlib internals are in an inconsistent state
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
 * Invalid arguments given to a clusterlib member function.
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
 * Invalid clusterlib method called (should not have called this method).
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
 * A publish() operation on a clusterlib object failed since versions
 * don't match.
 */
class PublishVersionException
    : public Exception
{
  public:
   /**
     * Constructor.
     */
    explicit PublishVersionException(const std::string &msg) throw() 
        : Exception(msg) {}
};

/**
 * A clusterlib object have been removed
 */
class ObjectRemovedException
    : public Exception
{
  public:
   /**
     * Constructor.
     */
    explicit ObjectRemovedException(const std::string &msg) throw() 
        : Exception(msg) {}
};

/**
 * Repository connection failure
 */
class RepositoryConnectionFailureException
    : public Exception
{
  public:
   /**
     * Constructor.
     */
    explicit RepositoryConnectionFailureException(
        const std::string &msg) throw() 
        : Exception(msg) {}
};

/**
 * Repository internals failure
 */
class RepositoryInternalsFailureException
    : public Exception
{
  public:
   /**
    * Constructor.
    */
    explicit RepositoryInternalsFailureException(
        const std::string &msg) throw() 
        : Exception(msg) {}
};

/**
 * System failure (gethostname, pthread_self, etc.).
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
 * Accessing the repository for a particular data is missing (not failure.
 */
class RepositoryDataMissingException
    : public Exception
{
  public:
   /**
     * Constructor.
     */
    explicit RepositoryDataMissingException(const std::string &msg) throw() 
        : Exception(msg) {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* _CLUSTERLIBEXCEPTIONS_H_ */

