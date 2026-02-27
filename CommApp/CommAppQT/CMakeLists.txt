cmake_minimum_required(VERSION 3.16)

project(IoTServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt6 modules ───────────────────────────────────────────────────────────────
# NOTE: Qt6::Network is intentionally NOT listed.
#       All networking is done through the project OOP classes in Socket.h /
#       Channel.h which use POSIX socket syscalls directly.
#       QSocketNotifier (part of Qt6::Core) integrates those fds into the
#       Qt event loop — no Qt networking module is required.
find_package(QT NAMES Qt6 REQUIRED COMPONENTS
    Widgets Charts Quick QuickWidgets Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets Charts Quick QuickWidgets Qml)

# ── Sources ───────────────────────────────────────────────────────────────────
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    Socket.h      # Abstract Socket + TCPSocket + UDPSocket  (POSIX)
    Channel.h     # Abstract Channel + ServerChannel + ClientChannel
)

# ── Executable ────────────────────────────────────────────────────────────────
qt_add_executable(IoTServer
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    Photos.qrc    # background image + social-media icons + Gauge.qml
)

# ── Link ──────────────────────────────────────────────────────────────────────
target_link_libraries(IoTServer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::Qml
    # Qt::Network intentionally removed — POSIX sockets used via Socket.h
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
