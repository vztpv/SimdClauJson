
// only 64bit..

#include "mimalloc-new-delete.h" //

#include <iostream>
#include "simdjson.h" // modified simdjson

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <set>

namespace clau {
	class UserType;

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
		std::string str_val;
	//	size_t count_ut = 0, count_it = 0;

		Data() : type(simdjson::internal::tape_type::NONE) { }

		Data(simdjson::internal::tape_type _type, long long _number_val, std::string&& _str_val) //, size_t count_ut, size_t count_it)
			:type(_type), int_val(_number_val), str_val(std::move(_str_val))//, count_ut(count_ut), count_it(count_it)
		{

		}

		bool operator==(const Data& other) const {
			//
			return this->str_val == other.str_val;
		}

		bool operator<(const Data& other) const {
			//
			return this->str_val < other.str_val;
		}
	};

	inline Data Convert(const simdjson::Token& token) {
		if (token.get_type() == simdjson::internal::tape_type::KEY_VALUE
			|| token.get_type() == simdjson::internal::tape_type::STRING) {
			return Data(token.get_type(), token.data.int_val, std::string(token.get_str())); // , token.data.count_ut, token.data.count_it);
		}
		return Data(token.get_type(), token.data.int_val, ""); // , token.data.count_ut, token.data.count_it);
	}

	class UserType {
	private:
		clau::Data name; // equal to key
		std::vector<std::pair<clau::Data, clau::Data>> itemTypeList; // 
		//std::set<clau::Data> chk_dup;
		std::vector<UserType*> userTypeList; // do not change to std::map.
		int type = -1; // 0 - object, 1 - array, 2 - virtual object, 3 - virtual array, -1, -2
		UserType* parent = nullptr;

	public:
		bool operator<(const UserType& other) const {
			return name.str_val < other.name.str_val;
		}
		bool operator==(const UserType& other) const {
			return name.str_val == other.name.str_val;
		}

	public:
		UserType(const UserType& other)
			: name(other.name), itemTypeList(other.itemTypeList), userTypeList(other.userTypeList),
			//chk_dup(other.chk_dup),
			type(other.type), parent(other.parent)
		{
			//
		}

		UserType(UserType&& other) {
			name = std::move(other.name);
			itemTypeList = std::move(other.itemTypeList);
			userTypeList = std::move(other.userTypeList);
			//chk_dup = std::move(other.chk_dup);
			type = std::move(other.type);
			parent = std::move(other.parent);
		}

		UserType& operator=(UserType&& other) {
			if (this == &other) {
				return *this;
			}
			name = std::move(other.name);
			itemTypeList = std::move(other.itemTypeList);
			userTypeList = std::move(other.userTypeList);
		//	chk_dup = std::move(other.chk_dup);
			type = std::move(other.type);
			parent = std::move(other.parent);

			return *this;
		}

	private:
		std::string get_name() const { return name.str_val; }
	
		void LinkUserType(UserType* ut) // friend?
		{

			//if (!ut->name.str_val.empty()) {
			//	if (chk_dup.find(ut->name) != chk_dup.end()) {
			///		throw "dup in LinkUserType";
			//	}
			//	else {
			//		chk_dup.insert(ut->name);
			//	}
			//}

			userTypeList.push_back(ut);

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
			remove();
		}
	public:
		bool has_key() const {
			return name.is_key();
		}

		bool is_object() const {
			return type == 0;
		}

		bool is_array() const {
			return type == 1;
		}
		
		static UserType make_object(const clau::Data& name) {
			UserType ut(name, 0);

			return ut;
		}

		static UserType make_array(const clau::Data& name) {
			UserType ut(name, 1);

			return ut;
		}
		
		// name key check?
		void add_object_element(const clau::Data& name, const clau::Data& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			if (this->type == 1) {
				throw "Error add object element to array in add_object_element ";
			}
			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_object_element";
			}

			//if (!name.str_val.empty()) {
			//	if (chk_dup.find(name) != chk_dup.end()) {
			//		throw "dup in add_object_element";
			//	}
			//	else {
			//		chk_dup.insert(name);
			//	}
		//	}

			itemTypeList.push_back({ name, data });
		}

		void add_array_element(const clau::Data& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			if (this->type == 0) {
				throw "Error add object element to array in add_array_element ";
			}
			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_array_element";
			}

			Data key;
			key.type = simdjson::internal::tape_type::KEY_VALUE;

			itemTypeList.push_back({ std::move(key), data });
		}

		void remove_all() {
			for (size_t i = 0; i < userTypeList.size(); ++i) {
				if (userTypeList[i]) {
					delete userTypeList[i];
					userTypeList[i] = nullptr;
				}
			}
			userTypeList = std::vector<UserType*>();
			itemTypeList.clear();
			//chk_dup.clear();
		}

		void add_object_with_key(UserType* object) {
			const auto& name = object->name;

			if (!object->has_key()) {
				throw "Error in add_object_with_key";
			}

			if (is_array()) {
				throw "Error in add_object_with_key";
			}

			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_object_with_key";
			}

			//if (!name.str_val.empty()) {
			///	if (chk_dup.find(name) != chk_dup.end()) {
			///		throw "dup in add_object_with_key";
			//	}
			///	else {
			//		chk_dup.insert(name);
			///	}
			//}

			userTypeList.push_back(new UserType(name));
			userTypeList.back()->parent = this;
			userTypeList.back()->type = type;
		}
		
		void add_array_with_key(UserType* _array) {
			const auto& name = _array->name;

			if (!_array->has_key()) {
				throw "Error in add_array_with_key";
			}

			if (is_array()) {
				throw "Error in add_array_with_key";
			}

			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_array_with_key";
			}

			//if (!name.str_val.empty()) {
			//	if (chk_dup.find(name) != chk_dup.end()) {
			///		throw "dup in add_array_with_key";
			//	}
			//	else {
			//		chk_dup.insert(name);
			//	}
		//	}
		//
			userTypeList.push_back(new UserType(name));
			userTypeList.back()->parent = this;
			userTypeList.back()->type = type;
		}

		void add_object_with_no_key(UserType* object) {
			const Data& name = object->name;

			if (object->has_key()) {
				throw "Error in add_object_with_no_key";
			}

			if (is_array()) {
				throw "Error in add_object_with_no_key";
			}

			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_object_with_no_key";
			}

			//if (!name.str_val.empty()) {
			//	if (chk_dup.find(name) != chk_dup.end()) {
			//		throw "dup in add_object_with_no_key";
			//	}
			//	else {
			//		chk_dup.insert(name);
			//	}
			//}

			userTypeList.push_back(new UserType(name));
			userTypeList.back()->parent = this;
			userTypeList.back()->type = type;
		}

		void add_array_with_no_key(UserType* _array) {
			const Data& name = _array->name;

			if (_array->has_key()) {
				throw "Error in add_array_with_no_key";
			}

			if (is_array()) {
				throw "Error in add_array_with_no_key";
			}

			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_array_with_no_key";
			}

			//if (!name.str_val.empty()) {
				//if (chk_dup.find(name) != chk_dup.end()) {
			//		throw "dup in add_array_with_no_key";
			//	}
			///	else {
			//		chk_dup.insert(name);
			//	}
		//	}

			userTypeList.push_back(new UserType(name));
			userTypeList.back()->parent = this;
			userTypeList.back()->type = type;
		}

		size_t item_type_list_size()const {
			return itemTypeList.size();
		}

		void reserve_item_type_list(size_t len) {
			if (len > 0) {
				itemTypeList.reserve(len);
			}
		}

	private:
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

		auto begin_item_type_list() {
			return itemTypeList.begin();
		}

		auto end_item_type_list() {
			return itemTypeList.end();
		}

		size_t find_user_type(const Data& name, size_t start) {
			// reverse search, to find fastly for last(the latest) item;
			for (size_t i = start; i > 0; --i) {
				if (userTypeList[i - 1]->name == name) {
					return i - 1;
				}
			}
			return size_t(-1); // error.
		}

		size_t find_last_user_type() {
			return userTypeList.size() - 1;
		}

		size_t find_user_type(const Data& name) {
			return find_user_type(name, userTypeList.size());
		}

		void add_user_type(const Data& name, int type) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.
			// todo - chk this->type == -1 .. one object or one array or data(true or false or null or string or number).
			
			bool _has_key = name.is_key();

			if (is_object() && !_has_key) {
				throw "Error not valid json in add_item_type";
			}
			else if (is_array() && _has_key) {
				throw "Error not valid json in add_item_type";
			}
			
			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_user_type";
			}

			//if (!name.str_val.empty()) {
			//	if (chk_dup.find(name) != chk_dup.end()) {
			//		throw "dup in add_user_type";
			//	}
			//	else {
			//		chk_dup.insert(name);
			//	}
		//	}

			userTypeList.push_back(new UserType(name));
			userTypeList.back()->parent = this;
			userTypeList.back()->type = type;

		}

		void add_user_type(Data&& name, int type) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.
			// todo - chk this->type == -1 .. one object or one array or data(true or false or null or string or number).
			bool _has_key = name.is_key();

			if (is_object() && !_has_key) {
				throw "Error not valid json in add_item_type";
			}
			else if (is_array() && _has_key) {
				throw "Error not valid json in add_item_type";
			}

			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_user_type";
			}
//
			//if (!name.str_val.empty()) {
		//		if (chk_dup.find(name) != chk_dup.end()) {
			//		throw "dup in add_user_type";
			//	}
		//		else {
		//			chk_dup.insert(name);
		//		}
		//	}

			userTypeList.push_back(new UserType(std::move(name)));
			userTypeList.back()->parent = this;
			userTypeList.back()->type = type;

		}

		void add_user_type(UserType* other) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_user_type";
			}


		//	if (!other->name.str_val.empty()) {
			//	if (chk_dup.find(other->name) != chk_dup.end()) {
			//		throw "dup in add_user_type";
			//	}
			//	else {
			//		chk_dup.insert(other->name);
			//	}
		//	}

			userTypeList.push_back(other);
			other->parent = this;
		}

		// add item_type in object? key = value
		void add_item_type(Data&& name, clau::Data&& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.
			bool _has_key = name.is_key();

			if (is_object() && !_has_key) {
				throw "Error not valid json in add_item_type";
			}
			else if (is_array() && _has_key) {
				throw "Error not valid json in add_item_type";
			}

			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_item_type";
			}

		//	if (!name.str_val.empty()) {
			//	if (chk_dup.find(name) != chk_dup.end()) {
			//		throw "dup in add_item_type";
			//	}
		//		else {
		//			chk_dup.insert(name);
		//		}
		//	}

			itemTypeList.push_back({std::move(name), std::move(data)});
		}

		void add_item_type(const Data& name, clau::Data&& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			bool _has_key = name.is_key();

			if (is_object() && !_has_key) {
				throw "Error not valid json in add_item_type";
			}
			else if (is_array() && _has_key) {
				throw "Error not valid json in add_item_type";
			}

			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_item_type";
			}
//
			//if (!name.str_val.empty()) {
			//	if (chk_dup.find(name) != chk_dup.end()) {
			//		throw "dup in add_item_type";
			//	}
			//	else {
			//		chk_dup.insert(name);
			//	}
			//}

			itemTypeList.push_back({ name, std::move(data) });
		}
		void add_item_type(const Data& name, const clau::Data& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.

			bool _has_key = name.is_key();

			if (is_object() && !_has_key) {
				throw "Error not valid json in add_item_type";
			}
			else if (is_array() && _has_key) {
				throw "Error not valid json in add_item_type";
			}
			
			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_item_type";
			}

			//if (!name.str_val.empty()) {
				//if (chk_dup.find(name) != chk_dup.end()) {
				//	throw "dup in add_item_type";
			//	}
			//	else {
			///		chk_dup.insert(name);
			//	}
		//	}

			itemTypeList.push_back({ name, data });
		}
		


		clau::Data& get_item_type_list(const Data& name) {
			for (size_t i = itemTypeList.size(); i > 0; --i) {
				if (name == itemTypeList[i].first) {
					return itemTypeList[i].second;
				}
			}
			throw "NOT EXIST in get_item_type_list";
		}
		const clau::Data& get_item_type_list(const Data& name) const {
			for (size_t i = itemTypeList.size(); i > 0; --i) {
				if (name == itemTypeList[i].first) {
					return itemTypeList[i].second;
				}
			}
			throw "NOT EXIST in get_item_type_list";
		}

		UserType*& get_user_type_list(const size_t idx) {
			return userTypeList[idx];
		}
		const UserType* get_user_type_list(const size_t idx) const {
			return userTypeList[idx];
		}
		size_t get_user_type_list_size() const {
			return userTypeList.size();
		}

		void reserve_user_type_list(size_t len) {
			if (len > 0) {
				userTypeList.reserve(len);
			}
		}

		UserType* get_parent() {
			return parent;
		}

		void remove() {
			for (size_t i = 0; i < userTypeList.size(); ++i) {
				if (userTypeList[i]) {
					delete userTypeList[i];
					userTypeList[i] = nullptr;
				}
			}
			userTypeList = std::vector<UserType*>();
			itemTypeList.clear();
			//chk_dup.clear();
		}

		friend class LoadData;
	};


	class LoadData
	{
	public:
		static int Merge(class UserType* next, class UserType* ut, class UserType** ut_next)
		{
			//check!!
			while (ut->get_user_type_list_size() >= 1
				&& (ut->get_user_type_list(0)->is_virtual()))
			{
				ut = ut->get_user_type_list(0);
			}

			bool chk_ut_next = false;

			while (true) {

				class UserType* _ut = ut;
				class UserType* _next = next;


				if (ut_next && _ut == *ut_next) {
					*ut_next = _next;
					chk_ut_next = true;
					std::cout << "chk "; //
				}

				for (size_t i = 0; i < _ut->get_user_type_list_size(); ++i) {
					if (_ut->get_user_type_list(i)->is_virtual()) {
						//_ut->get_user_type_list(i)->used();
					}
					else {
						_next->LinkUserType(_ut->get_user_type_list(i));
						_ut->get_user_type_list(i) = nullptr;
					}
				}

				for (auto x = _ut->begin_item_type_list(); x != _ut->end_item_type_list(); ++x) {
					_next->add_item_type(std::move(x->first), std::move(x->second));
				}

				_ut->remove();

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
		static bool __LoadData(const std::unique_ptr<simdjson::Token[], simdjson::dom::Free>& token_arr,
			int64_t token_arr_start, int64_t token_arr_len, class UserType* _global,
			int start_state, int last_state, class UserType** next, int* err)
		{

			std::vector<Data> varVec;
			std::vector<Data> valVec;


			if (token_arr_len <= 0) {
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

				switch (state)
				{
				case 0:
				{
					// Left 1
					if (token_arr[token_arr_start + i].get_type() == simdjson::internal::tape_type::START_OBJECT ||
						token_arr[token_arr_start + i].get_type() == simdjson::internal::tape_type::START_ARRAY) { // object start, array start
						
						if (!varVec.empty()) {
							nestedUT[braceNum]->reserve_item_type_list(nestedUT[braceNum]->item_type_list_size() + varVec.size());

							for (size_t x = 0; x < varVec.size(); ++x) {
								nestedUT[braceNum]->add_item_type(std::move(varVec[x]), std::move(valVec[x]));
							}

							varVec.clear();
							valVec.clear();
						}

						nestedUT[braceNum]->add_user_type(std::move(var), token_arr[token_arr_start + i].get_type() == simdjson::internal::tape_type::START_OBJECT ? 0 : 1); // object vs array
						class UserType* pTemp = nestedUT[braceNum]->get_user_type_list(nestedUT[braceNum]->find_last_user_type());
						var = Data();
						
				//		pTemp->reserve_user_type_list(token_arr[token_arr_start + i].data.count_ut);
				//		pTemp->reserve_item_type_list(token_arr[token_arr_start + i].data.count_it);

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
					else if (token_arr[token_arr_start + i].get_type() == simdjson::internal::tape_type::END_OBJECT ||
						token_arr[token_arr_start + i].get_type() == simdjson::internal::tape_type::END_ARRAY) {
						
						state = 0;

						if (!varVec.empty()) {
							nestedUT[braceNum]->reserve_item_type_list(nestedUT[braceNum]->item_type_list_size() + varVec.size());
							
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

							ut.add_user_type(temp, token_arr[token_arr_start + i].get_type() == simdjson::internal::tape_type::END_OBJECT ? 2 : 3); // json -> "var_name" = val  // clautext, # is line comment delimiter.
							class UserType* pTemp = ut.get_user_type_list(ut.find_user_type(temp));

							for (size_t i = 0; i < nestedUT[braceNum]->get_user_type_list_size(); ++i) {
								ut.get_user_type_list(0)->add_user_type((nestedUT[braceNum]->get_user_type_list(i)));
								nestedUT[braceNum]->get_user_type_list(i) = nullptr;
							}

							for (auto x = nestedUT[braceNum]->begin_item_type_list(); x != nestedUT[braceNum]->end_item_type_list(); ++x) {
								ut.get_user_type_list(0)->add_item_type(std::move(x->first), std::move(x->second));
							}

							nestedUT[braceNum]->remove();
							nestedUT[braceNum]->add_user_type((ut.get_user_type_list(0)));

							ut.get_user_type_list(0) = nullptr;

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

						if (token_arr[token_arr_start + i].get_type() == simdjson::internal::tape_type::KEY_VALUE) {
							Data data = Convert(token_arr[token_arr_start + i]);

							var = std::move(data);

							state = 0;
						}
						else {
							{

								val = Convert(token_arr[token_arr_start + i]);

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
				// throw std::string("error final state is not last_state!  : ") + toStr(state);
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

		static bool _LoadData(const std::unique_ptr<simdjson::Token[], simdjson::dom::Free>& token_arr, size_t length, class UserType& global, const int lex_thr_num, const int parse_num) // first, strVec.empty() must be true!!
		{
			const int pivot_num = parse_num - 1;
			size_t token_arr_len = length; // size?

			class UserType* before_next = nullptr;
			class UserType _global;

			bool first = true;
			int64_t sum = 0;

			{
				std::set<int64_t> _pivots;
				std::vector<int64_t> pivots;
				const int64_t num = token_arr_len; //

				if (pivot_num > 0) {
					std::vector<int64_t> pivot;
					pivots.reserve(pivot_num);
					pivot.reserve(pivot_num);

					for (int i = 0; i < pivot_num; ++i) {
						pivot.push_back(FindDivisionPlace(token_arr, (num / (pivot_num + 1)) * (i), (num / (pivot_num + 1)) * (i + 1) - 1));
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
						int64_t idx = pivots.empty() ? num - 1 : pivots[0];
						int64_t _token_arr_len = idx - 0 + 1;

						thr[0] = std::thread(__LoadData, std::cref(token_arr), 0, _token_arr_len, &__global[0], 0, 0, &next[0], &err[0]);
					}

					for (size_t i = 1; i < pivots.size(); ++i) {
						int64_t _token_arr_len = pivots[i] - (pivots[i - 1] + 1) + 1;

						thr[i] = std::thread(__LoadData, std::cref(token_arr), pivots[i - 1] + 1, _token_arr_len, &__global[i], 0, 0, &next[i], &err[i]);
					}

					if (pivots.size() >= 1) {
						int64_t _token_arr_len = num - 1 - (pivots.back() + 1) + 1;

						thr[pivots.size()] = std::thread(__LoadData, std::cref(token_arr), pivots.back() + 1, _token_arr_len, &__global[pivots.size()],
							0, 0, &next[pivots.size()], &err[pivots.size()]);
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
							std::cout << "Syntax Error\n";
							break;
						case -2:
							std::cout << "error final state is not last_state!\n";
							break;
						case -3:
							std::cout << "error x > buffer + buffer_len:\n";
							break;
						default:
							std::cout << "unknown parser error\n";
							break;
						}
					}

					// Merge
				//	try 
					{
						if (__global[0].get_user_type_list_size() > 0 && __global[0].get_user_type_list(0)->is_virtual()) {
							std::cout << "not valid file1\n";
							throw 1;
						}
						if (next.back() && next.back()->get_parent() != nullptr) {
							std::cout << "not valid file2\n";
							throw 2;
						}

						int err = Merge(&_global, &__global[0], &next[0]);
						if (-1 == err || (pivots.size() == 0 && 1 == err)) {
							std::cout << "not valid file3\n";
							throw 3;
						}

						for (size_t i = 1; i < pivots.size() + 1; ++i) {
							// linearly merge and error check...
							int err = Merge(next[i - 1], &__global[i], &next[i]);
							if (-1 == err) {
								std::cout << "not valid file4\n";
								throw 4;
							}
							else if (i == pivots.size() && 1 == err) {
								std::cout << "not valid file5\n";
								throw 5;
							}
						}
					}
					//catch (...) {
					//	throw "in Merge, error";
					//}

					before_next = next.back();

					auto c = std::chrono::steady_clock::now();
					auto dur2 = std::chrono::duration_cast<std::chrono::nanoseconds>(c - b);
					std::cout << "parse2 " << dur2.count() << "ns\n";
				}
			}
			int a = clock();
			global = std::move(_global);
			int b = clock();
			std::cout << "chk " << b - a << "ms\n";
			return true;
		}
		static bool parse(const std::unique_ptr<simdjson::Token[], simdjson::dom::Free>& tokens, size_t length, class UserType& global, int thr_num) {
			return LoadData::_LoadData(tokens, length, global, thr_num, thr_num);
		}

		static void _save(std::ostream& stream, UserType* ut, const int depth = 0) {
			for (auto x = ut->begin_item_type_list(); x != ut->end_item_type_list(); ++x) {

			

					if (!x->first.str_val.empty()) {
						stream << "\"" << x->first.str_val << "\" : ";
					}

					if (x->second.type == simdjson::internal::tape_type::TRUE_VALUE) {
						stream << "true";
					}
					else if (x->second.type == simdjson::internal::tape_type::FALSE_VALUE) {
						stream << "false";
					}
					else if (x->second.type == simdjson::internal::tape_type::DOUBLE) {
						stream << (x->second.float_val);
					}
					else if (x->second.type == simdjson::internal::tape_type::INT64) {
						stream << x->second.int_val;
					}
					else if (x->second.type == simdjson::internal::tape_type::UINT64) {
						stream << x->second.uint_val;
					}
					else if (x->second.type == simdjson::internal::tape_type::NULL_VALUE) {
						stream << "null ";
					}
					else if (x->second.type == simdjson::internal::tape_type::STRING) {
						stream << "\"" << x->second.str_val << "\"";
					}

					stream << " ";
			}

			for (size_t i = 0; i < ut->get_user_type_list_size(); ++i) {
				if (!ut->get_user_type_list(i)->get_name().empty()) {
					stream << "\"" << (char*)ut->get_user_type_list(i)->get_name().c_str() << "\"";
					if (ut->get_user_type_list(i)->is_object()) {
						stream << " : { \n";
					}
					else {
						stream << " : [ \n";
					}
				}
				else {
					if (ut->get_user_type_list(i)->is_object()) {
						stream << " { \n";
					}
					else {
						stream << " [ \n";
					}
				}

				_save(stream, ut->get_user_type_list(i), depth + 1);

				if (ut->get_user_type_list(i)->is_object()) {
					stream << " } \n";
				}
				else {
					stream << " ] \n";
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
}


int main(int argc, char* argv[])
{
	clau::UserType global;
	int a, b;
	int start = clock();

	{
		size_t len = 0;
		a = clock();

		simdjson::dom::parser parser;

		
		auto tweets = parser.load(argv[1], false, 0);
		if (tweets.error() != simdjson::error_code::SUCCESS) {
			std::cout << tweets.error() << " ";
			return 1;
		}

		len = parser.len();

		b = clock();

		std::cout << b - a << "ms\n";


		std::cout << "len " << len << "\n";
		tweets = parser.load(true, len);
		
		if (tweets.error() != simdjson::error_code::SUCCESS) {
			std::cout << tweets.error() << " ";
			return 2;
		}

		len = parser.len();

		b = clock();

		std::cout << b - a << "ms\n";

		
		a = clock();

		const std::unique_ptr<simdjson::Token[], simdjson::dom::Free>& token_arr = tweets.value().raw_tape();


		clau::LoadData::parse(token_arr, len, global, 8);

	}

	b = clock();

	std::cout << b - start << "ms\n";

	//clau::save("output.test", global);


	return 0;
}

