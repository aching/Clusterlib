/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#include <string>
  
#include "log.h"

using namespace std;
  
namespace clusterlib {

/**
 * \brief This class encapsulates a log4cxx configuration.
 */
class LogConfiguration
{
    public:
        LogConfiguration(const string &file) {
            PropertyConfigurator::configureAndWatch( file, 5000 );
        }
};

//enforces the configuration to be initialized
static LogConfiguration logConfig( "log4cxx.properties" );

}	/* End of 'namespace clusterlib' */
