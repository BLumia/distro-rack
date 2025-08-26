// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QColor>

enum class PackageManager {
    Unknown,
    Apt,
    Dnf,
    Pacman,
    Zypper,
    Xbps,
    Apk
};

struct KnownDistro {
    QString name;
    QString color;
    QString displayName;
    PackageManager packageManager;
    QString installableFileExtension;

    KnownDistro() : packageManager(PackageManager::Unknown) {}

    KnownDistro(const QString& n, const QString& c, const QString& dn,
                PackageManager pm, const QString& ext = "")
        : name(n), color(c), displayName(dn), packageManager(pm), installableFileExtension(ext) {}

    QString iconName() const {
        QString distroName = name == "arch" ? "archlinux" : name;
        return "distributor-logo-" + distroName;
    }

    static QString defaultIconName() {
        return "distributor-logo-generic";
    }
};

class KnownDistros {
public:
    static const QMap<QString, KnownDistro>& getDistros();
    static KnownDistro detectDistroFromImage(const QString& imageUrl);
    static QString generateDistroCSS();

private:
    static QMap<QString, KnownDistro> initializeDistros();
};

