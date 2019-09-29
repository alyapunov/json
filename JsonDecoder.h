#pragma once

#include "Common.h"
#include <vector>
#include <ostream>

enum Json_type_t {
	JSON_STRING,
	JSON_NUMBER,
	JSON_BOOL,
	JSON_NULL,
	JSON_ARRAY,
	JSON_OBJECT,
};

template <class T>
class CJsonEntry;

template <class T>
class CJsonDecoder;

template <class T>
std::ostream &operator<<(std::ostream &strm, const CJsonEntry<T> &entry);

template <class T>
class CJsonEntry {
public:
	Json_type_t type;
	union {
		struct CStringData {
			size_t pos;
			size_t size;
		} stringData;
		struct CNumberData {
			double number;
		} numberData;
		struct CBoolData {
			bool value;
		} boolData;
		struct CArrayObject {
			size_t size;
		} arrayObjectData;
	} data;
	void ExpectedType(Json_type_t expected, const char *message) const;
	size_t GetStringPos() const;
	size_t GetStringSize() const;
	char *ExtractString(char *to, size_t size, const char *message) const;
	bool StringEqual(const char *to) const;
	CDate GetDate() const;
	double GetNumber() const;
	int64_t GetInt() const;
	uint64_t GetUint() const;
	bool GetBool() const;
	size_t GetArraySize() const;
	size_t GetObjectSize() const;
	std::ostream &ToStream(std::ostream &strm) const;
	friend std::ostream &operator<<(std::ostream &strm, const CJsonEntry<T> &entry)
	{
		return entry.ToStream(strm);
	}
	friend class CJsonDecoder<T>;
private:
	CJsonEntry(const T &src_, Json_type_t type_, ssize_t parent_);
	static bool Escaped(char e, char &r);
	const T &src;
	ssize_t parent;
};


template <class T>
class CJsonDecoder {
public:
	static CJsonDecoder &Instance() { static __thread CJsonDecoder coder; return coder; }
	size_t Decode(const T& src, size_t size, size_t offset = 0);
	friend class CJsonEntry<T>;
	const CJsonEntry<T> &operator[] (size_t i) const;
	size_t Count() const { return entries.size(); }
private:
	const T *src;
	std::vector< CJsonEntry<T> > entries;
	CJsonDecoder() {}
	CJsonDecoder(const CJsonDecoder &) = delete;
	CJsonDecoder(const CJsonDecoder &&) = delete;
	void grabString(size_t &i, size_t size, CJsonEntry<T> &entry);
	void grabNumber(size_t &i, size_t size, CJsonEntry<T> &entry);
	void grabSpecial(size_t &i, size_t size, const char *str);
};

#include "JsonDecoder.hpp"