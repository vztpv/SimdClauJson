#ifndef SIMDJSON_INTERNAL_TAPE_TYPE_H
#define SIMDJSON_INTERNAL_TAPE_TYPE_H

namespace simdjson {
namespace internal {

/**
 * The possible types in the tape.
 */
enum class tape_type {
    NONE = 0,
    START_ARRAY = 1,
    START_OBJECT,
    END_ARRAY,
    END_OBJECT,
    STRING,
    INT64,
    UINT64,
    DOUBLE,
    TRUE_VALUE,
    FALSE_VALUE,
    NULL_VALUE,
    KEY_VALUE,
    COMMA,
    COLON
    //
}; // enum class tape_type

} // namespace internal

#include <string_view>

class TokenData {
public:
    union {
        long long int_val;
        unsigned long long uint_val;
        double float_val;
        size_t str_len;
    };
    const char* str_ptr = nullptr;
  //  std::string str_val;
    long long count_ut = 0, count_it = 0;

public:
    void set_str(const char* str, size_t len) {
        str_len = len;
        str_ptr = str;
      //  str_val = std::string(str, len);
    }

    std::string_view get_str() const {
       // return str_val;

        return std::string_view(str_ptr, str_len);
    }
};

class Token {
public:
    uint8_t type_or_is_key = 0;
    TokenData data;

public:
    bool is_key() const {
        return type_or_is_key == (uint8_t)internal::tape_type::KEY_VALUE;
    }

    internal::tape_type get_type() const {
        return (internal::tape_type)(type_or_is_key);
    }
    void set_type(internal::tape_type type) {
        type_or_is_key = (uint8_t)type;
    }

    void set_str(const char* str, size_t len) {
        data.set_str(str, len);
    }

    std::string_view get_str() const {
        return data.get_str();
    }
};

} // namespace simdjson

#endif // SIMDJSON_INTERNAL_TAPE_TYPE_H
