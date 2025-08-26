// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "ContainerModel.h"
#include "KnownDistros.h"
#include <QDebug>

ContainerModel::ContainerModel(QObject *parent) : QAbstractListModel(parent)
{
}

int ContainerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_containers.size();
}

QVariant ContainerModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_containers.size())
        return QVariant();

    const Container& container = m_containers[index.row()];

    switch (role) {
    case IdRole:
        return container.id;
    case NameRole:
        return container.name;
    case StatusRole:
        return container.status;
    case ImageRole:
        return container.image;
    case DistroRole:
        return container.distro;
    case DistroDisplayNameRole:
        return container.distroDisplayName;
    case DistroColorRole:
        return container.distroColor;
    case DistroIconNameRole:
        return KnownDistros::getDistros()[container.distro].iconName();
    case PackageManagerRole:
        return container.packageManager;
    case InstallableFileExtensionRole:
        return container.installableFileExtension;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ContainerModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[StatusRole] = "status";
    roles[ImageRole] = "image";
    roles[DistroRole] = "distro";
    roles[DistroDisplayNameRole] = "distroDisplayName";
    roles[DistroColorRole] = "distroColor";
    roles[DistroIconNameRole] = "distroIconName";
    roles[PackageManagerRole] = "packageManager";
    roles[InstallableFileExtensionRole] = "installableFileExtension";
    return roles;
}

void ContainerModel::updateContainers(const QList<Container>& containers)
{
    beginResetModel();
    m_containers = containers;
    endResetModel();
    emit containersChanged();
    qDebug() << "ContainerModel updated, count:" << m_containers.size();
}

void ContainerModel::clear()
{
    beginResetModel();
    m_containers.clear();
    endResetModel();
    emit containersChanged();
}

int ContainerModel::findContainerIndex(const QString& name) const
{
    for (int i = 0; i < m_containers.size(); ++i) {
        if (m_containers[i].name == name) {
            return i;
        }
    }
    return -1;
}

QModelIndex ContainerModel::getContainerModelIndex(const QString& name) const
{
    int row = findContainerIndex(name);
    if (row >= 0) {
        return index(row, 0);
    }
    return QModelIndex();
}


