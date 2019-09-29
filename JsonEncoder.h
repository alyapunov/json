#pragma once
#include <cstddef>

enum Json_flags_t {
	JSON_COMPACT = 0,
	JSON_SPACE = 1,
	JSON_NEW_LINE = 2,
};

enum Json_encode_type_t {
	JSON_ENCODE_GENERAL,
	JSON_ENCODE_ARRAY,
	JSON_ENCODE_MAP,
};

template <class ... Types>
struct CJsonArray;

template <class ... Types>
inline CJsonArray<Types...> JsonArray(Types ... Args);

template <class ... Types>
struct CJsonMap;

template <class ... Types>
inline CJsonMap<Types...> JsonMap(Types ... Args);

struct CJsonClose {};

inline CJsonClose JsonClose() { return CJsonClose(); }

template <class Strm>
class CJsonEncoder {
public:
	CJsonEncoder(Strm &strm_, Json_flags_t flags_ = JSON_NEW_LINE);
	CJsonEncoder(const CJsonEncoder &) = delete;
	CJsonEncoder(const CJsonEncoder &&a);
	~CJsonEncoder();

	template <class T>
	const CJsonEncoder &operator << (const T &t) const;
	const CJsonEncoder &operator << (Json_flags_t flags_) const;
	template <class ... Types>
	const CJsonEncoder operator << (const CJsonArray<Types...> &a) const;
	template <class ... Types>
	const CJsonEncoder operator << (const CJsonMap<Types...> &a) const;
	const CJsonEncoder &operator << (const CJsonClose &) const;

protected:
	CJsonEncoder(const CJsonEncoder<Strm> *parent_, Json_encode_type_t type_ , Json_flags_t flags_ = JSON_SPACE);
	void Next() const;
	const CJsonEncoder<Strm> &Close() const;

	const CJsonEncoder<Strm> *parent;
	Strm &strm;
	mutable size_t size;
	Json_encode_type_t type;
	mutable Json_flags_t flags;
	mutable bool closed;
};

#include "JsonEncoder.hpp"