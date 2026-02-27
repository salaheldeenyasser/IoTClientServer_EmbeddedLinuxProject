cmake_minimum_required(VERSION 3.16)

project(IoTServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt6 modules ────────────────────────────────────────────────────────────
find_package(QT NAMES Qt6 REQUIRED COMPONENTS
    Widgets Charts Quick QuickWidgets Network Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets Charts Quick QuickWidgets Network Qml)

# ── Sources ─────────────────────────────────────────────────────────────────
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# ── Executable ───────────────────────────────────────────────────────────────
qt_add_executable(IoTServer
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    Photos.qrc        # background image + social-media icons
    Channel.h
    Socket.h
)

# ── Link ─────────────────────────────────────────────────────────────────────
target_link_libraries(IoTServer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Qml
)

set_target_properties(IoTServer PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

include(GNUInstallDirs)
install(TARGETS IoTServer
    BUNDLE  DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(IoTServer)
