#pragma once

#include <iostream>
#include "simdjson.h" // modified simdjson

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <set>
#include <memory_resource>

namespace clau {
	using STRING = std::string;

	class Data {
	public:
		simdjson::internal::tape_type type;

		bool is_key() const {
			return type == simdjson::internal::tape_type::KEY_VALUE;
		}

		union {
			long long int_val;
			unsigned long long uint_val;
			double float_val;
		};
		STRING str_val;
		size_t count_ut = 0, count_it = 0;

		Data(const Data& other)
			: type(other.type), int_val(other.int_val), str_val(other.str_val), count_ut(other.count_ut), count_it(other.count_it) {

		}

		Data(Data&& other)
			: type(other.type), int_val(other.int_val), str_val(std::move(other.str_val)), count_ut(other.count_ut), count_it(other.count_it) {

		}

		Data() : type(simdjson::internal::tape_type::NONE), int_val(0) { }

		Data(simdjson::internal::tape_type _type, long long _number_val, STRING&& _str_val, size_t count_ut, size_t count_it)
			:type(_type), int_val(_number_val), str_val(std::move(_str_val)), count_ut(count_ut), count_it(count_it)
		{

		}

		bool operator==(const Data& other) const {
			if (this->type == other.type) {
				switch (this->type) {
				case simdjson::internal::tape_type::KEY_VALUE:
				case simdjson::internal::tape_type::STRING:
					return this->str_val == other.str_val;
					break;
				}
			}
			return false;
		}

		bool operator<(const Data& other) const {
			if (this->type == other.type) {
				switch (this->type) {
				case simdjson::internal::tape_type::KEY_VALUE:
				case simdjson::internal::tape_type::STRING:
					return this->str_val < other.str_val;
					break;
				}
			}
			return false;
		}

		Data& operator=(const Data& other) {
			if (this == &other) {
				return *this;
			}

			this->type = other.type;
			this->int_val = other.int_val;
			this->str_val = other.str_val;
			this->count_it = other.count_it;
			this->count_ut = other.count_ut;

			return *this;
		}


		Data& operator=(Data&& other) {
			if (this == &other) {
				return *this;
			}

			this->type = other.type;
			this->int_val = other.int_val;
			this->str_val = std::move(other.str_val);
			this->count_it = other.count_it;
			this->count_ut = other.count_ut;

			return *this;
		}
	};

	inline Data Convert(const simdjson::Token& token) {
		if (token.get_type() == simdjson::internal::tape_type::KEY_VALUE
			|| token.get_type() == simdjson::internal::tape_type::STRING) {
			return Data(token.get_type(), token.data.int_val, STRING(token.get_str()), token.data.count_ut, token.data.count_it);
		}
		return Data(token.get_type(), token.data.int_val, "", token.data.count_ut, token.data.count_it);
	}

	class UserType {
	public:
		enum { Item, ArrayOrObject };
	private:
		UserType* make_user_type(const clau::Data& name, int type) const {
			return new UserType(name, type);
		}

		UserType* make_user_type(clau::Data&& name, int type) const {
			return new UserType(std::move(name), type);
		}

	public:
		void set_name(const STRING& name)  {
			this->name.str_val = name;
		}

		bool is_user_type()const  { return true; }
		bool is_item_type()const  { return false; }
		UserType* clone() const  {
			UserType* temp = new UserType(this->name);

			temp->type = this->type;

			temp->parent = nullptr; // chk!

			temp->data.reserve(this->data.size());
			temp->data2.reserve(this->data2.size());

			for (auto x : this->data) {
				temp->data.push_back(x->clone());
			}
			for (auto x : this->data2) {
				temp->data2.push_back(x);
			}

			temp->order.reserve(this->order.size());

			for (auto x : this->order) {
				temp->order.push_back(x);
			}

			return temp;
		}

	private:
		clau::Data name; // equal to key
		std::vector<UserType*> data; // ut data?
		std::vector<clau::Data> data2; // it data
		int type = -1; // 0 - object, 1 - array, 2 - virtual object, 3 - virtual array, -1 - root  -2 - only in parse...
		UserType* parent = nullptr;

		std::vector<int> order; // ut data and it data order....
	public:
		inline const static size_t npos = -1; // ?
		// chk type?
		bool operator<(const UserType& other) const {
			return name.str_val < other.name.str_val;
		}
		bool operator==(const UserType& other) const {
			return name.str_val == other.name.str_val;
		}

	public:
		UserType(const UserType& other)
			: name(other.name),
			type(other.type), parent(other.parent)
		{
			this->data.reserve(other.data.size());
			for (auto& x : other.data) {
				this->data.push_back(x->clone());
			}

			this->data2.reserve(other.data2.size());
			for (auto& x : other.data2) {
				this->data2.push_back(x);
			}

			this->order.reserve(other.order.size());
			for (auto& x : other.order) {
				this->order.push_back(x);
			}
		}
		/*
		UserType(UserType&& other) {
			name = std::move(other.name);
			this->data = std::move(other.data);
			this->data2 = std::move(other.data2);
			type = std::move(other.type);
			parent = std::move(other.parent);
			order = std::move(other.order);
		}

		UserType& operator=(UserType&& other) noexcept {
			if (this == &other) {
				return *this;
			}

			name = std::move(other.name);
			data = std::move(other.data);
			data2 = std::move(other.data2);
			type = std::move(other.type);
			parent = std::move(other.parent);
			order = std::move(other.order);

			return *this;
		}
		*/
		Data get_name() const { return name; }


	private:
		void LinkUserType(UserType* ut) // friend?
		{
			data.push_back(ut);

			ut->parent = this;
		}
	private:
		UserType(clau::Data&& name, int type = -1) : name(std::move(name)), type(type)
		{

		}

		UserType(const clau::Data& name, int type = -1) : name(name), type(type)
		{
			//
		}
	public:
		UserType() : type(-1) {
			//
		}
		virtual ~UserType() {
			remove_all();
		}
	public:
		bool has_key() const {
			return name.is_key();
		}

		bool is_object() const {
			return type == 0 || type == 2;
		}

		bool is_array() const {
			return type == 1 || type == 3;
		}

		bool is_in_root() const {
			return get_parent()->type == -1;
		}

		bool is_root() const {
			return type == -1;
		}

		static UserType* make_object(const clau::Data& name) {
			UserType* ut = new UserType(name, 0);

			return ut;
		}

		static UserType* make_array(const clau::Data& name) {
			UserType* ut = new UserType(name, 1);

			return ut;
		}

		// name key check?
		void add_object_element(const clau::Data& name, const clau::Data& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			if (!name.is_key()) {
				throw "Error name is not key in add_object_element";
			}

			if (this->type == 1) {
				throw "Error add object element to array in add_object_element ";
			}
			if (this->type == -1 && this->data.size() >= 1) {
				throw "Error not valid json in add_object_element";
			}

			this->data2.push_back(name);
			this->data2.push_back(data);
			
		}

		void add_array_element(const clau::Data& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			if (this->type == 0) {
				throw "Error add object element to array in add_array_element ";
			}
			if (this->type == -1 && this->data.size() >= 1) {
				throw "Error not valid json in add_array_element";
			}

			this->data2.push_back(data); // (Type*)make_item_type(std::move(temp), data));

			this->order.push_back(Item);
		}

		void remove_all() {
			for (size_t i = 0; i < this->data.size(); ++i) {
				if (this->data[i]) {
					delete this->data[i];
					this->data[i] = nullptr;
				}
			}
			this->data.clear();
			this->data2.clear();
			this->order.clear();
		}

		void add_object_with_key(UserType* object) {
			const auto& name = object->name;

			if (!object->has_key()) {
				throw "Error in add_object_with_key";
			}

			if (is_array()) {
				throw "Error in add_object_with_key";
			}

			if (this->type == -1 && this->data.size() + this->data2.size() >= 1) {
				throw "Error not valid json in add_object_with_key";
			}

			this->data.push_back(object);
			((UserType*)this->data.back())->parent = this;
		}

		void add_array_with_key(UserType* _array) {
			const auto& name = _array->name;

			if (!_array->has_key()) {
				throw "Error in add_array_with_key";
			}

			if (is_array()) {
				throw "Error in add_array_with_key";
			}

			if (this->type == -1 && this->data.size() + this->data2.size() >= 1) {
				throw "Error not valid json in add_array_with_key";
			}

			this->data.push_back(_array);
			((UserType*)this->data.back())->parent = this;
		}

		void add_object_with_no_key(UserType* object) {
			const Data& name = object->name;

			if (object->has_key()) {
				throw "Error in add_object_with_no_key";
			}

			if (is_object()) {
				throw "Error in add_object_with_no_key";
			}

			if (this->type == -1 && this->data.size() + this->data2.size() >= 1) {
				throw "Error not valid json in add_object_with_no_key";
			}

			this->data.push_back(object);
			((UserType*)this->data.back())->parent = this;

			order.push_back(ArrayOrObject);
		}

		void add_array_with_no_key(UserType* _array) {
			const Data& name = _array->name;

			if (_array->has_key()) {
				throw "Error in add_array_with_no_key";
			}

			if (is_object()) {
				throw "Error in add_array_with_no_key";
			}

			if (this->type == -1 && this->data.size() + this->data2.size() >= 1) {
				throw "Error not valid json in add_array_with_no_key";
			}

			this->data.push_back(_array);
			((UserType*)this->data.back())->parent = this;

			order.push_back(ArrayOrObject);
		}

		 void reserve_data_list(size_t len) {
			data.reserve(len);
		}

		 void reserve_data2_list(size_t len) {
			data2.reserve(len);
		}


	private:

		void add_user_type(UserType* ut) {
			this->data.push_back(ut);
			ut->parent = this;

			if (!ut->get_name().is_key()) {
				order.push_back(ArrayOrObject);
			}
		}


		static UserType make_none() {
			Data temp;
			temp.str_val = "";
			temp.type = simdjson::internal::tape_type::STRING;

			UserType ut(temp, -2);

			return ut;
		}

		bool is_virtual() const {
			return type == 2 || type == 3;
		}

		static UserType make_virtual_object() {
			UserType ut;
			ut.type = 2;
			return ut;
		}

		static UserType make_virtual_array() {
			UserType ut;
			ut.type = 3;
			return ut;
		}

		void add_user_type(const Data& name, int type) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.
			// todo - chk this->type == -1 .. one object or one array or data(true or false or null or string or number).


			if (this->type == -1 && this->data.size() + this->data2.size() >= 1) {
				throw "Error not valid json in add_user_type";
			}

			this->data.push_back(make_user_type(name, type));

			((UserType*)this->data.back())->parent = this;
			
			if (!name.is_key()) {
				this->order.push_back(ArrayOrObject);
			}
		}

		void add_user_type(Data&& name, int type) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.
			// todo - chk this->type == -1 .. one object or one array or data(true or false or null or string or number).

			if (this->type == -1 && this->data.size() + this->data2.size() >= 1) {
				throw "Error not valid json in add_user_type";
			}

			this->data.push_back(make_user_type(std::move(name), type));
			((UserType*)this->data.back())->parent = this;
			
			if (!name.is_key()) {
				this->order.push_back(ArrayOrObject);
			}

		}

		// add item_type in object? key = value
		 void add_item_type(Data&& name, clau::Data&& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			if (this->type == -1 && this->data.size() + this->data2.size() >= 1) {
				throw "Error not valid json in add_item_type";
			}

			if (name.is_key()) {
				this->data2.push_back(std::move(name));
			}
			else {
				this->order.push_back(Item);
			}

			this->data2.push_back(std::move(data));
		}

		void add_item_type(const Data& name, clau::Data&& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			if (this->type == -1 && this->data.size() + this->data2.size() >= 1) {
				throw "Error not valid json in add_item_type";
			}
			

			if (name.is_key()) {
				this->data2.push_back(name);
			}
			else {
				this->order.push_back(Item);
			}

			this->data2.push_back(std::move(data));
		}
		void add_item_type(const Data& name, const clau::Data& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			if (this->type == -1 && this->data.size() + this->data2.size() >= 1) {
				throw "Error not valid json in add_item_type";
			}

			if (name.is_key()) {
				this->data2.push_back(name);
			}
			else {
				this->order.push_back(Item);
			}

			this->data2.push_back(data);
		}


	public:
		UserType* get_data_list(const Data& name) {
			for (size_t i = this->data.size(); i > 0; --i) {
				if (name == this->data[i - 1]->get_name()) {
					return this->data[i - 1];
				}
			}
			throw "NOT EXIST in get_item_type_list";
		}
		const UserType* get_data_list(const Data& name) const {
			for (size_t i = this->data.size(); i > 0; --i) {
				if (name == this->data[i - 1]->get_name()) {
					return this->data[i - 1];
				}
			}
			throw "NOT EXIST in get_item_type_list";
		}

		UserType*& get_data_list(size_t idx) {
			return this->data[idx];
		}
		const UserType* const& get_data_list(size_t idx) const {
			return this->data[idx];
		}

		Data& get_data2_list(size_t idx) {
			return this->data2[idx];
		}
		const Data& get_data2_list(size_t idx) const {
			return this->data2[idx];
		}

		size_t get_data_size() const {
			return this->data.size();
		}

		size_t get_data2_size() const {
			return this->data2.size();
		}

		void remove_data_list(size_t idx) {
			delete data[idx];
			data.erase(data.begin() + idx);
						
			if (is_array() || is_root()) {
				size_t count = 0;
				for (size_t i = 0; i < order.size(); ++i) {
					if (is_array_or_object(i)) {
						if (idx == count) {
							order.erase(order.begin() + i);
							break;
						}
						count++;
					}
				}
			}
		}

		void remove_data2_list(size_t idx) {
			data2.erase(data2.begin() + idx);

			if (is_array() || is_root()) {
				size_t count = 0;
				for (size_t i = 0; i < order.size(); ++i) {
					if (is_item(i)) {
						if (idx == count) {
							order.erase(order.begin() + i);
							break;
						}
						count++;
					}
				}
			}
		}

		UserType* get_parent() {
			return parent;
		}

		const UserType* get_parent() const {
			return parent;
		}

		size_t get_order_size() const {
			return order.size();
		}
		
		void reserve_order_list(size_t len) {
			order.reserve(len);
		}

		bool is_item(size_t no) const {
			return order[no] == Item;
		}
		bool is_array_or_object(size_t no) const {
			return order[no] == ArrayOrObject;
		}

		friend class LoadData;
	};


	class LoadData
	{
	public:
		static int Merge(class UserType* next, class UserType* ut, class UserType** ut_next)
		{

			//check!!
			while (ut->get_data_size() >= 1
				&& (ut->get_data_list(0)->is_user_type()) && ((UserType*)ut->get_data_list(0))->is_virtual())
			{
				ut = (UserType*)ut->get_data_list(0);
			}

			bool chk_ut_next = false;

			while (true) {

				class UserType* _ut = ut;
				class UserType* _next = next;


				if (ut_next && _ut == *ut_next) {
					*ut_next = _next;
					chk_ut_next = true;
				}

				size_t _size = _ut->get_data_size(); // bug fix.. _next == _ut?
				for (size_t i = 0; i < _size; ++i) {
					if (((UserType*)_ut->get_data_list(i))->is_virtual()) {
						//_ut->get_user_type_list(i)->used();
					}
					else {
						_next->LinkUserType((UserType*)_ut->get_data_list(i));
						_ut->get_data_list(i) = nullptr;
					}
				}
				
				_size = _ut->get_data2_size();
				for (size_t i = 0; i < _size; ++i) {
					if (_ut->get_data2_list(0).is_key()) {

						_next->add_item_type(std::move(_ut->get_data2_list(i)), std::move(_ut->get_data2_list(i + 1)));
						++i;
					}
					else {
						_next->add_item_type(Data(), std::move(_ut->get_data2_list(i)));
					}
				}

				_ut->remove_all();

				ut = ut->get_parent();
				next = next->get_parent();


				if (next && ut) {
					//
				}
				else {
					// right_depth > left_depth
					if (!next && ut) {
						return -1;
					}
					else if (next && !ut) {
						return 1;
					}

					return 0;
				}
			}
		}

	private:
		static bool __LoadData(const simdjson::Token* token_arr,
			int64_t token_arr_start, size_t token_arr_len, class UserType* _global,
			int start_state, int last_state, class UserType** next, int* err, int no)
		{

			std::vector<Data> varVec;
			std::vector<Data> valVec;


			if (token_arr_len <= 0) {
				*next = nullptr;
				return false;
			}

			class UserType& global = *_global;

			int state = start_state;
			size_t braceNum = 0;
			std::vector< class UserType* > nestedUT(1);
			Data var;
			Data val;

			nestedUT.reserve(10);
			nestedUT[0] = &global;

			int64_t count = 0;

			for (int64_t i = 0; i < token_arr_len; ++i) {
				if ((token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::COMMA) {
					continue;
				}

				if ((token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::COLON) {
					continue;
				}

				switch (state)
				{
				case 0:
				{
					// Left 1
					if ((token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::START_OBJECT ||
						(token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::START_ARRAY) { // object start, array start

						if (!varVec.empty()) {
							if (varVec[0] == Data()) { // no key
								nestedUT[braceNum]->reserve_data2_list(nestedUT[braceNum]->get_data2_size() + varVec.size());
								nestedUT[braceNum]->reserve_order_list(nestedUT[braceNum]->get_order_size() + varVec.size());
							}
							else {
								nestedUT[braceNum]->reserve_data2_list(nestedUT[braceNum]->get_data2_size() + varVec.size() * 2);
							}

							for (size_t x = 0; x < varVec.size(); ++x) {
								nestedUT[braceNum]->add_item_type(std::move(varVec[x]), std::move(valVec[x]));
							}

							varVec.clear();
							valVec.clear();
						}

						nestedUT[braceNum]->add_user_type(std::move(var), (token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::START_OBJECT ? 0 : 1); // object vs array
						class UserType* pTemp = (UserType*)(((UserType*)nestedUT[braceNum])->get_data_list(((UserType*)nestedUT[braceNum])->get_data_size() - 1));
						var = Data();


						//
						///pTemp->reserve_data_list((token_arr)[token_arr_start + i].data.count_it + (token_arr)[token_arr_start + i].data.count_ut);

						braceNum++;

						/// new nestedUT
						if (nestedUT.size() == braceNum) {
							nestedUT.push_back(nullptr);
						}

						/// initial new nestedUT.
						nestedUT[braceNum] = pTemp;

						state = 0;
					}
					// Right 2
					else if ((token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::END_OBJECT ||
						(token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::END_ARRAY) {

						state = 0;

						if (!varVec.empty()) {
							if ((token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::END_OBJECT) {
								nestedUT[braceNum]->reserve_data2_list(nestedUT[braceNum]->get_data2_size() + varVec.size() * 2);
							}
							else { // END_ARRAY
								nestedUT[braceNum]->reserve_data2_list(nestedUT[braceNum]->get_data2_size() + varVec.size());
								nestedUT[braceNum]->reserve_order_list(nestedUT[braceNum]->get_order_size() + varVec.size());
							}

							for (size_t x = 0; x < varVec.size(); ++x) {
								nestedUT[braceNum]->add_item_type(std::move(varVec[x]), std::move(valVec[x]));
							}


							varVec.clear();
							valVec.clear();
						}

						if (braceNum == 0) {
							class UserType ut = UserType::make_none();
							Data temp;
							temp.str_val = "#";
							temp.type = simdjson::internal::tape_type::KEY_VALUE;

							ut.add_user_type(temp, (token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::END_OBJECT ? 2 : 3); // json -> "var_name" = val  

							class UserType* pTemp = (UserType*)ut.get_data_list(0);

						//	pTemp->reserve_data_list((token_arr)[(token_arr)[token_arr_start + i].data.uint_val].data.count_it +
						//		(token_arr)[(token_arr)[token_arr_start + i].data.uint_val].data.count_ut);

							for (size_t i = 0; i < nestedUT[braceNum]->get_data_size(); ++i) {
								((UserType*)ut.get_data_list(0))->add_user_type((UserType*)(nestedUT[braceNum]->get_data_list(i)));
								nestedUT[braceNum]->get_data_list(i) = nullptr;
							}

							for (size_t i = 0; i < nestedUT[braceNum]->get_data2_size(); ++i) {
								if (nestedUT[braceNum]->get_data2_list(0).is_key()) {
									((UserType*)ut.get_data_list(0))->add_item_type(std::move((nestedUT[braceNum]->get_data2_list(i))),
										std::move((nestedUT[braceNum]->get_data2_list(i + 1))));
									++i;
								}
								else {
									((UserType*)ut.get_data_list(0))->add_item_type(Data(),
										std::move((nestedUT[braceNum]->get_data2_list(i))));
								}
							}
							

							nestedUT[braceNum]->remove_all();
							nestedUT[braceNum]->add_user_type((UserType*)(ut.get_data_list(0)));

							ut.get_data_list(0) = nullptr;

							braceNum++;
						}

						{
							if (braceNum < nestedUT.size()) {
								nestedUT[braceNum] = nullptr;
							}

							braceNum--;
						}
					}
					else {

						if ((token_arr)[token_arr_start + i].get_type() == simdjson::internal::tape_type::KEY_VALUE) {
							Data data = Convert((token_arr)[token_arr_start + i]);

							var = std::move(data);

							state = 0;
						}
						else {
							{

								val = Convert((token_arr)[token_arr_start + i]);

								varVec.push_back(var);
								valVec.push_back(val);

								var = Data();
								val = Data();

								state = 0;

							}
						}
					}
				}
				break;
				default:
					// syntax err!!
					*err = -1;
					return false; // throw "syntax error ";
					break;
				}
			}

			if (next) {
				*next = nestedUT[braceNum];
			}

			if (varVec.empty() == false) {
				for (size_t x = 0; x < varVec.size(); ++x) {
					nestedUT[braceNum]->add_item_type(std::move(varVec[x]), std::move(valVec[x]));
				}

				varVec.clear();
				valVec.clear();
			}

			if (state != last_state) {
				*err = -2;
				return false;
				// throw STRING("error final state is not last_state!  : ") + toStr(state);
			}

			return true;
		}

		// need random access..
		static int64_t FindDivisionPlace(const std::unique_ptr<simdjson::Token[], simdjson::dom::Free>& token_arr, int64_t start, int64_t last)
		{
			for (int64_t a = last; a >= start; --a) {
				auto& x = (token_arr)[a];

				if (x.get_type() == simdjson::internal::tape_type::END_OBJECT || x.get_type() == simdjson::internal::tape_type::END_ARRAY) { // right
					return a;
				}

				bool pass = false;
				if (x.get_type() == simdjson::internal::tape_type::START_OBJECT || x.get_type() == simdjson::internal::tape_type::START_ARRAY) { // left
					return a;
				}
				else if (x.get_type() == simdjson::internal::tape_type::KEY_VALUE) { // assignment
					//
					pass = true;
				}

				if (a < last && pass == false) {
					auto& y = token_arr[a + 1];
					if (!(y.get_type() == simdjson::internal::tape_type::KEY_VALUE)) // assignment
					{ // NOT
						return a;
					}
				}
			}
			return -1;
		}
	public:

		static bool _LoadData(std::vector< const simdjson::Token* >& token_arr, std::vector<size_t>& length, class UserType& global, const int lex_thr_num, const int parse_num) // first, strVec.empty() must be true!!
		{
			const int pivot_num = parse_num - 1;
			//size_t token_arr_len = length; // size?

			class UserType* before_next = nullptr;
			class UserType _global;

			bool first = true;
			int64_t sum = 0;

			{
				std::set<int64_t> _pivots;
				std::vector<int64_t> pivots;
				//const int64_t num = token_arr_len; //

				if (pivot_num > 0) {
					std::vector<int64_t> pivot;
					pivots.reserve(pivot_num);
					pivot.reserve(pivot_num);

					for (int i = 0; i < pivot_num; ++i) {
						pivot.push_back(i); /// chk...
					//	pivot.push_back(FindDivisionPlace(token_arr, (num / (pivot_num + 1)) * (i), (num / (pivot_num + 1)) * (i + 1) - 1));
					}

					for (size_t i = 0; i < pivot.size(); ++i) {
						if (pivot[i] != -1) {
							_pivots.insert(pivot[i]);
						}
					}

					for (auto& x : _pivots) {
						pivots.push_back(x);
					}
				}

				std::vector<class UserType*> next(pivots.size() + 1, nullptr);

				{
					std::vector<class UserType> __global(pivots.size() + 1, UserType::make_none());

					std::vector<std::thread> thr(pivots.size() + 1);
					std::vector<int> err(pivots.size() + 1, 0);
					{
					//	int64_t idx = pivots.empty() ? num - 1 : pivots[0];
					//	int64_t _token_arr_len = idx - 0 + 1;

						thr[0] = std::thread(__LoadData, (token_arr[0]), 0, length[0], &__global[0], 0, 0, &next[0], &err[0], 0);
					}

					for (size_t i = 1; i < pivots.size(); ++i) {
						int64_t _token_arr_len = pivots[i] - (pivots[i - 1] + 1) + 1;

						thr[i] = std::thread(__LoadData, (token_arr[i]), 0, length[i], &__global[i], 0, 0, &next[i], &err[i], i);
					}

					if (pivots.size() >= 1) {
					//	int64_t _token_arr_len = num - 1 - (pivots.back() + 1) + 1;

						thr[pivots.size()] = std::thread(__LoadData, (token_arr[pivots.size()]), 0, length[pivots.size()], &__global[pivots.size()],
							0, 0, &next[pivots.size()], &err[pivots.size()], pivots.size());
					}


					auto a = std::chrono::steady_clock::now();

					// wait
					for (size_t i = 0; i < thr.size(); ++i) {
						thr[i].join();
					}


					auto b = std::chrono::steady_clock::now();
					auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(b - a);
					std::cout << "parse1 " << dur.count() << "ms\n";

					for (size_t i = 0; i < err.size(); ++i) {
						switch (err[i]) {
						case 0:
							break;
						case -1:
						case -4:
							std::cout << "Syntax Error\n"; return false;
							break;
						case -2:
							std::cout << "error final state is not last_state!\n"; return false;
							break;
						case -3:
							std::cout << "error x > buffer + buffer_len:\n"; return false;
							break;
						default:
							std::cout << "unknown parser error\n"; return false;
							break;
						}
					}

					// Merge
					try 
					{	
						int i = 0;
						std::vector<int> chk(8, 0);
						auto x = next.begin();
						auto y = __global.begin();
						while (true) {
							if (y->get_data_size() + y->get_data2_size() == 0) {
								chk[i] = 1;
							}

							++x;
							++y;
							++i;

							if (x == next.end()) {
								break;
							}
						}

						int start = 0;
						int last = parse_num - 1;

						for (int i = 0; i < parse_num; ++i) {
							if (chk[i] == 0) {
								start = i;
								break;
							}
						}

						for (int i = parse_num - 1; i >= 0; --i) {
							if (chk[i] == 0) {
								last = i;
								break;
							}
						}

						if (__global[start].get_data_size() > 0 && __global[start].get_data_list(0)->is_user_type()
								&& ((UserType*)__global[start].get_data_list(0))->is_virtual()) {
							std::cout << "not valid file1\n";
							throw 1;
						}
						if (next[last] && next[last]->get_parent() != nullptr) {
							std::cout << "not valid file2\n";
							throw 2;
						}

					

						int err = Merge(&_global, &__global[start], &next[start]);
						if (-1 == err || (pivots.size() == 0 && 1 == err)) {
							std::cout << "not valid file3\n";
							throw 3;
						}

						for (int i = start + 1; i <= last; ++i) {

							if (chk[i]) {
								continue;
							}

							// linearly merge and error check...
							int before = i - 1;
							for (int k = i - 1; k >= 0; --k) {
								if (chk[k] == 0) {
									before = k;
									break;
								}
							}

 							int err = Merge(next[before], &__global[i], &next[i]);

							if (-1 == err) {
								std::cout << "chk " << i << " " << __global.size() << "\n";
								std::cout << "not valid file4\n";
								throw 4;
							}
							else if (i == pivots.size() && 1 == err) {
								std::cout << "not valid file5\n";
								throw 5;
							}
						}
					}
					catch (...) {
						//throw "in Merge, error";
						return false;
					}

					before_next = next.back();

					auto c = std::chrono::steady_clock::now();
					auto dur2 = std::chrono::duration_cast<std::chrono::nanoseconds>(c - b);
					std::cout << "parse2 " << dur2.count() << "ns\n";
				}
			}
			//int a = clock();

			Merge(&global, &_global, nullptr);

			/// global = std::move(_global);
			//int b = clock();
			//std::cout << "chk " << b - a << "ms\n";
			return true;
		}
		static bool parse(std::vector< const simdjson::Token* >& tokens, std::vector<size_t>& length, class UserType& global, int thr_num) {
			return LoadData::_LoadData(tokens, length, global, thr_num, thr_num);
		}

		static void _save(std::ostream& stream, UserType* ut, const int depth = 0) {
			for (size_t i = 0; i < ut->get_data2_size(); ++i) {
				auto& x = ut->get_data2_list(i);

				if (x.type == simdjson::internal::tape_type::KEY_VALUE || 
					x.type == simdjson::internal::tape_type::STRING) {
					stream << "\"";
					for (long long j = 0; j < x.str_val.size(); ++j) {
						switch (x.str_val[j]) {
						case '\\':
							stream << "\\\\";
							break;
						case '\"':
							stream << "\\\"";
							break;
						case '\n':
							stream << "\\n";
							break;

						default:
							if (isprint(x.str_val[j]))
							{
								stream << x.str_val[j];
							}
							else
							{
								int code = x.str_val[j];
								if (code > 0 && (code < 0x20 || code == 0x7F))
								{
									char buf[] = "\\uDDDD";
									sprintf(buf + 2, "%04X", code);
									stream << buf;
								}
								else {
									stream << x.str_val[j];
								}
							}
						}
					}
					
					stream << "\"";
						
					if (x.type == simdjson::internal::tape_type::KEY_VALUE) {
						stream << " : ";
					}
				}
				else if (x.type == simdjson::internal::tape_type::TRUE_VALUE) {
					stream << "true";
				}
				else if (x.type == simdjson::internal::tape_type::FALSE_VALUE) {
					stream << "false";
				}
				else if (x.type == simdjson::internal::tape_type::DOUBLE) {
					stream << (x.float_val);
				}
				else if (x.type == simdjson::internal::tape_type::INT64) {
					stream << x.int_val;
				}
				else if (x.type == simdjson::internal::tape_type::UINT64) {
					stream << x.uint_val;
				}
				else if (x.type == simdjson::internal::tape_type::NULL_VALUE) {
					stream << "null ";
				}

				if (ut->is_array() && i < ut->get_data2_size() - 1) {
					stream << ",";
				}
				else if (ut->is_object() && i % 2 == 1 && i < ut->get_data2_size() - 1) {
					stream <<  ",";
				}
				stream << " ";

			}
		
			for (size_t i = 0; i < ut->get_data_size(); ++i) {

				if (ut->is_object()) {
					stream << "\"" << ut->get_data_list(i)->get_name().str_val << "\" : ";
				}

				{ // ut is array
					if (((UserType*)ut->get_data_list(i))->is_object()) {
						stream << " { \n";
					}
					else {
						stream << " [ \n";
					}
				}

				_save(stream, (UserType*)ut->get_data_list(i), depth + 1);

				if (((UserType*)ut->get_data_list(i))->is_object()) {
					stream << " } \n";
				}
				else {
					stream << " ] \n";
				}


				if (i < ut->get_data_size() - 1) {
					stream << ",";
				}
			}
		}

		static void save(const std::string& fileName, class UserType& global) {
			std::ofstream outFile;
			outFile.open(fileName, std::ios::binary); // binary!

			_save(outFile, &global);

			outFile.close();
		}
	};


	class SimdClauJson {
		class _Parse {
		public:

			simdjson::dom::parser* parser;
			void operator()(bool x, int64_t y, int64_t z, int no, simdjson::dom::element* output)
			{
				try {
					*output = parser->parse(x, y, z, no);
				}
				catch (simdjson::simdjson_error e) {
					std::cout << e.what() << "\n";
					exit(-1);
				}
			}

		};

	public:
		// returns -1 then has error.
		static int Parse_One(const std::string& str, clau::Data& data) {
			simdjson::dom::parser parser;
			parser.doc.state = -1;

			auto result = parser.parse(str);

			if (result.error() != simdjson::error_code::SUCCESS) {
				std::cout << result.error() << " ";
				return -1;
			}

			data = clau::Convert(parser.doc.tape[0]);

			return 0;
		}
		static int Parse(const char* fileName, clau::UserType& global, int thr_num) {
			if (thr_num <= 0) {
				thr_num = std::thread::hardware_concurrency();
			}
			if (thr_num <= 0) {
				thr_num = 1;
			}


			int a, b;
			simdjson::dom::parser parser;

			parser.thr_num = thr_num; // chk..

			size_t len = 0;
			a = clock();

			parser.docs = std::vector<simdjson::dom::document>(parser.thr_num);
			for (int i = 0; i < parser.thr_num; ++i) {
				parser.docs[i].ori_doc = &parser.doc;
				parser.docs[i].state = 1;
				parser.docs[i].no = i;
			}

			auto tweets2 = parser.load(fileName, false, 0);

			if (tweets2.error() != simdjson::error_code::SUCCESS) {
				std::cout << tweets2.error() << " ";
				return -1;
			}


			len = parser.len();
			auto split = parser.split();

			b = clock();

			std::cout << b - a << "ms\n";


			//std::cout << "len " << len << "\n";

			a = clock();

			auto count = parser.count();
			std::vector<int64_t> start(parser.thr_num, 0);

			for (int i = 0; i < start.size(); ++i) {
				start[i] = split[i];
			}

			std::vector<int64_t> length(start.size());

			length[start.size() - 1] = len - start[start.size() - 1];

			for (int i = 0; i < start.size() - 1; ++i) {
				length[i] = start[i + 1] - start[i]; //  + 1] - len2[i];

				//std::cout << "length " << length[i] << " ";
			}

			size_t sum = 0;
			for (int i = length.size() - 1; i >= 0; --i) {
				if (length[i] > 0) {
					sum += length[i];
				}
			}

			//std::cout << sum << " " << len << "\n";

			std::vector<const simdjson::Token*> token_arr(parser.thr_num);
			std::vector<size_t> Len(parser.thr_num, 0);

			std::vector<simdjson::dom::element> tweets(parser.thr_num);
			std::vector<int> chk(parser.thr_num, false);

			try {
				std::vector<std::thread*> thr(parser.thr_num);
				bool first = true;

				for (int i = 0; i < parser.thr_num; ++i) {
					_Parse temp;
					temp.parser = &parser;
					
					if (length[i] > 0) {
						if (first) {
							temp.parser->first = i;
							first = false;
						}

						thr[i] = new std::thread(temp, true, length[i], start[i], i, &tweets[i]);
						chk[i] = true;
					}
					else {
						thr[i] = nullptr;
					}
				}

				for (int i = 0; i < parser.thr_num; ++i) {
					if (thr[i]) {
						thr[i]->join();
					}
				}

				for (int i = 0; i < parser.thr_num; ++i) {
					if (thr[i]) {
						delete thr[i];
					}
				}

				for (int i = 0; i < parser.thr_num; ++i) {
					if (chk[i]) {
						token_arr[i] = (tweets[i].raw_tape().get());
						Len[i] = (parser.len(i));
					}
					//	std::cout << "start " << start[i] << " ";
					//	std::cout << length[i] << " " << Len[i] << "\n";
				}
			}
			catch (simdjson::simdjson_error e) {
				std::cout << e.what() << "\n";
				return -2;
			}


			b = clock();
			std::cout << len << "\n";



			std::cout << b - a << "ms\n";


			//a = clock();

			{
				/*
				auto& stream = std::cout;

				for (int i = 0; i < token_arr.size(); ++i) {
					for (int j = 0; j < Len[i]; ++j) {
						if (token_arr[i][j].get_type() == simdjson::internal::tape_type::TRUE_VALUE) {
							stream << "true";
						}
						else if (token_arr[i][j].get_type() == simdjson::internal::tape_type::FALSE_VALUE) {
							stream << "false";
						}
						else if (token_arr[i][j].get_type() == simdjson::internal::tape_type::DOUBLE) {
							stream << (token_arr[i][j].data.float_val);
						}
						else if (token_arr[i][j].get_type() == simdjson::internal::tape_type::INT64) {
							stream << (token_arr[i][j].data.int_val);
						}
						else if (token_arr[i][j].get_type() == simdjson::internal::tape_type::UINT64) {
							stream << (token_arr[i][j].data.uint_val);
						}
						else if (token_arr[i][j].get_type() == simdjson::internal::tape_type::NULL_VALUE) {
							stream << "null ";
						}
						else if (token_arr[i][j].is_key()) {
							stream << "." << j << " \"" << (token_arr[i][j].get_str()) << "\"";
						}
						else if (token_arr[i][j].get_type() == simdjson::internal::tape_type::STRING) {
							stream << ".." << j << " \"" << (token_arr[i][j].get_str()) << "\"";
						}
						else {
							stream << "chk " << j << " " << (int)token_arr[i][j].get_type();
						}

						std::cout << " ";
					}std::cout << "\n";

				}*/


				if (!clau::LoadData::parse(token_arr, Len, global, parser.thr_num)) {
					return -3;
				}
			}

			return 0;
		}
	};

}
