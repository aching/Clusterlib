#ifndef _INCLUDED_XMLCONFIG_H_
#define _INCLUDED_XMLCONFIG_H_
#include <apr_xml.h>
#include <map>
#include <string>

namespace configurator {
    typedef std::map<std::string, std::string> Configuration;

    class ConfigurationException : public virtual std::exception {
    public:
        /**
         * Creates an instance of ConfigurationException with error message.
         * @param message the error message.
         */
        explicit ConfigurationException(const std::string &message);
        /**
         * Gets the error message of this exception.
         * @return the error message.
         */
        virtual const char *what() const throw();
        /**
         * Destroys the instance of ConfigurationException.
         */
        virtual ~ConfigurationException() throw();
    private:
        /**
         * Represents the error message.
         */
        std::string message;
    };

    class XmlConfig {
    public:
        static void Parse(const std::string &xmlFile, Configuration *config);
    private:
        XmlConfig();
    };
}
#endif
