# SimdClauJson
experimental json parser : modified simdjson/simdjson 0.9.7 + clauparser`s principle(Parallel Parsing) ( + mimalloc) 
(64bit, C++17~)
# modified simdjson -> TokenArray -> Parser -> Editable Parsing Tree.

# Using Other Repository...
https://github.com/simdjson/simdjson  Apache License 2.0.

https://github.com/microsoft/mimalloc   MIT License

# Modified simdjson
[modified] simdjson (used as tokenizer, to get array of token - all tokens)  ( Tape : uint64_t[] -> Token[] )

token : {, }, [, ], string(no key), key, int_value, uint_value, float_value, true_value, false_value, null_value.

# Why modified...? 
I want to use this to make a "Json Explorer".

Need to add, remove, update data easily.

using std::map, std::string, std::vector.

# Now Has Problem..
need many? memory...
bug with "" empty key.
