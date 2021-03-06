#ifndef SIMDJSON_DOM_DOCUMENT_H
#define SIMDJSON_DOM_DOCUMENT_H

#include "simdjson/common_defs.h"
#include <memory>
#include <ostream>

namespace simdjson {
namespace dom {

class element;

class Free {
 public:
	 void operator()(Token* p) const {
		 std::free(p);
	 }
 };



/**
 * A parsed JSON document.
 *
 * This class cannot be copied, only moved, to avoid unintended allocations.
 */

 class document {
 public:
	 size_t len;
	 std::vector<size_t> len2;

     std::vector<int64_t> split; 
	 int no = -1;
	 int64_t count;
	 
	 // for parse2?
	 int64_t start;
	 int64_t length;
	 int state = 0;
	 //
	 long long ut_count = 0;

	 document* ori_doc = nullptr;


	 /**
	  * Create a document container with zero capacity.
	  *
	  * The parser will allocate capacity as needed.
	  */
	 document() noexcept = default;
	 ~document() noexcept = default;

	 /**
	  * Take another document's buffers.
	  *
	  * @param other The document to take. Its capacity is zeroed and it is invalidated.
	  */
	 document(document&& other) noexcept = default;
	 /** @private */
	 document(const document&) = delete; // Disallow copying
	 /**
	  * Take another document's buffers.
	  *
	  * @param other The document to take. Its capacity is zeroed.
	  */
	 document& operator=(document&& other) noexcept = default;
	 /** @private */
	 document& operator=(const document&) = delete; // Disallow copying

	 /**
	  * Get the root element of this document as a JSON array.
	  */
	 element root() const noexcept;

	 /**
	  * @private Dump the raw tape for debugging.
	  *
	  * @param os the stream to output to.
	  * @return false if the tape is likely wrong (e.g., you did not parse a valid JSON).
	  */
	 bool dump_raw_tape(std::ostream& os) const noexcept;



	 /** @private Structural values. */
	 std::unique_ptr < Token[], Free > tape; // Free calls free()

	 /** @private String values.
	  *
	  * Should be at least byte_capacity.
	  */
	 std::unique_ptr<uint8_t[]> string_buf{};

	 /** @private Allocate memory to support
	  * input JSON documents of up to len bytes.
	  *
	  * When calling this function, you lose
	  * all the
	  *
	  * The memory allocation is strict: you
	  * can you use this function to increase
	  * or lower the amount of allocated memory.
	  * Passsing zero clears the memory.
	  */
	 error_code allocate(size_t len, bool option, size_t reserve_capacity) noexcept;
	 /** @private Capacity in bytes, in terms
	  * of how many bytes of input JSON we can
	  * support.
	  */
	 size_t capacity() const noexcept;


 private:
	 size_t allocated_capacity{ 0 };
	 friend class parser;
 }; // class document
} // namespace dom
} // namespace simdjson




#endif // SIMDJSON_DOM_DOCUMENT_H
