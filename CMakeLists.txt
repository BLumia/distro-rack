# SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

project(distro-rack VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Quick LinguistTools)

# Enable automatic moc, rcc and uic
qt6_standard_project_setup(
    I18N_TRANSLATED_LANGUAGES en zh_CN
)

# Add the executable
qt6_add_executable(distro-rack
    main.cpp
    DistroboxManager.cpp
    StateManager.cpp
    TaskModel.cpp
    ContainerModel.cpp
    ExportableAppsModel.cpp
    TerminalManager.cpp
    KnownDistros.cpp
)

# Add QML files as a module
qt6_add_qml_module(distro-rack
    URI DistroRack
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml/"
    QML_FILES
        qml/Main.qml
        qml/+dde/Main.qml
        qml/ContainerList.qml
        qml/ContainerDetails.qml
        qml/CreateContainerDialog.qml
        qml/ExportableAppsDialog.qml
        qml/TaskManagerDialog.qml
)

# Add translations
qt6_add_translations(distro-rack
    TS_FILE_DIR translations # Before Qt 6.9, the argument TS_OUTPUT_DIRECTORY was called TS_FILE_DIR. That name is still an alias for TS_OUTPUT_DIRECTORY to keep older project files working.
    RESOURCE_PREFIX "/i18n"
)

# Link libraries
target_link_libraries(distro-rack PRIVATE
    Qt6::Core
    Qt6::Quick
)

# Install binary
install(TARGETS distro-rack
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
