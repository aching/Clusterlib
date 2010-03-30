#include "clusterlibinternal.h"
#include <json.h>
#include <sstream>
#include <cstring>
#include "log.h"

using namespace std;
using namespace boost;

DEFINE_LOGGER(J_LOG, "json");

namespace json {

/*JSONValue::JSONNull----------------------------------------------------------------------------*/

    JSONValue::JSONNull JSONValue::Null;

    JSONValue::JSONNull::JSONNull() {
    }

    JSONValue::JSONNull::JSONNull(const JSONValue::JSONNull &other) {
    }

    bool JSONValue::JSONNull::operator==(const JSONNull &other) const {
        // Always true, since they represent the same Null in JSONValue.
        return true;
    }

    const JSONValue::JSONNull &JSONValue::JSONNull::operator=(const JSONNull &other) {
        // Assign does nothing
        return other;
    }

/*JSONValue--------------------------------------------------------------------------------------*/

    JSONValue::JSONValue() : value(JSONValue::Null) {
    }

    JSONValue::JSONValue(const JSONValue &other) : value(other.value) {
    }

    template<> void JSONValue::set(const int64_t &value) {
        this->value = (JSONInteger) value;
    }

    template<> void JSONValue::set(const int32_t &value) {
        this->value = (JSONInteger) value;
    }

    template<> void JSONValue::set(const int16_t &value) {
        this->value = (JSONInteger) value;
    }

    template<> void JSONValue::set(const int8_t &value) {
        this->value = (JSONInteger) value;
    }

    template<> void JSONValue::set(const uint64_t &value) {
        this->value = (JSONUInteger) value;
    }

    template<> void JSONValue::set(const uint32_t &value) {
        this->value = (JSONUInteger) value;
    }

    template<> void JSONValue::set(const uint16_t &value) {
        this->value = (JSONUInteger) value;
    }

    template<> void JSONValue::set(const uint8_t &value) {
        this->value = (JSONUInteger) value;
    }

    template<> void JSONValue::set(const long double &value) {
        this->value = (JSONFloat) value;
    }

    template<> void JSONValue::set(const double &value) {
        this->value = (JSONFloat) value;
    }

    template<> void JSONValue::set(const float &value) {
        this->value = (JSONFloat) value;
    }

    template<> void JSONValue::set(const JSONNull &value) {
        this->value = value;
    }

    template<> void JSONValue::set(const JSONArray &value) {
        this->value = value;
    }

    template<> void JSONValue::set(const JSONBoolean &value) {
        this->value = value;
    }

    template<> void JSONValue::set(const JSONString &value) {
        this->value = value;
    }

    void JSONValue::set(const char *value) {
        this->value = string(value);
    }

    template<> void JSONValue::set(const JSONObject &value) {
        this->value = value;
    }

    const type_info &JSONValue::type() const {
        return value.type();
    }

/*JSONCodec--------------------------------------------------------------------------------------*/
    class JSONCodecHelper {
    public:
        /**
         * Encodes a JSONValue object into the given output string stream using JSON encoding.
         * @param object the JSONValue to be encoded.
         * @param out the output stream where the result should be written to.
         * @throws JSONValueException if the value type cannot be supported.
         */
        static void encode(const JSONValue &object, ostringstream *out) {
            ostringstream &ss = *out;
            const type_info &type = object.type();
            // For integer and floating point numbers, output to
            // the stringstream directly.
            if (type == typeid(JSONValue::JSONInteger)) {
                ss << object.get<JSONValue::JSONInteger>();
            } else if (type == typeid(JSONValue::JSONUInteger)) {
                ss << object.get<JSONValue::JSONUInteger>();
            } else if (type == typeid(JSONValue::JSONFloat)) {
                ss << object.get<JSONValue::JSONFloat>();
            } else if (type == typeid(JSONValue::JSONBoolean)) {
                JSONValue::JSONBoolean value = object.get<JSONValue::JSONBoolean>();
                ss << (value ? "true" : "false");
            } else if (type == typeid(JSONValue::JSONNull)) {
                ss << "null";
            } else if (type == typeid(JSONValue::JSONString)) {
                // Escape the strings
                encode(object.get<JSONValue::JSONString>(), out);
            } else if (type == typeid(JSONValue::JSONArray)) {
                ss << '[';
                JSONValue::JSONArray array = object.get<JSONValue::JSONArray>();
                // Encode each item in the array
                for (JSONValue::JSONArray::const_iterator iter = array.begin(); iter != array.end(); ++iter) {
                    if (iter != array.begin()) {
                        ss << ',';
                    }
                    encode(*iter, out);
                }
                ss << ']';
            } else if (type == typeid(JSONValue::JSONObject)) {
                ss << '{';
                JSONValue::JSONObject obj = object.get<JSONValue::JSONObject>();
                // Encode each entry in the map
                for (JSONValue::JSONObject::const_iterator iter = obj.begin(); iter != obj.end(); ++iter) {
                    if (iter != obj.begin()) {
                        ss << ',';
                    }
                    // Encode the property name
                    encode(iter->first, out);
                    ss << ':';
                    // Encode the property value
                    encode(iter->second, out);
                }
                ss << '}';
            } else {
                throw JSONValueException(string("Value type ") + object.type().name() + " is unknown!");
            }
        }

        static void decode(JSONValue *out, istringstream *in) {
            istringstream &ss = *in;

            // Skip all blanks and peek at the next character to decide what to do.
            skipBlanks(in);
            int nextCh = ss.get();
            
            switch (nextCh) {
            case '[':
            {
                // Array
                JSONValue::JSONArray array;
                skipBlanks(in);
                while (ss.peek() != ']') {
                    // Decode the next JSON value
                    JSONValue nextValue;
                    decode(&nextValue, in);
                    array.push_back(nextValue);

                    // Skip blanks and test for the item separator
                    skipBlanks(in);
                    switch (ss.peek()) {
                    case ',':
                        // Consume the item separator
                        ss.get();
                        break;
                    case ']':
                        // If it is not the separator, see if it is the end of array
                        break;
                    default:
                        // If both not, there is an error
                        ostringstream pos;
                        pos << (int)ss.tellg();
                        throw JSONParseException("',' or ']' is expected at " + pos.str());
                    }
                }

                // Consume the last ']'
                ss.get();
                out->set(array);
                break;
            }
            case '{':
            {
                // Object
                JSONValue::JSONObject object;
                skipBlanks(in);
                while (ss.peek() != '}') {
                    // Decode the key
                    JSONValue key;
                    decode(&key, in);
                    if (key.type() != typeid(JSONValue::JSONString)) {
                        // The key should be a string
                        ostringstream pos;
                        pos << (int)ss.tellg();
                        throw JSONParseException("A string (property name) is expected at " + pos.str());
                    }

                    // Check for ':'
                    skipBlanks(in);
                    if (ss.get() != ':') {
                        ostringstream pos;
                        pos << (int)ss.tellg();
                        throw JSONParseException("':' is expected at " + pos.str());
                    }

                    // Decode the value
                    JSONValue value;
                    decode(&value, in);
                    
                    object.insert(make_pair(key.get<JSONValue::JSONString>(), value));

                    // Skip blanks and test for the item separator
                    skipBlanks(in);
                    switch (ss.peek()) {
                    case ',':
                        // Consume the item separator
                        ss.get();
                        break;
                    case '}':
                        // If it is not the separator, see if it is the end of array
                        break;
                    default:
                        // If both not, there is an error
                        ostringstream pos;
                        pos << (int)ss.tellg();
                        throw JSONParseException("',' or '}' is expected at " + pos.str());
                    }
                }

                // Consume the last '}'
                ss.get();
                out->set(object);
                break;
            }
            case '"':
            {
                // String
                int ch = 0;
                ostringstream oss;
                while ((ch = ss.get()) != '"') {
                    if (ch == EOF) {
                        // EOF found before '"'
                        throw JSONParseException("Unexpected EOF found when parsing string.");
                    }

                    if (ch != '\\') {
                        // Non-escaped
                        oss << (char)ch;
                    } else {
                        // Escaped
                        ch = ss.get();
                        switch (ch) {
                        case 'b':
                            oss << '\b';
                            break;
                        case 'f':
                            oss << '\f';
                            break;
                        case 'n':
                            oss << '\n';
                            break;
                        case 'r':
                            oss << '\r';
                            break;
                        case 't':
                            oss << '\t';
                            break;
                        case '"':
                        case '\\':
                        case '/':
                            oss << (char)ch;
                            break;
                        case 'u':
                        {
                            // Unicode escape \uXXXX
                            char real = 0;
                            for (int i = 0; i < 4; ++i) {
                                real <<= 4;
                                ch = ss.get();
                                if (ch >= '0' && ch <= '9') {
                                    real |= ch - '0';
                                } else if (ch >= 'a' && ch <= 'f') {
                                    real |= ch - 'a' + 10;
                                } else if (ch >= 'A' && ch <= 'F') {
                                    real |= ch - 'A' + 10;
                                } else {
                                    ostringstream pos;
                                    pos << (int)ss.tellg();
                                    throw JSONParseException("Expect 4 hex digits at " + pos.str());
                                }
                            }
                            oss << real;
                            break;
                        }
                        default:
                        {
                            ostringstream pos;
                            pos << (int)ss.tellg();
                            throw JSONParseException(string("Unexpected escaped character ") + (char) ch + " found at " + pos.str());
                        }
                        }
                    }
                }
                out->set(oss.str());
                break;
            }
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                // Number
                ostringstream oss;
                bool floating = false;
                bool negative = false;
                char ch = nextCh;

                if (ch == '-') {
                    // It is negative
                    negative = true;
                    oss << (char) ch;
                    ch = ss.get();
                    if (ch < '0' || ch > '9') {
                        ostringstream pos;
                        pos << (int)ss.tellg();
                        throw JSONParseException("Digit expected at " + pos.str());
                    }
                }
                
                oss << (char) ch;
                ch = ss.peek();

                // Read the integer part
                while (ch >= '0' && ch <= '9') {
                    oss << (char)ss.get();
                    ch = ss.peek();
                }

                if (ch == '.') {
                    floating = true;
                    oss << (char)ss.get();

                    // Read the fraction part
                    ch = ss.peek();
                    if (ch < '0' || ch > '9') {
                        // Must have at least one digit as the fraction part
                        ostringstream pos;
                        pos << (int)ss.tellg();
                        throw JSONParseException("Digit expected at " + pos.str());
                    }

                    while (ch >= '0' && ch <= '9') {
                        oss << (char)ss.get();
                        ch = ss.peek();
                    }
                }

                if (ch == 'e' || ch == 'E') {
                    floating = true;
                    oss << (char)ss.get();
                    
                    // Read the exponential part
                    ch = ss.peek();
                    if (ch == '+' || ch == '-') {
                        oss << (char)ss.get();
                        ch = ss.peek();
                    }

                    if (ch < '0' || ch > '9') {
                        // Must have at least one digit as the exponential part
                        ostringstream pos;
                        pos << (int)ss.tellg();
                        throw JSONParseException("Digit expected at " + pos.str());
                    }

                    while (ch >= '0' && ch <= '9') {
                        oss << (char)ss.get();
                        ch = ss.peek();
                    }
                }

                // Set the value with proper type (floating point or integer)
                istringstream iss(oss.str());
                if (floating) {
                    JSONValue::JSONFloat value;
                    iss >> value;
                    out->set(value);
                } else {
                    if (!negative) {
                        // Only use JSONUInteger when required, or
                        // else there will be problems with casting to
                        // types with less resolution
                        JSONValue::JSONUInteger value;
                        iss >> value;
                        if (value > 
                            static_cast<JSONValue::JSONUInteger>(
                                numeric_limits<JSONValue::JSONInteger>::
                                max())) {
                            out->set(value);
                        }
                        else {
                            JSONValue::JSONInteger integerValue;
                            integerValue = static_cast<JSONValue::JSONInteger>(
                                value);
                            out->set(integerValue);
                        }
                    }
                    else {
                        JSONValue::JSONInteger value;
                        iss >> value;
                        out->set(value);
                    }
                }
                break;
            }
            case 't':
            {
                // Boolean true
                char buf[4] = {0,0,0,0};
                ss.read(buf, 3);
                if (strcmp(buf, "rue") != 0) {
                    ostringstream pos;
                    pos << (int)ss.tellg();
                    throw JSONParseException("'true' is expected at " + pos.str());
                }
                out->set(true);
                break;
            }
            case 'f':
            {
                // Boolean false
                char buf[5] = {0,0,0,0,0};
                ss.read(buf, 4);
                if (strcmp(buf, "alse") != 0) {
                    ostringstream pos;
                    pos << (int)ss.tellg();
                    throw JSONParseException("'false' is expected at " + pos.str());
                }
                out->set(false);
                break;
            }
            case 'n':
            {
                // Null
                char buf[4] = {0,0,0,0};
                ss.read(buf, 3);
                if (strcmp(buf, "ull") != 0) {
                    ostringstream pos;
                    pos << (int)ss.tellg();
                    throw JSONParseException("'null' is expected at " + pos.str());
                }
                out->set(JSONValue::Null);
                break;
            }
            default:
            {
                ostringstream pos;
                pos << (int)ss.tellg();
                throw JSONParseException("Expect an array, object, string, number, boolean or null at " + pos.str());
            }
            }
            skipBlanks(in);
        }

    private:
        /**
         * Skips the blank spaces, including tabs, newlines, and spaces.
         * @param in the input stream where the blanks are skipped.
         */
        static void skipBlanks(istringstream *in) {
            istringstream &ss = *in;
            int nextCh;
            while ((nextCh = ss.peek())) {
                // Skip all blanks
                if (nextCh != ' ' && nextCh != '\t' && nextCh != '\r' && nextCh != '\n') {
                    return;
                }
                ss.get();
            }
        }

        /**
         * Encodes a string. Only ASCII characters are supported. All control characters, double-quotes and backslashes
         * are escaped properly. The encoded string will be double-quoted.
         * @param str the string to be encoded.
         * @param out the output stream where the encoded string is written to.
         */
        static void encode(const JSONValue::JSONString &str, ostringstream *out) {
            ostringstream &ss = *out;
            ss << '"';
            for (JSONValue::JSONString::const_iterator iter = str.begin(); iter != str.end(); ++iter) {
                switch (*iter) {
                case '\n':
                    ss << "\\n";
                    break;
                case '\t':
                    ss << "\\t";
                    break;
                case '\b':
                    ss << "\\b";
                    break;
                case '\f':
                    ss << "\\f";
                    break;
                case '\r':
                    ss << "\\r";
                    break;
                case '\\':
                case '"':
                case '/':
                    ss << '\\' << *iter;
                    break;
                default:
                    if (((unsigned char)*iter <= 31) || ((unsigned char)*iter >= 127)) {
                        // Other control char in ASCII
                        char buf[7]={0,0,0,0,0,0,0};
                        snprintf(buf, 7, "\\u%04x", (unsigned char)*iter);
                        ss << buf;
                    } else {
                        ss << *iter;
                    }
                }
            }
            ss << '"';
        }
    };

    JSONCodec::JSONCodec() {
    }

    string JSONCodec::encode(const JSONValue &object) {
        TRACE(J_LOG, "encode");

        ostringstream ss;
        // 35 is enough even for 128-bit floating point number
        ss.precision(35);
        JSONCodecHelper::encode(object, &ss);
        return ss.str();
    }

    JSONValue JSONCodec::decode(const string &message, size_t *pos) {
        TRACE(J_LOG, "decode");

        LOG_DEBUG(J_LOG, 
                  "decode: message (%s), pos (%d)", 
                  message.c_str(), 
                  (pos != NULL) ? *pos : 0);
        istringstream ss(message);

        if (pos != NULL) {
            ss.seekg(*pos);
        }

        JSONValue object;
        JSONCodecHelper::decode(&object, &ss);

        if (pos != NULL) {
            *pos = ss.tellg();
        }

        return object;
    }
};
