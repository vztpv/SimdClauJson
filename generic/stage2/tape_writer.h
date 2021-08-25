namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace {
namespace stage2 {

struct tape_writer {
	/** The next place to write to tape */
	Token* next_tape_loc;

	size_t count = 0;


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



	simdjson_really_inline void append_str(std::string_view&& str, bool key) noexcept;


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
	next_tape_loc->type = internal::tape_type::INT64;

	next_tape_loc->int_val = value;

	next_tape_loc++;

	count++;
}

simdjson_really_inline void tape_writer::append_u64(uint64_t value) noexcept {
	next_tape_loc->type = internal::tape_type::UINT64;

	next_tape_loc->uint_val = value;

	next_tape_loc++;

	count++;
}

/** Write a double value to tape. */
simdjson_really_inline void tape_writer::append_double(double value) noexcept {
	next_tape_loc->type = internal::tape_type::DOUBLE;

	next_tape_loc->float_val = value;

	next_tape_loc++;

	count++;
}

simdjson_really_inline void tape_writer::skip() noexcept {
	next_tape_loc++;

	count++;
}

simdjson_really_inline void tape_writer::skip_large_integer() noexcept {
	next_tape_loc++;

	count++;
}

simdjson_really_inline void tape_writer::skip_double() noexcept {
	next_tape_loc++;

	count++;
}

simdjson_really_inline void tape_writer::append(internal::tape_type t) noexcept {
	next_tape_loc->type = t;

	next_tape_loc++;

	count++;
}

simdjson_really_inline void tape_writer::append_str(std::string_view&& str, bool key) noexcept {

	next_tape_loc->str_val = str;

	next_tape_loc->type = simdjson::internal::tape_type::STRING;


	if (key) {
		next_tape_loc->type = simdjson::internal::tape_type::KEY_VALUE;
	}

	next_tape_loc++;

	count++;
}


} // namespace stage2
} // unnamed namespace
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson
