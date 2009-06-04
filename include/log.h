/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#ifndef __LOG_H__
#define __LOG_H__

#include <log4cxx/logger.h> 
#include <log4cxx/propertyconfigurator.h> 
#include <log4cxx/helpers/exception.h> 

#define PRINTIP(x) ((uint8_t*)&x)[0], ((uint8_t*)&x)[1], \
                   ((uint8_t*)&x)[2], ((uint8_t*)&x)[3]

#define IPFMT "%u.%u.%u.%u"

#define DECLARE_LOGGER(varName) \
extern log4cxx::helpers::LoggerPtr varName;

#define DEFINE_LOGGER(varName, logName) \
static log4cxx::LoggerPtr varName = log4cxx::Logger::getLogger( logName );

#define MAX_BUFFER_SIZE 20000

#define SPRINTF_LOG_MSG(buffer, fmt, args...) \
    char buffer[MAX_BUFFER_SIZE]; \
    snprintf( buffer, MAX_BUFFER_SIZE, fmt, ##args );

#define LOG_TRACE(logger, fmt, args...) \
    if (logger->isDebugEnabled()) { \
        SPRINTF_LOG_MSG( __tmp, fmt, ##args ); \
        LOG4CXX_TRACE( logger, __tmp ); \
    }

#define LOG_DEBUG(logger, fmt, args...) \
    if (logger->isDebugEnabled()) { \
        SPRINTF_LOG_MSG( __tmp, fmt, ##args ); \
        LOG4CXX_DEBUG( logger, __tmp ); \
    }

#define LOG_INFO(logger, fmt, args...) \
    if (logger->isInfoEnabled()) { \
        SPRINTF_LOG_MSG( __tmp, fmt, ##args ); \
        LOG4CXX_INFO( logger, __tmp ); \
    }

#define LOG_WARN(logger, fmt, args...) \
    if (logger->isWarnEnabled()) { \
        SPRINTF_LOG_MSG( __tmp, fmt, ##args ); \
        LOG4CXX_WARN( logger, __tmp ); \
    }

#define LOG_ERROR(logger, fmt, args...) \
    if (logger->isErrorEnabled()) { \
        SPRINTF_LOG_MSG( __tmp, fmt, ##args ); \
        LOG4CXX_ERROR( logger, __tmp ); \
    }

#define LOG_FATAL(logger, fmt, args...) \
    if (logger->isFatalEnabled()) { \
        SPRINTF_LOG_MSG( __tmp, fmt, ##args ); \
        LOG4CXX_FATAL( logger, __tmp ); \
    }

#ifdef DISABLE_TRACE
#   define TRACE(logger, x)
#else   
#   define TRACE(logger, x) \
class Trace \
{ \
 public: \
    Trace(const void* p) : _p(p) { \
        LOG_TRACE(logger, "%s %p Enter", __PRETTY_FUNCTION__, p); \
    } \
    ~Trace() { \
        LOG_TRACE(logger, "%s %p Exit", __PRETTY_FUNCTION__, _p); \
    } \
    const void* _p; \
} traceObj(x);
#endif  /* DISABLE_TRACE */
    
#endif  /* __LOG_H__ */

