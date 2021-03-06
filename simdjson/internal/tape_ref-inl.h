#ifndef SIMDJSON_INLINE_TAPE_REF_H
#define SIMDJSON_INLINE_TAPE_REF_H

#include "simdjson/internal/tape_ref.h"
#include <cstring>

namespace simdjson {
namespace internal {

//
// tape_ref inline implementation
//
simdjson_really_inline tape_ref::tape_ref() noexcept : doc{ nullptr }, json_index{ 0 } {}
simdjson_really_inline tape_ref::tape_ref(const dom::document* _doc, size_t _json_index) noexcept : doc{ _doc }, json_index{ _json_index } {}


simdjson_really_inline bool tape_ref::is_document_root() const noexcept {
    return json_index == 1; // should we ever change the structure of the tape, this should get updated.
}

// Some value types have a specific on-tape word value. It can be faster
// to check the type by doing a word-to-word comparison instead of extracting the
// most significant 8 bits.

simdjson_really_inline bool tape_ref::is_double() const noexcept {
    return doc->tape[json_index].get_type() == tape_type::DOUBLE;
}
simdjson_really_inline bool tape_ref::is_int64() const noexcept {
    return doc->tape[json_index].get_type() == tape_type::INT64;
}
simdjson_really_inline bool tape_ref::is_uint64() const noexcept {
    return doc->tape[json_index].get_type() == tape_type::UINT64;
}
simdjson_really_inline bool tape_ref::is_false() const noexcept {
    return doc->tape[json_index].get_type() == tape_type::FALSE_VALUE;
}
simdjson_really_inline bool tape_ref::is_true() const noexcept {
    return doc->tape[json_index].get_type() == tape_type::TRUE_VALUE;
}
simdjson_really_inline bool tape_ref::is_null_on_tape() const noexcept {
    return doc->tape[json_index].get_type() == tape_type::NULL_VALUE;
}

inline size_t tape_ref::after_element() const noexcept {
    return json_index + 1;
    /*
    switch (tape_ref_type()) {
    case tape_type::START_ARRAY:
    case tape_type::START_OBJECT:
        return matching_brace_index();
    case tape_type::UINT64:
    case tape_type::INT64:
    case tape_type::DOUBLE:
        return json_index + 2;
    default:
        return json_index + 1;
    }
    */
}
simdjson_really_inline tape_type tape_ref::tape_ref_type() const noexcept {
    return static_cast<tape_type>(doc->tape[json_index].get_type());
}

template<typename T>
simdjson_really_inline T tape_ref::next_tape_value() const noexcept {
    static_assert(sizeof(T) == sizeof(uint64_t), "next_tape_value() template parameter must be 64-bit");
    // Though the following is tempting...
    //  return *reinterpret_cast<const T*>(&doc->tape[json_index + 1]);
    // It is not generally safe. It is safer, and often faster to rely
    // on memcpy. Yes, it is uglier, but it is also encapsulated.
    T x;
    std::memcpy(&x, &doc->tape[json_index + 1], sizeof(uint64_t));
    return x;
}
/*
simdjson_really_inline uint32_t internal::tape_ref::get_string_length() const noexcept {
    size_t string_buf_index = size_t(tape_value());
    uint32_t len;
    std::memcpy(&len, &doc->string_buf[string_buf_index], sizeof(len));
    return len;
}

simdjson_really_inline const char* internal::tape_ref::get_c_str() const noexcept {
    size_t string_buf_index = size_t(tape_value());
    return reinterpret_cast<const char*>(&doc->string_buf[string_buf_index + sizeof(uint32_t)]);
}
*/

inline std::string_view internal::tape_ref::get_string_view() const noexcept {
    return std::string_view(
        get_c_str(),
        get_string_length()
    );
}

} // namespace internal
} // namespace simdjson

#endif // SIMDJSON_INLINE_TAPE_REF_H
