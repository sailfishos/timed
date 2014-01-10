Name:       timed

Summary:    Time daemon
Version:    2.86
Release:    1
Group:      System/System Control
License:    LGPLv2
URL:        https://github.com/nemomobile/timed
Source0:    %{name}-%{version}.tar.bz2
Requires:   tzdata
Requires:   tzdata-timed
Requires:   systemd
Requires:   oneshot
%{_oneshot_groupadd_requires_pre}
%{_oneshot_requires_post}
%{_oneshot_groupadd_requires_post}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(contextprovider-1.0)
BuildRequires:  pkgconfig(libpcrecpp)
BuildRequires:  pkgconfig(QtCore) >= 4.7
BuildRequires:  pkgconfig(dsme_dbus_if)
BuildRequires:  pkgconfig(systemd)
BuildRequires:  libiodata-devel >= 0.19
BuildRequires:  libxslt
BuildRequires: oneshot

%description
The time daemon (%{name}) managing system time, time zone and settings,
executing actions at given time and managing the event queue.


%package tests
Summary:    Test cases for %{name}
Group:      Development/System
Requires:   %{name} = %{version}-%{release}
Requires:   testrunner-lite

%description tests
Simple automated test cases, to be executed in cita.

%package tools
Summary:    Command line tools for communication with the time daemon
Group:      Development/Tools
Requires:   %{name} = %{version}-%{release}

%description tools
Command line timed tools: 'simple-client' - a command line
simple client for time daemon; 'fake-dialog-ui' - a command
line voland implementation; 'ticker' - a command line clock
and signal notification; timedclient - add, modify, remove,
and query alarms.

%package devel
Summary:    Development package for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(QtCore) >= 4.6

%description devel
Header files and shared lib symlink for %{name}.

%prep
%setup -q -n %{name}-%{version}

%build
export TIMED_VERSION=%{version}
mkdir -p src/h/timed
ln -sf ../../lib/qmacro.h src/h/timed

%qmake  \
    -recursive "CONFIG += MEEGO dsme_dbus_if"

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%qmake_install

#install -m 644 -D src/doc/timed.8 %{buildroot}/%{_mandir}/man8/timed.8
#install -m 644 -D src/doc/libtimed.3 %{buildroot}/%{_mandir}/man3/libtimed.3
#install -m 644 src/doc/libtimed-voland.3 %{buildroot}/%{_mandir}/man3/libtimed-voland.3

# The file %{buildroot}/lib/systemd/user/%{name}.service is installed by make install
install -d %{buildroot}%{_libdir}/systemd/user/pre-user-session.target.wants/
ln -s ../%{name}.service %{buildroot}%{_libdir}/systemd/user/pre-user-session.target.wants/%{name}.service

# Missing executable flags.
chmod 755 %{buildroot}%{_datadir}/backup-framework/scripts/timed-restore-script.sh
chmod 755 %{buildroot}%{_oneshotdir}/setcaps-%{name}.sh

# Timed changes time zone by linking /var/lib/timed/localtime to zones in /usr/share/zoneinfo.
# The links are done in the post section
install -d %{buildroot}/var/lib/timed
touch %{buildroot}/var/lib/timed/localtime

%pre
groupadd -rf timed
groupadd-user timed

%post
# Make /etc/localtime a link to /var/lib/timed/localtime to make system time zone follow timed.
# Remove existing link so that copying the UTC file will not overwrite anything during reinstall.
rm -f /var/lib/timed/localtime
cp /usr/share/zoneinfo/UTC /var/lib/timed/localtime
ln -sf /var/lib/timed/localtime /etc/localtime

/sbin/ldconfig
add-oneshot --now setcaps-%{name}.sh
if [ "$1" -ge 1 ]; then
systemctl-user daemon-reload || :
systemctl-user restart %{name}.service || :
fi

%preun
if [ "$1" -eq 0 ]; then
systemctl-user stop %{name}.service
fi

%postun
/sbin/ldconfig
if [ "$1" -eq 0 ]; then
systemctl-user stop {%name}.service || :
systemctl-user daemon-reload || :
getent group time >/dev/null && groupdel timed
fi

%files
%defattr(-,root,root,-)
%doc COPYING debian/changelog debian/copyright
%config(noreplace) %{_sysconfdir}/dbus-1/system.d/%{name}.conf
%config(noreplace) %{_sysconfdir}/aegisfs.d/timed.aegisfs.conf
%config(noreplace) %{_sysconfdir}/%{name}.rc
%{_sysconfdir}/osso-cud-scripts/timed-clear-device.sh
%{_sysconfdir}/osso-rfs-scripts/timed-restore-original-settings.sh
%{_bindir}/%{name}
%{_bindir}/timed-aegis-session-helper
%{_libdir}/lib%{name}.so.*
%{_libdir}/libtimed-voland.so.*
%{_datadir}/backup-framework/applications/timedbackup.conf
%{_datadir}/backup-framework/scripts/timed-backup-script.sh
%{_datadir}/backup-framework/scripts/timed-restore-script.sh
%{_datadir}/contextkit/providers/com.nokia.time.context
# %{_mandir}/man3/libtimed.3.gz
# %{_mandir}/man3/libtimed-voland.3.gz
# %{_mandir}/man8/timed.8.gz
%{_libdir}/systemd/user/%{name}.service
%{_libdir}/systemd/user/pre-user-session.target.wants/%{name}.service
%{_oneshotdir}/setcaps-%{name}.sh
%dir %attr(0774,-,timed) /var/lib/timed
%ghost /var/lib/timed/localtime

%files tests
%defattr(-,root,root,-)
%doc COPYING
/opt/tests/%{name}-tests
%config(noreplace) %{_sysconfdir}/dbus-1/system.d/org.fakeofono.conf

%files tools
%defattr(-,root,root,-)
%doc COPYING
%{_bindir}/timedclient

%files devel
%defattr(-,root,root,-)
%doc COPYING
%{_includedir}/%{name}/*
%{_includedir}/timed-voland/*
%{_libdir}/libtimed.so
%{_libdir}/libtimed-voland.so
%{_libdir}/pkgconfig/timed.pc
%{_libdir}/pkgconfig/timed-voland.pc
%{_datadir}/qt4/mkspecs/features/timed.prf
%{_datadir}/qt4/mkspecs/features/timed-voland.prf
