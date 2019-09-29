#pragma once
#include "JsonEncoder.h"
#include <assert.h>
#include <type_traits>

/* {{{ JsonPack implementation */
template <class Strm, class T>
inline void
JsonPack(Strm &strm, const T &t);

template <class Strm, class T>
inline void
JsonPack(Strm &strm, const T &t)
{
	if (std::is_arithmetic<T>::value)
		strm << t;
	else
		strm << '\"' << t << '\"';
}

template <class Strm>
inline void
JsonPack(Strm &strm, char t)
{
	strm << '\"' << t << '\"';
}

template <class Strm>
inline void
JsonPack(Strm &strm, unsigned char t)
{
	strm << '\"' << t << '\"';
}

template <class Strm>
inline void
JsonPack(Strm &strm, bool t)
{
	strm << (t ? "true" : "false");
}

template <class Strm>
inline void
JsonPack(Strm &strm, std::nullptr_t)
{
	strm << "null";
}
/* }}} JsonPack implementation */

/* {{{ CJsonArray */
template <class T, class ... Types>
struct CJsonArray<T, Types...> {
	const T t;
	CJsonArray<Types...> more;
	CJsonArray(const T &t_, Types ... Args) : t(t_), more(Args...) {}
	template <class Strm>
	void Put(Strm &strm) const
	{
		strm << t;
		more.Put(strm);
	}
};

template <>
struct CJsonArray<> {
	template <class Strm>
	void Put(Strm &) const {}
};

template <class ... Types>
inline CJsonArray<Types...> JsonArray(Types ... Args)
{
	return CJsonArray<Types...>(Args...);
}
/* }}} CJsonArray */

/* {{{ CJasonMap */
template <class ... Types>
struct CJsonMap : public CJsonArray<Types ...> {
	CJsonMap(Types ... Args) : CJsonArray<Types ...>(Args...) {}
};

template <class ... Types>
inline CJsonMap<Types...> JsonMap(Types ... Args)
{
	return CJsonMap<Types...>(Args...);
}
/* }}} CJsonMap */

/* {{{ CJsonEncoder implementaion */
template<class Strm>
CJsonEncoder<Strm>::CJsonEncoder(Strm &strm_, Json_flags_t flags_) :
	parent(NULL), strm(strm_), size(0), type(JSON_ENCODE_GENERAL), flags(flags_), closed(false) {
}

template<class Strm>
CJsonEncoder<Strm>::CJsonEncoder(const CJsonEncoder &&a) :
	parent(a.parent), strm(a.strm), size(a.size), type(a.type), flags(a.flags), closed(a.closed)
{
	a.closed = true;
}

template<class Strm>
CJsonEncoder<Strm>::~CJsonEncoder()
{
	Close();
}

template <class Strm> template <class T>
inline const CJsonEncoder<Strm>&
CJsonEncoder<Strm>::operator << (const T &t) const
{
	Next();
	JsonPack(strm, t);
	return *this;
}

template <class Strm>
inline const CJsonEncoder<Strm>&
CJsonEncoder<Strm>::operator << (Json_flags_t flags_) const
{
	flags = flags_;
	return *this;
}

template <class Strm> template <class ... Types>
inline const CJsonEncoder<Strm>
CJsonEncoder<Strm>::operator << (const CJsonArray<Types...> &a) const
{
	CJsonEncoder res = CJsonEncoder(this, JSON_ENCODE_ARRAY);
	a.Put(res);
	return res;
}

template <class Strm> template <class ... Types>
inline const CJsonEncoder<Strm>
CJsonEncoder<Strm>::operator << (const CJsonMap<Types...> &a) const
{
	CJsonEncoder res = CJsonEncoder(this, JSON_ENCODE_MAP);
	a.Put(res);
	return res;
}

template <class Strm>
inline const CJsonEncoder<Strm>&
CJsonEncoder<Strm>::operator << (const CJsonClose &) const
{
	return Close();
}

template <class Strm>
CJsonEncoder<Strm>::CJsonEncoder(const CJsonEncoder<Strm> *parent_, Json_encode_type_t type_ , Json_flags_t flags_) :
	parent(parent_), strm(parent->strm), size(0), type(type_), flags(flags_), closed(false)
{
	parent->Next();
	if (type == JSON_ENCODE_ARRAY)
		strm << '[';
	else if(type == JSON_ENCODE_MAP)
		strm << '{';
}

template <class Strm>
inline void
CJsonEncoder<Strm>::Next() const
{
	assert(!closed);
	if (size++ == 0)
		return;
	if (type == JSON_ENCODE_ARRAY) {
		strm << ',';
	} else if (type == JSON_ENCODE_MAP) {
		if (size % 2 == 0)
			strm << ':';
		else
			strm << ',';
	}
	if (flags == JSON_SPACE)
		strm << ' ';
	else if (flags == JSON_NEW_LINE)
		strm << '\n';
}

template <class Strm>
inline const CJsonEncoder<Strm> &
CJsonEncoder<Strm>::Close() const
{
	if (closed)
		return *parent;
	closed = true;
	if (type == JSON_ENCODE_ARRAY) {
		strm << ']';
	} else if (type == JSON_ENCODE_MAP) {
		assert(size % 2 == 0);
		strm << '}';
	}
	else if (flags == JSON_SPACE)
		strm << ' ';
	else if (flags == JSON_NEW_LINE)
		strm << '\n';
	return *parent;
}
/* }}} CJsonEncoder implementaion */
