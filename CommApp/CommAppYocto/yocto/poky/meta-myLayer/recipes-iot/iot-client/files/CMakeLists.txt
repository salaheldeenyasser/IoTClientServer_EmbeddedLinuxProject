cmake_minimum_required(VERSION 3.16)

project(iot-client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Executable ───────────────────────────────────────────────────────────────
add_executable(iot_client
    client_main.cpp
    # Header-only OOP classes — list them so IDEs show them in the project
    Socket.h
    Channel.h
)

# ── Compile definitions (override at cmake time for custom IP / GPIO) ─────────
# Example: cmake -DDEFAULT_SERVER_IP=192.168.0.50 -DLED_GPIO=27 ..
if(DEFINED DEFAULT_SERVER_IP)
    target_compile_definitions(iot_client PRIVATE
        DEFAULT_SERVER_IP="${DEFAULT_SERVER_IP}")
endif()

if(DEFINED DEFAULT_SERVER_PORT)
    target_compile_definitions(iot_client PRIVATE
        DEFAULT_SERVER_PORT=${DEFAULT_SERVER_PORT})
endif()

if(DEFINED LED_GPIO)
    target_compile_definitions(iot_client PRIVATE
        LED_GPIO=${LED_GPIO})
endif()

# ── Install ───────────────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS iot_client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
