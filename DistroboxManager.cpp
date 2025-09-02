// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "DistroboxManager.h"
#include "KnownDistros.h"
#include "StateManager.h"
#include "TerminalManager.h"
#include "ContainerModel.h"
#include "ExportableAppsModel.h"
#include "KnownDistros.h"
#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

DistroboxManager::DistroboxManager(QObject *parent)
    : QObject(parent)
    , m_listProcess(new QProcess(this))
    , m_operationProcess(new QProcess(this))
{
    connect(m_listProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &DistroboxManager::onListProcessFinished);
}

void DistroboxManager::onListProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QString output = m_listProcess->readAllStandardOutput();
        qDebug() << "Distrobox list output:" << output;
        parseContainerList(output);
    } else {
        QString error = m_listProcess->readAllStandardError();
        QString stdOutput = m_listProcess->readAllStandardOutput();
        qDebug() << "Distrobox list error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
        emit commandError("Failed to list containers. Exit code: " + QString::number(exitCode) +
                          " Error: " + error + " Output: " + stdOutput);
    }
}

QStringList DistroboxManager::buildCreateArguments(const QString &name, const QString &image, bool nvidia, bool init, const QString &homeDir, const QStringList &volumes)
{
    QStringList args;
    args << "--yes" << "--name" << name << "--image" << image;

    if (!homeDir.isEmpty()) {
        args << "--home" << homeDir;
    }

    if (init) {
        args << "--init";
        args << "--additional-packages" << "systemd";
    }

    if (nvidia) {
        args << "--nvidia";
    }

    for (const QString &volume : volumes) {
        if (!volume.isEmpty()) {
            args << "--volume" << volume;
        }
    }

    return args;
}

void DistroboxManager::runCommand(const QString &command)
{
    // Split command into program and arguments
    QStringList args = command.split(' ', Qt::SkipEmptyParts);
    if (args.isEmpty()) {
        emit commandError("Empty command");
        return;
    }

    QString program = args.first();
    args.removeFirst();

    m_operationProcess->start(program, args);
}

void DistroboxManager::listContainers()
{
    // Check if a list operation is already running
    if (m_listProcess->state() != QProcess::NotRunning) {
        qDebug() << "List process already running, ignoring new request";
        return;
    }

    qDebug() << "Running distrobox list command";

    QStringList args;
    args << "ls" << "--no-color";
    m_listProcess->start("distrobox", args);
}

void DistroboxManager::parseContainerList(const QString &output)
{
    QList<Container> containers;
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    qDebug() << "Parsing lines:" << lines;

    // Skip the first line which is the header
    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.isEmpty()) continue;

        // Parse the line with format: ID|NAME|STATUS|IMAGE
        QStringList parts = line.split('|');
        if (parts.size() >= 4) {
            Container container;
            container.id = parts[0].trimmed();
            container.name = parts[1].trimmed();
            container.status = parts[2].trimmed();
            container.image = parts[3].trimmed();

            // add distro detection
            KnownDistro detectedDistro = KnownDistros::detectDistroFromImage(container.image);

            if (!detectedDistro.name.isEmpty()) {
                container.distro = detectedDistro.name;
                container.distroDisplayName = detectedDistro.displayName;
                container.distroColor = detectedDistro.color;
                container.packageManager = static_cast<int>(detectedDistro.packageManager);
                container.installableFileExtension = detectedDistro.installableFileExtension;
                qDebug() << "Detected distro:" << detectedDistro.name << "for container:" << container.name;
            } else {
                container.distro = "";
                container.distroDisplayName = "Unknown";
                container.distroColor = "#808080";
                container.packageManager = static_cast<int>(PackageManager::Unknown);
                container.installableFileExtension = "";
            }

            containers.append(container);
            qDebug() << "Parsed container:" << container.name;
        } else {
            qDebug() << "Skipping line with insufficient parts:" << line;
        }
    }

    qDebug() << "Emitting container list with" << containers.size() << "containers";
    emit containerListUpdated(containers);
}

void DistroboxManager::createContainer(const QString &name, const QString &image, bool nvidia, bool init)
{
    createContainer(name, image, nvidia, init, QString(), QStringList());
}

void DistroboxManager::createContainer(const QString &name, const QString &image, bool nvidia, bool init, const QString &homeDir, const QStringList &volumes)
{
    // Check if an operation is already running
    if (m_operationProcess->state() != QProcess::NotRunning) {
        qDebug() << "Operation process already running, cannot create container";
        emit commandError("Another operation is already running. Please wait for it to complete.");
        return;
    }

    qDebug() << "Creating container:" << name << "with image:" << image;

    // send the task started signal
    emit taskStarted("Create Container", "Creating container " + name + " with image " + image +
                     ". This may take a while as it needs to download the container image...");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Container creation output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Container creation error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // disconnect all connections to avoid multiple triggers
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container created output:" << output;
            emit taskOutputReceived(output);
            emit taskStatusUpdated("successful");
            emit taskFinished("successful");
            emit containerCreated();
            // reload the container list to show the new created container
            listContainers();
        } else {
            QString error = m_operationProcess->readAllStandardError();
            QString stdOutput = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container creation error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            QString errorMessage = "Failed to create container. Exit code: " + QString::number(exitCode) +
                                   " Error: " + error + " Output: " + stdOutput;
            emit taskOutputReceived(errorMessage);
            emit taskStatusUpdated("failed");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox create` command
    QStringList args = buildCreateArguments(name, image, nvidia, init, homeDir, volumes);
    args.prepend("create");
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::deleteContainer(const QString &containerName)
{
    // Check if an operation is already running
    if (m_operationProcess->state() != QProcess::NotRunning) {
        qDebug() << "Operation process already running, cannot delete container";
        emit commandError("Another operation is already running. Please wait for it to complete.");
        return;
    }

    qDebug() << "Deleting container:" << containerName;

    // send the task started signal
    emit taskStarted("Delete Container", "Deleting container " + containerName + "...");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Container deletion output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Container deletion error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // disconnect all connections to avoid multiple triggers
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container deleted output:" << output;
            emit taskOutputReceived(output);
            emit taskStatusUpdated("successful");
            emit taskFinished("successful");
            emit containerDeleted();
            // reload the container list to show the updated container list
            listContainers();
        } else {
            QString error = m_operationProcess->readAllStandardError();
            QString stdOutput = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container deletion error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            QString errorMessage = "Failed to delete container. Exit code: " + QString::number(exitCode) +
                                   " Error: " + error + " Output: " + stdOutput;
            emit taskOutputReceived(errorMessage);
            emit taskStatusUpdated("failed");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox rm` command
    QStringList args;
    args << "rm" << "--force" << containerName;
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::listExportableApps(const QString &containerName)
{
    qDebug() << "Running distrobox enter command to list exportable apps for container:" << containerName;

    QProcess *appsProcess = new QProcess(this);

    connect(appsProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, containerName, appsProcess](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = appsProcess->readAllStandardOutput();
            qDebug() << "Distrobox enter output:" << output;
            parseExportableApps(output, containerName);
        } else {
            QString error = appsProcess->readAllStandardError();
            QString stdOutput = appsProcess->readAllStandardOutput();
            qDebug() << "Distrobox enter error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            emit commandError("Failed to list exportable apps. Exit code: " + QString::number(exitCode) +
                              " Error: " + error + " Output: " + stdOutput);
        }
        appsProcess->deleteLater();
    });

    // use `distrobox enter` to enter the container and list exportable apps
    QStringList command;
    command << "enter" << containerName << "--" << "sh" << "-c"
        << "echo \"EXPORTED_APPS:\" && ls ${XDG_DATA_HOME:-$HOME/.local/share}/applications/" + containerName + "-*.desktop 2>/dev/null; echo \"DESKTOP_FILES:\" && for file in $(grep --files-without-match \"NoDisplay=true\" /usr/share/applications/*.desktop); do echo \"# START FILE $file\"; cat \"$file\"; done";

    appsProcess->start("distrobox", command);
}

void DistroboxManager::parseExportableApps(const QString &output, const QString &containerName)
{
    QList<ExportableApp> apps;
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    qDebug() << "Parsing app lines:" << lines;

    // Get the list of exported apps
    QSet<QString> exportedApps;
    bool inExportedSection = false;
    bool inDesktopFilesSection = false;

    for (const QString &line : lines) {
        if (line == "EXPORTED_APPS:") {
            inExportedSection = true;
            inDesktopFilesSection = false;
            continue;
        }

        if (line == "DESKTOP_FILES:") {
            inExportedSection = false;
            inDesktopFilesSection = true;
            continue;
        }

        if (inExportedSection) {
            exportedApps.insert(line);
        }
    }

    QString currentFile;
    QString currentName;
    QString currentExec;
    QString currentIcon;
    QString currentFilePath; // for storing the file path

    for (const QString &line : lines) {
        if (line.startsWith("# START FILE ")) {
            // if we have collected an app's info, add it to the list
            if (!currentFile.isEmpty() && !currentName.isEmpty()) {
                ExportableApp app;
                app.name = currentName;
                app.path = currentFilePath.trimmed(); // use the full path, remove possible spaces and other characters

                // check if it's exported - fix the logic: use the full path matching
                QString expectedExportedPath = QString("%1/.local/share/applications/%2-%3").arg(qgetenv("HOME").constData()).arg(containerName).arg(currentFile);
                app.exported = exportedApps.contains(expectedExportedPath);
                app.operationInProgress = false;

                apps.append(app);
                qDebug() << "Parsed app:" << app.name << "Path:" << app.path << "Expected exported path:" << expectedExportedPath << "Is exported:" << app.exported;
            }

            // reset the variables to start processing a new app
            currentFile = line.mid(12).trimmed(); // remove the "# START FILE " prefix and remove spaces
            currentFilePath = currentFile; // save the full path
            // extract the file name from the full path
            QStringList pathParts = currentFile.split("/");
            if (!pathParts.isEmpty()) {
                currentFile = pathParts.last();
            }
            currentName = "";
            currentExec = "";
            currentIcon = "";
        } else if (line.startsWith("Name=")) {
            currentName = line.mid(5); // remove the "Name=" prefix
        } else if (line.startsWith("Exec=")) {
            currentExec = line.mid(5); // remove the "Exec=" prefix
        } else if (line.startsWith("Icon=")) {
            currentIcon = line.mid(5); // remove the "Icon=" prefix
        }
    }

    // add the last app (if it exists)
    if (!currentFile.isEmpty() && !currentName.isEmpty()) {
        ExportableApp app;
        app.name = currentName;
        app.path = currentFilePath.trimmed(); // use the full path, remove possible spaces and other characters

        // check if it's exported - fix the logic: use the full path matching
        QString expectedExportedPath = QString("%1/.local/share/applications/%2-%3").arg(qgetenv("HOME").constData()).arg(containerName).arg(currentFile);
        app.exported = exportedApps.contains(expectedExportedPath);
        app.operationInProgress = false;

        apps.append(app);
        qDebug() << "Parsed app:" << app.name << "Path:" << app.path << "Expected exported path:" << expectedExportedPath << "Is exported:" << app.exported;
    }

    qDebug() << "Emitting app list with" << apps.size() << "apps";
    emit exportableAppsUpdated(apps);
}

void DistroboxManager::exportApp(const QString &containerName, const QString &desktopFilePath)
{
    qDebug() << "Exporting app:" << desktopFilePath << "from container:" << containerName;

    // create a process instance for this operation
    QProcess *exportProcess = new QProcess(this);

    connect(exportProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, containerName, desktopFilePath, exportProcess](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = exportProcess->readAllStandardOutput();
            qDebug() << "Export app output:" << output;
            emit exportAppResult(output);
            // let the QML side decide if it needs to reload the app list to avoid race conditions
            // listExportableApps(containerName);
        } else {
            QString error = exportProcess->readAllStandardError();
            QString stdOutput = exportProcess->readAllStandardOutput();
            qDebug() << "Export app error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            emit commandError("Failed to export app. Exit code: " + QString::number(exitCode) +
                              " Error: " + error + " Output: " + stdOutput);
        }
        // clean up the process
        exportProcess->deleteLater();
    });

    // use `distrobox enter` to enter the container and export the app
    QStringList command;
    command << "enter" << containerName << "--"
            << "distrobox-export" << "--app" << desktopFilePath;

    exportProcess->start("distrobox", command);
}

void DistroboxManager::unexportApp(const QString &containerName, const QString &desktopFilePath)
{
    qDebug() << "Unexporting app:" << desktopFilePath << "from container:" << containerName;

    // create a process instance for this operation
    QProcess *unexportProcess = new QProcess(this);

    connect(unexportProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, containerName, desktopFilePath, unexportProcess](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = unexportProcess->readAllStandardOutput();
            qDebug() << "Unexport app output:" << output;
            emit unexportAppResult(output);
            // let the QML side decide if it needs to reload the app list to avoid race conditions
            // listExportableApps(containerName);
        } else {
            QString error = unexportProcess->readAllStandardError();
            QString stdOutput = unexportProcess->readAllStandardOutput();
            qDebug() << "Unexport app error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            emit commandError("Failed to unexport app. Exit code: " + QString::number(exitCode) +
                              " Error: " + error + " Output: " + stdOutput);
        }
        // clean up the process
        unexportProcess->deleteLater();
    });

    // use `distrobox enter` to enter the container and unexport the app
    QStringList command;
    command << "enter" << containerName << "--"
            << "distrobox-export" << "-d" << "--app" << desktopFilePath;

    unexportProcess->start("distrobox", command);
}

bool isInFlatpak() {
    if (!qgetenv("FLATPAK_ID").isEmpty()) {
        return true;
    }

    return QFile::exists("/.flatpak-info");
}

void DistroboxManager::spawnTerminal(const QString &containerName)
{
    qDebug() << "Spawning terminal for container:" << containerName;

    // get the terminal config from StateManager
    StateManager* stateManager = StateManager::instance();
    TerminalManager* terminalManager = qobject_cast<TerminalManager*>(stateManager->getTerminalManager());
    if (!stateManager || !terminalManager) {
        qWarning() << "StateManager or TerminalManager not available";
        return;
    }

    QString selectedTerminal = terminalManager->selectedTerminal();
    TerminalConfig config = terminalManager->getTerminalConfig(selectedTerminal);

    qDebug() << "Using terminal:" << config.displayName << "(" << config.command << ")";

    // build the command to execute
    QStringList command;
    if (isInFlatpak()) {
        command << "flatpak-spawn" << "--host" << config.command << config.separator;
    }

    command << "distrobox" << "enter" << containerName;

    // start the terminal
    if (isInFlatpak()) {
        // use the correct parameters in Flatpak environment
        QProcess::startDetached("flatpak-spawn", QStringList() << "--host" << config.command << config.separator << "distrobox" << "enter" << containerName);
    } else {
        QProcess::startDetached(config.command, QStringList() << config.separator << "distrobox" << "enter" << containerName);
    }

    // send the terminal spawned signal
    emit terminalSpawned();
}

void DistroboxManager::upgradeContainer(const QString &containerName)
{
    qDebug() << "Upgrading container:" << containerName;

    // send the task started signal
    emit taskStarted("Upgrade Container", "Upgrading container " + containerName +
                     ". This may take some time as it needs to update packages...");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Container upgrade output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Container upgrade error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // disconnect all connections to avoid multiple triggers
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container upgrade completed output:" << output;
            emit taskOutputReceived(output);
            emit taskStatusUpdated("successful");
            emit taskFinished("successful");
            // reload the container list to show the updated state
            listContainers();
        } else {
            QString error = m_operationProcess->readAllStandardError();
            QString stdOutput = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container upgrade error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            QString errorMessage = "Failed to upgrade container. Exit code: " + QString::number(exitCode) +
                                   " Error: " + error + " Output: " + stdOutput;
            emit taskOutputReceived(errorMessage);
            emit taskStatusUpdated("failed");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    QStringList args;
    args << "upgrade" << containerName;
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::upgradeAllContainers()
{
    // Check if an operation is already running
    if (m_operationProcess->state() != QProcess::NotRunning) {
        qDebug() << "Operation process already running, cannot upgrade all containers";
        emit commandError("Another operation is already running. Please wait for it to complete.");
        return;
    }

    qDebug() << "Upgrading all containers";

    // send the task started signal
    emit taskStarted("Upgrade All Containers", "Upgrading all containers. This may take a very long time...");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Upgrade all output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Upgrade all error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // disconnect all connections to avoid multiple triggers
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = m_operationProcess->readAllStandardOutput();
            qDebug() << "Upgrade all completed output:" << output;
            emit taskOutputReceived(output);
            emit taskStatusUpdated("successful");
            emit taskFinished("successful");
            // reload the container list to show the updated state
            listContainers();
        } else {
            QString error = m_operationProcess->readAllStandardError();
            QString stdOutput = m_operationProcess->readAllStandardOutput();
            qDebug() << "Upgrade all error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            QString errorMessage = "Failed to upgrade all containers. Exit code: " + QString::number(exitCode) +
                                   " Error: " + error + " Output: " + stdOutput;
            emit taskOutputReceived(errorMessage);
            emit taskStatusUpdated("failed");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox upgrade --all` command
    QStringList args;
    args << "upgrade" << "--all";
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::cloneContainer(const QString &sourceContainer, const QString &newName)
{
    qDebug() << "Cloning container:" << sourceContainer << "to:" << newName;

    // send the task started signal
    emit taskStarted("Clone Container", "Cloning container " + sourceContainer + " to " + newName + "...");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Container clone output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Container clone error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // disconnect all connections to avoid multiple triggers
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container clone completed output:" << output;
            emit taskOutputReceived(output);
            emit taskStatusUpdated("successful");
            emit taskFinished("successful");
            emit containerCreated(); // cloning is also creating a new container
            // reload the container list to show the new cloned container
            listContainers();
        } else {
            QString error = m_operationProcess->readAllStandardError();
            QString stdOutput = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container clone error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            QString errorMessage = "Failed to clone container. Exit code: " + QString::number(exitCode) +
                                   " Error: " + error + " Output: " + stdOutput;
            emit taskOutputReceived(errorMessage);
            emit taskStatusUpdated("failed");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox create --clone` command
    QStringList args;
    args << "create" << "--clone" << sourceContainer << "--name" << newName;
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::stopContainer(const QString &containerName)
{
    qDebug() << "Stopping container:" << containerName;

    // send the task started signal
    emit taskStarted("Stop Container", "Stopping container " + containerName + "...");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Container stop output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Container stop error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // disconnect all connections to avoid multiple triggers
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container stop completed output:" << output;
            emit taskOutputReceived(output);
            emit taskStatusUpdated("successful");
            emit taskFinished("successful");
            // reload the container list to show the updated state
            listContainers();
        } else {
            QString error = m_operationProcess->readAllStandardError();
            QString stdOutput = m_operationProcess->readAllStandardOutput();
            qDebug() << "Container stop error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            QString errorMessage = "Failed to stop container. Exit code: " + QString::number(exitCode) +
                                   " Error: " + error + " Output: " + stdOutput;
            emit taskOutputReceived(errorMessage);
            emit taskStatusUpdated("failed");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox stop` command, add the -y parameter to auto-confirm
    QStringList args;
    args << "stop" << "--yes" << containerName;
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::stopAllContainers()
{
    qDebug() << "Stopping all containers";

    // send the task started signal
    emit taskStarted("Stop All Containers", "Stopping all running containers...");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Stop all output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Stop all error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // disconnect all connections to avoid multiple triggers
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = m_operationProcess->readAllStandardOutput();
            qDebug() << "Stop all completed output:" << output;
            emit taskOutputReceived(output);
            emit taskStatusUpdated("successful");
            emit taskFinished("successful");
            // reload the container list to show the updated state
            listContainers();
        } else {
            QString error = m_operationProcess->readAllStandardError();
            QString stdOutput = m_operationProcess->readAllStandardOutput();
            qDebug() << "Stop all error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            QString errorMessage = "Failed to stop all containers. Exit code: " + QString::number(exitCode) +
                                   " Error: " + error + " Output: " + stdOutput;
            emit taskOutputReceived(errorMessage);
            emit taskStatusUpdated("failed");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox stop --all` command, add the -y parameter to auto-confirm
    QStringList args;
    args << "stop" << "-y" << "--all";
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::assembleFromFile(const QString &filePath)
{
    qDebug() << "Assembling containers from file:" << filePath;

    // send the task started signal
    emit taskStarted("Assemble from File", "Creating containers from configuration file: " + filePath +
                     ". This may take some time as it needs to download container images...");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Assemble output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Assemble error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // disconnect all connections to avoid multiple triggers
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = m_operationProcess->readAllStandardOutput();
            qDebug() << "Assemble completed output:" << output;
            emit taskOutputReceived(output);
            emit taskStatusUpdated("successful");
            emit taskFinished("successful");
            emit containerCreated(); // Assemble is also creating a container
            // reload the container list to show the new created container
            listContainers();
        } else {
            QString error = m_operationProcess->readAllStandardError();
            QString stdOutput = m_operationProcess->readAllStandardOutput();
            qDebug() << "Assemble error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            QString errorMessage = "Failed to assemble containers. Exit code: " + QString::number(exitCode) +
                                   " Error: " + error + " Output: " + stdOutput;
            emit taskOutputReceived(errorMessage);
            emit taskStatusUpdated("failed");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox assemble` command
    QStringList args;
    args << "assemble" << "--file" << filePath;
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::assembleFromUrl(const QString &url)
{
    qDebug() << "Assembling containers from URL:" << url;

    // send the task started signal
    emit taskStarted("Assemble from URL", "Creating containers from URL: " + url +
                     ". This may take some time as it needs to download the configuration and container images...");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Assemble URL output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Assemble URL error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        // disconnect all connections to avoid multiple triggers
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = m_operationProcess->readAllStandardOutput();
            qDebug() << "Assemble URL completed output:" << output;
            emit taskOutputReceived(output);
            emit taskStatusUpdated("successful");
            emit taskFinished("successful");
            emit containerCreated(); // Assemble is also creating a container
            // reload the container list to show the new created container
            listContainers();
        } else {
            QString error = m_operationProcess->readAllStandardError();
            QString stdOutput = m_operationProcess->readAllStandardOutput();
            qDebug() << "Assemble URL error. Exit code:" << exitCode << "Error:" << error << "Stdout:" << stdOutput;
            QString errorMessage = "Failed to assemble from URL. Exit code: " + QString::number(exitCode) +
                                   " Error: " + error + " Output: " + stdOutput;
            emit taskOutputReceived(errorMessage);
            emit taskStatusUpdated("failed");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox assemble` command
    QStringList args;
    args << "assemble" << "--file" << url;
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::generateEntry(const QString &containerName, const QString &iconPath)
{
    if (containerName.isEmpty()) {
        qWarning() << "Container name cannot be empty for generate entry";
        return;
    }

    qDebug() << "Generating desktop entry for container:" << containerName << "with icon:" << iconPath;

    // send the task started signal
    emit taskStarted("Generate Entry", "Generating desktop entry for container " + containerName);

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Generate entry output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Generate entry error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, containerName](int exitCode, QProcess::ExitStatus exitStatus) {
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            qDebug() << "Desktop entry generated successfully for:" << containerName;
            emit taskStatusUpdated("Desktop entry generated successfully");
            emit taskFinished("successful");
            // maybe need to reload the container list or state
            listContainers();
        } else {
            QString errorMessage = QString("Failed to generate desktop entry for %1. Exit code: %2")
                                 .arg(containerName).arg(exitCode);
            qWarning() << errorMessage;
            emit taskStatusUpdated("Failed to generate desktop entry");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox generate-entry` command
    QStringList args;
    args << "generate-entry" << containerName;

    // if a custom icon path is provided, add the --icon parameter
    if (!iconPath.isEmpty()) {
        args << "--icon" << iconPath;
    }

    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::deleteEntry(const QString &containerName)
{
    if (containerName.isEmpty()) {
        qWarning() << "Container name cannot be empty for delete entry";
        return;
    }

    qDebug() << "Deleting desktop entry for container:" << containerName;

    // send the task started signal
    emit taskStarted("Delete Entry", "Deleting desktop entry for container " + containerName);

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Delete entry output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Delete entry error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, containerName](int exitCode, QProcess::ExitStatus exitStatus) {
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            qDebug() << "Desktop entry deleted successfully for:" << containerName;
            emit taskStatusUpdated("Desktop entry deleted successfully");
            emit taskFinished("successful");
        } else {
            QString errorMessage = QString("Failed to delete desktop entry for %1. Exit code: %2")
                                 .arg(containerName).arg(exitCode);
            qWarning() << errorMessage;
            emit taskStatusUpdated("Failed to delete desktop entry");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox generate-entry --delete` command
    QStringList args;
    args << "generate-entry" << containerName << "--delete";
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::generateAllEntries()
{
    qDebug() << "Generating desktop entries for all containers";

    // send the task started signal
    emit taskStarted("Generate All Entries", "Generating desktop entries for all containers");

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Generate all entries output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Generate all entries error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            qDebug() << "Desktop entries generated successfully for all containers";
            emit taskStatusUpdated("Desktop entries generated successfully for all containers");
            emit taskFinished("successful");
            // reload the container list
            listContainers();
        } else {
            QString errorMessage = QString("Failed to generate desktop entries for all containers. Exit code: %1")
                                 .arg(exitCode);
            qWarning() << errorMessage;
            emit taskStatusUpdated("Failed to generate desktop entries");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // build the `distrobox generate-entry --all` command
    QStringList args;
    args << "generate-entry" << "--all";
    m_operationProcess->start("distrobox", args);
}

void DistroboxManager::installPackage(const QString &containerName, const QString &packagePath)
{
    if (containerName.isEmpty() || packagePath.isEmpty()) {
        qWarning() << "Container name and package path cannot be empty for package installation";
        return;
    }

    qDebug() << "Installing package:" << packagePath << "into container:" << containerName;

    // send the task started signal
    emit taskStarted("Install Package", "Installing " + QFileInfo(packagePath).fileName() + " into container " + containerName);

    // disconnect all previous connections to avoid multiple triggers
    disconnect(m_operationProcess, nullptr, this, nullptr);

    connect(m_operationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = m_operationProcess->readAllStandardOutput();
        qDebug() << "Package install output:" << output;
        emit taskOutputReceived(output);
    });

    connect(m_operationProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString errorOutput = m_operationProcess->readAllStandardError();
        qDebug() << "Package install error output:" << errorOutput;
        emit taskOutputReceived(errorOutput);
    });

    connect(m_operationProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, containerName, packagePath](int exitCode, QProcess::ExitStatus exitStatus) {
        disconnect(m_operationProcess, nullptr, this, nullptr);

        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            qDebug() << "Package installed successfully into:" << containerName;
            emit taskStatusUpdated("Package installed successfully");
            emit taskFinished("successful");
        } else {
            QString errorMessage = QString("Failed to install package into %1. Exit code: %2")
                                 .arg(containerName).arg(exitCode);
            qWarning() << errorMessage;
            emit taskStatusUpdated("Failed to install package");
            emit taskFinished("failed");
            emit commandError(errorMessage);
        }
    });

    // get the package file extension to determine the package manager type
    QString extension = QFileInfo(packagePath).suffix().toLower();
    QString packageManager;
    QString installCmd;

    if (extension == "deb") {
        packageManager = "apt-get";
        installCmd = "sudo apt-get install -y";
    } else if (extension == "rpm") {
        packageManager = "dnf";
        installCmd = "sudo dnf install -y";
    } else {
        QString errorMessage = QString("Unsupported package format: .%1. Only .deb and .rpm are supported.").arg(extension);
        qWarning() << errorMessage;
        emit taskStatusUpdated("Unsupported package format");
        emit taskFinished("failed");
        emit commandError(errorMessage);
        return;
    }

    // generate a temporary file name
    QString tempFileName = QString("net.blumia.distro-rack.user_package.%1").arg(extension);
    QString tempPath = QString("/tmp/%1").arg(tempFileName);

    // build the composite command: copy the file to the container and then install
    QStringList args;
    args << "enter" << containerName << "--";
    args << "sh" << "-c";

    QString command = QString("cp '%1' '%2' && %3 '%2'")
                     .arg(packagePath)
                     .arg(tempPath)
                     .arg(installCmd);

    args << command;

    qDebug() << "Executing package install command:" << args;
    m_operationProcess->start("distrobox", args);
}
