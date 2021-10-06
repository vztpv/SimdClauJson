#include "generic/stage2/json_iterator.h"
#include "generic/stage2/tape_writer.h"

namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace {
namespace stage2 {

    struct tape_builder {
        template<bool STREAMING>
        simdjson_warn_unused static simdjson_really_inline error_code parse_document(
            dom_parser_implementation& dom_parser,
            dom::document& doc) noexcept;

        /** Called when a non-empty document starts. */
        simdjson_warn_unused simdjson_really_inline error_code visit_document_start(json_iterator& iter) noexcept;
        /** Called when a non-empty document ends without error. */
        simdjson_warn_unused simdjson_really_inline error_code visit_document_end(json_iterator& iter) noexcept;

        /** Called when a non-empty array starts. */
        simdjson_warn_unused simdjson_really_inline error_code visit_array_start(json_iterator& iter, const uint8_t* value) noexcept;
        /** Called when a non-empty array ends. */
        simdjson_warn_unused simdjson_really_inline error_code visit_array_end(json_iterator& iter, const uint8_t* value) noexcept;
        /** Called when an empty array is found. */
        simdjson_warn_unused simdjson_really_inline error_code visit_empty_array(json_iterator& iter) noexcept;

        /** Called when a non-empty object starts. */
        simdjson_warn_unused simdjson_really_inline error_code visit_object_start(json_iterator& iter, const uint8_t* value) noexcept;
        /**
         * Called when a key in a field is encountered.
         *
         * primitive, visit_object_start, visit_empty_object, visit_array_start, or visit_empty_array
         * will be called after this with the field value.
         */
        simdjson_warn_unused simdjson_really_inline error_code visit_key(json_iterator& iter, const uint8_t* key) noexcept;
        /** Called when a non-empty object ends. */
        simdjson_warn_unused simdjson_really_inline error_code visit_object_end(json_iterator& iter, const uint8_t* value) noexcept;
        /** Called when an empty object is found. */
        simdjson_warn_unused simdjson_really_inline error_code visit_empty_object(json_iterator& iter) noexcept;

        /**
         * Called when a string, number, boolean or null is found.
         */
        simdjson_warn_unused simdjson_really_inline error_code visit_primitive(json_iterator& iter, const uint8_t* value) noexcept;
        /**
         * Called when a string, number, boolean or null is found at the top level of a document (i.e.
         * when there is no array or object and the entire document is a single string, number, boolean or
         * null.
         *
         * This is separate from primitive() because simdjson's normal primitive parsing routines assume
         * there is at least one more token after the value, which is only true in an array or object.
         */
        simdjson_warn_unused simdjson_really_inline error_code visit_root_primitive(json_iterator& iter, const uint8_t* value) noexcept;

        simdjson_warn_unused simdjson_really_inline error_code visit_string(json_iterator& iter, const uint8_t* value, bool key = false) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code visit_number(json_iterator& iter, const uint8_t* value) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code visit_true_atom(json_iterator& iter, const uint8_t* value) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code visit_false_atom(json_iterator& iter, const uint8_t* value) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code visit_null_atom(json_iterator& iter, const uint8_t* value) noexcept;
        
        simdjson_warn_unused simdjson_really_inline error_code visit_colon(json_iterator& iter) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code visit_comma(json_iterator& iter) noexcept;

        simdjson_warn_unused simdjson_really_inline error_code visit_root_string(json_iterator& iter, const uint8_t* value) noexcept; 
        
        simdjson_warn_unused simdjson_really_inline error_code visit_root_number(json_iterator& iter, const uint8_t* value) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code visit_root_true_atom(json_iterator& iter, const uint8_t* value) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code visit_root_false_atom(json_iterator& iter, const uint8_t* value) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code visit_root_null_atom(json_iterator& iter, const uint8_t* value) noexcept;

        /** Called each time a new field or element in an array or object is found. */
        simdjson_warn_unused simdjson_really_inline error_code increment_count(json_iterator& iter) noexcept;

        /** Next location to write to tape */
        tape_writer tape;
        size_t length;
        std::vector<size_t> len2; //

    private:
        /** Next write location in the string buf for stage 2 parsing */
        uint8_t* current_string_buf_loc;

        simdjson_really_inline tape_builder(dom::document& doc) noexcept;

        simdjson_really_inline uint32_t next_tape_index(json_iterator& iter) const noexcept;
        simdjson_really_inline void start_container(json_iterator& iter, internal::tape_type start, const uint8_t* value) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code end_container(json_iterator& iter, internal::tape_type start, internal::tape_type end, const uint8_t* ptr) noexcept;
        simdjson_warn_unused simdjson_really_inline error_code empty_container(json_iterator& iter, internal::tape_type start, internal::tape_type end) noexcept;
        simdjson_really_inline uint8_t* on_start_string(json_iterator& iter) noexcept;
        simdjson_really_inline void on_end_string(uint8_t* dst) noexcept;
        simdjson_really_inline size_t len() const noexcept;

    }; // class tape_builder

    template<bool STREAMING>
    simdjson_warn_unused simdjson_really_inline error_code tape_builder::parse_document(
        dom_parser_implementation& dom_parser,
        dom::document& doc) noexcept {

        if (doc.no >= 0) {
            dom_parser.docs[doc.no] = &doc;
        }
        else {
            dom_parser.doc = &doc;
        }

        json_iterator iter(dom_parser, STREAMING ? dom_parser.next_structural_index : doc.start);
        iter.first = dom_parser.first;

        if (doc.ori_doc) {
            iter.no = doc.no;
            //doc.string_buf.reset(reinterpret_cast<uint8_t*>(internal::allocate_padded_buffer(dom_parser.len)));
    
            //std::memcpy(static_cast<void*>(doc.string_buf.get()), dom_parser.buf, dom_parser.len);
        }

        doc.string_buf.reset(reinterpret_cast<uint8_t*>(internal::allocate_padded_buffer(dom_parser.len)));

        
        tape_builder builder(doc);
        
        builder.tape.option = dom_parser.option; //

        
        if (!dom_parser.option) {
            builder.tape.split = std::vector<int64_t>(dom_parser.thr_num, 0); // remove?
            builder.len2 = std::vector<size_t>(dom_parser.thr_num, 0);
        }
        if (doc.state == 0) {
            auto x = iter.walk_document<STREAMING>(builder);
            doc.len = dom_parser.n_structural_indexes;
            doc.len2 = builder.len2;
            
            doc.split = std::vector<int64_t>(dom_parser.thr_num, 0);
            size_t i = 0;
            for (auto& x : doc.split) {
                auto num = dom_parser.n_structural_indexes;
                x = num / dom_parser.thr_num * i;
                ++i;
            }

            for (int i = 0; i < doc.split.size(); ++i) {
                if (i > 0 && doc.split[i - 1] > doc.split[i]) {
                    doc.split[i] = doc.split[i - 1];
                }

                if (i > 0) {
                    json_iterator iter(dom_parser, doc.split[i] - 1);

                    char now = *iter.peek();

                    if (now == ':') {
                        doc.split[i] += 2;
                    }

                    if (i < doc.split.size() - 1) {
                        iter.advance();
                        char next = *iter.peek();
                        if (next == ':') {
                            doc.split[i] += 3;
                        }
                    }
                }
            }


            doc.count = builder.tape.count;
            return x;
        }
        else if (doc.state == -1) {
            auto x = iter.walk_document<STREAMING>(builder);
           
            return x;
        }
        else {
            auto x = iter.walk_document2<STREAMING>(builder);
            doc.count = builder.tape.count;
            doc.len = doc.count;
            doc.ut_count = iter.ut_count;
            return x;
        }
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_root_primitive(json_iterator& iter, const uint8_t* value) noexcept {
        return iter.visit_root_primitive(*this, value);
    }
    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_primitive(json_iterator& iter, const uint8_t* value) noexcept {
        return iter.visit_primitive(*this, value);
    }
    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_empty_object(json_iterator& iter) noexcept {
        return empty_container(iter, internal::tape_type::START_OBJECT, internal::tape_type::END_OBJECT);
    }
    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_empty_array(json_iterator& iter) noexcept {
        return empty_container(iter, internal::tape_type::START_ARRAY, internal::tape_type::END_ARRAY);
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_document_start(json_iterator& iter) noexcept {
        // start_container(iter);
        return SUCCESS;
    }
    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_object_start(json_iterator& iter, const uint8_t* value) noexcept {
        start_container(iter, internal::tape_type::START_OBJECT, value);
        return SUCCESS;
    }
    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_array_start(json_iterator& iter, const uint8_t* value) noexcept {
        start_container(iter, internal::tape_type::START_ARRAY, value);
        return SUCCESS;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_object_end(json_iterator& iter, const uint8_t* value) noexcept {
        return end_container(iter, internal::tape_type::START_OBJECT, internal::tape_type::END_OBJECT, value);
    }
    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_array_end(json_iterator& iter, const uint8_t* value) noexcept {
        return end_container(iter, internal::tape_type::START_ARRAY, internal::tape_type::END_ARRAY, value);
    }
    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_document_end(json_iterator& iter) noexcept {
        return SUCCESS;
    }
    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_key(json_iterator& iter, const uint8_t* key) noexcept {
        return visit_string(iter, key, true);
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_colon(json_iterator& iter) noexcept {
       
        tape.append(simdjson::internal::tape_type::COLON);

        return SUCCESS;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_comma(json_iterator& iter) noexcept {

        tape.append(simdjson::internal::tape_type::COMMA);

        return SUCCESS;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::increment_count(json_iterator& iter) noexcept {
    //    iter.dom_parser.open_containers[iter.depth].count++; // we have a key value pair in the object at parser.dom_parser.depth - 1

        return SUCCESS;
    }

    simdjson_really_inline tape_builder::tape_builder(dom::document& doc) noexcept : tape{ doc.tape.get(), doc.tape.get() } //, current_string_buf_loc{ doc.string_buf.get() } {}
    {
       current_string_buf_loc = doc.string_buf.get();
       

        length = doc.length;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_string(json_iterator& iter, const uint8_t* value, bool key) noexcept {
         if (!tape.option) {


            tape.count++;


            return SUCCESS;
        }
           
         iter.log_value(key ? "key" : "string");
        uint8_t* dst = on_start_string(iter);
        uint8_t* start = dst;

        dst = stringparsing::parse_string(value + 1, dst);
        if (dst == nullptr) {
            iter.log_error("Invalid escape in string");
            return STRING_ERROR;
        }
        on_end_string(dst);


        tape.append_str((char*)start, (size_t)(dst - start), key);
        return SUCCESS;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_root_string(json_iterator& iter, const uint8_t* value) noexcept {
        return visit_string(iter, value);
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_number(json_iterator& iter, const uint8_t* value) noexcept {
        if (!tape.option) {
            
          
            tape.count++;

           

            return SUCCESS;
        }

        iter.log_value("number");
        return numberparsing::parse_number(value, tape);
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_root_number(json_iterator& iter, const uint8_t* value) noexcept {
        if (!tape.option) {
            

            

            tape.count++;

            

            return SUCCESS;
        }

        //
        // We need to make a copy to make sure that the string is space terminated.
        // This is not about padding the input, which should already padded up
        // to len + SIMDJSON_PADDING. However, we have no control at this stage
        // on how the padding was done. What if the input string was padded with nulls?
        // It is quite common for an input string to have an extra null character (C string).
        // We do not want to allow 9\0 (where \0 is the null character) inside a JSON
        // document, but the string "9\0" by itself is fine. So we make a copy and
        // pad the input with spaces when we know that there is just one input element.
        // This copy is relatively expensive, but it will almost never be called in
        // practice unless you are in the strange scenario where you have many JSON
        // documents made of single atoms.
        //
        std::unique_ptr<uint8_t[]>copy(new (std::nothrow) uint8_t[iter.remaining_len() + SIMDJSON_PADDING]);
        if (copy.get() == nullptr) { return MEMALLOC; }
        std::memcpy(copy.get(), value, iter.remaining_len());
        std::memset(copy.get() + iter.remaining_len(), ' ', SIMDJSON_PADDING);
        error_code error = visit_number(iter, copy.get());
        return error;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_true_atom(json_iterator& iter, const uint8_t* value) noexcept {
        if (!tape.option) {
           

          

            tape.count++;

            

            return SUCCESS;
        }

        iter.log_value("true");
        if (!atomparsing::is_valid_true_atom(value)) { return T_ATOM_ERROR; }
        tape.append(internal::tape_type::TRUE_VALUE);
        return SUCCESS;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_root_true_atom(json_iterator& iter, const uint8_t* value) noexcept {
        if (!tape.option) {
           
           
            tape.count++;

            

            return SUCCESS;
        }

        iter.log_value("true");
        if (!atomparsing::is_valid_true_atom(value, iter.remaining_len())) { return T_ATOM_ERROR; }
        tape.append(internal::tape_type::TRUE_VALUE);
        return SUCCESS;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_false_atom(json_iterator& iter, const uint8_t* value) noexcept {
        if (!tape.option) {
          

           
            tape.count++;

            

            return SUCCESS;
        }

        iter.log_value("false");

        if (!atomparsing::is_valid_false_atom(value)) { return F_ATOM_ERROR; }
        tape.append(internal::tape_type::FALSE_VALUE);
        return SUCCESS;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_root_false_atom(json_iterator& iter, const uint8_t* value) noexcept {
        if (!tape.option) {
            
           

            tape.count++;


            return SUCCESS;
        }

        iter.log_value("false");
        if (!atomparsing::is_valid_false_atom(value, iter.remaining_len())) { return F_ATOM_ERROR; }
        tape.append(internal::tape_type::FALSE_VALUE);
        return SUCCESS;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_null_atom(json_iterator& iter, const uint8_t* value) noexcept {
        if (!tape.option) {
           
          
            tape.count++;

          

            return SUCCESS;
        }

        iter.log_value("null");
        if (!atomparsing::is_valid_null_atom(value)) { return N_ATOM_ERROR; }
        tape.append(internal::tape_type::NULL_VALUE);
        return SUCCESS;
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::visit_root_null_atom(json_iterator& iter, const uint8_t* value) noexcept {
        if (!tape.option) {
           

            tape.count++;

          
            
            return SUCCESS;
        }

        iter.log_value("null");
        if (!atomparsing::is_valid_null_atom(value, iter.remaining_len())) { return N_ATOM_ERROR; }
        tape.append(internal::tape_type::NULL_VALUE);
        return SUCCESS;
    }

    // private:

    simdjson_really_inline uint32_t tape_builder::next_tape_index(json_iterator& iter) const noexcept {
        return uint32_t(tape.next_tape_loc - iter.dom_parser.docs[iter.no]->tape.get());
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::empty_container(json_iterator& iter, internal::tape_type start, internal::tape_type end) noexcept {
        if (!tape.option) {
            
            

            tape.count++;

            

            tape.count++;

            
            return SUCCESS;
        }

        auto start_index = next_tape_index(iter);
        tape.append(start);
        tape.append(end);
        return SUCCESS;
    }

    simdjson_really_inline void tape_builder::start_container(json_iterator& iter, internal::tape_type start, const uint8_t* value) noexcept {
       // iter.dom_parser.//open_containers[iter.depth].tape_index = next_tape_index(iter);
  //      iter.dom_parser.open_containers[iter.depth].count = 0;
        if (!tape.option) {

          

            tape.count++;

            
           
            return;
        }

        tape.append(start);
        //tape.skip(); // We don't actually *write* the start element until the end.
    }

    simdjson_warn_unused simdjson_really_inline error_code tape_builder::end_container(json_iterator& iter, internal::tape_type start, internal::tape_type end,
        const  uint8_t* value) noexcept {
        // Write the ending tape element, pointing at the start location
      //  const uint32_t start_tape_index = iter.dom_parser.open_containers[iter.depth].tape_index;
        if (!tape.option) {
           

            tape.count++;

           
          
            return SUCCESS;
        }
        
        tape.append(end);
        return SUCCESS;
    }

    simdjson_really_inline uint8_t* tape_builder::on_start_string(json_iterator& iter) noexcept {
        return current_string_buf_loc + sizeof(uint32_t);
    }



    simdjson_really_inline void tape_builder::on_end_string(uint8_t* dst) noexcept {
        uint32_t str_length = uint32_t(dst - (current_string_buf_loc + sizeof(uint32_t)));
        // TODO check for overflow in case someone has a crazy string (>=4GB?)
        // But only add the overflow check when the document itself exceeds 4GB
        // Currently unneeded because we refuse to parse docs larger or equal to 4GB.

        memcpy(current_string_buf_loc, &str_length, sizeof(uint32_t));

        // NULL termination is still handy if you expect all your strings to
        // be NULL terminated? It comes at a small cost
        *dst = 0;
        current_string_buf_loc = dst + 1;
    }
    simdjson_really_inline size_t tape_builder::len() const noexcept {
        return tape.count;
    }


} // namespace stage2
} // unnamed namespace
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson


#include "../../simdclaujson.h"

