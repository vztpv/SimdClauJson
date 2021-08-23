
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
		bool is_key;
		union {
			long long int_val;
			unsigned long long uint_val;
			double float_val;
		};
		std::string str_val;
	};

	Data Convert(const simdjson::Token& token) {
		Data result;

		result.type = token.type;
		result.is_key = token.is_key;
		result.int_val = token.int_val;
		result.str_val = std::string(token.str_val);

		return result;
	}

	class UserType {
	private:
		std::string name; // equal to key
		std::map<std::string, std::vector<clau::Data>> itemTypeList; // 
		std::set<std::string> chk_dup;
		std::vector<UserType*> userTypeList; // do not change to std::map.
		int type = -1; // 0 - object, 1 - array, 2 - virtual object or virtual array, -1, -2
		UserType* parent = nullptr;

	public:

		UserType(const UserType& other)
			: name(other.name), itemTypeList(other.itemTypeList), userTypeList(other.userTypeList),
			chk_dup(other.chk_dup),
			type(other.type), parent(other.parent)
		{
			//
		}

		UserType(UserType&& other) {
			name = std::move(other.name);
			itemTypeList = std::move(other.itemTypeList);
			userTypeList = std::move(other.userTypeList);
			chk_dup = std::move(other.chk_dup);
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
			chk_dup = std::move(other.chk_dup);
			type = std::move(other.type);
			parent = std::move(other.parent);

			return *this;
		}

	public:
		std::string get_name() const { return name; }

		void LinkUserType(UserType* ut) // friend?
		{
			userTypeList.push_back(ut);

			if (!ut->name.empty()) {
				if (chk_dup.find(ut->name) != chk_dup.end()) {
					throw "dup in LinkUserType";
				}
				else {
					chk_dup.insert(ut->name);
				}
			}

			ut->parent = this;
		}

		UserType(const std::string& name = "", int type = -1) : name(name), type(type)
		{
			//
		}

		virtual ~UserType() {
			remove();
		}
	public:
		bool is_object() const {
			return type == 0;
		}

		bool is_array() const {
			return type == 1;
		}

		bool is_virtual() const {
			return type == 2;
		}

		static UserType make_virtual() {
			UserType ut("#", 2);

			return ut;
		}

		auto begin_item_type_list() {
			return itemTypeList.begin();
		}

		auto end_item_type_list() {
			return itemTypeList.end();
		}

		size_t find_user_type(const std::string& name, size_t start) {
			// reverse search, to find fastly for last(the latest) item;
			for (size_t i = start; i > 0; --i) {
				if (userTypeList[i - 1]->name == name) {
					return i - 1;
				}
			}
			return size_t(-1); // error.
		}

		size_t find_user_type(const std::string& name) {
			return find_user_type(name, userTypeList.size());
		}

		void add_user_type(const std::string& name, int type) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.
			// todo - chk this->type == -1 .. one object or one array or data(true or false or null or string or number).

			if (this->type == 0 && name.empty()) {
				throw "Error add array-element to object in add_user_type";
			}
			if (this->type == 1 && !name.empty()) {
				throw "Error add object-element to array in add_user_type";
			}
			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_user_type";
			}

			if (!name.empty()) {
				if (chk_dup.find(name) != chk_dup.end()) {
					throw "dup in add_user_type";
				}
				else {
					chk_dup.insert(name);
				}
			}

			userTypeList.push_back(new UserType(name));
			userTypeList.back()->parent = this;
			userTypeList.back()->type = type;

		}

		void add_user_type(UserType* other) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.
			if (this->type == 0 && other->name.empty()) {
				throw "Error add array-element to object in add_user_type";
			}
			if (this->type == 1 && !other->name.empty()) {
				throw "Error add object-element to array in add_user_type";
			}
			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_user_type";
			}


			if (!other->name.empty()) {
				if (chk_dup.find(other->name) != chk_dup.end()) {
					throw "dup in add_user_type";
				}
				else {
					chk_dup.insert(other->name);
				}
			}

			userTypeList.push_back(other);
			other->parent = this;
		}

		// add item_type in object? key = value
		void add_item_type(const std::string& name, const clau::Data& data) {
			// todo - chk this->type == 0 (object) but name is empty
			// todo - chk this->type == 1 (array) but name is not empty.
			if (this->type == 0 && name.empty()) {
				throw "Error add array-element to object in add_item_type";
			}
			if (this->type == 1 && !name.empty()) {
				throw "Error add object-element to array in add_item_type";
			}
			if (this->type == -1 && userTypeList.size() + itemTypeList.size() >= 1) {
				throw "Error not valid json in add_item_type";
			}

			if (!name.empty()) {
				if (chk_dup.find(name) != chk_dup.end()) {
					throw "dup in add_item_type";
				}
				else {
					chk_dup.insert(name);
				}
			}

			auto iter = itemTypeList.find(name);
			if (iter == itemTypeList.end()) {
				itemTypeList.insert(std::make_pair(name, std::vector<clau::Data>{ data }));
			}
			else if (!name.empty()) {
				throw "Error duplicated.. in add_item_type";
				// err .. iter->second.push_back(data);
			}
			else {
				iter->second.push_back(data);
			}
		}

		// add item_type in array? value value  ...
		void add_item_array(std::vector<clau::Data>&& arr) {
			if (arr.empty()) {
				return; // throw error?
			}

			if (this->type == 0) {
				throw "Error add array-element to object in add_item_array";
			}
			if (this->type == -1 && userTypeList.size() + itemTypeList.size() + arr.size() > 1) {
				throw "Error not valid json in add_item_array";
			}

			auto iter = itemTypeList.find("");

			if (iter == itemTypeList.end()) {
				itemTypeList.insert(std::make_pair("", std::vector<clau::Data>{}));
				iter = itemTypeList.find("");
			}



			if (iter->second.empty()) {
				iter->second = std::move(arr);
			}
			else {
				iter->second.reserve(iter->second.size() + arr.size());

				for (size_t i = 0; i < arr.size(); ++i) {
					iter->second.push_back(std::move(arr[i]));
				}
			}
		}

		std::vector<clau::Data>& get_item_type_list(const std::string& name) {
			return itemTypeList.at(name);
		}
		const std::vector<clau::Data>& get_item_type_list(const std::string& name) const {
			return itemTypeList.at(name);
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
			userTypeList.reserve(len);
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
			chk_dup.clear();
		}
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
					std::cout << "chk ";
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
					for (size_t i = 0; i < x->second.size(); ++i) {
						_next->add_item_type(x->first, x->second[i]);
					}
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

			std::vector<std::string> varVec;
			std::vector<Data> valVec;


			if (token_arr_len <= 0) {
				return false;
			}

			class UserType& global = *_global;

			int state = start_state;
			size_t braceNum = 0;
			std::vector< class UserType* > nestedUT(1);
			std::string var;
			Data val;

			nestedUT.reserve(10);
			nestedUT[0] = &global;


			int64_t count = 0;

			for (int64_t i = 0; i < token_arr_len; ++i) {
				/*
				std::cout << braceNum << " " << state << " ";

				switch (token_arr[i].type) {
				case simdjson::internal::tape_type::INT64:
					std::cout << "int64 " << token_arr[i].int_val;
					break;
				case simdjson::internal::tape_type::UINT64:
					std::cout << "uint64 " << token_arr[i].uint_val;
					break;
				case simdjson::internal::tape_type::DOUBLE:
					std::cout << "float " << token_arr[i].float_val;
					break;
				case simdjson::internal::tape_type::STRING:
					std::cout << "string " << token_arr[i].str_val;
					break;
					case (simdjson::internal::tape_type)((unsigned long long)simdjson::internal::tape_type::STRING | (unsigned long long)simdjson::internal::tape_type::KEY_VALUE) :
						std::cout << "key " << token_arr[i].str_val;
						break;
					case simdjson::internal::tape_type::START_ARRAY:
						count++;
						std::cout << "[ ";
						break;
					case simdjson::internal::tape_type::START_OBJECT:
						count++;
						std::cout << "{ ";
						break;
					case simdjson::internal::tape_type::END_ARRAY:
						count--;
						std::cout << "] ";
						break;
					case simdjson::internal::tape_type::END_OBJECT:
						count--;
						std::cout << "} ";
						break;

					case simdjson::internal::tape_type::TRUE_VALUE:
						std::cout << "true";
						break;

					case simdjson::internal::tape_type::FALSE_VALUE:
						std::cout << "false ";
						break;


					case simdjson::internal::tape_type::NULL_VALUE:
						std::cout << "null";
						break;
				}
				std::cout << "\n";
				*/
				switch (state)
				{
				case 0:
				{
					// Left 1
					if (token_arr[token_arr_start + i].type == simdjson::internal::tape_type::START_OBJECT ||
						token_arr[token_arr_start + i].type == simdjson::internal::tape_type::START_ARRAY) { // object start, array start
						if (!varVec.empty()) {

							if (varVec[0].empty()) {
								nestedUT[braceNum]->add_item_array(std::move(valVec));
							}
							else {
								for (size_t x = 0; x < varVec.size(); ++x) {
									nestedUT[braceNum]->add_item_type((varVec[x]), (valVec[x]));
								}
							}

							varVec.clear();
							valVec.clear();
						}

						nestedUT[braceNum]->add_user_type(var, token_arr[token_arr_start + i].type == simdjson::internal::tape_type::START_OBJECT ? 0 : 1); // object vs array
						class UserType* pTemp = nestedUT[braceNum]->get_user_type_list(nestedUT[braceNum]->find_user_type(var));
						var.clear();

						braceNum++;

						/// new nestedUT
						if (nestedUT.size() == braceNum) {
							nestedUT.push_back(nullptr);
						}

						/// initial new nestedUT.
						nestedUT[braceNum] = pTemp;
						///

						state = 0;
					}
					// Right 2
					else if (token_arr[token_arr_start + i].type == simdjson::internal::tape_type::END_OBJECT ||
						token_arr[token_arr_start + i].type == simdjson::internal::tape_type::END_ARRAY) {
						state = 0;

						if (!varVec.empty()) {

							if (varVec[0].empty()) {
								nestedUT[braceNum]->add_item_array(std::move(valVec));
							}
							else {
								for (size_t x = 0; x < varVec.size(); ++x) {
									nestedUT[braceNum]->add_item_type((varVec[x]), (valVec[x]));
								}
							}

							varVec.clear();
							valVec.clear();
						}

						if (braceNum == 0) {
							class UserType ut("", -2);

							ut.add_user_type("#", 2); // json -> "var_name" = val  // clautext, # is line comment delimiter.
							class UserType* pTemp = ut.get_user_type_list(ut.find_user_type("#"));

							for (size_t i = 0; i < nestedUT[braceNum]->get_user_type_list_size(); ++i) {
								ut.get_user_type_list(0)->add_user_type((nestedUT[braceNum]->get_user_type_list(i)));
								nestedUT[braceNum]->get_user_type_list(i) = nullptr;
							}

							for (auto x = nestedUT[braceNum]->begin_item_type_list(); x != nestedUT[braceNum]->end_item_type_list(); ++x) {
								for (size_t i = 0; i < x->second.size(); ++i) {
									ut.get_user_type_list(0)->add_item_type(x->first, x->second[i]);
								}
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

						if (token_arr[token_arr_start + i].type == simdjson::internal::tape_type::KEY_VALUE) {
							Data data = Convert(token_arr[token_arr_start + i]);

							var = (data.str_val);

							state = 0;
						}
						else {
							{

								val = Convert(token_arr[token_arr_start + i]);

								varVec.push_back(var);
								valVec.push_back(val);

								var.clear();
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
					nestedUT[braceNum]->add_item_type((varVec[x]), (valVec[x]));
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
				auto x = (token_arr)[a];

				if (x.type == simdjson::internal::tape_type::END_OBJECT || x.type == simdjson::internal::tape_type::END_ARRAY) { // right
					return a;
				}

				bool pass = false;
				if (x.type == simdjson::internal::tape_type::START_OBJECT || x.type == simdjson::internal::tape_type::START_ARRAY) { // left
					return a;
				}
				else if (x.type == simdjson::internal::tape_type::KEY_VALUE) { // assignment
					//
					pass = true;
				}

				if (a < last && pass == false) {
					auto y = token_arr[a + 1];
					if (!(x.type == simdjson::internal::tape_type::KEY_VALUE)) // assignment
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
			class UserType _global("");

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
					std::vector<class UserType> __global(pivots.size() + 1, UserType("", -2));

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
					try {
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
					catch (...) {
						throw "in Merge, error";
					}

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
	};

	bool parse(const std::unique_ptr<simdjson::Token[], simdjson::dom::Free>& tokens, size_t length, class UserType& global, int thr_num) {
		return LoadData::_LoadData(tokens, length, global, thr_num, thr_num);
	}

	void _save(std::ostream& stream, UserType* ut, const int depth = 0) {
		for (auto x = ut->begin_item_type_list(); x != ut->end_item_type_list(); ++x) {

			for (size_t i = 0; i < x->second.size(); ++i) {

				if (!x->first.empty()) {
					stream << x->first << " = ";
				}

				if (x->second[i].type == simdjson::internal::tape_type::TRUE_VALUE) {
					stream << "true";
				}
				else if (x->second[i].type == simdjson::internal::tape_type::FALSE_VALUE) {
					stream << "false";
				}
				else if (x->second[i].type == simdjson::internal::tape_type::DOUBLE) {
					stream << (x->second[i].float_val);
				}
				else if (x->second[i].type == simdjson::internal::tape_type::INT64) {
					stream << x->second[i].int_val;
				}
				else if (x->second[i].type == simdjson::internal::tape_type::UINT64) {
					stream << x->second[i].uint_val;
				}
				else if (x->second[i].type == simdjson::internal::tape_type::NULL_VALUE) {
					stream << "null ";
				}
				else if (x->second[i].type == simdjson::internal::tape_type::STRING) {
					stream << "\"" << x->second[i].str_val << "\"";
				}

				stream << " ";


			}
		}

		for (size_t i = 0; i < ut->get_user_type_list_size(); ++i) {
			if (!ut->get_user_type_list(i)->get_name().empty()) {
				stream << "\"" << (char*)ut->get_user_type_list(i)->get_name().c_str() << "\"";
				stream << " = { \n";
			}
			else {
				stream << " { \n";
			}

			_save(stream, ut->get_user_type_list(i), depth + 1);

			stream << " } \n";
		}
	}

	void save(const std::string& fileName, class UserType& global) {
		std::ofstream outFile;
		outFile.open(fileName, std::ios::binary); // binary!

		_save(outFile, &global);

		outFile.close();
	}
}


int main(void)
{
	simdjson::dom::parser parser;
	simdjson::dom::element tweets;

	int a = clock();

	size_t len;

	tweets = parser.load("citylots.json");
	len = parser.len();


	int b = clock();

	std::cout << b - a << "ms\n";

	clau::UserType global;

	a = clock();

	const std::unique_ptr<simdjson::Token[], simdjson::dom::Free>& token_arr = tweets.raw_tape();


	clau::parse(token_arr, len, global, 8);

	b = clock();


	std::cout << b - a << "ms\n";

	//clau::save("output.test", global);


	return 0;
}
