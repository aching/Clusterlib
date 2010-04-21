/* 
 * ===========================================================================
 * $header$
 * $Revision$
 * $Date$
 * ===========================================================================
 */

#ifndef _CL_JSONEXEPTIONS_H_
#define _CL_JSONEXEPTIONS_H_

namespace json {

/**
 * Generic exception for the json namespace.
 */
class Exception 
    : public clusterlib::Exception
{
  public:
    /**
     * Constructor.
     */
    explicit Exception(const std::string &msg) throw()
        : clusterlib::Exception(msg) {}
};

/**
 * Defines the exception of a JSON parsing error.
 */
class JSONParseException 
    : public Exception
{
  public:
    /**
     * Creates an instance of JSONParseException with error message.
     *
     * @param msg the error message.
     */
    explicit JSONParseException(const std::string &msg) throw()
        : Exception(msg) {}
};

/**
 * Defines the exception of a JSON value error. For example, the
 * value is not compatible with its type.
 */
class JSONValueException 
    : public Exception 
{
  public:
    explicit JSONValueException(const std::string &msg) throw()
        : Exception(msg) {}
};

}	/* End of 'json' */

#endif	/* _CL_JSONEXEPTIONS_H_ */


