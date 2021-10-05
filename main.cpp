
#include "mimalloc-new-delete.h" //

// only 64bit..
//#include "new_simdclaujson.h"
#include "simdclaujson.h"

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
		//clau2::Node node;
		//std::vector<clau2::MemoryPool> pool;

		//if (SimdClauJson2::Parse(argv[1], &node, pool, 8) < 0) {
		//	return -1;
		//}
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

