// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "KnownDistros.h"
#include <QDebug>

QMap<QString, KnownDistro> KnownDistros::initializeDistros() {
    QMap<QString, KnownDistro> distros;

    // TODO: make this an external config file?
    distros["alma"] = KnownDistro("alma", "#dadada", "AlmaLinux", PackageManager::Dnf, ".rpm");
    distros["alpine"] = KnownDistro("alpine", "#2147ea", "Alpine Linux", PackageManager::Apk, ".apk");
    distros["amazon"] = KnownDistro("amazon", "#de5412", "Amazon Linux", PackageManager::Dnf, ".rpm");
    distros["arch"] = KnownDistro("arch", "#12aaff", "Arch Linux", PackageManager::Pacman, ".pkg.tar.xz");
    distros["centos"] = KnownDistro("centos", "#ff6600", "CentOS", PackageManager::Dnf, ".rpm");
    distros["clearlinux"] = KnownDistro("clearlinux", "#56bbff", "Clear Linux", PackageManager::Unknown);
    distros["crystal"] = KnownDistro("crystal", "#8839ef", "Crystal Linux", PackageManager::Pacman, ".pkg.tar.xz");
    distros["debian"] = KnownDistro("debian", "#da5555", "Debian", PackageManager::Apt, ".deb");
    distros["deepin"] = KnownDistro("deepin", "#0050ff", "Deepin", PackageManager::Apt, ".deb");
    distros["fedora"] = KnownDistro("fedora", "#3b6db3", "Fedora", PackageManager::Dnf, ".rpm");
    distros["gentoo"] = KnownDistro("gentoo", "#daaada", "Gentoo", PackageManager::Unknown);
    distros["kali"] = KnownDistro("kali", "#000000", "Kali Linux", PackageManager::Apt, ".deb");
    distros["mageia"] = KnownDistro("mageia", "#b612b6", "Mageia", PackageManager::Dnf, ".rpm");
    distros["mint"] = KnownDistro("mint", "#6fbd20", "Linux Mint", PackageManager::Apt, ".deb");
    distros["neon"] = KnownDistro("neon", "#27ae60", "KDE Neon", PackageManager::Apt, ".deb");
    distros["opensuse"] = KnownDistro("opensuse", "#daff00", "openSUSE", PackageManager::Zypper, ".rpm");
    distros["oracle"] = KnownDistro("oracle", "#ff0000", "Oracle Linux", PackageManager::Dnf, ".rpm");
    distros["redhat"] = KnownDistro("redhat", "#ff6662", "Red Hat", PackageManager::Dnf, ".rpm");
    distros["rhel"] = KnownDistro("rhel", "#ff6662", "RHEL", PackageManager::Dnf, ".rpm");
    distros["rocky"] = KnownDistro("rocky", "#91ff91", "Rocky Linux", PackageManager::Dnf, ".rpm");
    distros["slackware"] = KnownDistro("slackware", "#6145a7", "Slackware", PackageManager::Unknown);
    distros["ubuntu"] = KnownDistro("ubuntu", "#FF4400", "Ubuntu", PackageManager::Apt, ".deb");
    distros["vanilla"] = KnownDistro("vanilla", "#7f11e0", "Vanilla OS", PackageManager::Apt, ".deb");
    distros["void"] = KnownDistro("void", "#abff12", "Void Linux", PackageManager::Xbps, ".xbps");

    return distros;
}

const QMap<QString, KnownDistro>& KnownDistros::getDistros() {
    static QMap<QString, KnownDistro> distros = initializeDistros();
    return distros;
}

KnownDistro KnownDistros::detectDistroFromImage(const QString& imageUrl) {
    const auto& distros = getDistros();

    // convert the image URL to lowercase for matching
    QString lowerUrl = imageUrl.toLower();

    // iterate through all known distros, find the matching one
    for (auto it = distros.begin(); it != distros.end(); ++it) {
        const QString& distroName = it.key();
        if (lowerUrl.contains(distroName)) {
            qDebug() << "Detected distro:" << distroName << "from image:" << imageUrl;
            return it.value();
        }
    }

    // special case handling
    if (lowerUrl.contains("toolbx")) {
        // toolbx images are usually based on specific distros
        if (lowerUrl.contains("fedora")) {
            return distros["fedora"];
        } else if (lowerUrl.contains("arch")) {
            return distros["arch"];
        } else if (lowerUrl.contains("ubuntu")) {
            return distros["ubuntu"];
        }
    }

    // if no matching distro is found, return the default value
    qDebug() << "No distro detected for image:" << imageUrl;
    return KnownDistro();
}

QString KnownDistros::generateDistroCSS() {
    QString css;
    const auto& distros = getDistros();

    for (auto it = distros.begin(); it != distros.end(); ++it) {
        const KnownDistro& distro = it.value();
        css += QString(".%1 {\n    --distro-color: %2;\n}\n")
                .arg(distro.name)
                .arg(distro.color);
    }

    return css;
}
