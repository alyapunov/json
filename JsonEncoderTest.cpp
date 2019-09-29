#include "JsonEncoder.h"
#include "Common.h"
#include <fstream>
#include <streambuf>
#include <sstream>
#include <cstring>

template <class T>
void f(T &t)
{
	t << 'a' << 34 << "ok";
}

template <class T>
void g(T &t)
{
	t << 'a' << 34 << "b" << 82;
}

template <class Strm>
void Do(Strm &strm)
{
	CDate date1, date2;
	date1.Load("2001-12-26");
	date2.Load("22.03.1900");
	std::string str("str-test");

	CJsonEncoder<std::ostream> je(strm);
	je << 42 << date1 << date2;
	je << 41 << JSON_SPACE << "test" << true << false << (std::nullptr_t )NULL << JSON_NEW_LINE;
	je << 0.1 << JSON_SPACE << 1e10 << 1e-10 << JSON_NEW_LINE;
	je << 0 << JSON_SPACE << 1u << -1 << (unsigned short)2 << (short)-2
	   << 3ul << -3l << 4ull << -4ll << 'a' << (unsigned char)'b' << 0.5 << 0.8f << JSON_NEW_LINE;
	je << str;
	je << date1 << JSON_SPACE << date1 << date2 << date2 << date1 << date1 << JSON_NEW_LINE;
	je << "more string";
	je << JsonArray();
	je << JsonArray() << 1 << 2 << 3 << 4;
	je << JsonArray() << date1 << date1 << "test" << 't' << true << false << (std::nullptr_t )NULL;
	je << JsonArray() << 1 << JsonArray() << 2 << 3 << 4 << CJsonClose() << 5;
	je << JsonArray() << JsonArray() << 2 << 3 << 4 << CJsonClose() << 5;
	je << JsonArray() << 1 << JsonArray() << 2 << 3 << 4;
	je << JsonArray(1, 2, 3);
	je << JsonArray(1, 2, JsonArray(3, 4));
	je << JsonArray(JsonArray(1, 2), JsonArray(3, 4));
	je << JsonArray(JsonArray(1, 2), JsonArray(3, 4)) << JsonArray();
	je << JsonArray(JSON_COMPACT, JsonArray(JSON_COMPACT, 1, 2), JsonArray(JSON_COMPACT, 3, 4)) << JsonArray(JSON_COMPACT);
	f(je);
	f(je << JsonArray());
	je << "test" << 0.5 << str;
	je << JsonMap();
	je << JsonMap() << 1 << 2 << 3 << 4;
	je << JsonMap() << date1 << date1 << "test" << 't' << true << false << 5 << (std::nullptr_t )NULL;
	je << JsonMap() << 1 << 2 << 3 << JsonMap() << 4 << 5 << CJsonClose() << 6 << 7;
	je << JsonMap() << 3 << JsonMap() << 4 << 5 << CJsonClose() << 6 << 7;
	je << JsonMap() << 1 << 2 << 3 << JsonMap() << 4 << 5;
	je << JsonMap() << 3 << JsonMap() << 4 << 5;
	je << JsonMap(1, 2, 3, 4);
	je << JsonMap(1, 2, 3, JsonMap(4, 6));
	je << JsonMap(JsonMap(1, 2), JsonMap(3, 4));
	je << JsonMap(JsonMap(1, 2), JsonMap(3, 4)) << "a" << "b";
	je << JsonMap(JSON_COMPACT, JsonMap(JSON_COMPACT, 1, 2), JsonMap(JSON_COMPACT, 3, 4)) << "a" << "b";
	g(je << JsonMap());
	je << "test" << 0.5 << str;
	je << JsonMap() << "a" << JsonArray(1, 2, 3) << JsonClose() << "b" << JsonArray(4, 5, 6);
	je << JsonArray() << JsonMap("a", 1, "b", 2) << JsonClose() << JsonMap("c", 3, "d", 4);
}

static bool file_exists(const char *name)
{
	std::ifstream f(name);
	return f.good();
}

static const char *find_input()
{
	const char *n1 = "./jsonenc.result";
	const char *n2 = "../jsonenc.result";
	if (file_exists(n1))
		return n1;
	if (file_exists(n2))
		return n2;
	std::cout << "Failed to find input file." << std::endl;
	exit(1);
}

int main(int, char **)
{
	const char *filename = find_input();
	std::ifstream f(filename);
	std::string ref((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

	std::stringstream test;
	Do(test);

	if (test.str() == ref) {
		std::cout << "OK" << std::endl;
	} else {
		const char *filename = "./jsonenc.reject";
		std::cout << "Failed. see 'diff " << filename <<" jsonenc.result' for details." << std::endl;
		std::ofstream f(filename);
		f << test.str();
		return 1;
	}
}
