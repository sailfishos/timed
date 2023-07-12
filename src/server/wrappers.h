/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2011 Nokia Corporation.                           **
**   Copyright (C) 2019 Jolla Ltd.                                        **
**   Copyright (c) 2019 Open Mobile Platform LLC.                         **
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
#ifndef WRAPPERS_H
#define WRAPPERS_H

struct ticker_t
{
private:
    time_t t;

public:
    ticker_t()
        : t(0)
    {}
    explicit ticker_t(time_t v)
        : t(v)
    {}
    time_t value() const { return t; }
    bool is_valid() const { return t > 0; }
    bool operator<(const ticker_t &y) const { return value() < y.value(); }
    bool operator<=(const ticker_t &y) const { return value() <= y.value(); }
    int operator-(const ticker_t &y) const { return value() - y.value(); }
    ticker_t operator+(int diff) const { return ticker_t(value() + diff); }
    void operator-=(time_t diff) { t -= diff; }
};

struct cookie_t
{
    enum {
        /* NB: While "cookie_t" is using unsigned integer value, some parts
     *     of the sw stack treat it as a signed integer -> to avoid
     *     confusion the hight bit is kept cleared so that cookie values
     *     stay positive also under such circumstances.
     */
        SharedBit = 1 << 30,
        ValueMask = SharedBit - 1,
    };

private:
    unsigned val;

public:
    cookie_t()
        : val(0)
    {}
    explicit cookie_t(unsigned v)
        : val(v)
    {}
    unsigned value() const { return val; }
    bool is_valid() const { return val > 0; }
    const cookie_t &operator++()
    {
        val = (val & SharedBit) | ((val + 1) & ValueMask);
        return *this;
    }
    bool operator<(const cookie_t &y) const { return val < y.val; }
    void setShared(bool shared)
    {
        val &= ValueMask;
        if (shared)
            val |= SharedBit;
    }
    bool shared() const { return val & SharedBit; }
};

#endif
