#ifndef SIMDJSON_INTERNAL_TAPE_TYPE_H
#define SIMDJSON_INTERNAL_TAPE_TYPE_H

namespace simdjson {
namespace internal {

/**
 * The possible types in the tape.
 */
enum class tape_type {
    ROOT = 'r',
    START_ARRAY = '[',
    START_OBJECT = '{',
    END_ARRAY = ']',
    END_OBJECT = '}',
    STRING = '"',
    INT64 = 'l',
    UINT64 = 'u',
    DOUBLE = 'd',
    TRUE_VALUE = 't',
    FALSE_VALUE = 'f',
    NULL_VALUE = 'n',
    KEY_VALUE = 'k' //
}; // enum class tape_type

} // namespace internal

#include <string_view>

class Token {
public:
    internal::tape_type type;
    bool is_key; // key is string.
    union {
        long long int_val;
        unsigned long long uint_val;
        double float_val;
    };
    std::string_view str_val;
};

} // namespace simdjson

#endif // SIMDJSON_INTERNAL_TAPE_TYPE_H
