#pragma once
#include "JsonDecoder.h"
#include <assert.h>
#include <math.h>

template <class T>
CJsonEntry<T>::CJsonEntry(const T &src_, Json_type_t type_, ssize_t parent_) :
	type(type_), src(src_), parent(parent_)
{
}

template <class T>
void
CJsonEntry<T>::ExpectedType(Json_type_t expected, const char *message) const
{
	if (type != expected)
		throw CException(message);
}

template <class T>
size_t
CJsonEntry<T>::GetStringPos() const
{
	ExpectedType(JSON_STRING, "JSON: string expected");
	return data.stringData.pos;
}

template <class T>
size_t
CJsonEntry<T>::GetStringSize() const
{
	ExpectedType(JSON_STRING, "JSON: string expected");
	return data.stringData.size;
}

template <class T>
bool
CJsonEntry<T>::Escaped(char e, char &r)
{
	switch (e) {
	case '\"':
		r = '\"';
		return true;
	case '\\':
		r = '\\';
		return true;
	case '/':
		r = '/';
		return true;
	case 'r':
		r = '\r';
		return true;
	case 'n':
		r = '\n';
		return true;
	case 't':
		r = '\t';
		return true;
	default:
		return false;
	}
}

template <class T>
char *
CJsonEntry<T>::ExtractString(char *to, size_t size, const char *message) const
{
	ExpectedType(JSON_STRING, "JSON: string expected");
	char *save = to;
	size_t fin = data.stringData.pos + data.stringData.size;
	for (size_t i = data.stringData.pos; i < fin; i++) {
		if (size == 0)
			throw CException(message);
		if (src[i] == '\\' && i + 1 < fin && Escaped(src[i + 1], *to))
			i++;
		else
			*to = src[i];
		to++;
		size--;
	}
	if (size == 0)
		throw CException(message);
	*to++ = 0;
	return save;
}

template <class T>
bool
CJsonEntry<T>::StringEqual(const char *to) const
{
	ExpectedType(JSON_STRING, "JSON: string expected");
	size_t fin = data.stringData.pos + data.stringData.size;
	for (size_t i = data.stringData.pos; i < fin; i++) {
		char c;
		if (src[i] == '\\' && i + 1 < fin && Escaped(src[i + 1], c))
			i++;
		else
			c = src[i];
		if (*to++ != c)
			return false;
	}
	return *to == 0;
}

template <class T>
CDate
CJsonEntry<T>::GetDate() const
{
	ExpectedType(JSON_STRING, "JSON: date expected");
	char str[32];
	ExtractString(str, 32, "JSON: date expected");
	try {
		return CDate(str);
	} catch (CException &e) {
		throw CException("JSON: date expected");
	}
}

template <class T>
double
CJsonEntry<T>::GetNumber() const
{
	ExpectedType(JSON_NUMBER, "JSON: number expected");
	return data.numberData.number;
}

template <class T>
int64_t
CJsonEntry<T>::GetInt() const
{
	ExpectedType(JSON_NUMBER, "JSON: int expected");
	int64_t i = (int64_t)data.numberData.number;
	if ((double)i != data.numberData.number)
		throw CException("JSON: int expected");
	return i;
}

template <class T>
uint64_t
CJsonEntry<T>::GetUint() const
{
	ExpectedType(JSON_NUMBER, "JSON: unsigned int expected");
	uint64_t i = (uint64_t)data.numberData.number;
	if ((double)i != data.numberData.number)
		throw CException("JSON: unsigned int expected");
	return i;
}

template <class T>
bool
CJsonEntry<T>::GetBool() const
{
	ExpectedType(JSON_BOOL, "JSON: bool expected");
	return data.boolData.value;
}

template <class T>
size_t
CJsonEntry<T>::GetArraySize() const
{
	ExpectedType(JSON_ARRAY, "JSON: array expected");
	return data.arrayObjectData.size;
}

template <class T>
size_t
CJsonEntry<T>::GetObjectSize() const
{
	ExpectedType(JSON_OBJECT, "JSON: object expected");
	return data.arrayObjectData.size;
}

template <class T>
std::ostream &
CJsonEntry<T>::ToStream(std::ostream &strm) const
{
	std::vector<char> buf;
	switch (type) {
	case JSON_STRING:
		buf.resize(GetStringSize() + 1);
		strm << "STRING \""
		     << ExtractString(&buf[0], buf.size(), "FATAL ERROR")
		     << "\"" << std::endl;
		break;
	case JSON_NUMBER:
		strm << "NUMBER " << GetNumber() << std::endl;
		break;
	case JSON_BOOL:
		strm << "NUMBER " << (GetBool() ? "true" : "false") << std::endl;
		break;
	case JSON_NULL:
		strm << "NULL" << std::endl;
		break;
	case JSON_ARRAY:
		strm << "ARRAY " << GetArraySize() << std::endl;
		break;
	case JSON_OBJECT:
		strm << "OBJECT " << GetObjectSize() << std::endl;
		break;
	default:
		assert(false);
	};
	return strm;
}


template <class T>
inline const CJsonEntry<T> &
CJsonDecoder<T>::operator[] (size_t i) const
{
	assert(i < entries.size());
	return entries[i];
}

template <class T>
inline void
CJsonDecoder<T>::grabString(size_t &i, size_t size, CJsonEntry<T> &entry)
{
	assert((*src)[i] == '"');
	i++;
	entry.data.stringData.pos = i;
	for (; i < size; i++) {
		if ((*src)[i] == '\\')
			i++;
		else if ((*src)[i] == '"')
			break;
	}
	if (i >= size)
		entries.clear();
	else
		entry.data.stringData.size = i - entry.data.stringData.pos;
}

template <class T>
inline void
CJsonDecoder<T>::grabNumber(size_t &i, size_t size, CJsonEntry<T> &entry)
{
	assert(((*src)[i] >= '0' && (*src)[i] <= '9') || (*src)[i] == '-' || (*src)[i] == '+' || (*src)[i] == '.');
	double sign = (*src)[i] == '-' ? -1 : 1;
	bool hasNumbers = false;
	if ((*src)[i] == '-' || (*src)[i] == '+') {
		i++;
	}
	double d = 0;
	while (i < size && (*src)[i] >= '0' && (*src)[i] <= '9') {
		hasNumbers = true;
		d = d * 10 + ((*src)[i] - '0');
		i++;
	}
	if (i < size && (*src)[i] == '.') {
		i++;
		double k = 1.;
		while (i < size && (*src)[i] >= '0' && (*src)[i] <= '9') {
			hasNumbers = true;
			k = k * 0.1;
			d += k * ((*src)[i] - '0');
			i++;
		}
	}
	if (!hasNumbers && i < size)
		throw CException("Broken JSON: Invalid number");
	if (i < size && ((*src)[i] == 'e' || (*src)[i] == 'E')) {
		hasNumbers = false;
		i++;
		double esign = 1;
		if (i < size && ((*src)[i] == '-' || (*src)[i] == '+')) {
			if ((*src)[i] == '-')
				esign = -1;
			i++;
		}
		double e = 0;
		while (i < size && (*src)[i] >= '0' && (*src)[i] <= '9') {
			hasNumbers = true;
			e = e * 10 + ((*src)[i] - '0');
			i++;
		}
		d *= exp10(e * esign);
		if (!hasNumbers && i < size)
			throw CException("Broken JSON: Invalid number");
	}
	if (i >= size) {
		entries.clear();
		return;
	}
	i--;
	entry.data.numberData.number = sign * d;
}

template <class T>
inline void
CJsonDecoder<T>::grabSpecial(size_t &i, size_t size, const char *str)
{
	assert((*src)[i] == *str);
	i++;
	str++;
	while (i < size && *str) {
		if ((*src)[i] != *str)
			throw CException("Broken JSON: Enexpected symbol in the stream");
		i++;
		str++;
	}
	i--;
	if (*str)
		entries.clear();
}

template <class T>
inline size_t
CJsonDecoder<T>::Decode(const T&src_, size_t size, size_t offset)
{
	src = &src_;
	entries.clear();

	size_t processed = 0, i = offset;
	while (i < size && std::isspace((*src)[i])) {
		i++;
		processed++;
	}

	ssize_t curr = -1;
	bool delimited = true;
	for (; i < size; i++) {
		char c = (*src)[i];
		if (std::isspace(c))
			continue;
		if (c == ',') {
			if (delimited)
				throw CException("Broken JSON: Enexpected delimiter in the stream");
			if (curr < 0 || (entries[curr].type != JSON_ARRAY && entries[curr].type != JSON_OBJECT))
				throw CException("Broken JSON: Enexpected delimiter in the stream");
			if (entries[curr].type == JSON_OBJECT && entries[curr].data.arrayObjectData.size % 2 != 0)
				throw CException("Broken JSON: Wrong delimiter in an object");
			delimited = true;
			continue;
		} else if (c == ':') {
			if (delimited)
				throw CException("Broken JSON: Enexpected delimiter in the stream");
			if (curr < 0 || entries[curr].type != JSON_OBJECT)
				throw CException("Broken JSON: Wrong delimiter in the stream");
			if (entries[curr].data.arrayObjectData.size % 2 != 1)
				throw CException("Broken JSON: Wrong delimiter in an onject");
			delimited = true;
			continue;
		}
		if (c == ']' || c == '}') {
			if (delimited)
				throw CException("Broken JSON: Enexpected delimiter in the stream");
		} else {
			bool arrBegin =
				curr >= 0 &&
				(entries[curr].type == JSON_ARRAY || entries[curr].type == JSON_OBJECT) &&
				entries[curr].data.arrayObjectData.size == 0;
			if (arrBegin && delimited)
				throw CException("Broken JSON: Enexpected delimiter in the stream");
			else if (!arrBegin && !delimited)
				throw CException("Broken JSON: Expected delimiter in the stream");
		}
		delimited = false;
		if (c == '"') {
			entries.push_back(CJsonEntry<T>(*src, JSON_STRING, curr));
			grabString(i, size, entries.back());
		} else if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.') {
			entries.push_back(CJsonEntry<T>(*src, JSON_NUMBER, curr));
			grabNumber(i, size, entries.back());
		} else if (c == 't') {
			entries.push_back(CJsonEntry<T>(*src, JSON_BOOL, curr));
			entries.back().data.boolData.value = true;
			grabSpecial(i, size, "true");
		} else if (c == 'f') {
			entries.push_back(CJsonEntry<T>(*src, JSON_BOOL, curr));
			entries.back().data.boolData.value = false;
			grabSpecial(i, size, "false");
		} else if (c == 'n') {
			entries.push_back(CJsonEntry<T>(*src, JSON_NULL, curr));
			grabSpecial(i, size, "null");
		} else if (c == '[') {
			entries.push_back(CJsonEntry<T>(*src, JSON_ARRAY, curr));
			entries.back().data.arrayObjectData.size = 0;
		} else if (c == ']') {
			if (curr < 0 || entries[curr].type != JSON_ARRAY)
				throw CException("Broken JSON: Enexpected ']' in the stream");
		} else if (c == '{') {
			entries.push_back(CJsonEntry<T>(*src, JSON_OBJECT, curr));
			entries.back().data.arrayObjectData.size = 0;
		} else if (c == '}') {
			if (curr < 0 || entries[curr].type != JSON_OBJECT)
				throw CException("Broken JSON: Enexpected '}' in the stream");
			else if (entries[curr].data.arrayObjectData.size % 2 != 0)
				throw CException("Broken JSON: Enexpected end of an object");
		} else {
			throw CException("Broken JSON: Enexpected symbol in the stream");
		}
		if (curr >= 0 && c != ']' && c != '}') {
			assert(entries[curr].type == JSON_ARRAY || entries[curr].type == JSON_OBJECT);
			entries[curr].data.arrayObjectData.size++;
		}
		if (c == '[' || c == '{') {
			curr = entries.size() - 1;
		} else if (c == ']' || c == '}') {
			curr = entries[curr].parent;
		}
		if (curr < 0)
			break;
	}

	if (curr != -1)
		entries.clear();

	if (!entries.empty()) {
		i++;
		processed = i - offset;
		while (i < size && std::isspace((*src)[i])) {
			i++;
			processed++;
		}
	}

	return processed;
}
