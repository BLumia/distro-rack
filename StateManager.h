// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QSettings>
#include <QVariant>

// Model includes
#include "TaskModel.h"
#include "ContainerModel.h"
#include "ExportableAppsModel.h"
#include "TerminalManager.h"

// Forward declarations
class DistroboxManager;

// central state manager
class StateManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(TaskModel* taskModel READ taskModel CONSTANT)
    Q_PROPERTY(ContainerModel* containerModel READ containerModel CONSTANT)
    Q_PROPERTY(ExportableAppsModel* exportableAppsModel READ exportableAppsModel CONSTANT)
    Q_PROPERTY(TerminalManager* terminalManager READ terminalManager CONSTANT)
    Q_PROPERTY(QString selectedContainerName READ selectedContainerName WRITE setSelectedContainerName NOTIFY selectedContainerNameChanged)
    Q_PROPERTY(QString selectedTaskId READ selectedTaskId WRITE setSelectedTaskId NOTIFY selectedTaskIdChanged)
    Q_PROPERTY(DistroboxManager* distroboxManager READ distroboxManager CONSTANT)

public:
    explicit StateManager(QObject *parent = nullptr);

    // get the singleton instance
    static StateManager* instance();

    // property accessors
    TaskModel* taskModel() const { return m_taskModel; }
    ContainerModel* containerModel() const { return m_containerModel; }
    ExportableAppsModel* exportableAppsModel() const { return m_exportableAppsModel; }
    TerminalManager* terminalManager() const { return m_terminalManager; }
    DistroboxManager* distroboxManager() const { return m_distroboxManager; }

    QString selectedContainerName() const { return m_selectedContainerName; }
    void setSelectedContainerName(const QString& name);

    QString selectedTaskId() const { return m_selectedTaskId; }
    void setSelectedTaskId(const QString& taskId);

    // settings management
    Q_INVOKABLE QVariant getSetting(const QString& key, const QVariant& defaultValue = QVariant()) const;
    Q_INVOKABLE void setSetting(const QString& key, const QVariant& value);

    // task management
    Q_INVOKABLE QString createTask(const QString& name, const QString& description, const QString& target = QString());
    Q_INVOKABLE void updateTaskStatus(const QString& taskId, const QString& status);
    Q_INVOKABLE void updateTaskOutput(const QString& taskId, const QString& output);
    Q_INVOKABLE void updateTaskError(const QString& taskId, const QString& errorMessage);
    Q_INVOKABLE void finishTask(const QString& taskId, bool success, const QString& errorMessage = QString());
    Q_INVOKABLE void clearEndedTasks();

    // terminal management - use method access to avoid Meta Object type issues
    Q_INVOKABLE QObject* getTerminalManager() const;

    // dialog state management
    Q_INVOKABLE void showTaskManager();
    Q_INVOKABLE void showExportableApps(const QString& containerName);
    Q_INVOKABLE void showCreateContainer();

signals:
    void selectedContainerNameChanged();
    void selectedTaskIdChanged();
    void taskManagerRequested();
    void exportableAppsRequested(const QString& containerName);
    void createContainerRequested();

private slots:
    void onTaskStarted(const QString& name, const QString& description);
    void onTaskOutputReceived(const QString& output);
    void onTaskStatusUpdated(const QString& status);
    void onTaskFinished(const QString& status);

private:
    static StateManager* s_instance;

    TaskModel* m_taskModel;
    ContainerModel* m_containerModel;
    ExportableAppsModel* m_exportableAppsModel;
    TerminalManager* m_terminalManager;
    DistroboxManager* m_distroboxManager;
    QSettings* m_settings;

    QString m_selectedContainerName;
    QString m_selectedTaskId;
    QString m_currentTaskId; // the ID of the currently executing task

    // helper methods
    QString generateTaskId() const;
    void connectDistroboxManagerSignals();
};
