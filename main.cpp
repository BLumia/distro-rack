// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlFileSelector>
#include <qtenvironmentvariables.h>
#include "DistroboxManager.h"
#include "StateManager.h"
#include "ContainerModel.h"
#include "ExportableAppsModel.h"
#include "TerminalManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // set the application information for QSettings
    QCoreApplication::setOrganizationName("DistroRack");
    QCoreApplication::setApplicationName("DistroRack");

    app.setWindowIcon(QIcon::fromTheme("terminal-distrobox-icon"));

    QQmlApplicationEngine engine;
    QQmlFileSelector* selector = new QQmlFileSelector(&engine);
    selector->setExtraSelectors(QStringList() << qgetenv("XDG_CURRENT_DESKTOP").toLower());

    // register types to QML
    qmlRegisterType<TaskModel>("DistroRack", 1, 0, "TaskModel");
    qmlRegisterUncreatableType<ContainerModel>("DistroRack", 1, 0, "ContainerModel",
                                               "ContainerModel is managed by StateManager");
    qmlRegisterUncreatableType<ExportableAppsModel>("DistroRack", 1, 0, "ExportableAppsModel",
                                                     "ExportableAppsModel is managed by StateManager");
    qmlRegisterUncreatableType<TerminalManager>("DistroRack", 1, 0, "TerminalManager",
                                                 "TerminalManager is managed by StateManager");
    qmlRegisterUncreatableType<StateManager>("DistroRack", 1, 0, "StateManager",
                                             "StateManager is a singleton, use StateManager.instance()");

    // create the state manager instance
    StateManager* stateManager = StateManager::instance();

    // expose the state manager to QML
    engine.rootContext()->setContextProperty("stateManager", stateManager);

    const QUrl url(QStringLiteral("qrc:/qt/qml/DistroRack/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    // Load containers when the application starts
    stateManager->distroboxManager()->listContainers();

    return app.exec();
}
