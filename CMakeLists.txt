cmake_minimum_required(VERSION 3.16)

project(distro-rack VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Quick LinguistTools)

# Enable automatic moc, rcc and uic
qt6_standard_project_setup()

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