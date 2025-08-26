// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QSettings>
#include <QVariant>

// terminal configuration structure
struct TerminalConfig {
    QString name;
    QString displayName;
    QString command;
    QString separator;
    bool isCustom;

    TerminalConfig() = default;
    TerminalConfig(const QString& name, const QString& displayName, const QString& command, const QString& separator, bool isCustom = false)
        : name(name), displayName(displayName), command(command), separator(separator), isCustom(isCustom) {}
};

// terminal model class
class TerminalModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        DisplayNameRole,
        CommandRole,
        SeparatorRole,
        IsCustomRole
    };
    Q_ENUM(Roles)

    explicit TerminalModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // terminal management methods
    Q_INVOKABLE void addTerminal(const TerminalConfig& config);
    Q_INVOKABLE void removeTerminal(const QString& name);
    Q_INVOKABLE void updateTerminal(const QString& name, const TerminalConfig& config);
    Q_INVOKABLE QVariantMap getTerminal(int index) const;
    Q_INVOKABLE QVariantMap getTerminalByName(const QString& name) const;
    Q_INVOKABLE int getTerminalIndex(const QString& name) const;
    Q_INVOKABLE void loadTerminals();
    Q_INVOKABLE void saveTerminals();

    // property access
    const QList<TerminalConfig>& terminals() const { return m_terminals; }

signals:
    void terminalsChanged();
    void terminalAdded(const QString& name);
    void terminalRemoved(const QString& name);
    void terminalUpdated(const QString& name);

private:
    QList<TerminalConfig> m_terminals;
    QSettings* m_settings;

    int findTerminalIndex(const QString& name) const;
    void initializeDefaultTerminals();
    TerminalConfig variantMapToTerminal(const QVariantMap& map) const;
    QVariantMap terminalToVariantMap(const TerminalConfig& terminal) const;
};

// terminal manager
class TerminalManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(TerminalModel* terminalModel READ terminalModel CONSTANT)
    Q_PROPERTY(QString selectedTerminal READ selectedTerminal WRITE setSelectedTerminal NOTIFY selectedTerminalChanged)
    Q_PROPERTY(QString defaultTerminal READ defaultTerminal NOTIFY defaultTerminalChanged)

public:
    explicit TerminalManager(QObject *parent = nullptr);

    // property accessors
    TerminalModel* terminalModel() const { return m_terminalModel; }
    QString selectedTerminal() const { return m_selectedTerminal; }
    void setSelectedTerminal(const QString& name);
    QString defaultTerminal() const;

    // terminal operation methods
    Q_INVOKABLE bool addCustomTerminal(const QString& name, const QString& displayName, const QString& command, const QString& separator);
    Q_INVOKABLE bool removeCustomTerminal(const QString& name);
    Q_INVOKABLE bool updateCustomTerminal(const QString& name, const QString& displayName, const QString& command, const QString& separator);
    Q_INVOKABLE QStringList getAvailableTerminals() const;
    Q_INVOKABLE bool isTerminalAvailable(const QString& command) const;
    Q_INVOKABLE QString detectDefaultTerminal() const;
    Q_INVOKABLE void resetToDefaults();

    // terminal launch methods
    Q_INVOKABLE TerminalConfig getTerminalConfig(const QString& name) const;

signals:
    void selectedTerminalChanged();
    void defaultTerminalChanged();
    void terminalConfigChanged();

private slots:
    void onTerminalsChanged();

private:
    TerminalModel* m_terminalModel;
    QSettings* m_settings;
    QString m_selectedTerminal;

    void loadSettings();
    void saveSettings();
    bool isCommandAvailable(const QString& command) const;
};
