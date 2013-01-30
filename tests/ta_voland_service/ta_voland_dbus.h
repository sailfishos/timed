/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef TA_VOLAND_DBUS_H_
# define TA_VOLAND_DBUS_H_

/** D-Bus name of ta_voland service */
# define TA_VOLAND_DBUS_SERVICE   "com.nokia.ta_voland"

/** Name of ta_voland interface on D-Bus */
# define TA_VOLAND_DBUS_INTERFACE "com.nokia.ta_voland"

/** Default ta_voland object path on D-Bus */
# define TA_VOLAND_DBUS_PATH      "/com/nokia/ta_voland"

/** Get pid
 *
 * Returns a PID of the reminder service
 *
 * @return INT32 pid
 */
# define TA_VOLAND_DBUS_METHOD_PID  "pid"

/** Emulate user interaction
 *
 * Emulates a human user selecting the given button in the reminder
 * dialog for the timed event with given cookie. Returns true if the
 * parameters are valid and the action is performed, false otherwise.
 *
 * @param cookie UINT32 unique alarm identifier
 * @param button INT32 button index
 *
 * @return BOOLEAN ok
 */
# define TA_VOLAND_DBUS_METHOD_ANSWER "answer"

/** Get cookie of currently show alarm dialog
 *
 * Returns the timed cookie of the topmost opened reminder dialog, or
 * invalid cookie (zero) if there is no opened dialog.
 *
 * @return UINT32 cookie; unique alarm identifier, or 0
 */
# define TA_VOLAND_DBUS_METHOD_TOP "top"

/** Terminate notification service
 *
 * Ask the notification service to terminate
 */
# define TA_VOLAND_DBUS_METHOD_QUIT "quit"

#endif /* TA_VOLAND_DBUS_H_ */
