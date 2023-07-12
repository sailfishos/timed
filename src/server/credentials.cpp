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

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <QDBusReply>

#include "../common/log.h"

#include "../lib/interface.h"
#include <grp.h>
#include <pwd.h>

#include "credentials.h"

#define CSTR(s) (s).toLocal8Bit().constData()
#define UTF8(s) (s).toUtf8().constData()

// returning:
//   ~0, if error
//   uid of the caller, if simple uid creds

/* FIXME: this makes a synchronous roundtrip to dbus daemon
   * and back during which time the timed process will be blocked.
   *
   * Note: We can't really handle this asynchronously without
   * handling the whole add_event asynchronously and this would
   * require modifications to the timed event state machine and
   * delaying sending add_event replies from QDBusAbstractAdaptor.
   * At the moment I do not know how to handle either of those ...
   * ;-(
   */
uint32_t get_name_owner_from_dbus_sync(const QDBusConnection &bus, const QString &name)
{
    // TODO make 'dbus_daemon' interface via QDBusAbstractInterface
    QString service = "org.freedesktop.DBus";
    QString path = "/org/freedesktop/DBus";
    QString interface = "org.freedesktop.DBus";
    QString method = "GetConnectionUnixUser";
    // It seems, we can't get GID just by asking dbus daemon.

    QDBusMessage req = QDBusMessage::createMethodCall(service, path, interface, method);
    req << name;

    QDBusReply<uint> reply = bus.call(req);

    if (reply.isValid())
        return reply.value();
    else {
        log_error("%s: did not get a valid reply", CSTR(method));
        return ~0;
    }
}

uid_t nameToUid(string name)
{
    passwd *info = getpwnam(name.c_str());
    if (info)
        return info->pw_uid;
    // couldn't get uid for username
    return -1;
}

string uidToName(uid_t u)
{
    passwd *info = getpwuid(u);
    if (info)
        return info->pw_name;
    // couldn't get name for uid
    return "nobody";
}

gid_t nameToGid(string name)
{
    group *info = getgrnam(name.c_str());
    if (info)
        return info->gr_gid;
    // couldn't get gid for groupname
    return -1;
}

string gidToName(gid_t g)
{
    group *info = getgrgid(g);
    if (info)
        return info->gr_name;
    // couldn't get name for gid
    return "nobody";
}

bool credentials_t::apply() const
{
    if (setgid(nameToGid(gid)) != 0 || setuid(nameToUid(uid)) != 0) {
        log_error("uid cred_set() failed");
        return false;
    }
    return true;
}

bool credentials_t::apply_and_compare()
{
    if (!apply()) // can't apply: nothing to check
        return false;

    credentials_t current = credentials_t::from_current_process();

    ostringstream os;

    bool id_matches = true;

#define COMMA (os << (id_matches ? "" : ", "))

    if (current.uid != uid)
        COMMA << "current uid='" << current.uid << "' (requested uid='" << uid << "')",
            id_matches = false;

    if (current.gid != gid)
        COMMA << "current gid='" << current.gid << "' (requested gid='" << gid << "')",
            id_matches = false;

    bool equal = id_matches;
    bool ret = id_matches;

#undef COMMA

    if (!equal)
        log_warning("applied and wanted credentials differ: %s", os.str().c_str());

    return ret;
}

credentials_t credentials_t::from_current_process()
{
    return credentials_t(uidToName(getuid()), gidToName(getgid()));
}

credentials_t::credentials_t(const QDBusMessage &message)
    : uid("nobody")
    , gid("nobody")
{
    QString sender = message.service();
    uint32_t user_id = get_name_owner_from_dbus_sync(Maemo::Timed::bus(), sender);

    if (user_id == ~0u)
        log_warning("can't get user (uid) of the caller, already terminated?");
    else {
        passwd *info = getpwuid(user_id);
        if (info) {
            uid = info->pw_name;
            gid = gidToName(info->pw_gid);
        }
    }
}

iodata::record *credentials_t::save() const
{
    iodata::record *r = new iodata::record;
    r->add("uid", uid);
    r->add("gid", gid);
    return r;
}

credentials_t::credentials_t(const iodata::record *r)
{
    uid = r->get("uid")->str();
    gid = r->get("gid")->str();
}

string credentials_t::str() const
{
    ostringstream os;
    os << "{uid='" << uid << "', gid='" << gid << "'";
    os << "}";
    return os.str();
}
