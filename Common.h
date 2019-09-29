#pragma once
#include <iostream>
#include <sstream>
#include <cstring>

struct CException {
	const char *message;
	char extra_info[50];
	CException(const char *m) : message(m) { extra_info[0] = 0; }
	CException(const char *m, const char *extra) : message(m)
	{
		size_t len = std::min(strlen(extra), sizeof(extra_info) - 1);
		memcpy(extra_info, extra, len);
		extra_info[len] = 0;
	}
	template <class T>
	void out(T& strm) {
	}
	template <class T, class U, typename ...ARGS>
	void out(T& strm, const U& val, const ARGS&... more) {
		strm << ", " << val;
		out(strm, more...);
	}
	template <class U, typename ...ARGS> void SetExtra(const char *header, const U& val, const ARGS&... args)
	{
		std::stringstream ss;
		ss << header << ": " << val;
		out(ss, args...);
	}
	CException() : message("") { extra_info[0] = 0; }
};

class CDate {
public:
	CDate();
	CDate(const char *str);

	const char *Str() const;
	void Load(const char *str);

	bool operator <(const CDate &a) const;
	bool operator ==(const CDate &a) const;
	int operator-(const CDate &a) const;
	CDate operator+(int a) const;
	CDate operator-(int a) const;
	void operator+=(int a);
	void operator-=(int a);
	int ToInt() const;
	void FromInt(int date);
	void Check() const;
	bool IsSet() const;

	friend std::ostream& operator <<(std::ostream& strm, const CDate &date);
	friend std::istream& operator >>(std::istream& strm, CDate &date);

	uint16_t year;
	uint8_t month;
	uint8_t day;
};

template <class T>
class CPool {
public:
	static CPool &Instance();
	template <class ... TYPES>
	inline T *New(TYPES ... args);

	inline void Delete(T *t);

private:
	CPool();
	~CPool();

private:
	union Entry {
		T t;
		Entry *next;
	};

	Entry *list;
};

template <class T>
CPool<T>::CPool() : list(0)
{
}

template <class T>
CPool<T>::~CPool()
{
	while (list) {
		Entry *e = list;
		list = list->next;
		free(e);
	}
}

template <class T>
CPool<T> &CPool<T>::Instance()
{
	static CPool<T> instance;
	return instance;
}

template <class T>
template <class ... TYPES>
T *CPool<T>::New(TYPES ... args)
{
	void *p;
	if (list) {
		p = (void *)list;
		list = list->next;
	} else {
		p = malloc(sizeof(Entry));
	}
	return new(p) T(args...);
}

template <class T>
void CPool<T>::Delete(T *t)
{
	t->~T();
	Entry *e = (Entry *)t;
	e->next = list;
	list = e;
}

const char *
format_time(time_t t);
