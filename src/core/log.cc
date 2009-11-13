/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */
  
#include "clusterlibinternal.h"
  
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

/**
 * Enforces the configuration to be initialized.
 */
static LogConfiguration logConfig("log4cxx.properties");

}	/* End of 'namespace clusterlib' */
