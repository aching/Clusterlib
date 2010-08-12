/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */
  
#include "clusterlibinternal.h"
  
using namespace std;
using namespace log4cxx;

namespace clusterlib  {

/**
 * The amount of delay prior to checking if the filename has changed
 * or been created.  Changes to the log configuration file will be
 * propagated after the delay.
 */
static const long CheckFileDelayMsecs = 5000;

/**
 * \brief This class encapsulates a log4cxx configuration.
 */
class LogConfiguration
{
  public:
    LogConfiguration(const string &file) {
        PropertyConfigurator::configureAndWatch(file, CheckFileDelayMsecs);
    }
};

/**
 * Enforces the configuration to be initialized.
 */
static LogConfiguration logConfig("log4cxx.properties");

}	/* End of 'namespace clusterlib' */
