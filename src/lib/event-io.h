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
#ifndef EVENT_IO_H
#define EVENT_IO_H

#include <stdint.h>

#include <QMap>
#include <QString>
#include <QVector>

#include "qmacro.h"

namespace Maemo {
namespace Timed {
struct attribute_io_t;
struct cred_modifier_io_t;
struct action_io_t;
struct button_io_t;
struct recurrence_io_t;
struct event_io_t;
struct event_list_io_t;
} // namespace Timed
} // namespace Maemo

struct Maemo::Timed::attribute_io_t
{
    QMap<QString, QString> txt;
};

struct Maemo::Timed::cred_modifier_io_t
{
    QString token;
    bool accrue;
    cred_modifier_io_t()
        : accrue(false)
    {}
    bool operator==(const cred_modifier_io_t &crio) const;
};

struct Maemo::Timed::action_io_t
{
    attribute_io_t attr;
    uint32_t flags;
    QVector<cred_modifier_io_t> cred_modifiers;
    action_io_t() { flags = 0; }
};

struct Maemo::Timed::button_io_t
{
    attribute_io_t attr;
    uint32_t snooze;
    button_io_t() { snooze = 0; }
};

struct Maemo::Timed::recurrence_io_t
{
    qulonglong mins;
    uint32_t hour;
    uint32_t mday;
    uint32_t wday;
    uint32_t mons;
    uint32_t flags;
    recurrence_io_t()
    {
        mins = hour = mday = wday = mons = 0;
        flags = 0;
    }
};

struct Maemo::Timed::event_io_t
{
    // 1. alarm trigger time
    int32_t ticker;
    uint32_t t_year, t_month, t_day, t_hour, t_minute;
    QString t_zone;
    // 2. Attributes and flags
    attribute_io_t attr;
    uint32_t flags;
    // 3. Dialog buttons
    QVector<button_io_t> buttons;
    // 4. Actions
    QVector<action_io_t> actions;
    // 5. Recurrence info
    QVector<recurrence_io_t> recrs;
    // 6. Additional parameters
    int32_t tsz_max, tsz_length;
    QVector<cred_modifier_io_t> cred_modifiers;
    event_io_t()
    {
        ticker = t_year = t_month = t_day = t_hour = t_minute = flags = tsz_max = 0;
        tsz_length = +1;
    }
};

struct Maemo::Timed::event_list_io_t
{
    QVector<event_io_t> ee;
};
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::attribute_io_t &x);
const QDBusArgument &operator>>(const QDBusArgument &in,Maemo::Timed::attribute_io_t &x);

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::cred_modifier_io_t &x);
const QDBusArgument &operator>>(const QDBusArgument &in,Maemo::Timed::cred_modifier_io_t &x);

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::action_io_t &x);
const QDBusArgument &operator>>(const QDBusArgument &in,Maemo::Timed::action_io_t &x);

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::button_io_t &x);
const QDBusArgument &operator>>(const QDBusArgument &in,Maemo::Timed::button_io_t &x);

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::recurrence_io_t &x);
const QDBusArgument &operator>>(const QDBusArgument &in,Maemo::Timed::recurrence_io_t &x);

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::event_io_t &x);
const QDBusArgument &operator>>(const QDBusArgument &in,Maemo::Timed::event_io_t &x);

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::event_list_io_t &x);
const QDBusArgument &operator>>(const QDBusArgument &in,Maemo::Timed::event_list_io_t &x);
#else
declare_qtdbus_io(Maemo::Timed::attribute_io_t);
declare_qtdbus_io(Maemo::Timed::cred_modifier_io_t);
declare_qtdbus_io(Maemo::Timed::action_io_t);
declare_qtdbus_io(Maemo::Timed::button_io_t);
declare_qtdbus_io(Maemo::Timed::recurrence_io_t);
declare_qtdbus_io(Maemo::Timed::event_io_t);
declare_qtdbus_io(Maemo::Timed::event_list_io_t);
#endif
#endif
