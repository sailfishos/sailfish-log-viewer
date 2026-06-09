Name:       sailfish-log-viewer
Summary:    Log viewer tools
Version:    1.0.22
Release:    1
License:    BSD
URL:        https://github.com/sailfishos/sailfish-log-viewer
Source0:    %{name}-%{version}.tar.bz2

Requires:   sailfishsilica-qt5
BuildRequires: pkgconfig(sailfishapp)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(mlite5)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: pkgconfig(qofonoext)
BuildRequires: pkgconfig(libglibutil)
BuildRequires: desktop-file-utils
BuildRequires: qt5-qttools-linguist

# license macro requires rpm >= 4.11
BuildRequires: pkgconfig(rpm)
%define license_support %(pkg-config --exists 'rpm >= 4.11'; echo $?)

%description
Application for gathering ofono logs on Sailfish OS

%package ofono
Summary:    Ofono logger

%description ofono
%{summary}.

%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5 CONFIG+=ofono CONFIG+=sailfish-log-viewer CONFIG+=app_settings
%qtc_make %{?_smp_mflags} logger-ofono

%install
rm -rf %{buildroot}
%qmake5_install -C ofono

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%files ofono
%global privileges_dir %{_datarootdir}/mapplauncherd/privileges.d
%dir %{privileges_dir}
%defattr(-,root,root,-)
%{_datadir}/jolla-settings/entries/sailfish-log-viewer-ofono.json
%{_datadir}/translations/sailfish-log-viewer-ofono*.qm
%{_datadir}/applications/sailfish-log-viewer-ofono.desktop
%{_datadir}/sailfish-log-viewer-ofono/qml
%{_datadir}/sailfish-log-viewer-ofono/settings
%{_bindir}/sailfish-log-viewer-ofono
%{_datadir}/icons/hicolor/*/apps/sailfish-log-viewer-ofono.png
%{privileges_dir}/sailfish-log-viewer-ofono
%if %{license_support} == 0
%license LICENSE
%endif
