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
    KEY_VALUE //
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

public:
    void set_str(const char* str, size_t len) {
        str_len = len;
        str_ptr = str;
    }

    std::string_view get_str() const {
        return std::string_view(str_ptr, str_len);
    }
};

class Token {
public:
    uint8_t type_or_is_key = 0;
    std::unique_ptr<TokenData> data;

public:
    bool is_key() const {
        return type_or_is_key & 1;
    }
    void set_is_key(bool chk) {
        if (chk) {
            type_or_is_key = ((type_or_is_key >> 1) << 1) | 1;
        }
        else {
            type_or_is_key = ((type_or_is_key >> 1) << 1) | 0;
        }
    }
    internal::tape_type get_type() const {
        return (internal::tape_type)(type_or_is_key >> 1);
    }
    void set_type(internal::tape_type type) {
        bool has_key = is_key();

        type_or_is_key = (uint8_t(type)) << 1;
        type_or_is_key += has_key ? 1 : 0;
    }

    void set_str(const char* str, size_t len) {
        if (data) {
            data->set_str(str, len);
        }
    }

    std::string_view get_str() const {
        if (data) {
            return data->get_str();
        }
        return std::string_view("");
    }
};

} // namespace simdjson

#endif // SIMDJSON_INTERNAL_TAPE_TYPE_H
