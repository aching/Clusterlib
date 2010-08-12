/*
 * unknownhashrange.h --
 *
 * Definition of class UnknownHashRange; it represents a HashRange
 * subclass that has no known type.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_UNKNOWNHASHRANGE_H_
#define _CL_UNKNOWNHASHRANGE_H_

namespace clusterlib {

class UnknownHashRange 
    : public HashRange
{
  public:
    static std::string name();

    virtual std::string getName() const;

    virtual bool isBegin();

    virtual bool isEnd();
    
    virtual bool operator< (const HashRange &other) const;
    
    virtual bool operator== (const HashRange &other) const;

    virtual HashRange & operator= (const HashRange &other);

    virtual json::JSONValue toJSONValue() const;

    virtual void set(const json::JSONValue &jsonValue);

    virtual HashRange &create() const;

    virtual UnknownHashRange & operator++ ();

    /**
     * Virtual destructor.
     */ 
    virtual ~UnknownHashRange() {}

    /**
     * Constructor.
     */
    explicit UnknownHashRange(const json::JSONValue &jsonValue)
        : m_jsonValue(jsonValue)
    {
    }

    /**
     * No argument constructor.
     */
    UnknownHashRange() {}

  private:
    /**
     * The underlying JSONValue.
     */
    json::JSONValue m_jsonValue;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_UNKNOWNHASHRANGE_H_ */
