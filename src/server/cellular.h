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
#ifndef MAEMO_TIMED_CELLULAR_H
#define MAEMO_TIMED_CELLULAR_H

#include <QString>

#if OFONO
#include "networkoperator.h"
#include "networktime.h"
#endif

#include "../lib/nanotime.h"

struct cellular_operator_t
{
    std::string mcc, mnc;
    std::string alpha2;
    // int mcc_value ; // =0, if mcc="", -1, else if mcc is not a number
    cellular_operator_t();
    cellular_operator_t(const std::string &mcc_s, const std::string &mnc_s);
#if OFONO
    cellular_operator_t(const QString &mcc_s, const QString &mnc_s);
    cellular_operator_t(const NetworkTimeInfo &cnti);
#endif
    bool operator==(const cellular_operator_t &x) const; // same mcc & mnc
    bool operator!=(const cellular_operator_t &x) const; // mcc or mnc differ
    std::string id() const;                              // like "310/07"
    std::string location() const;                        // "FI" or "001/10" for invalid MCC
    bool known_mcc() const;                              // location()==iso_3166_location()
    bool empty() const;
    std::string str() const;

private:
    void init();
};

struct cellular_time_t
{
    time_t value;
    nanotime_t ts;
    cellular_time_t();
#if OFONO
    cellular_time_t(const NetworkTimeInfo &cnti);
#endif
    bool is_valid() const { return (bool) value; }
    std::string str() const;
};

struct cellular_offset_t
{
    cellular_operator_t oper;
    QString modem;
    int offset;
    int dst;
    time_t timestamp;
    bool sender_time; // is timestamp received in the same NITZ package as UTC time
    cellular_offset_t();
#if OFONO
    cellular_offset_t(const NetworkTimeInfo &cnti);
#endif
    std::string str() const;
    bool is_valid() const { return (bool) timestamp; }
};

#endif //MAEMO_TIMED_CELLULAR_H
