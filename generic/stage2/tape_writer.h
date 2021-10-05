



namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace {
namespace stage2 {


struct tape_writer {
	/** The next place to write to tape */
	Token* next_tape_loc;
	
	Token* start;

	size_t count = 0;

	size_t count2 = 0;


	std::vector<Token*> _stack;

	bool option = false;

	std::vector<int64_t> split;

	/** Write a signed 64-bit value to tape. */
	simdjson_really_inline void append_s64(int64_t value) noexcept;

	/** Write an unsigned 64-bit value to tape. */
	simdjson_really_inline void append_u64(uint64_t value) noexcept;

	/** Write a double value to tape. */
	simdjson_really_inline void append_double(double value) noexcept;

	/**
		* Append null, true, false, {, }, [, ]    ?
		*/
	simdjson_really_inline void append(internal::tape_type t) noexcept;



	simdjson_really_inline void append_str(char* str, size_t len, bool key) noexcept;


	/**
		* Skip the current tape entry without writing.
		*
		* Used to skip the start of the container, since we'll come back later to fill it in when the
		* container ends.
		*/
	simdjson_really_inline void skip() noexcept;

	/**
		* Skip the number of tape entries necessary to write a large u64 or i64.
		*/
	simdjson_really_inline void skip_large_integer() noexcept;

	/**
		* Skip the number of tape entries necessary to write a double.
		*/
	simdjson_really_inline void skip_double() noexcept;

	/**
		* Write a value to a known location on tape.
		*
		* Used to go back and write out the start of a container after the container ends.
		*/
		//simdjson_really_inline static void write(uint64_t& tape_loc, uint64_t val, internal::tape_type t) noexcept;

private:
}; // struct number_writer

simdjson_really_inline void tape_writer::append_s64(int64_t value) noexcept {
	if (option) {

		next_tape_loc->set_type(internal::tape_type::INT64);

		next_tape_loc->data = TokenData();
		next_tape_loc->data.int_val = value;

		next_tape_loc++;
	
	}
	else {
		
	}

	
	count++;
	count2++;
}

simdjson_really_inline void tape_writer::append_u64(uint64_t value) noexcept {
	if (option) {
		next_tape_loc->set_type(internal::tape_type::UINT64);

		next_tape_loc->data = TokenData();
		next_tape_loc->data.uint_val = value;

		next_tape_loc++;
	}

	else {

	}

	count++;
	count2++;
}

/** Write a double value to tape. */
simdjson_really_inline void tape_writer::append_double(double value) noexcept {
	if (option) {
		next_tape_loc->set_type(internal::tape_type::DOUBLE);

		next_tape_loc->data = TokenData();
		next_tape_loc->data.float_val = value;
	
		next_tape_loc++;
	}

	else {
		
	}

	count++;
	count2++;
}

simdjson_really_inline void tape_writer::skip() noexcept {
	if (option) {
		next_tape_loc++;
	}

	else {
		
	}

	count++;
	count2++;
}

simdjson_really_inline void tape_writer::skip_large_integer() noexcept {
	if (option) {
		next_tape_loc++;
	}

	else {
		
	}

	count++;
	count2++;
}

simdjson_really_inline void tape_writer::skip_double() noexcept {
	if (option) {
		next_tape_loc++;
	}

	else {
		
	}

	count++;
	count2++;

}

simdjson_really_inline void tape_writer::append(internal::tape_type t) noexcept {
	if (option) {
		next_tape_loc->set_type(t);

		next_tape_loc++;
	}

	else {
		
	}

	count++;
	count2++;
}

simdjson_really_inline void tape_writer::append_str(char* str, size_t len, bool key) noexcept {
	if (option) {

		next_tape_loc->data = TokenData();

		next_tape_loc->set_str(str, len);

		next_tape_loc->set_type(simdjson::internal::tape_type::STRING);

		if (key) {
			next_tape_loc->set_type(simdjson::internal::tape_type::KEY_VALUE);
			count2--;
		}

		next_tape_loc++;
	}

	else {

	}

	count++;
	count2++;
}


} // namespace stage2
} // unnamed namespace
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson
