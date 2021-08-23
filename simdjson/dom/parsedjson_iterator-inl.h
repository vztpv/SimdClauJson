#ifndef SIMDJSON_INLINE_PARSEDJSON_ITERATOR_H
#define SIMDJSON_INLINE_PARSEDJSON_ITERATOR_H

#include "simdjson/dom/parsedjson_iterator.h"
#include "simdjson/portability.h"
#include <cstring>

#ifndef SIMDJSON_DISABLE_DEPRECATED_API

namespace simdjson {

// VS2017 reports deprecated warnings when you define a deprecated class's methods.
SIMDJSON_PUSH_DISABLE_WARNINGS
SIMDJSON_DISABLE_DEPRECATED_WARNING


SIMDJSON_POP_DISABLE_WARNINGS
} // namespace simdjson

#endif // SIMDJSON_DISABLE_DEPRECATED_API


#endif // SIMDJSON_INLINE_PARSEDJSON_ITERATOR_H
