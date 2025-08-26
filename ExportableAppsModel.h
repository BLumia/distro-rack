// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QVariant>

struct ExportableApp {
    QString name;
    QString path;
    bool exported;
    bool operationInProgress;

    ExportableApp() = default;
    ExportableApp(const QString& name, const QString& path, bool exported)
        : name(name), path(path), exported(exported), operationInProgress(false) {}
};

class ExportableAppsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        ExportedRole,
        OperationInProgressRole
    };
    Q_ENUM(Roles)

    explicit ExportableAppsModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateApps(const QList<ExportableApp>& apps);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void setOperationInProgress(const QString& appPath, bool inProgress);
    Q_INVOKABLE QVariantMap getApp(int index) const;
    Q_INVOKABLE int getAppIndex(const QString& appPath) const;

    const QList<ExportableApp>& apps() const { return m_apps; }

signals:
    void appsChanged();
    void operationStatusChanged(const QString& appPath, bool inProgress);

private:
    QList<ExportableApp> m_apps;

    int findAppIndex(const QString& appPath) const;
};
