// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "ExportableAppsModel.h"

#include <QDebug>

ExportableAppsModel::ExportableAppsModel(QObject *parent) : QAbstractListModel(parent)
{
}

int ExportableAppsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_apps.size();
}

QVariant ExportableAppsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_apps.size())
        return QVariant();

    const ExportableApp& app = m_apps[index.row()];

    switch (role) {
    case NameRole:
        return app.name;
    case PathRole:
        return app.path;
    case ExportedRole:
        return app.exported;
    case OperationInProgressRole:
        return app.operationInProgress;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ExportableAppsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PathRole] = "path";
    roles[ExportedRole] = "exported";
    roles[OperationInProgressRole] = "operationInProgress";
    return roles;
}

void ExportableAppsModel::updateApps(const QList<ExportableApp>& apps)
{
    QHash<QString, bool> operationStates;
    for (const ExportableApp& app : m_apps) {
        if (app.operationInProgress) {
            operationStates[app.path] = true;
        }
    }

    beginResetModel();
    m_apps = apps;

    // restore previous operation states
    for (ExportableApp& app : m_apps) {
        if (operationStates.contains(app.path)) {
            app.operationInProgress = operationStates[app.path];
        }
    }

    endResetModel();
    emit appsChanged();
    qDebug() << "ExportableAppsModel updated, count:" << m_apps.size();
}

void ExportableAppsModel::clear()
{
    beginResetModel();
    m_apps.clear();
    endResetModel();
    emit appsChanged();
}

void ExportableAppsModel::setOperationInProgress(const QString& appPath, bool inProgress)
{
    int index = findAppIndex(appPath);
    if (index >= 0) {
        m_apps[index].operationInProgress = inProgress;
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, {OperationInProgressRole});
        emit operationStatusChanged(appPath, inProgress);
    }
}

QVariantMap ExportableAppsModel::getApp(int index) const
{
    if (index < 0 || index >= m_apps.size())
        return QVariantMap();

    const ExportableApp& app = m_apps[index];
    QVariantMap map;
    map["name"] = app.name;
    map["path"] = app.path;
    map["exported"] = app.exported;
    map["operationInProgress"] = app.operationInProgress;

    return map;
}

int ExportableAppsModel::getAppIndex(const QString& appPath) const
{
    return findAppIndex(appPath);
}

int ExportableAppsModel::findAppIndex(const QString& appPath) const
{
    for (int i = 0; i < m_apps.size(); ++i) {
        if (m_apps[i].path == appPath) {
            return i;
        }
    }
    return -1;
}


