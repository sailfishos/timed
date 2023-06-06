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
#include "tz.h"
#include "../common/log.h"
#include "misc.h"
#include "olson.h"
#include "tzdata.h"

tz_oracle_t::tz_oracle_t()
{
    timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(waiting_for_nitz_timeout()));
}

tz_oracle_t::~tz_oracle_t()
{
    delete timer;
}

void tz_oracle_t::output(olson *zone)
{
    output(zone, new suggestion_t, true);
}

void tz_oracle_t::output(olson *zone, suggestion_t *s, bool sure)
{
    log_notice("Time zone magically detected: '%s' (%s"
               "sure)",
               zone->name().c_str(),
               sure ? "" : "not ");
    emit cellular_zone_detected(zone, *s, sure);

    delete s;
}

void tz_oracle_t::set_by_offset(const cellular_offset_t &data)
{
    if (!operators.contains(data.modem)) {
        log_notice("NITZ offset received for unknown modem: %s: %s",
                   data.modem.toStdString().c_str(),
                   data.str().c_str());
        return;
    }

    const cellular_operator_t &oper(operators[data.modem].oper);
    if (oper.known_mcc()) {
        set<olson *> m, r, result; // main, real
        tzdata::by_country(oper.location(), tzdata::Main_Zones, m);
        tzdata::by_country(oper.location(), tzdata::Real_Zones, r);
        set<olson *> rm = r;               // real minus main
        set_change<olson *>(rm, m, false); // false: rm-=m
        bool found = false;
        if (data.dst <= 0) // sometimes '-1' means "winter time", i.e. set dst to zero
        {
            found = found or tzdata::filter(m, data.timestamp, data.offset, 0, result);
            log_debug("after try 1: found=%d", found);
            found = found or tzdata::filter(rm, data.timestamp, data.offset, 0, result);
            log_debug("after try 2: found=%d", found);
        }
        if (data.dst != 0) // either "summer" (>0) or really not specified (<0)
        {
            found = found or tzdata::filter(m, data.timestamp, data.offset, data.dst, result);
            log_debug("after try 3: found=%d", found);
            found = found or tzdata::filter(rm, data.timestamp, data.offset, data.dst, result);
            log_debug("after try 4: found=%d", found);
        }
        if (not data.sender_time) // sender time not really known, try plus/minus day
        {
            const int day = 60 * 60 * 24;
            found = found or tzdata::filter(r, data.timestamp - day, data.offset, data.dst, result);
            log_debug("after try 5: found=%d", found);
            found = found or tzdata::filter(r, data.timestamp + day, data.offset, data.dst, result);
            log_debug("after try 6: found=%d", found);
        }
        if (not found and data.dst != -1) {
            log_debug("no matching zones found, trying to ignore DST=%d flag", data.dst);
            found = found or tzdata::filter(m, data.timestamp, data.offset, -1, result);
            log_debug("after try 7: found=%d", found);
            found = found or tzdata::filter(rm, data.timestamp, data.offset, -1, result);
            log_debug("after try 8: found=%d", found);
        }
        log_notice("%d candidates selected: %s", result.size(), tzdata::set_str(result).c_str());
        olson *zone = NULL;
        if (result.size() == 0) {
            zone = olson::by_offset(data.offset);
            if (zone == NULL) {
                log_error("failed to set time zone by offset=%d", data.offset);
                return;
            }
        } else {
            zone = *result.begin();
            for (set<olson *>::const_iterator it = result.begin(); it != result.end(); ++it)
                if (*it == operators[data.modem].stat.last_zone) {
                    zone = *it;
                    break;
                }
        }
        suggestion_t *s = new suggestion_t;
        for (set<olson *>::const_iterator it = rm.begin(); it != rm.end(); ++it)
            s->add(*it, 20);
        for (set<olson *>::const_iterator it = m.begin(); it != m.end(); ++it)
            s->add(*it, 10);
        s->add(tzdata::device_default(), 10);
        for (set<olson *>::const_iterator it = result.begin(); it != result.end(); ++it)
            s->add(*it, 5);
        output(zone, s, result.size() == 1);
    } else // some weird operator, or even empty
    {
        log_notice("setting timezone %s for a strange operator %s",
                   data.str().c_str(),
                   oper.str().c_str());
        olson *zone = olson::by_offset(data.offset);
        if (zone == NULL) {
            log_error("failed to set time zone by offset=%d", data.offset);
            return;
        }
        output(zone);
        // TODO: find by offset worldwide ???
    }
}

void tz_oracle_t::set_by_operator(const QString &modem)
{
    // bool known = oper.known_mcc() ;

    if (!operators.contains(modem)) {
        log_error("cannot set timezone according to operator settings for modem: unknown modem: %s",
                  modem.toStdString().c_str());
        return;
    }

    if (operators[modem].oper.known_mcc()) {
        set<olson *> all_zones, main_zones, real_zones;
        tzdata::by_country(operators[modem].oper.location(), tzdata::All_Zones, all_zones);
        tzdata::by_country(operators[modem].oper.location(), tzdata::Main_Zones, main_zones);
        tzdata::by_country(operators[modem].oper.location(), tzdata::Real_Zones, real_zones);
        olson *guess = NULL;
        if (operators[modem].stat.last_zone
            and all_zones.count(
                operators[modem].stat.last_zone)) // zone contained in this country: take it
            guess = operators[modem].stat.last_zone;
        else {
            operators[modem].stat.last_zone = NULL;
            guess = tzdata::device_default();
            if (not all_zones.count(guess)) // we're not in home country
                if (olson *first = tzdata::country_default(
                        operators[modem].oper.location())) // let's be paranoid
                    guess = first;
        }
        log_assert(guess, "oops, guessed NULL pointer");
        log_info("zone '%s' magically guessed by operator identity", guess->name().c_str());
        bool sure = real_zones.size() == 1; // it's saingle zone country
        suggestion_t *s = new suggestion_t;
        for (set<olson *>::const_iterator it = real_zones.begin(); it != real_zones.end(); ++it)
            s->add(*it, 20);
        for (set<olson *>::const_iterator it = main_zones.begin(); it != main_zones.end(); ++it)
            s->add(*it, 10);
        s->add(tzdata::device_default(), 10);
        output(guess, s, sure);
    } else // some weird operator located nowhere
    {
        if (operators[modem].stat.last_zone == NULL) {
            log_notice("can't guess zone for operator %s", operators[modem].oper.str().c_str());
            return;
        } else {
            log_notice("guessing last used zone '%s' for operator %s",
                       operators[modem].stat.last_zone->name().c_str(),
                       operators[modem].oper.str().c_str());
            output(operators[modem].stat.last_zone);
        }
    }
}

void tz_oracle_t::cellular_offset(const cellular_offset_t &data)
{
    timer->stop(); // preventing setting tz by operator only
    if (data.oper.empty()) {
        log_error("operatorless NITZ received: %s", data.str().c_str());
    } else {
        if (!operators.contains(data.modem)) {
            log_notice("received NITZ from operator for unknown modem: %s: %s",
                       data.modem.toStdString().c_str(),
                       data.str().c_str());
            operators.insert(data.modem,
                             operator_status_t(data.oper)); // add this new operator for the modem.
        } else if (operators[data.modem].oper != data.oper) {
            log_error("unexpected operator change in NITZ package %s (current operator %s)",
                      data.str().c_str(),
                      operators[data.modem].oper.str().c_str());
            operators[data.modem].oper = data.oper;
        }
    }
    set_by_offset(data);
}

void tz_oracle_t::cellular_operator(const cellular_operator_t &o, const QString &modem)
{
    timer->stop();

    bool empty = o.empty();
    bool same = operators.contains(modem) and o == operators[modem].oper;
    bool same_country = operators.contains(modem) and o.known_mcc()
                        and o.location() == operators[modem].oper.location();

    log_debug("o=%s, empty=%d, same=%d, same_country=%d", o.str().c_str(), empty, same, same_country);
    if (empty) // disconnected: do nothing
    {
        log_debug("empty operator");
        return;
    }

    if (same) // same as current or same as last: do nothing
        return;

    if (operators.contains(modem)) {
        // operator is changing
        operators[modem].oper = o;
    } else {
        // new modem operator info.
        log_debug("new operator for modem: %s", modem.toStdString().c_str());
        operators.insert(modem, operator_status_t(o));
    }

    log_debug();

    if (same_country) // nothing to do?  not sure about this, what if small_country proves false?  could have a different tz required...
        return;

    bool small_country = operators[modem].oper.known_mcc()
                         and tzdata::is_single_zone_country(operators[modem].oper.location());

    if (small_country)
        set_by_operator(modem);
    else {
        timer->setProperty("modem", modem);
        timer->start(nitz_wait_ms);
    }
}

void tz_oracle_t::waiting_for_nitz_timeout()
{
    timer->stop();
    QString modem = timer->property("modem").toString();
    set_by_operator(modem);
}

void suggestion_t::add(olson *zone, int score)
{
    s[zone] = score;
}
