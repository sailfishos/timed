/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2011 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
**   Author: Simo Piiroinen <simo.piiroinen@nokia.com>                    **
**   Author: Victor Portnov <ext-victor.portnov@nokia.com>                **
**                                                                        **
**     This file is part of Timed                                         **
**                                                                        **
**     Timed is free software; you can redistribute it and/or modify      **
**     it under the terms of the GNU Lesser General Public License        **
**     version 2.1 as published by the Free Software Foundation.          **
**                                                                        **
**     Timed is distributed in the hope that it will be useful, but       **
**     WITHOUT ANY WARRANTY;  without even the implied warranty  of       **
**     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               **
**     See the GNU Lesser General Public License  for more details.       **
**                                                                        **
**   You should have received a copy of the GNU  Lesser General Public    **
**   License along with Timed. If not, see http://www.gnu.org/licenses/   **
**                                                                        **
***************************************************************************/
#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <string>
#include <QObject>
using namespace std;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <iodata-qt6/iodata>
#else
#include <iodata-qt5/iodata>
#endif

struct recurrence_pattern_t;

#include "wrappers.h"

struct switch_timezone
{
    char *old_tz;
    switch_timezone(const char *);
    switch_timezone(const string &);
    ~switch_timezone();
    void constructor(const char *);
};

ticker_t now();
ticker_t ticker_align(ticker_t start, int step, ticker_t target);
ticker_t mktime_oversea(struct tm *t, const string &s);
ticker_t mktime_local(struct tm *t);
// int seconds_west_of_greenwich(const string &s, ticker_t x=ticker_t()) ;
bool zone_info(int *seconds_east_of_gmt,
               const string &zone,
               string *abbreviation = NULL,
               ticker_t x = ticker_t());
string tm_str(const struct tm *tm);

struct broken_down_t
{
    static const unsigned int YEAR0 = 1970, YEARX = 2038;
    uint32_t year, month, day, hour, minute;
    broken_down_t() { year = month = day = hour = minute = 0; }
    broken_down_t(uint32_t y, uint32_t m, uint32_t d, uint32_t h, uint32_t mi)
        : year(y)
        , month(m)
        , day(d)
        , hour(h)
        , minute(mi)
    {}

    bool is_valid() const;
    struct tm *to_struct_tm(struct tm *dest = NULL) const;
    void from_struct_tm(const struct tm *);
    void from_time_t(const time_t &time, int *wday = NULL);
    bool same_struct_tm(const struct tm *) const;
    void increment_day();
    bool find_a_good_day(const recurrence_pattern_t *p, int &wday, bool &today, unsigned max_year);
    bool find_a_good_minute(const recurrence_pattern_t *p);
    bool find_a_good_minute_with_increment(const recurrence_pattern_t *p, bool increment_flag);
    void increment_min(unsigned int amount);
    bool is_a_regular_day() const;
    time_t mktime_strict(int dst = -1) const;

    static unsigned int month_length(unsigned year, unsigned month);
    string str() const;
    iodata::record *save();
    void load(const iodata::record *r);
};

#include <QDebug>
inline unsigned int broken_down_t::month_length(unsigned y, unsigned m)
{
    static unsigned int month_length_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    // log_debug("y=%d m=%d month_length_table[m-1]=%d+%d", y, m, month_length_table[m-1], (m==2 && (y&0x03)==0)) ;
    return month_length_table[m - 1]
           + (m == 2 and (y & 0x03) == 0 and (y % 100 != 0 or y % 400 == 0));
}

inline bool broken_down_t::is_valid() const
{
    return YEAR0 <= year && year < YEARX && 1 <= month && month <= 12 && 1 <= day
           && day <= month_length(year, month) &&
           /* 0 <= hour && */ hour < 24 &&
           /* 0 <= minute && */ minute < 60;
}

std::string dst_signature(time_t t);

#endif
