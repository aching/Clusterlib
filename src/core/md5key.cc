#include "clusterlibinternal.h"
#include <istream>
#include "md5key.h"

using namespace std;

#include "md5/md5.h"

namespace clusterlib
{

HashRange
Md5Key::hashKey() const
{
    MD5 context;
    /**
     * Hacky, but update() requires an unsigned char *.
     */
    context.update(reinterpret_cast<unsigned char *>(
                       const_cast<char *>(m_key.c_str())), 
                   m_key.size());
    context.finalize();
    
    /* Since HashRange is only 8 bytes instead of 16, only use the top
     * 8 bytes */
    char *hexOutput = context.hex_digest();
    hexOutput[16] = '\0';
    HashRange res = ::strtoll(hexOutput, NULL, 16);
    delete hexOutput;

    return res;
}

};	/* End of 'namespace clusterlib' */
