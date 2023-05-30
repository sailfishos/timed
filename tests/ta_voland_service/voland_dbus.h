/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef VOLAND_DBUS_H_
#define VOLAND_DBUS_H_

/** D-Bus name of voland service */
#define VOLAND_DBUS_SERVICE "com.nokia.voland"

/** Name of voland interface on D-Bus */
#define VOLAND_DBUS_INTERFACE "com.nokia.voland"

/** Default voland object path on D-Bus */
#define VOLAND_DBUS_PATH "/com/nokia/voland"

/** Open dialogs request
 *
 * Array of events to be added on top of the current dialog stack.
 *
 * Each event has - in addition to cookie and some other integer
 * parameter - array of (key,val) attribute pairs plus an array
 * of button data that is also array of (key,val) attribute pairs.
 * (sample data below)
 *
 *
 * @param events ARRAY of VARIANT of STRUCT of (UINT32 cookie,
 *               INT32 tick, ARRAY of DICT of (STRING key, STRING val),
 *               ARRAY of ARRAY of DICT of (STRING key, STRING val))
 *
 * @return BOOLEAN ok
 *
 * t:method_call, i:com.nokia.voland, m:open o:/com/nokia/voland
 * array('v'):[
 *     variant('(uua{ss}a(a{ss}))'):
 *         struct:(
 *             uint32:108
 *             uint32:0
 *             array('{ss}'):[
 *                 dict:{
 *                     string:'APPLICATION'
 *                     string:'test'
 *                 }
 *                 dict:{
 *                     string:'PLUGIN'
 *                     string:'libclockalarm'
 *                 }
 *                 dict:{
 *                     string:'TITLE'
 *                     string:'Alarm3'
 *                 }
 *             ]
 *             array('(a{ss})'):[
 *                 struct:(
 *                     array('{ss}'):[
 *                         dict:{
 *                             string:'TAG'
 *                             string:'BAF'
 *                         }
 *                         dict:{
 *                             string:'TITLE'
 *                             string:'Button1'
 *                         }
 *                     ]
 *                 )
 *                 struct:(
 *                     array('{ss}'):[
 *                         dict:{
 *                             string:'TAG'
 *                             string:'BAF'
 *                         }
 *                         dict:{
 *                             string:'TITLE'
 *                             string:'Button2'
 *                         }
 *                     ]
 *                 )
 *             ]
 *         )
 *
 *     variant('(uua{ss}a(a{ss}))'):
 *         :
 *         :
 *
 *     variant('(uua{ss}a(a{ss}))'):
 *         :
 *         :
 *
 * ]
 *
 */
#define VOLAND_DBUS_METHOD_OPEN "open"

/** Close dialog request
 *
 * @param cookie UINT32 unique alarm identifier
 *
 * @return BOOLEAN ok
 */
#define VOLAND_DBUS_METHOD_CLOSE "close"

#endif /* VOLAND_DBUS_H_ */
/*
 */
