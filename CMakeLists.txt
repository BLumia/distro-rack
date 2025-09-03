# SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

project(distro-rack VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include GNUInstallDirs for standard installation directories
include(GNUInstallDirs)

# Find required packages
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Quick QuickControls2 LinguistTools)

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
        qml/NoContainersView.qml
        qml/ThemedIcon.qml
        qml/+dde/ThemedIcon.qml
        qml/+kde/ThemedIcon.qml
        qml/SettingsDialog.qml
        qml/TerminalSettingsTab.qml
        qml/ActionButton.qml
)

# Define package version for C++
target_compile_definitions(distro-rack PRIVATE DISTRO_RACK_VERSION="${PROJECT_VERSION}")

# Add translations
qt6_add_translations(distro-rack
    TS_FILE_DIR translations # Before Qt 6.9, the argument TS_OUTPUT_DIRECTORY was called TS_FILE_DIR. That name is still an alias for TS_OUTPUT_DIRECTORY to keep older project files working.
    RESOURCE_PREFIX "/i18n"
)

# Link libraries
target_link_libraries(distro-rack PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
)

# Install binary
install(TARGETS distro-rack
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install desktop file
install(FILES dist/net.blumia.distro-rack.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

# Install AppStream metainfo file
install(FILES dist/net.blumia.distro-rack.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
)

# CPack configuration for creating packages
# Basic package information
set(CPACK_PACKAGE_NAME "distro-rack")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Gary Wang")
set(CPACK_PACKAGE_CONTACT "opensource@blumia.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Qt6-based GUI for managing Distrobox containers")
set(CPACK_PACKAGE_DESCRIPTION "DistroRack is a modern Qt6-based graphical user interface for managing Distrobox containers. It provides an intuitive way to create, manage, and interact with containerized Linux distributions.")

# DEB package specific settings
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Gary Wang <opensource@blumia.net>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/blumia/distro-rack")

# Enable automatic dependency detection
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# Keep distrobox as explicit dependency since it's a runtime requirement not detected by shlibdeps
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "qml6-module-qtquick"
    "qml6-module-qtquick-controls"
    "qml6-module-qtquick-dialogs"
    "qml6-module-qtquick-layouts"
    "qml6-module-qtquick-window"
    "distrobox")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "libdtk6declarative")

# License file
if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()

# README file
if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
endif()

# Include CPack module AFTER setting all variables
include(CPack)
