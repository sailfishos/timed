#ifndef TIMED_F_H
#define TIMED_F_H

#if __MEEGO__
#  define F_CREDS_UID 1
//#  define F_CREDS_NOBODY 1
#endif

#if __HARMATTAN__
#  define F_CREDS_AEGIS_LIBCREDS 1
#  define F_CELLULAR_QT 1
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
