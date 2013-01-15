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
#ifndef TIMED_F_H
#define TIMED_F_H

#if __MEEGO__
#  define F_CREDS_UID 1
//#  define F_CREDS_NOBODY 1
#  define F_FORCE_DEBUG_PATH "/var/cache/timed/DEBUG"
#  define OFONO 1
#endif

#if __HARMATTAN__
#  define F_CREDS_AEGIS_LIBCREDS 1
#  define OFONO 1
#  define F_SCRATCHBOX 1
#  define F_ACTING_DEAD 1
#  define F_IMAGE_TYPE 1
#  define F_HOME_LOG 1
#  define F_FORCE_DEBUG_PATH "/var/cache/timed/DEBUG"
#  define F_FORCE_HOME_LOG_PATH "/var/cache/timed/HOME_LOG"
#endif

#if F_CREDS_AEGIS_LIBCREDS
#  define F_TOKENS_AS_CREDENTIALS 1
#endif

#if F_CREDS_AEGIS_LIBCREDS || F_CREDS_UID
#  define F_UID_AS_CREDENTIALS 1
#  define F_DBUS_INFO_AS_CREDENTIALS 1
#endif

#if __MEEGO__ && __HARMATTAN__
#  warning The Meego and Harmattan feature sets are mutualy exclusive
#endif

#if F_CREDS_AEGIS_LIBCREDS + F_CREDS_UID + F_CREDS_NOBODY != 1
#  warning 'aegis_libcreds', 'uid' and 'nobody' credential features are mutualy exclusive
#endif

#endif//TIMED_F_H
