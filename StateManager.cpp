// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "StateManager.h"
#include "DistroboxManager.h"
#include "TerminalManager.h"
#include <QUuid>
#include <QDateTime>
#include <QDebug>
#include <QVariantMap>

// static member initialization
StateManager* StateManager::s_instance = nullptr;

StateManager::StateManager(QObject *parent)
    : QObject(parent)
    , m_taskModel(new TaskModel(this))
    , m_containerModel(new ContainerModel(this))
    , m_exportableAppsModel(new ExportableAppsModel(this))
    , m_terminalManager(new TerminalManager(this))
    , m_distroboxManager(new DistroboxManager(this))
    , m_settings(new QSettings(this))
{
    s_instance = this;
    connectDistroboxManagerSignals();
}

StateManager* StateManager::instance()
{
    if (!s_instance) {
        s_instance = new StateManager();
    }
    return s_instance;
}

void StateManager::setSelectedContainerName(const QString& name)
{
    if (m_selectedContainerName != name) {
        m_selectedContainerName = name;
        emit selectedContainerNameChanged();
        qDebug() << "Selected container changed to:" << name;
    }
}

void StateManager::setSelectedTaskId(const QString& taskId)
{
    if (m_selectedTaskId != taskId) {
        m_selectedTaskId = taskId;
        emit selectedTaskIdChanged();
        qDebug() << "Selected task changed to:" << taskId;
    }
}

QVariant StateManager::getSetting(const QString& key, const QVariant& defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void StateManager::setSetting(const QString& key, const QVariant& value)
{
    m_settings->setValue(key, value);
    qDebug() << "Setting saved:" << key << "=" << value;
}

QString StateManager::createTask(const QString& name, const QString& description, const QString& target)
{
    QString taskId = generateTaskId();
    Task task(taskId, name, description, target);
    m_taskModel->addTask(task);

    qDebug() << "Task created:" << taskId << name;
    return taskId;
}

void StateManager::updateTaskStatus(const QString& taskId, const QString& status)
{
    Task* task = m_taskModel->getTask(taskId);
    if (!task) return;

    if (status == "pending") {
        task->status = TaskStatus::Pending;
    } else if (status == "executing") {
        task->status = TaskStatus::Executing;
    } else if (status == "successful") {
        task->status = TaskStatus::Successful;
        task->endTime = QDateTime::currentDateTime();
    } else if (status == "failed") {
        task->status = TaskStatus::Failed;
        task->endTime = QDateTime::currentDateTime();
    }

    m_taskModel->updateTask(taskId, *task);
    qDebug() << "Task status updated:" << taskId << status;
}

void StateManager::updateTaskOutput(const QString& taskId, const QString& output)
{
    Task* task = m_taskModel->getTask(taskId);
    if (!task) return;

    task->output += output;
    m_taskModel->updateTask(taskId, *task);
}

void StateManager::updateTaskError(const QString& taskId, const QString& errorMessage)
{
    Task* task = m_taskModel->getTask(taskId);
    if (!task) return;

    task->errorMessage = errorMessage;
    m_taskModel->updateTask(taskId, *task);
    qDebug() << "Task error updated:" << taskId << errorMessage;
}

void StateManager::finishTask(const QString& taskId, bool success, const QString& errorMessage)
{
    Task* task = m_taskModel->getTask(taskId);
    if (!task) return;

    task->status = success ? TaskStatus::Successful : TaskStatus::Failed;
    task->endTime = QDateTime::currentDateTime();
    if (!success && !errorMessage.isEmpty()) {
        task->errorMessage = errorMessage;
    }

    m_taskModel->updateTask(taskId, *task);
    qDebug() << "Task finished:" << taskId << (success ? "successfully" : "with error");
}

void StateManager::clearEndedTasks()
{
    m_taskModel->clearEndedTasks();
    qDebug() << "Ended tasks cleared";
}



QObject* StateManager::getTerminalManager() const
{
    return m_terminalManager;
}

void StateManager::showTaskManager()
{
    emit taskManagerRequested();
}

void StateManager::showExportableApps(const QString& containerName)
{
    emit exportableAppsRequested(containerName);
}

void StateManager::showCreateContainer()
{
    emit createContainerRequested();
}

void StateManager::onTaskStarted(const QString& name, const QString& description)
{
    // create a new task and set it as the current task
    m_currentTaskId = createTask(name, description, m_selectedContainerName);
    updateTaskStatus(m_currentTaskId, "executing");

    // automatically open the task manager to show the new task
    showTaskManager();
}

void StateManager::onTaskOutputReceived(const QString& output)
{
    if (!m_currentTaskId.isEmpty()) {
        updateTaskOutput(m_currentTaskId, output);
    }
}

void StateManager::onTaskStatusUpdated(const QString& status)
{
    if (!m_currentTaskId.isEmpty()) {
        updateTaskStatus(m_currentTaskId, status);
    }
}

void StateManager::onTaskFinished(const QString& status)
{
    if (!m_currentTaskId.isEmpty()) {
        updateTaskStatus(m_currentTaskId, status);
        if (status == "successful" || status == "failed") {
            m_currentTaskId.clear(); // clear the current task ID
        }
    }
}

QString StateManager::generateTaskId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void StateManager::connectDistroboxManagerSignals()
{
    // connect container-related signals - use the strong type version
    connect(m_distroboxManager, QOverload<const QList<Container>&>::of(&DistroboxManager::containerListUpdated),
            this, [this](const QList<Container>& containers) {
                m_containerModel->updateContainers(containers);
            });

    // connect application-related signals - use the strong type version
    connect(m_distroboxManager, QOverload<const QList<ExportableApp>&>::of(&DistroboxManager::exportableAppsUpdated),
            this, [this](const QList<ExportableApp>& apps) {
                m_exportableAppsModel->updateApps(apps);
            });

    // connect task-related signals
    connect(m_distroboxManager, &DistroboxManager::taskStarted,
            this, &StateManager::onTaskStarted);
    connect(m_distroboxManager, &DistroboxManager::taskOutputReceived,
            this, &StateManager::onTaskOutputReceived);
    connect(m_distroboxManager, &DistroboxManager::taskStatusUpdated,
            this, &StateManager::onTaskStatusUpdated);
    connect(m_distroboxManager, &DistroboxManager::taskFinished,
            this, &StateManager::onTaskFinished);
}
