/*
 * md5key.h --
 *
 * Implementation of md5key based on RSA Data Security, Inc. makes no
 * representations concerning either the merchantability of this
 * software or the suitability of this software for any particular
 * purpose. It is provided "as is" without express or implied warranty
 * of any kind.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _CL_MD5KEY_H_
#define _CL_MD5KEY_H_

namespace clusterlib
{

/**
 * Definition of Md5Key class
 */
class Md5Key : public Key
{
  public:
    Md5Key(std::string key) 
        : m_key(key) {}

    virtual HashRange hashKey() const;

    virtual ~Md5Key() {}

  private:
    /** The string key */
    std::string m_key;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_MD5KEY_H_ */
