/*
 * cliformat.h --
 *
 * Definition of class CliFormat; it manages output to the console.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CLIFORMAT_H_
#define _CLIFORMAT_H_

/**
 * CliFormat class.
 */
class CliFormat {
  public:
    /** 
     * Print out a numeric data attribute. 
     *
     * @param attribute the attribute name
     * @param data the attribute data
     */
    static void attributeOut(std::string attribute, int32_t data) 
    {
        std::cout << " " << attribute << " : " << data << std::endl;
    }

    /** 
     * Print out a string data attribute. 
     *
     * @param attribute the attribute name
     * @param data the attribute data
     */
    static void attributeOut(std::string attribute, std::string data) 
    {
        std::cout << " " << attribute << " : " << data << std::endl;
    }

    /** 
     * Print out a vector of string for an attribute
     *
     * @param attribute the attribute name
     * @param data the attribute data in a vector
     */
    static void attributeOut(std::string attribute, 
                             std::vector<std::string> dataVec) 
    {
        std::cout << " " << attribute << " : ";
        for (size_t i = 0; i < dataVec.size(); i++)
            std::cout << dataVec[i] <<  " ";
        std::cout << std::endl;
    }

  private:
    /**
     * Constructor.  Shouldn't be instantiated.
     */
    CliFormat();
};

#endif	/* !_CLIFORMAT_H__H_ */
