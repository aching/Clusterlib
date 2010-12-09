/*
 * json.h --
 *
 * Definition of json namespace and classes.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _CL_JSON_H_
#define _CL_JSON_H_

#include "clusterlib.h"

/**
 * Defines the namespace of JSON encoder and decoder. The utility
 * class JSONCodec is the single entry of parsing and composing JSON
 * messages.
 */
namespace json  {
    
/**
 * Defines the value in JSON. The value can be a number (integer
 * or floating point), a boolean, a string, a vector or a map.
 */
class JSONValue
{
  public:
    typedef std::string JSONString;
    typedef std::deque<JSONValue> JSONArray;
    typedef std::map<JSONString, JSONValue> JSONObject;
    typedef int64_t JSONInteger;
    typedef uint64_t JSONUInteger;
    typedef long double JSONFloat;
    typedef bool JSONBoolean;
    
    /**
     * Defines the type for NULL value in JSON.
     */
    class JSONNull {
      public:

        /**
         * Compares if the two instance equal. For Null values in
         * JSONValue, they always equal.  

         * @param other the other Null value to be compared.  
         * @return true
         */
        bool operator==(const JSONNull &other) const;

        /**
         * Assigns the Null value to another. Since Null values in 
         * JSONValue are the same, it does nothing.
         *
         * @param other the other Null value whose value assigns to 
         *        this instance.
         * @return the instance represented by other parameter.
         */
        const JSONNull &operator=(const JSONNull &other);

        /**
         * Creates a copy of the Null value.
         * @param other the Null value to be copied.
         */
        JSONNull(const JSONNull &other);

      private:
        /**
         * Creates a new instance of NullType. This constructor is
         * private so that
         */
        JSONNull();

        friend class JSONValue;
    };

    /**
     * Creates a new instance of JSONValue. The value is empty.
     */
    JSONValue();

    /**
     * Creates a new instance of JSONValue. The new instance is a copy
     * of the given instance.
     *
     * @param other the JSONValue instance to be copied.
     */
    JSONValue(const JSONValue &other);

    /**
     * Creates a new instance of JSONValue with value set.  T is the
     * type of the value.
     *
     * @param value the value.
     */
    template<class T> JSONValue(const T &value) {
        set(value);
    }

    /**
     * Gets the value. A copy of the value is returned.
     * T is the type of the value.
     *
     * @return the value.
     * @throws JSONValueException 
     *         if the value is not compatible with the return type.
     */
    template<class T> T get() const {
        try {
            return boost::any_cast<T>(value);
        } catch (const std::bad_cast &ex) {
            bool success;
            std::string currentDemangledTypeName = 
                clusterlib::Exception::demangleName(type().name(), success);
            std::string newDemangledTypeName = 
                clusterlib::Exception::demangleName(typeid(T).name(), success);
            throw JSONValueException(
                "get: Current type name (" + currentDemangledTypeName +
                ") cannot be converted to desired type name (" + 
                newDemangledTypeName + ")");
        }
    }

    /**
     * Gets the value. A copy of the value is returned.  T is the type
     * of the value.
     *
     * @param value the pointer where the copy of the value is stored.
     * @return the value.
     * @throws JSONValueException if the value is not compatible with the 
     *         return type.
     */
    template<class T> T &get(T *value) const {
        *value = get<T>();
        return *value;
    }

    /**
     * Sets the value. Only integer, float, double, bool, vector of
     * JSONValue, and map of string and JSONValue can be set.  T the
     * type of the value.
     * 
     * @param value the value to be set.
     */
    template<class T> void set(const T &value);

    /**
     * Sets the value as string.
     *
     * @param value the string to be set.
     */
    void set(const char *value);

    /**
     * Sets the value. Only integer, float, double, bool, vector of
     * JSONValue, and map of string to JSONValue can be set.  T the
     * type of the value.
     *
     * @param value the value to be set.
     */
    template<class T> const T &operator=(const T &value) {
        set(value);
        return value;
    }

    /**
     * Represents the Null value in JSON.
     */
    static JSONNull Null;

    /**
     * Gets the type of the value.
     *
     * @return the type of the value.
     */
    const std::type_info &type() const;

  private:
    /**
     * Represents the value stored in JSONValue.
     */
    boost::any value;
};

/**
 * Template specialization to allow conversion of JSONInteger to
 * JSONUInteger if the value if positive or 0.  If the conversion is
 * not possible without loss of precision, throws a
 * JSONValueException().
 *
 * @return a valid JSONUInteger
 */
template<> 
inline JSONValue::JSONUInteger JSONValue::get<JSONValue::JSONUInteger>() const
{
    try {
        if (type() == typeid(JSONValue::JSONInteger)) {
            if (boost::any_cast<JSONValue::JSONInteger>(value) >= 0) {
                return static_cast<JSONValue::JSONUInteger>(
                    boost::any_cast<JSONValue::JSONInteger>(value));
            }
        }
        return boost::any_cast<JSONValue::JSONUInteger>(value);
    } 
    catch (const std::bad_cast &ex) {
        bool success;
        std::string currentDemangledTypeName = 
            clusterlib::Exception::demangleName(type().name(), success);
        std::string newDemangledTypeName = 
            clusterlib::Exception::demangleName(
                typeid(JSONValue::JSONUInteger).name(), success);
        std::ostringstream oss;
        oss << "get<JSONValue::JSONUInteger>: Current type name (" 
            << currentDemangledTypeName 
            << ") cannot be converted to desired type name (" 
            << newDemangledTypeName << ") with value "  
            << boost::any_cast<JSONValue::JSONUInteger>(value);
        throw JSONValueException(oss.str());
    }
}

/**
 * Template specialization to allow conversion of JSONUInteger to
 * JSONInteger if the value does not exceed the maximum JSONInteger.
 * If the conversion is not possible without loss of precision, throws
 * a JSONValueException().
 *
 * @return a valid JSONInteger
 */
template<> 
inline JSONValue::JSONInteger JSONValue::get<JSONValue::JSONInteger>() const
{
    try {
        if (type() == typeid(JSONValue::JSONUInteger)) {
            if (boost::any_cast<JSONValue::JSONUInteger>(value) <= 
                static_cast<JSONValue::JSONUInteger>(
                    std::numeric_limits<JSONValue::JSONInteger>::max())) {
                return static_cast<JSONValue::JSONInteger>(
                    boost::any_cast<JSONValue::JSONUInteger>(value));
            }
        }
        return boost::any_cast<JSONValue::JSONInteger>(value);
    } 
    catch (const std::bad_cast &ex) {
        bool success;
        std::string currentDemangledTypeName = 
            clusterlib::Exception::demangleName(type().name(), success);
        std::string newDemangledTypeName = 
            clusterlib::Exception::demangleName(
                typeid(JSONValue::JSONInteger).name(), success);
        std::ostringstream oss;
        oss << "get<JSONValue::JSONInteger>: Current type name (" 
            << currentDemangledTypeName 
            << ") cannot be converted to desired type name (" 
            << newDemangledTypeName << ") with value " 
            << boost::any_cast<JSONValue::JSONUInteger>(value);
        throw JSONValueException(oss.str());
    }
}

template<> 
inline void JSONValue::set(const int64_t &value) {
    this->value = (JSONInteger) value;
}

template<> 
inline void JSONValue::set(const int32_t &value) {
    this->value = (JSONInteger) value;
}

template<> 
inline void JSONValue::set(const int16_t &value) {
    this->value = (JSONInteger) value;
}

template<> 
inline void JSONValue::set(const int8_t &value) {
    this->value = (JSONInteger) value;
}

template<> 
inline void JSONValue::set(const uint64_t &value) {
    this->value = (JSONUInteger) value;
}

template<>
inline void JSONValue::set(const uint32_t &value) {
    this->value = (JSONUInteger) value;
}

template<>
inline void JSONValue::set(const uint16_t &value) {
    this->value = (JSONUInteger) value;
}

template<>
inline void JSONValue::set(const uint8_t &value) {
    this->value = (JSONUInteger) value;
}

template<>
inline void JSONValue::set(const long double &value) {
    this->value = (JSONFloat) value;
}

template<>
inline void JSONValue::set(const double &value) {
    this->value = (JSONFloat) value;
}

template<>
inline void JSONValue::set(const float &value) {
    this->value = (JSONFloat) value;
}

template<>
inline void JSONValue::set(const JSONNull &value) {
    this->value = value;
}

template<>
inline void JSONValue::set(const JSONArray &value) {
    this->value = value;
}

template<>
inline void JSONValue::set(const JSONBoolean &value) {
    this->value = value;
}

template<>
inline void JSONValue::set(const JSONString &value) {
    this->value = value;
}

template<> 
inline void JSONValue::set(const JSONObject &value) {
    this->value = value;
}

/**
 * Defines the utility class of JSON parser and composer. This is
 * the single entry of the namespace.
 */
class JSONCodec
{
  public:
    /**
     * Encodes the JSONValue to a JSON message.
     *
     * @param object the value to be encoded.
     * @return the JSON message representing the map.
     * @throws JSONValueException if the value type cannot be supported.
     */
    static std::string encode(const JSONValue &object);

    /**
     * Decodes the JSON message into a map.
     *
     * @param message the JSON message to be decoded.
     * @param pos the start parsing position and the end position of the
     *        JSON message. If it is NULL, the start position is 0.
     * @return the value representing the JSON message.
     * @throws JSONParseException if the JSON message is malformed.
     */
    static JSONValue decode(const std::string &message, size_t *pos = NULL);

  private:
    /**
     * Creates a new instance of JSONUtil. This private
     * constructor prohibits the creation of a new instance.
     */
    JSONCodec();
};

}

#endif
