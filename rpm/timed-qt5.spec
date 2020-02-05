Name:       timed-qt5

Summary:    Time daemon
Version:    3.6
Release:    1
License:    LGPLv2
URL:        https://git.sailfishos.org/mer-core/timed
Source0:    %{name}-%{version}.tar.bz2
Source1:    %{name}.privileges
Requires:   tzdata
Requires:   tzdata-timed
Requires:   systemd
Requires:   oneshot
Requires:   sailfish-setup >= 0.1.10
%{_oneshot_requires_post}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(libpcrecpp)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Test)
BuildRequires:  pkgconfig(dsme_dbus_if)
BuildRequires:  pkgconfig(systemd)
BuildRequires:  libiodata-qt5-devel >= 0.19
BuildRequires:  libxslt
BuildRequires:  oneshot
BuildRequires:  pkgconfig(sailfishaccesscontrol) >= 0.0.1

%description
The time daemon (%{name}) managing system time, time zone and settings,
executing actions at given time and managing the event queue.

%package tests
Summary:    Test cases for %{name}
Requires:   %{name} = %{version}-%{release}
Requires:   testrunner-lite

%description tests
Simple automated test cases, to be executed in cita.

%package tools
Summary:    Command line tools for communication with the time daemon
Requires:   %{name} = %{version}-%{release}

%description tools
timedclient - add, modify, remove, and query alarms.

%package devel
Summary:    Development package for %{name}
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(Qt5Core)

%description devel
Header files and shared lib symlink for %{name}.

%prep
%setup -q -n %{name}-%{version}

%build
export TIMED_VERSION=%{version}
mkdir -p src/h/timed-qt5
ln -sf ../../lib/qmacro.h src/h/timed-qt5

%qmake5  \
    -recursive "CONFIG += dsme_dbus_if ofono"

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

# The file %{buildroot}/lib/systemd/user/%{name}.service is installed by make install
install -d %{buildroot}%{_libdir}/systemd/user/pre-user-session.target.wants/
ln -s ../%{name}.service %{buildroot}%{_libdir}/systemd/user/pre-user-session.target.wants/%{name}.service

mkdir -p %{buildroot}%{_datadir}/mapplauncherd/privileges.d
install -m 644 -p %{SOURCE1} %{buildroot}%{_datadir}/mapplauncherd/privileges.d/

# Missing executable flags.
chmod 755 %{buildroot}%{_oneshotdir}/setcaps-%{name}.sh

# Timed changes time zone by linking /var/lib/timed/localtime to zones in /usr/share/zoneinfo.
# Initial links are done in the post section
install -d %{buildroot}/var/lib/timed
touch %{buildroot}/var/lib/timed/localtime
install -d %{buildroot}/var/lib/timed/shared_events
install -d %{buildroot}/var/lib/timed/shared_settings
# Make /etc/localtime a link to /var/lib/timed/localtime to make system time zone follow timed.
install -d %{buildroot}%{_sysconfdir}
ln -sf /var/lib/timed/localtime %{buildroot}%{_sysconfdir}/localtime

%post
# Remove existing link so that copying the UTC file will not overwrite anything during reinstall.
rm -f /var/lib/timed/localtime
cp /usr/share/zoneinfo/UTC /var/lib/timed/localtime

/sbin/ldconfig
add-oneshot --now setcaps-%{name}.sh
if [ "$1" -ge 1 ]; then
systemctl-user daemon-reload || :
systemctl-user restart %{name}.service || :
fi

%preun
if [ "$1" -eq 0 ]; then
  systemctl-user stop %{name}.service || :
fi

%postun
/sbin/ldconfig
if [ "$1" -eq 0 ]; then
  systemctl-user stop {%name}.service || :
  systemctl-user daemon-reload || :
fi

%files
%defattr(-,root,root,-)
%license COPYING copyright
%{_sysconfdir}/dbus-1/system.d/%{name}.conf
%{_sysconfdir}/%{name}.rc
%{_sysconfdir}/localtime
%{_bindir}/%{name}
%{_libdir}/lib%{name}.so.*
%{_libdir}/libtimed-voland-qt5.so.*
%{_datadir}/mapplauncherd/privileges.d/*
%{_libdir}/systemd/user/%{name}.service
%{_libdir}/systemd/user/pre-user-session.target.wants/%{name}.service
%{_oneshotdir}/setcaps-%{name}.sh
%dir %attr(0775,-,timed) /var/lib/timed
%dir %attr(02770,root,sailfish-alarms) /var/lib/timed/shared_events
%dir %attr(02775,root,sailfish-datetime) /var/lib/timed/shared_settings
%ghost /var/lib/timed/localtime
%ghost /var/lib/timed/shared_events/events.data
%ghost /var/lib/timed/shared_settings/settings.data

%files tests
%defattr(-,root,root,-)
/opt/tests/%{name}-tests
%{_sysconfdir}/dbus-1/system.d/org.fakeofono.conf

%files tools
%defattr(-,root,root,-)
%{_bindir}/timedclient-qt5

%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}
%{_includedir}/timed-voland-qt5
%{_libdir}/lib%{name}.so
%{_libdir}/libtimed-voland-qt5.so
%{_libdir}/pkgconfig/timed-qt5.pc
%{_libdir}/pkgconfig/timed-voland-qt5.pc
%{_datadir}/qt5/mkspecs/features/%{name}.prf
%{_datadir}/qt5/mkspecs/features/timed-voland-qt5.prf
