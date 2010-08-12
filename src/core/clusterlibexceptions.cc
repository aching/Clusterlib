/* 
 * ===========================================================================
 * $header$
 * $Revision$
 * $Date$
 * ===========================================================================
 */
#include "clusterlibinternal.h"
#include <execinfo.h>
#include <cxxabi.h>

using namespace std;

namespace clusterlib {

Exception::Exception(const string &msg)
    : m_message(msg) 
{
    const int32_t maxDepth = 50;
    
    void **mangledArr = 
        static_cast<void **>(malloc(sizeof(void *)*maxDepth));
    int actualDepth = backtrace(mangledArr, maxDepth);
    char **symbols = backtrace_symbols(mangledArr, actualDepth);
    
    m_message.append("\nbacktrace:\n");
    string tempMangledFuncName;
    bool success = false;
    for (int32_t i = 0; i < actualDepth; ++i) {
        tempMangledFuncName = symbols[i];
        size_t startNameIndex = tempMangledFuncName.find('(');
        if (startNameIndex != string::npos) {
            tempMangledFuncName.erase(0, startNameIndex + 1);
        }
        size_t endNameIndex = tempMangledFuncName.find('+');
        if (endNameIndex != string::npos) {
            tempMangledFuncName[endNameIndex] = '\0';
            endNameIndex++;
        }
        size_t endOffsetIndex = tempMangledFuncName.rfind(')');
        if (endOffsetIndex != string::npos) {
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

    LOG_INFO(CL_LOG, "Exception: %s", what());
}
    
string 
Exception::demangleName(const char *mangledName, bool &success)
{
    string demangledString;
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

}	/* End of 'namespace clusterlib' */
