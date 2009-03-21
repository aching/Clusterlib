/* 
 * ===========================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ===========================================================================
 */

#ifndef __CLUSTEREXCEPTION_H__
#define __CLUSTEREXCEPTION_H__

namespace clusterlib {
/**
 * \brief A cluster related exception.
 */
class ClusterException 
    : public std::exception
{
    public:        
        /**
         * Constructor.
         */
        ClusterException(const std::string &msg) throw() : m_message(msg) {}
        
        /**
         * Destructor.
         */
        ~ClusterException() throw() {}
        
        /**
         * Returns detailed description of the exception.
         */
        const char *what() const throw() 
	{
            return m_message.c_str();
        }

    private:
        /**
         * The detailed message associated with this exception.
         */
        std::string m_message;
};

}	/* End of 'namespace clusterlib' */

#endif	/* _CLUSTEREXCEPTION_H_ */

