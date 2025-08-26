// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QVariant>

struct Container {
    QString id;
    QString name;
    QString status;
    QString image;
    QString distro;
    QString distroDisplayName;
    QString distroColor;
    int packageManager;
    QString installableFileExtension;
    
    Container() = default;
    Container(const QString& id, const QString& name, const QString& status, const QString& image)
        : id(id), name(name), status(status), image(image), packageManager(0) {}
};

class ContainerModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        StatusRole,
        ImageRole,
        DistroRole,
        DistroDisplayNameRole,
        DistroColorRole,
        DistroIconNameRole,
        PackageManagerRole,
        InstallableFileExtensionRole
    };
    Q_ENUM(Roles)
    
    explicit ContainerModel(QObject *parent = nullptr);
    
    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateContainers(const QList<Container>& containers);
    Q_INVOKABLE void clear();

    Q_INVOKABLE QModelIndex getContainerModelIndex(const QString& name) const;

    const QList<Container>& containers() const { return m_containers; }
    
signals:
    void containerAdded(const QString& name);
    void containerUpdated(const QString& name);
    void containerRemoved(const QString& name);
    void containersChanged();
    
private:
    QList<Container> m_containers;
    
    int findContainerIndex(const QString& name) const;
};

