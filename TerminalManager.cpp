// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "TerminalManager.h"
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QVariantMap>
#include <QUuid>

TerminalModel::TerminalModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_settings(new QSettings(this))
{
    initializeDefaultTerminals();
}

int TerminalModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_terminals.size();
}

QVariant TerminalModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_terminals.size())
        return QVariant();

    const TerminalConfig& terminal = m_terminals[index.row()];

    switch (role) {
    case NameRole:
        return terminal.name;
    case DisplayNameRole:
        return terminal.displayName;
    case CommandRole:
        return terminal.command;
    case SeparatorRole:
        return terminal.separator;
    case IsCustomRole:
        return terminal.isCustom;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> TerminalModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[DisplayNameRole] = "displayName";
    roles[CommandRole] = "command";
    roles[SeparatorRole] = "separator";
    roles[IsCustomRole] = "isCustom";
    return roles;
}

void TerminalModel::addTerminal(const TerminalConfig& config)
{
    // check if a terminal with the same name already exists
    if (findTerminalIndex(config.name) >= 0) {
        qWarning() << "Terminal with name" << config.name << "already exists";
        return;
    }

    beginInsertRows(QModelIndex(), m_terminals.size(), m_terminals.size());
    m_terminals.append(config);
    endInsertRows();

    saveTerminals();
    emit terminalsChanged();
    emit terminalAdded(config.name);
    qDebug() << "Terminal added:" << config.name;
}

void TerminalModel::removeTerminal(const QString& name)
{
    int index = findTerminalIndex(name);
    if (index < 0) {
        qWarning() << "Terminal not found:" << name;
        return;
    }

    // do not allow deleting non-custom terminals
    if (!m_terminals[index].isCustom) {
        qWarning() << "Cannot remove built-in terminal:" << name;
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_terminals.removeAt(index);
    endRemoveRows();

    saveTerminals();
    emit terminalsChanged();
    emit terminalRemoved(name);
    qDebug() << "Terminal removed:" << name;
}

void TerminalModel::updateTerminal(const QString& name, const TerminalConfig& config)
{
    int index = findTerminalIndex(name);
    if (index < 0) {
        qWarning() << "Terminal not found:" << name;
        return;
    }

    // do not allow modifying the core attributes of non-custom terminals
    if (!m_terminals[index].isCustom) {
        qWarning() << "Cannot modify built-in terminal:" << name;
        return;
    }

    m_terminals[index] = config;
    m_terminals[index].isCustom = true; // ensure it's marked as custom

    QModelIndex modelIndex = this->index(index);
    emit dataChanged(modelIndex, modelIndex);

    saveTerminals();
    emit terminalsChanged();
    emit terminalUpdated(name);
    qDebug() << "Terminal updated:" << name;
}

QVariantMap TerminalModel::getTerminal(int index) const
{
    if (index < 0 || index >= m_terminals.size())
        return QVariantMap();

    return terminalToVariantMap(m_terminals[index]);
}

QVariantMap TerminalModel::getTerminalByName(const QString& name) const
{
    int index = findTerminalIndex(name);
    return index >= 0 ? getTerminal(index) : QVariantMap();
}

int TerminalModel::getTerminalIndex(const QString& name) const
{
    return findTerminalIndex(name);
}

void TerminalModel::loadTerminals()
{
    beginResetModel();

    // first load the default terminals
    initializeDefaultTerminals();

    // load custom terminals
    m_settings->beginGroup("CustomTerminals");
    QStringList customTerminals = m_settings->childGroups();

    for (const QString& terminalId : customTerminals) {
        m_settings->beginGroup(terminalId);

        TerminalConfig config;
        config.name = m_settings->value("name").toString();
        config.displayName = m_settings->value("displayName").toString();
        config.command = m_settings->value("command").toString();
        config.separator = m_settings->value("separator").toString();
        config.isCustom = true;

        // check if it already exists (avoid duplicates)
        if (findTerminalIndex(config.name) < 0 && !config.name.isEmpty()) {
            m_terminals.append(config);
        }

        m_settings->endGroup();
    }

    m_settings->endGroup();

    endResetModel();
    emit terminalsChanged();
    qDebug() << "Terminals loaded, count:" << m_terminals.size();
}

void TerminalModel::saveTerminals()
{
    // only save custom terminals
    m_settings->beginGroup("CustomTerminals");
    m_settings->remove(""); // clear existing configurations

    int customIndex = 0;
    for (const TerminalConfig& terminal : m_terminals) {
        if (terminal.isCustom) {
            QString terminalId = QString("terminal_%1").arg(customIndex++);
            m_settings->beginGroup(terminalId);

            m_settings->setValue("name", terminal.name);
            m_settings->setValue("displayName", terminal.displayName);
            m_settings->setValue("command", terminal.command);
            m_settings->setValue("separator", terminal.separator);

            m_settings->endGroup();
        }
    }

    m_settings->endGroup();
    qDebug() << "Custom terminals saved";
}

int TerminalModel::findTerminalIndex(const QString& name) const
{
    for (int i = 0; i < m_terminals.size(); ++i) {
        if (m_terminals[i].name == name) {
            return i;
        }
    }
    return -1;
}

void TerminalModel::initializeDefaultTerminals()
{
    m_terminals.clear();

    // add predefined terminal configurations
    m_terminals.append(TerminalConfig("gnome-terminal", "GNOME Terminal", "gnome-terminal", "--", false));
    m_terminals.append(TerminalConfig("konsole", "Konsole", "konsole", "-e", false));
    m_terminals.append(TerminalConfig("xfce4-terminal", "Xfce Terminal", "xfce4-terminal", "-x", false));
    m_terminals.append(TerminalConfig("deepin-terminal", "Deepin Terminal", "deepin-terminal", "-e", false));
    m_terminals.append(TerminalConfig("kitty", "Kitty", "kitty", "--", false));
    m_terminals.append(TerminalConfig("alacritty", "Alacritty", "alacritty", "-e", false));
    m_terminals.append(TerminalConfig("tilix", "Tilix", "tilix", "-e", false));
    m_terminals.append(TerminalConfig("terminator", "Terminator", "terminator", "-x", false));
}

TerminalConfig TerminalModel::variantMapToTerminal(const QVariantMap& map) const
{
    TerminalConfig config;
    config.name = map.value("name").toString();
    config.displayName = map.value("displayName").toString();
    config.command = map.value("command").toString();
    config.separator = map.value("separator").toString();
    config.isCustom = map.value("isCustom").toBool();

    return config;
}

QVariantMap TerminalModel::terminalToVariantMap(const TerminalConfig& terminal) const
{
    QVariantMap map;
    map["name"] = terminal.name;
    map["displayName"] = terminal.displayName;
    map["command"] = terminal.command;
    map["separator"] = terminal.separator;
    map["isCustom"] = terminal.isCustom;

    return map;
}

TerminalManager::TerminalManager(QObject *parent)
    : QObject(parent)
    , m_terminalModel(new TerminalModel(this))
    , m_settings(new QSettings(this))
{
    connect(m_terminalModel, &TerminalModel::terminalsChanged,
            this, &TerminalManager::onTerminalsChanged);

    loadSettings();
    m_terminalModel->loadTerminals();
}

void TerminalManager::setSelectedTerminal(const QString& name)
{
    if (m_selectedTerminal != name) {
        m_selectedTerminal = name;
        saveSettings();
        emit selectedTerminalChanged();
        qDebug() << "Selected terminal changed to:" << name;
    }
}

QString TerminalManager::defaultTerminal() const
{
    return detectDefaultTerminal();
}

bool TerminalManager::addCustomTerminal(const QString& name, const QString& displayName, const QString& command, const QString& separator)
{
    if (name.isEmpty() || command.isEmpty()) {
        qWarning() << "Name and command cannot be empty";
        return false;
    }

    // check if the command is available
    if (!isCommandAvailable(command)) {
        qWarning() << "Command not available:" << command;
        return false;
    }

    TerminalConfig config(name, displayName, command, separator, true);
    m_terminalModel->addTerminal(config);

    return true;
}

bool TerminalManager::removeCustomTerminal(const QString& name)
{
    // check if it's a custom terminal
    QVariantMap terminal = m_terminalModel->getTerminalByName(name);
    if (terminal.isEmpty() || !terminal["isCustom"].toBool()) {
        qWarning() << "Cannot remove built-in terminal:" << name;
        return false;
    }

    m_terminalModel->removeTerminal(name);

    // if the deleted terminal is the currently selected one, switch to the default terminal
    if (m_selectedTerminal == name) {
        setSelectedTerminal(detectDefaultTerminal());
    }

    return true;
}

bool TerminalManager::updateCustomTerminal(const QString& name, const QString& displayName, const QString& command, const QString& separator)
{
    if (name.isEmpty() || command.isEmpty()) {
        qWarning() << "Name and command cannot be empty";
        return false;
    }

    // check if it's a custom terminal
    QVariantMap terminal = m_terminalModel->getTerminalByName(name);
    if (terminal.isEmpty() || !terminal["isCustom"].toBool()) {
        qWarning() << "Cannot update built-in terminal:" << name;
        return false;
    }

    // check if the command is available
    if (!isCommandAvailable(command)) {
        qWarning() << "Command not available:" << command;
        return false;
    }

    TerminalConfig config(name, displayName, command, separator, true);
    m_terminalModel->updateTerminal(name, config);

    return true;
}

QStringList TerminalManager::getAvailableTerminals() const
{
    QStringList available;

    for (const TerminalConfig& terminal : m_terminalModel->terminals()) {
        if (isCommandAvailable(terminal.command)) {
            available.append(terminal.name);
        }
    }

    return available;
}

bool TerminalManager::isTerminalAvailable(const QString& command) const
{
    return isCommandAvailable(command);
}

QString TerminalManager::detectDefaultTerminal() const
{
    // TODO: this is not correct...
    // first try to detect the default terminal (GNOME environment) via gsettings
    QProcess process;
    process.start("gsettings", QStringList() << "get" << "org.gnome.desktop.default-applications.terminal" << "exec");
    process.waitForFinished(3000);

    if (process.exitCode() == 0) {
        QString output = process.readAllStandardOutput().trimmed();
        output = output.remove('\'').remove('"'); // remove quotes

        // find the matching terminal in the terminal list
        for (const TerminalConfig& terminal : m_terminalModel->terminals()) {
            if (terminal.command == output || terminal.name == output) {
                qDebug() << "Detected default terminal from gsettings:" << terminal.name;
                return terminal.name;
            }
        }
    }

    // fallback to detecting available terminals
    QStringList availableTerminals = getAvailableTerminals();
    if (!availableTerminals.isEmpty()) {
        qDebug() << "Using first available terminal:" << availableTerminals.first();
        return availableTerminals.first();
    }

    // fallback to gnome-terminal
    qDebug() << "Using fallback terminal: gnome-terminal";
    return "gnome-terminal";
}

void TerminalManager::resetToDefaults()
{
    // clear all custom terminals
    QList<TerminalConfig> customTerminals;
    for (const TerminalConfig& terminal : m_terminalModel->terminals()) {
        if (terminal.isCustom) {
            customTerminals.append(terminal);
        }
    }

    for (const TerminalConfig& terminal : customTerminals) {
        m_terminalModel->removeTerminal(terminal.name);
    }

    // reset the selected terminal
    setSelectedTerminal(detectDefaultTerminal());

    emit terminalConfigChanged();
}

TerminalConfig TerminalManager::getTerminalConfig(const QString& name) const
{
    for (const TerminalConfig& terminal : m_terminalModel->terminals()) {
        if (terminal.name == name) {
            return terminal;
        }
    }

    // return the default configuration
    return TerminalConfig("gnome-terminal", "GNOME Terminal", "gnome-terminal", "--", false);
}

void TerminalManager::onTerminalsChanged()
{
    emit terminalConfigChanged();
}

void TerminalManager::loadSettings()
{
    m_selectedTerminal = m_settings->value("selected-terminal", detectDefaultTerminal()).toString();
}

void TerminalManager::saveSettings()
{
    m_settings->setValue("selected-terminal", m_selectedTerminal);
}

bool TerminalManager::isCommandAvailable(const QString& command) const
{
    // use the `which` command to check if the program is available
    QProcess process;
    process.start("which", QStringList() << command);
    process.waitForFinished(3000);

    return process.exitCode() == 0;
}
