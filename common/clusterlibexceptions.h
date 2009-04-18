/* 
 * ===========================================================================
 * $header$
 * $Revision$
 * $Date$
 * ===========================================================================
 */

#ifndef __CLUSTERLIBEXCEPTION_H__
#define __CLUSTERLIBEXCEPTION_H__

#include <execinfo.h>
#include <cxxabi.h>

namespace clusterlib {
/**
 * \brief A general cluster related exception.
 */
class Exception 
    : public std::exception
{
  public:        
    /**
     * Constructor.
     */
    Exception(const std::string &msg) throw() 
        : m_message(msg) 
    {
        const int32_t maxDepth = 50;
        const int32_t maxFuncNameSize = 256;

        char *funcName = 
            static_cast<char *>(malloc(sizeof(char)*maxFuncNameSize));
        void **mangledArr = 
            static_cast<void **>(malloc(sizeof(void *)*maxDepth));
        int actualDepth = backtrace(mangledArr, maxDepth);
        char **symbols = backtrace_symbols(mangledArr, actualDepth);

        m_message.append("\nbacktrace:\n");
        int32_t status = -1;
        size_t actualFuncNameSize = 0;
        char *start = NULL;
        std::string tempMangledFuncName;
        for (int32_t i = 0; i < actualDepth; i++) {
            tempMangledFuncName = symbols[i];
            size_t startNameIndex = tempMangledFuncName.find('(');
            if (startNameIndex != std::string::npos) {
                tempMangledFuncName.erase(0, startNameIndex + 1);
            }
            size_t endNameIndex = tempMangledFuncName.find('+');
            if (endNameIndex != std::string::npos) {
                tempMangledFuncName[endNameIndex] = '\0';
                endNameIndex++;
            }
            size_t endOffsetIndex = tempMangledFuncName.rfind(')');
            if (endOffsetIndex != std::string::npos) {
                tempMangledFuncName[endOffsetIndex] = '\0';
            }
            actualFuncNameSize = maxFuncNameSize;
            start = abi::__cxa_demangle(&tempMangledFuncName[0],
                                        funcName,
                                        &actualFuncNameSize,
                                        &status);
            if (status == 0) {
                funcName = start;
                m_message.append(funcName, 0, actualFuncNameSize);
            }
            else {
                m_message.append(tempMangledFuncName);
            }

            m_message.append(" offset: ");
            m_message.append(&tempMangledFuncName[endNameIndex]);
            m_message.append("\n");
        }
        
        free(symbols);
        free(funcName);
        free(mangledArr);
    }
    
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
 * Clusterlib internals are in an inconsistent state
 */
class InconsistentInternalStateException 
    : public Exception
{
  public:
   /**
     * Constructor.
     */
    InconsistentInternalStateException(const std::string &msg) throw() 
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
    InvalidArgumentsException(const std::string &msg) throw() 
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
    InvalidMethodException(const std::string &msg) throw() 
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
    ObjectRemovedException(const std::string &msg) throw() 
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
    RepositoryConnectionFailureException(const std::string &msg) throw() 
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
    RepositoryInternalsFailureException(const std::string &msg) throw() 
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
    SystemFailureException(const std::string &msg) throw() 
        : Exception(msg) {}
};



}	/* End of 'namespace clusterlib' */

#endif	/* _CLUSTERLIBEXCEPTION_H_ */

