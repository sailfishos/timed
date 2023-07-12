/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef XTIMED_DBUS_H_
#define XTIMED_DBUS_H_

/** D-Bus name of timed service */
#define TIMED_DBUS_SERVICE "com.nokia.time"

/** Name of timed interface on D-Bus */
#define TIMED_DBUS_INTERFACE "com.nokia.time"

/** Default timed object path on D-Bus */
#define TIMED_DBUS_PATH "/com/nokia/time"

/** Report user responce to alarm back to timed
 *
 * @param cookie UINT32 unique alarm identifier
 * @param button INT32 user responce code
 *
 * @return BOOLEAN ok
 */
#define TIMED_DBUS_METHOD_DIALOG_RESPONSE "dialog_response"

#endif /* XTIMED_DBUS_H_ */
