%define nagios_plugins_dir %{_libdir}/nagios/plugins

Name: 		nagios-snmp-plugins
Summary: 	Plugins for Nagios to monitor remote disk and processes via SNMP
Version:	1.1
Release:	1.%{?dist}.im
Source: 	http://www.softwareforge.de/releases/nagios-snmp-plugins/nagios-snmp-plugins-%{version}.tar.gz
Copyright: 	GPLv2
BuildRoot: 	%{_tmppath}/%{name}-root
Group: 		Applications/System
Packager: 	Henning P. Schmiedehausen <henning@intermeta.de>
Distribution: 	INTERMETA RPMs
Vendor: 	INTERMETA - Gesellschaft fuer Mehrwertdienste mbH

BuildRequires:  autoconf, automake
BuildRequires:  net-snmp-devel
BuildRequires:  openssl-devel
BuildRequires:  tcp_wrappers-devel
Requires:       nagios-plugins

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

make %{?_smp_mflags}

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{nagios_plugins_dir}

install check_snmp_disk %{buildroot}%{nagios_plugins_dir}
install check_snmp_proc %{buildroot}%{nagios_plugins_dir}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)

%doc README COPYING AUTHORS NEWS

%attr(755,root,root) %{nagios_plugins_dir}/check_snmp_disk
%attr(755,root,root) %{nagios_plugins_dir}/check_snmp_proc

%changelog
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
