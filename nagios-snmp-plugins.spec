Summary:	Plugins for Nagios to monitor remote disk and processes via SNMP
Name: 		nagios-snmp-plugins
Version:	1.2
Release:	1%{?dist}
License:	GPL v2
Group:		Networking
Source0: 	http://www.softwareforge.de/releases/nagios-snmp-plugins/nagios-snmp-plugins-%{version}.tar.gz
BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:  net-snmp-devel
Requires:       nagios-plugins
BuildRoot: 	%{_tmppath}/%{name}-root

%define		_sysconfdir	/etc/nagios/plugins
%define		plugindir	%{_libdir}/nagios/plugins

%description
These plugins allow you to monitor disk space and running processes on
a remote machine via SNMP.

%prep
%setup -q

%build
%configure \
	--bindir=%{plugindir}
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
%{__make} install \
	DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf %{buildroot}

%files
%defattr(644,root,root,755)
%doc AUTHORS NEWS README
%config(noreplace) %verify(not md5 mtime size) %{_sysconfdir}/%{name}.cfg
%attr(755,root,root) %{plugindir}/*

%changelog
* Sun Jan 27 2008 Henning P. Schmiedehausen <hps@intermeta.de> - 1.2-1.im
- Release 1.2

* Mon Aug 27 2007 Henning P. Schmiedehausen <hps@intermeta.de> - 1.1-1.im
- Update Spec file for newer distributions
- explicit GPLv2

* Mon Mar 30 2004 Henning P. Schmiedehausen <hps@intermeta.de> 1.0-1t
- reworked for Nagios
- added autoconf
- rebuilt on RH9

* Sun Mar 3 2002  Henning P. Schmiedehausen <hps@intermeta.de> 0.2.1-1t
- rebuilt for RH SNMP Upgrade

* Sun Jan 27 2002 Henning P. Schmiedehausen <hps@intermeta.de> 0.1-1t
- initial release
