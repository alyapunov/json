#include "JsonDecoder.h"
#include <string>
#include <cstring>
#include <fstream>
#include <streambuf>
#include <sstream>

void out()
{
	std::cout << std::endl;
}

template <class T, class ... Types>
void out(const T& t, Types ... Args)
{
	std::cout << t;
	out(Args ...);
}

template <class ... Types>
void fail(Types ... Args)
{
	out(Args ...);
	exit(1);
}

template <class ... Types>
void fail_if(bool b, Types ... Args)
{
	if (b)
		fail(Args ...);
}

bool deq(double d1, double d2)
{
	return fabs(d1 - d2) < 1e-10 * std::max(fabs(d1), fabs(d2));
}

static bool file_exists(const char *name)
{
	std::ifstream f(name);
	return f.good();
}

static const char *find_input()
{
	const char *n1 = "./jsondec.result";
	const char *n2 = "../jsondec.result";
	if (file_exists(n1))
		return n1;
	if (file_exists(n2))
		return n2;
	std::cout << "Failed to find input file." << std::endl;
	exit(1);
}

void
test1()
{
	const char *filename = find_input();
	std::ifstream f(filename);
	std::string ref((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	std::string input = ref.substr(0, ref.find("----------"));

	std::stringstream test;
	test << input;
	test << "----------\n";

	try {
		size_t processed = 0;
		while (processed < input.size()) {
			CJsonDecoder<std::string> &jd = CJsonDecoder<std::string>::Instance();
			processed += jd.Decode(input, input.size(), processed);
			for (size_t i = 0; i < jd.Count(); i++)
				test << jd[i];
		}
	} catch (CException &e) {
		test << e.message;
	}

	if (test.str() == ref) {
		std::cout << "OK" << std::endl;
	} else {
		const char *rej = "./jsondec.reject";
		std::cout << "Failed. see 'diff " << rej <<" " << filename << "' for details." << std::endl;
		std::ofstream f(rej);
		f << test.str();
	}
}

void
test2()
{
	const char *testMessage = "";
	try {
		std::string str;
		bool passed;
		char chars[64];
		CJsonDecoder<std::string> &jd = CJsonDecoder<std::string>::Instance();
		size_t processed;

		testMessage = "Failed at test 1.1: ";
		str = "\"test\"";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() == 0, testMessage, "not parsed");
		fail_if(jd.Count() > 1, testMessage, "wrong parsed");
		jd[0].ExtractString(chars, sizeof(chars), "tool big string");
		fail_if(strcmp(chars, "test") != 0, testMessage, "wrong result", chars);

		testMessage = "Failed at test 1.2.1: ";
		str = "\t\"test\"   ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() == 0, testMessage, "not parsed");
		fail_if(jd.Count() > 1, testMessage, "wrong parsed");
		jd[0].ExtractString(chars, sizeof(chars), "tool big string");
		fail_if(strcmp(chars, "test") != 0, testMessage, "wrong result", chars);
		fail_if(!jd[0].StringEqual("test"), testMessage, "wrong result", chars);
		fail_if(jd[0].StringEqual("tesa"), testMessage, "wrong result", chars);
		fail_if(jd[0].StringEqual("tes"), testMessage, "wrong result", chars);
		fail_if(jd[0].StringEqual("testa"), testMessage, "wrong result", chars);

		testMessage = "Failed at test 1.2.2: ";
		str = "\t\"te\\tst\"   ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() == 0, testMessage, "not parsed");
		fail_if(jd.Count() > 1, testMessage, "wrong parsed");
		jd[0].ExtractString(chars, sizeof(chars), "tool big string");
		fail_if(strcmp(chars, "te\tst") != 0, testMessage, "wrong result", chars);
		fail_if(!jd[0].StringEqual("te\tst"), testMessage, "wrong result", chars);
		fail_if(jd[0].StringEqual("te\tsa"), testMessage, "wrong result", chars);
		fail_if(jd[0].StringEqual("te\ts"), testMessage, "wrong result", chars);
		fail_if(jd[0].StringEqual("te\tsta"), testMessage, "wrong result", chars);

		testMessage = "Failed at test 1.2.3: ";
		str = "\t\"te\tst\"   ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() == 0, testMessage, "not parsed");
		fail_if(jd.Count() > 1, testMessage, "wrong parsed");
		jd[0].ExtractString(chars, sizeof(chars), "tool big string");
		fail_if(strcmp(chars, "te\tst") != 0, testMessage, "wrong result", chars);
		fail_if(!jd[0].StringEqual("te\tst"), testMessage, "wrong result", chars);
		fail_if(jd[0].StringEqual("te\tsa"), testMessage, "wrong result", chars);
		fail_if(jd[0].StringEqual("te\ts"), testMessage, "wrong result", chars);
		fail_if(jd[0].StringEqual("te\tsta"), testMessage, "wrong result", chars);

		testMessage = "Failed at test 1.3: ";
		str = "\"test]}[]{}";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 0, testMessage, "wrong num processed");
		fail_if(jd.Count() != 0, testMessage, "was parsed");

		testMessage = "Failed at test 1.4: ";
		str = "\t\n\"test,:890-e1.";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 2, testMessage, "wrong num processed");
		fail_if(jd.Count() != 0, testMessage, "was parsed");

		testMessage = "Failed at test 1.5: ";
		str = "11.22 ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() == 0, testMessage, "not parsed");
		fail_if(jd.Count() > 1, testMessage, "wrong parsed");
		fail_if(!deq(strtod(str.c_str(), 0), jd[0].GetNumber()), testMessage, "wrong result");

		testMessage = "Failed at test 1.6: ";
		str = "\t11e10    ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() == 0, testMessage, "not parsed");
		fail_if(jd.Count() > 1, testMessage, "wrong parsed");
		fail_if(!deq(strtod(str.c_str(), 0), jd[0].GetNumber()), testMessage, "wrong result");

		testMessage = "Failed at test 1.7: ";
		str = "\t-13e12    ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() == 0, testMessage, "not parsed");
		fail_if(jd.Count() > 1, testMessage, "wrong parsed");
		fail_if(!deq(strtod(str.c_str(), 0), jd[0].GetNumber()), testMessage, "wrong result");

		testMessage = "Failed at test 1.8: ";
		str = "\t-1E-10    ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() == 0, testMessage, "not parsed");
		fail_if(jd.Count() > 1, testMessage, "wrong parsed");
		fail_if(!deq(strtod(str.c_str(), 0), jd[0].GetNumber()), testMessage, "wrong result");

		testMessage = "Failed at test 1.9: ";
		str = "\t-1e-10";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 1, testMessage, "wrong num processed");
		fail_if(jd.Count() != 0, testMessage, "was parsed");

		testMessage = "Failed at test 1.10: ";
		str = "\t. ";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.11: ";
		str = "\t.. ";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.12: ";
		str = "\t.. ";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.13: ";
		str = "\t.e10 ";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.14: ";
		str = "\t1.ee10 ";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.15: ";
		str = "\t0.5 ";
		processed = jd.Decode(str, str.size());
		fail_if(!deq(strtod(str.c_str(), 0), jd[0].GetNumber()), testMessage, "wrong result");
		passed = false;
		try {
			jd[0].GetInt();
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.16: ";
		str = "\t.5 ";
		processed = jd.Decode(str, str.size());
		fail_if(!deq(strtod(str.c_str(), 0), jd[0].GetNumber()), testMessage, "wrong result");
		passed = false;
		try {
			jd[0].GetInt();
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.17: ";
		str = "\t-123456 ";
		processed = jd.Decode(str, str.size());
		fail_if(jd[0].GetInt() != -123456, testMessage, "was parsed");

		testMessage = "Failed at test 1.18: ";
		str = "tru";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 0, testMessage, "wrong num processed");
		fail_if(jd.Count() != 0, testMessage, "was parsed");

		testMessage = "Failed at test 1.19: ";
		str = "true";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 1, testMessage, "was not parsed");
		fail_if(jd[0].GetBool() != true, testMessage, "wrong result");

		testMessage = "Failed at test 1.20: ";
		str = "fa";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 0, testMessage, "wrong num processed");
		fail_if(jd.Count() != 0, testMessage, "was parsed");

		testMessage = "Failed at test 1.21: ";
		str = "false";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 1, testMessage, "was not parsed");
		fail_if(jd[0].GetBool() != false, testMessage, "wrong result");

		testMessage = "Failed at test 1.22: ";
		str = "nu";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 0, testMessage, "wrong num processed");
		fail_if(jd.Count() != 0, testMessage, "was parsed");

		testMessage = "Failed at test 1.23: ";
		str = "null";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 1, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_NULL, testMessage, "wrong result");

		testMessage = "Failed at test 1.24: ";
		str = "trust";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.25: ";
		str = "truefalse";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 4, testMessage, "wrong num processed");
		fail_if(jd.Count() != 1, testMessage, "was not parsed");
		fail_if(jd[0].GetBool() != true, testMessage, "wrong result");

		testMessage = "Failed at test 1.26: ";
		str = "truefalse";
		processed = jd.Decode(str, str.size(), 4);
		fail_if(processed != 5, testMessage, "wrong num processed");
		fail_if(jd.Count() != 1, testMessage, "was not parsed");
		fail_if(jd[0].GetBool() != false, testMessage, "wrong result");

		testMessage = "Failed at test 1.27: ";
		str = ",";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.28: ";
		str = " [] ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 1, testMessage, "was not parsed");
		fail_if(jd[0].GetArraySize() != 0, testMessage, "wrong result");

		testMessage = "Failed at test 1.29: ";
		str = " {} ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 1, testMessage, "was not parsed");
		fail_if(jd[0].GetObjectSize() != 0, testMessage, "wrong result");

		testMessage = "Failed at test 1.30: ";
		str = " [1]";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 2, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_ARRAY, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");

		testMessage = "Failed at test 1.31: ";
		str = " [1, 2, 3]";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 4, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_ARRAY, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 2, testMessage, "wrong result");
		fail_if(jd[3].GetInt() != 3, testMessage, "wrong result");

		testMessage = "Failed at test 1.32: ";
		str = "[1,2][3,4]";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 5, testMessage, "wrong num processed");
		fail_if(jd.Count() != 3, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_ARRAY, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 2, testMessage, "wrong result");
		processed = jd.Decode(str, str.size(), processed);
		fail_if(processed != 5, testMessage, "wrong num processed");
		fail_if(jd.Count() != 3, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_ARRAY, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 3, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 4, testMessage, "wrong result");

		testMessage = "Failed at test 1.33: ";
		str = "[1,2]strage things";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 5, testMessage, "wrong num processed");
		fail_if(jd.Count() != 3, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_ARRAY, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 2, testMessage, "wrong result");

		testMessage = "Failed at test 1.34: ";
		str = "[1,2],";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 5, testMessage, "wrong num processed");
		fail_if(jd.Count() != 3, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_ARRAY, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 2, testMessage, "wrong result");

		testMessage = "Failed at test 1.35: ";
		str = "[,]";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.36: ";
		str = "[1,]";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.37: ";
		str = "[,1]";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.38: ";
		str = "[1:2]";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");


		testMessage = "Failed at test 1.40: ";
		str = " {1:2}";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 3, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_OBJECT, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 2, testMessage, "wrong result");

		testMessage = "Failed at test 1.41: ";
		str = " {1: 2, 3:4} ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 5, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_OBJECT, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 2, testMessage, "wrong result");
		fail_if(jd[3].GetInt() != 3, testMessage, "wrong result");
		fail_if(jd[4].GetInt() != 4, testMessage, "wrong result");

		testMessage = "Failed at test 1.42: ";
		str = "{1:2}{3:4}";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 5, testMessage, "wrong num processed");
		fail_if(jd.Count() != 3, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_OBJECT, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 2, testMessage, "wrong result");
		processed = jd.Decode(str, str.size(), processed);
		fail_if(processed != 5, testMessage, "wrong num processed");
		fail_if(jd.Count() != 3, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_OBJECT, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 3, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 4, testMessage, "wrong result");

		testMessage = "Failed at test 1.43: ";
		str = "{1:2}strage things";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 5, testMessage, "wrong num processed");
		fail_if(jd.Count() != 3, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_OBJECT, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 2, testMessage, "wrong result");

		testMessage = "Failed at test 1.44: ";
		str = "{1:2},";
		processed = jd.Decode(str, str.size());
		fail_if(processed != 5, testMessage, "wrong num processed");
		fail_if(jd.Count() != 3, testMessage, "was not parsed");
		fail_if(jd[0].type != JSON_OBJECT, testMessage, "wrong result");
		fail_if(jd[1].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 2, testMessage, "wrong result");

		testMessage = "Failed at test 1.45: ";
		str = "{,}";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.46: ";
		str = "{:}";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.47: ";
		str = "{1:}";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.48: ";
		str = "{1,2}";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.49: ";
		str = "{1:2:3}";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.50: ";
		str = "{1:2,3:}";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.51: ";
		str = "{1:2,3,4}";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.52: ";
		str = "{1:2:3:4}";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.53: ";
		str = "{1,2,3,4}";
		passed = false;
		try {
			processed = jd.Decode(str, str.size());
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.54: ";
		str = "[{1:2,3:true,4:[5, 6]},\"test\"]";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 11, testMessage, "was not parsed");
		fail_if(jd[0].GetArraySize() != 2, testMessage, "wrong result");
		fail_if(jd[1].GetObjectSize() != 6, testMessage, "wrong result");
		fail_if(jd[2].GetInt() != 1, testMessage, "wrong result");
		fail_if(jd[3].GetInt() != 2, testMessage, "wrong result");
		fail_if(jd[4].GetInt() != 3, testMessage, "wrong result");
		fail_if(jd[5].GetBool() != true, testMessage, "wrong result");
		fail_if(jd[6].GetInt() != 4, testMessage, "wrong result");
		fail_if(jd[7].GetArraySize() != 2, testMessage, "wrong result");
		fail_if(jd[8].GetInt() != 5, testMessage, "wrong result");
		fail_if(jd[9].GetInt() != 6, testMessage, "wrong result");
		fail_if(!jd[10].StringEqual("test"), testMessage, "wrong result");

		testMessage = "Failed at test 1.55: ";
		str = "\"2011-03-02\"";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 1, testMessage, "was not parsed");
		fail_if(jd[0].GetDate().year != 2011, testMessage, "wrong result");
		fail_if(jd[0].GetDate().month != 3, testMessage, "wrong result");
		fail_if(jd[0].GetDate().day != 2, testMessage, "wrong result");

		testMessage = "Failed at test 1.56: ";
		str = " \" 2011-03-02 \" ";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		fail_if(jd.Count() != 1, testMessage, "was not parsed");
		fail_if(jd[0].GetDate().year != 2011, testMessage, "wrong result");
		fail_if(jd[0].GetDate().month != 3, testMessage, "wrong result");
		fail_if(jd[0].GetDate().day != 2, testMessage, "wrong result");

		testMessage = "Failed at test 1.57: ";
		str = "\"2011-03-32\"";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		passed = false;
		try {
			jd[0].GetDate();
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

		testMessage = "Failed at test 1.58: ";
		str = "\"2011-03-2\"";
		processed = jd.Decode(str, str.size());
		fail_if(processed != str.size(), testMessage, "wrong num processed");
		passed = false;
		try {
			jd[0].GetDate();
		} catch (CException &e) {
			passed = true;
		}
		fail_if(!passed, testMessage, "was parsed");

	} catch (CException &e) {
		fail(testMessage, "unexpected exception: ", e.message);
	}

}

int
main(int, char **)
{
	test1();
	test2();
	std::cout << "OK" << std::endl;
}