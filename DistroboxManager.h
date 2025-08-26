// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QProcess>
#include <QVariantList>
#include "ContainerModel.h"
#include "ExportableAppsModel.h"

class DistroboxManager : public QObject
{
    Q_OBJECT

public:
    explicit DistroboxManager(QObject *parent = nullptr);

    struct ContainerInfo {
        QString id;
        QString name;
        QString status;
        QString image;
    };

signals:
    void commandResult(const QString &result);
    void commandError(const QString &error);
    void commandFinished(const QString &output);
    void containerListUpdated(const QList<Container> &containers);
    void exportableAppsUpdated(const QList<ExportableApp> &apps);
    void exportAppResult(const QString &result);
    void unexportAppResult(const QString &result);
    void terminalSpawned();
    void containerCreated();
    void containerDeleted();

    void taskStarted(const QString &name, const QString &description);
    void taskOutputReceived(const QString &output);
    void taskStatusUpdated(const QString &status);
    void taskFinished(const QString &status);

public slots:
    void runCommand(const QString &command);
    void listContainers();
    void listExportableApps(const QString &containerName);
    void exportApp(const QString &containerName, const QString &desktopFilePath);
    void unexportApp(const QString &containerName, const QString &desktopFilePath);
    void spawnTerminal(const QString &containerName);
    void createContainer(const QString &name, const QString &image, bool nvidia = false, bool init = false);
    void createContainer(const QString &name, const QString &image, bool nvidia, bool init, const QString &homeDir, const QStringList &volumes);
    void deleteContainer(const QString &containerName);
    void upgradeContainer(const QString &containerName);
    void upgradeAllContainers();
    void cloneContainer(const QString &sourceContainer, const QString &newName);
    void stopContainer(const QString &containerName);
    void stopAllContainers();
    void assembleFromFile(const QString &filePath);
    void assembleFromUrl(const QString &url);
    void generateEntry(const QString &containerName, const QString &iconPath = QString());
    void deleteEntry(const QString &containerName);
    void generateAllEntries();
    void installPackage(const QString &containerName, const QString &packagePath);

private:
    QProcess *m_listProcess;
    QProcess *m_operationProcess;

    void onListProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void parseContainerList(const QString &output);
    void parseExportableApps(const QString &output, const QString &containerName);
    QStringList buildCreateArguments(const QString &name, const QString &image, bool nvidia, bool init, const QString &homeDir = QString(), const QStringList &volumes = QStringList());
};
