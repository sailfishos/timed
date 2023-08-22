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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <iodata-qt6/iodata>
#else
#include <iodata-qt5/iodata>
#endif

#include "../lib/nanotime.h"
#include "../lib/wall-declarations.h"
#include "../lib/wall-info.h"

#include "tz.h"

using namespace std;

class Timed;

struct source_t
{
    virtual ~source_t(){};
    virtual const char *name() const = 0;
    virtual bool available() const = 0;
    virtual void load(const iodata::item *);
    virtual iodata::item *save() const;
};

struct utc_source_t : public source_t
{
    virtual ~utc_source_t() {}
    nanotime_t value;
    virtual nanotime_t value_at_zero() const { return nanotime_t(0, 0); }
};

struct manual_utc_t : public utc_source_t
{
    manual_utc_t() { value = nanotime_t::systime_at_zero(); }
    virtual ~manual_utc_t() {}
    const char *name() const { return "manual_utc"; }
    bool available() const { return true; }
    nanotime_t value_at_zero() const { return value; }
};

struct nitz_utc_t : public utc_source_t
{
    nitz_utc_t();
    virtual ~nitz_utc_t() {}
    const char *name() const { return "nitz_utc"; }
    bool available() const;
    nanotime_t value_at_zero() const;
};

struct ntp_utc_t : public utc_source_t
{
    virtual ~ntp_utc_t() {}
    const char *name() const { return "ntp_utc"; }
    bool available() const { return false; }
};

struct gps_utc_t : public utc_source_t
{
    virtual ~gps_utc_t() {}
    const char *name() const { return "gps_utc"; }
    bool available() const { return false; }
};

struct offset_source_t : public source_t
{
    int value;
    virtual int offset() const { return value; }
    offset_source_t() { value = -1; }
    virtual ~offset_source_t() {}
    bool available() const { return value != -1; }
};

struct manual_offset_t : public offset_source_t
{
    virtual ~manual_offset_t() {}
    const char *name() const { return "manual_offset"; }
    void load(const iodata::item *);
    iodata::item *save() const;
};

struct nitz_offset_t : public offset_source_t
{
    virtual ~nitz_offset_t() {}
    const char *name() const { return "nitz_offset"; }
};

struct zone_source_t : public source_t
{
    string value;
    virtual ~zone_source_t() {}
    virtual string zone() const { return value; }
    bool available() const { return !value.empty(); }
    void load(const iodata::item *);
    iodata::item *save() const;
};

struct manual_zone_t : public zone_source_t
{
    virtual ~manual_zone_t() {}
    const char *name() const { return "manual_zone"; }
};

struct cellular_zone_t : public zone_source_t
{
    virtual ~cellular_zone_t() {}
    suggestion_t suggestions;
    const char *name() const { return "cellular_zone"; }
};

struct key_int_t : public source_t
{
    map<string, int> m;
    typedef map<string, int>::iterator iterator;
    typedef map<string, int>::const_iterator const_iterator;
    virtual ~key_int_t() {}
    int get(const string &key, int default_value = 0) const;
    bool available() const { return !m.empty(); }
    iodata::item *save() const;
    void load(const iodata::item *);
};

struct app_snooze_t : public key_int_t
{
    virtual ~app_snooze_t() {}
    const char *name() const { return "app_snooze"; }
};

struct source_settings : public QObject
{
    static const int min_snooze = 30;

    source_settings(Timed *owner);
    virtual ~source_settings();
    Timed *o;

    zone_source_t *manual_zone;
    cellular_zone_t *cellular_zone;
    offset_source_t *manual_offset, *nitz_offset;
    utc_source_t *manual_utc, *nitz_utc, *gps_utc, *ntp_utc;
    key_int_t *app_snooze;

    bool time_nitz, local_cellular, auto_dst;
    bool format_24;
    bool alarms_are_enabled;
    int default_snooze_value;

    int default_snooze() const;
    int default_snooze(int new_value);

    int get_app_snooze(const string &app);
    int get_app_snooze(const string &app, int default_value);
    int set_app_snooze(const string &app, int value);
    void remove_app_snooze(const string &app);

    int offset() const;
    string zone() const;
    string human_readable_timezone() const;
    string etc_localtime() const;
    nanotime_t value_at_zero() const;

    void set_system_time(const nanotime_t &);

    map<string, source_t *> src;

    void load(const iodata::record *, const string &defult_tz);
    iodata::record *save() const;

    Maemo::Timed::WallClock::wall_info_pimple_t *get_wall_clock_info(const nanotime_t &) const;
    bool wall_clock_settings(const Maemo::Timed::WallClock::wall_settings_pimple_t &p);

    static string symlink_target(int offset_sec);
    static bool east_of_greenwich(string &result, int offset_sec, bool reverse = false);
    static string symlink_target(string zone);
    static int check_target(string path);

    void fix_etc_localtime();
    void postload_fix_manual_zone();
    void postload_fix_manual_offset();
    void process_kernel_notification(const nanotime_t &jump_forwards);

    Q_OBJECT;
public Q_SLOTS:
    void cellular_time_slot(const cellular_time_t &T);
    void cellular_zone_slot(olson *tz, suggestion_t s, bool sure);
};

#endif
