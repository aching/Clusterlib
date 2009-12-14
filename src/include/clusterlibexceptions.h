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
    Exception(const std::string &msg) 
        : m_message(msg) 
    {
        const int32_t maxDepth = 50;

        void **mangledArr = 
            static_cast<void **>(malloc(sizeof(void *)*maxDepth));
        int actualDepth = backtrace(mangledArr, maxDepth);
        char **symbols = backtrace_symbols(mangledArr, actualDepth);

        m_message.append("\nbacktrace:\n");
        std::string tempMangledFuncName;
        bool success = false;
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
            m_message.append(
                demangleName(&tempMangledFuncName[0], success));

            m_message.append(" offset: ");
            m_message.append(&tempMangledFuncName[endNameIndex]);
            m_message.append("\n");
        }
        
        free(symbols);
        free(mangledArr);
    }

    /**
     * Demangle any name if possible to a string.
     * 
     * @param mangledName pointer to the mangled name
     * @param success true on success, false on failure
     * @return a string with the demangled name if successful, 
     *         original string on failure
     */ 
    static std::string demangleName(const char *mangledName, bool &success) 
    {
        std::string demangledString;
        const size_t maxMangledNameSize = 512;
        size_t actualMangledNameSize = maxMangledNameSize;
        int status = -1;
        char *demangledName = 
            static_cast<char *>(malloc(sizeof(char)*maxMangledNameSize));

        abi::__cxa_demangle(mangledName,
                            demangledName,
                            &actualMangledNameSize,
                            &status);
        if (status == 0) {
            demangledString = demangledName;
            success = true;
        }
        else {
            demangledString = mangledName;
            success = false;
        }
        free(demangledName);

        return demangledString;
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
    AlreadyConnectedException(const std::string &msg) throw()
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

