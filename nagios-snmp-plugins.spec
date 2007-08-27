#
# Distribution
#
%{!?fc1:%define fc1 0}
%{!?rh9:%define rh9 0}
%{!?rh7:%define rh7 0}
%{!?rhel3:%define rhel3 0}

Name: 		nagios-snmp-plugins
Summary: 	Plugins for Nagios to monitor remote disk and processes via SNMP
Version:	1.0
Release:	1t.%{_distbuild}
Source: 	ftp://ftp.hometree.net:/pub/nagios-snmp-plugins/nagios-snmp-plugins-%{version}.tar.gz
Copyright: 	GPL
BuildRoot: 	%{_tmppath}/%{name}-root
Group: 		Applications/System
Packager: 	Henning P. Schmiedehausen <henning@intermeta.de>
Distribution: 	INTERMETA RPMs
Vendor: 	INTERMETA - Gesellschaft fuer Mehrwertdienste mbH

%description
These plugins allow you to monitor disk space and running processes on
a remote machine via SNMP.

%prep
%setup -q

%build
aclocal
autoheader
automake --add-missing
autoconf
%configure

make

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}/usr/lib/nagios/plugins

install -s check_snmp_disk %{buildroot}/usr/lib/nagios/plugins
install -s check_snmp_proc %{buildroot}/usr/lib/nagios/plugins

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)

%doc README COPYING AUTHORS NEWS ChangeLog

%attr(755,root,root) /usr/lib/nagios/plugins/check_snmp_disk
%attr(755,root,root) /usr/lib/nagios/plugins/check_snmp_proc

%changelog
* Mon Mar 30 2004 Henning P. Schmiedehausen <hps@intermeta.de> 1.0-1t
- reworked for Nagios
- added autoconf
- rebuilt on RH9

* Sun Mar 3 2002  Henning P. Schmiedehausen <hps@intermeta.de> 0.2.1-1t
- rebuilt for RH SNMP Upgrade

* Sun Jan 27 2002 Henning P. Schmiedehausen <hps@intermeta.de> 0.1-1t
- initial release
