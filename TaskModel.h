// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QDateTime>
#include <QVariant>

// task status enum - needs to be defined in a namespace to use Q_ENUM_NS
namespace TaskManagerEnums {
    Q_NAMESPACE
    enum class TaskStatus {
        Pending,
        Executing,
        Successful,
        Failed
    };
    Q_ENUM_NS(TaskStatus)
}
using TaskStatus = TaskManagerEnums::TaskStatus;

// task information structure
struct Task {
    QString id;
    QString name;
    QString description;
    QString target;  // target container name
    TaskStatus status;
    QString output;
    QString errorMessage;
    QDateTime startTime;
    QDateTime endTime;

    Task() = default;
    Task(const QString& id, const QString& name, const QString& description, const QString& target = QString())
        : id(id), name(name), description(description), target(target)
        , status(TaskStatus::Pending), startTime(QDateTime::currentDateTime()) {}

    bool isEnded() const {
        return status == TaskStatus::Successful || status == TaskStatus::Failed;
    }

    QString statusString() const {
        switch (status) {
            case TaskStatus::Pending: return "pending";
            case TaskStatus::Executing: return "executing";
            case TaskStatus::Successful: return "successful";
            case TaskStatus::Failed: return "failed";
        }
        return "unknown";
    }
};

// task model class
class TaskModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        TargetRole,
        StatusRole,
        StatusStringRole,
        OutputRole,
        ErrorMessageRole,
        StartTimeRole,
        EndTimeRole,
        IsEndedRole
    };
    Q_ENUM(Roles)

    explicit TaskModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // task management methods
    Q_INVOKABLE void addTask(const Task& task);
    Q_INVOKABLE void updateTask(const QString& taskId, const Task& task);
    Q_INVOKABLE void removeTask(const QString& taskId);
    Q_INVOKABLE void clearEndedTasks();
    Q_INVOKABLE Task* getTask(const QString& taskId);
    Q_INVOKABLE int getTaskIndex(const QString& taskId) const;

    // property access
    const QList<Task>& tasks() const { return m_tasks; }

signals:
    void taskAdded(const QString& taskId);
    void taskUpdated(const QString& taskId);
    void taskRemoved(const QString& taskId);
    void taskStatusChanged(const QString& taskId, const QString& status);

private:
    QList<Task> m_tasks;

    int findTaskIndex(const QString& taskId) const;
};

