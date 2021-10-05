
#include "mimalloc-new-delete.h" //

// only 64bit..
#include "new_simdclaujson.h"
#include "simdclaujson.h"

class SimdClauJson2 {
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

	static int Parse(const char* fileName, clau2::Node* global, std::vector<clau2::MemoryPool>& pool, int thr_num) {
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

			for (int i = 0; i < parser.thr_num; ++i) {
				_Parse temp;
				temp.parser = &parser;

				if (length[i] > 0) {
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
		}


		b = clock();
		std::cout << len << "\n";



		std::cout << b - a << "ms\n";


		a = clock();

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


			clau2::LoadData::parse(token_arr, Len, global, &pool, parser.thr_num);
		}

		return 0;
	}
};


int main(int argc, char* argv[])
{
	{
		//int a, b;
		//a = clock();
		//char* str;
		//int length;

		//FILE* file;

		///fopen_s(&file, argv[1], "rb");
		//fseek(file, 0, SEEK_END);
		//length = ftell(file);
		//fclose(file);

		//fopen_s(&file, argv[1], "rb");
		//str = new char[length + 1];

		//fread(str, sizeof(char), length, file);
		//fclose(file);
		//str[length] = '\0';

		//auto x = nlohmann::json::parse(str);
		
		//b = clock();
		//std::cout << b - a << "ms\n";
		//std::cout << x.is_object() << "\n";
	}

	int a = clock();
	{
		clau::UserType global;

		if (clau::SimdClauJson::Parse(argv[1], global, 8) < 0) {
			return -1;
		}
	}
	int b = clock();
	
	std::cout << b - a << "ms\n";

	
	a = clock();
	{
		clau2::Node node;
		std::vector<clau2::MemoryPool> pool;

		if (SimdClauJson2::Parse(argv[1], &node, pool, 8) < 0) {
			return -1;
		}
	}
	b = clock();

	std::cout << "test " << b - a << "ms\n";

	{
		//simdjson::Token token;
		//clau::SimdClauJson::Parse_One("false", token);
		//std::cout << (token.get_type() == simdjson::internal::tape_type::FALSE_VALUE) << "\n";
	}
	//clau::LoadData::save("output.txt2", global); // debug..
	//clau::LoadData::_save(std::cout, &global);

	return 0;
}

