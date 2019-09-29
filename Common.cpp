#include "Common.h"
#include <cctype>
#include <string>
#include <string.h>

CDate::CDate() : year(1), month(1), day(1)
{
}

bool CDate::IsSet() const
{
	return year != 1 || month != 1 || day != 1;
}

CDate::CDate(const char *str)
{
	Load(str);
}

const char *
CDate::Str() const
{
	static char buf[16];
	int i = 0;
	buf[i++] = '0' + year / 1000;
	buf[i++] = '0' + year % 1000 / 100;
	buf[i++] = '0' + year % 100 / 10;
	buf[i++] = '0' + year % 10;
	buf[i++] = '-';
	buf[i++] = '0' + month / 10;
	buf[i++] = '0' + month % 10;
	buf[i++] = '-';
	buf[i++] = '0' + day / 10;
	buf[i++] = '0' + day % 10;
	buf[i] = 0;
	return buf;
}

static void
TrimLR(const char **begin, const char **end)
{
	while (*begin < *end && std::isspace((int)(unsigned char)**begin))
		++*begin;
	while (*begin < *end && std::isspace((int)(unsigned char)*((*end) - 1)))
		--*end;
}

template <typename T>
static bool
Enum(const char *str, size_t len, T &num)
{
	num = 0;
	for (size_t i = 0; i < len; i++)
		if (str[i] < '0' || str[i] > '9')
			return false;
		else
			num = num * 10 + T(str[i] - '0');
	return true;
}

void
CDate::Load(const char *str)
{
	const char *begin = str;
	const char *end = str + strlen(str);
	TrimLR(&begin, &end);
	if (end - begin >= 2 && begin[0] == end[-1] && (begin[0] == '"' || begin[0] == '\'')) {
		++begin;
		--end;
		TrimLR(&begin, &end);
	}
	if (end - begin == 10 && begin[4] == '-' && begin[7] == '-') {
		if (!Enum(begin, 4, year) || !Enum(begin + 5, 2, month) || !Enum(begin + 8, 2, day))
			throw CException("Wrong date format");
	} else if (end - begin == 10 && begin[2] == '.' && begin[5] == '.') {
		if (!Enum(begin, 2, day) || !Enum(begin + 3, 2, month) || !Enum(begin + 6, 4, year))
			throw CException("Wrong date format");
	} else {
		throw CException("Wrong date format");
	}
	Check();
}

bool
CDate::operator <(const CDate &a) const
{
	return year < a.year ? true :
	       a.year < year ? false :
	       month < a.month ? true :
	       a.month < month ? false :
	       day < a.day;
}

bool
CDate::operator ==(const CDate &a) const
{
	return year == a.year && month == a.month && day == a.day;
}

void
CDate::Check() const
{
	static const uint8_t calendar[2][12] =
		{{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		 {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
	int v = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0) ? 1 : 0;
	if (day == 0  || month == 0 || year == 0 || month > 12 || year > 3000 || day > calendar[v][month - 1])
		throw CException("Wrong date format");
}

int
CDate::operator-(const CDate &a) const
{
	return ToInt() - a.ToInt();
}

int
CDate::ToInt() const
{
	int res = 365 * (year - 1);
	res += (year - 1) / 4;
	res -= (year - 1) / 100;
	res += (year - 1) / 400;
	static const int offs[2][12] =
		{{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
		 {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};
	int v = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0) ? 1 : 0;
	res += offs[v][month - 1];
	res += day - 1;
	return res;
}

void
CDate::FromInt(int date)
{
	if (date < 0)
		throw CException("Date overflow");
	year = 1;
	int k = 400 * 365 + 97;
	int y = date / k;
	year += 400 * y;
	date -= k * y;
	k = 100 * 365 + 24;
	y = date / k;
	if (y == 4)
		y--;
	year += 100 * y;
	date -= k * y;
	k = 4 * 365 + 1;
	year += 4 * (date / k);
	date = date % k;
	k = 365;
	y = date / k;
	if (y == 4)
		y--;
	year += y;
	date -= k * y;
	static const int offs[2][12] =
		{{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
		 {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};
	int v = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0) ? 1 : 0;
	month = 12;
	for (uint8_t m = 1; m < 12; m++) {
		if (date <= offs[v][m]) {
			month = m;
			break;
		}
	}
	day = date - offs[v][month - 1] + 1;
}

CDate CDate::operator+(int a) const
{
	CDate d;
	d.FromInt(ToInt() + a);
	return d;
}

CDate CDate::operator-(int a) const
{
	CDate d;
	d.FromInt(ToInt() - a);
	return d;
}

void CDate::operator+=(int a)
{
	FromInt(ToInt() + a);
}

void CDate::operator-=(int a)
{
	FromInt(ToInt() - a);
}

std::ostream& operator <<(std::ostream& strm, const CDate &date)
{
	strm << date.Str();
	return strm;
}

std::istream& operator >>(std::istream& strm, CDate &date)
{
	char str[16];
	const size_t lim = sizeof(str) / sizeof(str[0]) - 1;
	size_t len = 0;
	while (len < lim) {
		strm.get(str[len]);
		if (strm.eof())
			break;
		if (std::isspace(str[len])) {
			if (len == 0)
				continue;
			else
				break;
		}
		len++;
	}
	str[len] = 0;
	date.Load(str);
	return strm;
}

const char *
format_time(time_t t)
{
	static __thread char buf[32];
	static __thread time_t last_t = 0;
	if (t != last_t) {
		struct tm *tm = localtime(&t);
		sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
		        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
		        tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	return buf;
}
