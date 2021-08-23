# SimdClauJson
experimental json parser : modified simdjson/simdjson 0.9.7 + clauparser`s principle ( + mimalloc) 

# modified simdjson -> TokenArray -> Parser -> Editable Parsing Tree.

# Using Other Repository...
https://github.com/simdjson/simdjson

https://github.com/microsoft/mimalloc

# Modified simdjson
[modified] simdjson (used as tokenizer, to get array of token - all tokens)  ( Tape : char[] -> Token[] )

token : {, }, [, ], string(no key), key, int_value, uint_value, float_value, true_value, false_value, null_value.

# Why modified...? 
I want to use this to make a "Json Explorer".

Need to add, remove, update data easily.

using std::map, std::string, std::vector.
