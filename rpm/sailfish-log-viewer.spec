Name:       sailfish-log-viewer
Summary:    Log viewer tools
Version:    1.1.0
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
%{summary}.

%package nfc
Summary: NFC logger

%description nfc
Application for gathering nfcd logs.

%package ofono
Summary: Ofono logger

%description ofono
Application for gathering ofono logs.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 CONFIG+=nfc CONFIG+=ofono CONFIG+=sailfish-log-viewer CONFIG+=app_settings
%make_build logger-nfc logger-ofono

%install
%qmake5_install -C nfc
%qmake5_install -C ofono

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%files nfc
%global privileges_dir %{_datarootdir}/mapplauncherd/privileges.d
%dir %{privileges_dir}
%{privileges_dir}/%{name}-nfc
%{_datadir}/jolla-settings/entries/%{name}-nfc.json
%{_datadir}/translations/%{name}-nfc*.qm
%{_datadir}/applications/%{name}-nfc.desktop
%{_datadir}/%{name}-nfc/qml
%{_datadir}/%{name}-nfc/settings
%{_bindir}/%{name}-nfc
%{_datadir}/icons/hicolor/*/apps/%{name}-nfc.png
%if %{license_support} == 0
%license LICENSE
%endif

%files ofono
%global privileges_dir %{_datarootdir}/mapplauncherd/privileges.d
%dir %{privileges_dir}
%{privileges_dir}/%{name}-ofono
%{_datadir}/jolla-settings/entries/%{name}-ofono.json
%{_datadir}/translations/%{name}-ofono*.qm
%{_datadir}/applications/%{name}-ofono.desktop
%{_datadir}/%{name}-ofono/qml
%{_datadir}/%{name}-ofono/settings
%{_bindir}/%{name}-ofono
%{_datadir}/icons/hicolor/*/apps/%{name}-ofono.png
%if %{license_support} == 0
%license LICENSE
%endif
