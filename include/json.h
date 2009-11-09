/*
 * json.h --
 *
 * Definition of json namespace and classes.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _JSON_H_
#define _JSON_H_
#include <string>
#include <deque>
#include <map>
#include <iostream>
#include <boost/any.hpp>

/**
 * Defines the namespace of JSON encoder and decoder. The utility
 * class JSONCodec is the single entry of parsing and composing JSON
 * messages.
 */
namespace json 
{

/**
 * Defines the base exception class of all JSON errors.
 */
class JSONException : public virtual std::exception {
  public:
    /**
     * Creates an instance of JSONException with error message.
     * @param message the error message.
     */
    explicit JSONException(const std::string &message);
    /**
     * Gets the error message of this exception.
     * @return the error message.
     */
    virtual const char *what() const throw();
    /**
     * Destroys the instance of JSONException.
     */
    virtual ~JSONException() throw();
  private:
    /**
     * Represents the error message.
     */
    std::string message;
};

/**
 * Defines the exception of a JSON parsing error.
 */
 class JSONParseException : public virtual JSONException {
   public:
     /**
      * Creates an instance of JSONParseException with error message.
      * @param message the error message.
      */
     explicit JSONParseException(const std::string &message);
 };
 
 /**
  * Defines the exception of a JSON value error. For example, the
  * value is not compatible with its type.
  */
 class JSONValueException : public virtual JSONException {
    public:
        /**
         * Creates an instance of JSONParseException with error message.
         * @param message the error message.
         */
     explicit JSONValueException(const std::string &message);
 };
    
/**
 * Defines the value in JSON. The value can be a number (integer
 * or floating point), a boolean, a string, a vector or a map.
 */
class JSONValue {
  public:
    typedef std::string JSONString;
    typedef std::deque<JSONValue> JSONArray;
    typedef std::map<JSONString, JSONValue> JSONObject;
    typedef int64_t JSONInteger;
    typedef long double JSONFloat;
    typedef bool JSONBoolean;
    
    /**
     * Defines the type for NULL value in JSON.
     */
    class JSONNull {
      public:
        /**
         * Compares if the two instance equal. For Null values in
         * JSONValue, they always equal.  @param other the other
         * Null value to be compared.  @return true
         */
        bool operator==(const JSONNull &other) const;
        /**
         * Assigns the Null value to another. Since Null values in 
         * JSONValue are the same, it does nothing.
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
     * @param other the JSONValue instance to be copied.
     */
    JSONValue(const JSONValue &other);
    /**
     * Creates a new instance of JSONValue with value set.
     * @param T the type of the value.
     * @param value the value.
     */
    template<class T> JSONValue(const T &value) {
        set(value);
    }
    /**
     * Gets the value. A copy of the value is returned.
     * @param T the type of the value.
     * @return the value.
     * @throws JSONValueException 
     *         if the value is not compatible with the return type.
     */
    template<class T> T get() const {
        try {
            return boost::any_cast<T>(value);
        } catch (const std::bad_cast &ex) {
            throw JSONValueException("get: " + std::string(type().name()) + 
                                     " cannot be converted to " + 
                                     typeid(T).name());
        }
    }
    /**
     * Gets the value. A copy of the value is returned.
     * @param T the type of the value.
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
     * JSONValue, and map of string and JSONValue can be set.
     * @param T the type of the value.
     * @param value the value to be set.
     */
    template<class T> void set(const T &value);
    /**
     * Sets the value as string.
     * @param value the string to be set.
     */
    void set(const char *value);
    /**
     * Sets the value. Only integer, float, double, bool, vector of
     * JSONValue, and map of string to JSONValue can be set.
     * @param T the type of the value.
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
 * Defines the utility class of JSON parser and composer. This is
 * the single entry of the namespace.
 */
class JSONCodec {
  public:
    /**
     * Encodes the JSONValue to a JSON message.
     * @param object the value to be encoded.
     * @return the JSON message representing the map.
     * @throws JSONValueException if the value type cannot be supported.
     */
    static std::string encode(const JSONValue &object);
    /**
     * Decodes the JSON message into a map.
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

};

#endif
