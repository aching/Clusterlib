#include "gui.h"
#include PATH_APR_XML_H
#include "xmlconfig.h"

using namespace std;

namespace configurator {

ConfigurationException::ConfigurationException(const string &message) {
    this->message = message;
}
    
ConfigurationException::~ConfigurationException() throw() {
}
    
const char *
ConfigurationException::what() const throw() {
    return message.c_str();
}

XmlConfig::XmlConfig() {
}

void
XmlConfig::Parse(const string &xmlFile, Configuration *config) {
    apr_pool_t *pool = NULL;
    apr_file_t *file = NULL;
    if (apr_pool_create(&pool, NULL) != APR_SUCCESS) {
        throw ConfigurationException(
            "Not enough memory to parse XML configuration.");
    }
    
    try {
        // Open the file
        if (apr_file_open(&file, 
                          xmlFile.c_str(), 
                          APR_READ | APR_BUFFERED, 
                          APR_OS_DEFAULT, 
                          pool) != APR_SUCCESS) {
            throw ConfigurationException(
                "XML configuration file " + xmlFile + " does not exist.");
        }
        
        // Parse the XML
        apr_xml_parser *parser = NULL;
        apr_xml_doc *doc = NULL;
        if (apr_xml_parse_file(pool, &parser, &doc, file, 4 * 1024) != 
            APR_SUCCESS) {
            char buf[4096];
            apr_xml_parser_geterror(parser, buf, sizeof(buf));
            throw ConfigurationException(
                "XML configuration file invalid (" + string(buf) + ").");
        }
        
        // Close file
        if (file) {
            apr_file_close(file);
            file = NULL;
        }
        
        if (strcmp(doc->root->name, "configuration") != 0) {
            throw ConfigurationException(
                "The root element of XML configuration should be "
                "'configuration'.");
        }
        
        apr_xml_elem *property = doc->root->first_child;
        
        // Iterate all the immediate child of root element
        while (property != NULL) {
            // Configuration sanity check
            if (strcmp(property->name, "property") != 0) {
                throw ConfigurationException(
                    "'property' element is expected, but '" + 
                    string(property->name) + "' found.");
            } 
            else if (property->attr != NULL) {
                throw ConfigurationException(
                    "'property' element must have no attribute.");
            }
            
            apr_xml_elem *nameElem = property->first_child;
            
            if (nameElem == NULL) {
                throw ConfigurationException(
                    "'name' element is expected under 'property' element.");
            } 
            else if (strcmp(nameElem->name, "name") != 0) {
                    throw ConfigurationException(
                        "'name' element is expected, but '" + 
                        string(nameElem->name) + "' found.");
            } 
            else if (nameElem->attr != NULL) {
                throw ConfigurationException(
                    "'name' element must have no attribute.");
            }
            
            apr_xml_elem *valueElem = nameElem->next;
            if (valueElem == NULL) {
                throw ConfigurationException(
                    "'value' element is expected under 'property' element.");
            } 
            else if (strcmp(valueElem->name, "value") != 0) {
                throw ConfigurationException(
                    "'value' element is expected, but '" + 
                    string(valueElem->name) + "' found.");
            } 
            else if (valueElem->attr != NULL) {
                throw ConfigurationException(
                    "'value' element must have no attribute.");
            }
            
            if (valueElem->next != NULL) {
                throw ConfigurationException(
                    "Only one 'name' element and one 'value' element "
                    "are expected under 'property' element, but '" + 
                    string(valueElem->next->name) + "' found.");
            }

            // Extract the name and value strings
            ostringstream name, value;
            
            if (nameElem->first_cdata.first != NULL) {
                apr_text *text = nameElem->first_cdata.first;
                do {
                    name << text->text;
                    text = text->next;
                } while (text);
            }
            
            if (valueElem->first_cdata.first != NULL) {
                apr_text *text = valueElem->first_cdata.first;
                do {
                    value << text->text;
                    text = text->next;
                } while (text);
            }
            
            (*config)[name.str()] = value.str();
            
            // Scan the next property
            property = property->next;
        }
        
        // Destroy the pool
        if (pool) {
            apr_pool_destroy(pool);
            pool = NULL;
        }
    } catch (ConfigurationException &ex) {
        // Release resources
        if (file) {
            apr_file_close(file);
            file = NULL;
        }
        if (pool) {
            apr_pool_destroy(pool);
            pool = NULL;
        }
        throw ex;
    }
}

}
