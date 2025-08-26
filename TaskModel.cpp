// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "TaskModel.h"
#include <QDebug>

TaskModel::TaskModel(QObject *parent) : QAbstractListModel(parent)
{
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_tasks.size();
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_tasks.size())
        return QVariant();

    const Task& task = m_tasks[index.row()];

    switch (role) {
    case IdRole:
        return task.id;
    case NameRole:
        return task.name;
    case DescriptionRole:
        return task.description;
    case TargetRole:
        return task.target;
    case StatusRole:
        return static_cast<int>(task.status);
    case StatusStringRole:
        return task.statusString();
    case OutputRole:
        return task.output;
    case ErrorMessageRole:
        return task.errorMessage;
    case StartTimeRole:
        return task.startTime;
    case EndTimeRole:
        return task.endTime;
    case IsEndedRole:
        return task.isEnded();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> TaskModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "taskId";
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    roles[TargetRole] = "target";
    roles[StatusRole] = "status";
    roles[StatusStringRole] = "statusString";
    roles[OutputRole] = "output";
    roles[ErrorMessageRole] = "errorMessage";
    roles[StartTimeRole] = "startTime";
    roles[EndTimeRole] = "endTime";
    roles[IsEndedRole] = "isEnded";
    return roles;
}

void TaskModel::addTask(const Task& task)
{
    beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
    m_tasks.append(task);
    endInsertRows();

    emit taskAdded(task.id);
}

void TaskModel::updateTask(const QString& taskId, const Task& task)
{
    int index = findTaskIndex(taskId);
    if (index >= 0) {
        m_tasks[index] = task;
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex);
        emit taskUpdated(taskId);
        emit taskStatusChanged(taskId, task.statusString());
    }
}

void TaskModel::removeTask(const QString& taskId)
{
    int index = findTaskIndex(taskId);
    if (index >= 0) {
        beginRemoveRows(QModelIndex(), index, index);
        m_tasks.removeAt(index);
        endRemoveRows();
        emit taskRemoved(taskId);
    }
}

void TaskModel::clearEndedTasks()
{
    for (int i = m_tasks.size() - 1; i >= 0; --i) {
        if (m_tasks[i].isEnded()) {
            beginRemoveRows(QModelIndex(), i, i);
            QString taskId = m_tasks[i].id;
            m_tasks.removeAt(i);
            endRemoveRows();
            emit taskRemoved(taskId);
        }
    }
}

Task* TaskModel::getTask(const QString& taskId)
{
    int index = findTaskIndex(taskId);
    return index >= 0 ? &m_tasks[index] : nullptr;
}

int TaskModel::getTaskIndex(const QString& taskId) const
{
    return findTaskIndex(taskId);
}

int TaskModel::findTaskIndex(const QString& taskId) const
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == taskId) {
            return i;
        }
    }
    return -1;
}
