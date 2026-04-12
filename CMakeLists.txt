cmake_minimum_required(VERSION 3.16)

project(MqttMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(PahoMqttCpp REQUIRED)

add_executable(MqttMonitor
    main.cpp
    resources.qrc
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    src/core/MqttClient.cpp
    src/core/MqttClient.h
    src/core/MessageBuffer.cpp
    src/core/MessageBuffer.h
    src/core/ConfigStore.cpp
    src/core/ConfigStore.h
    src/bridge/MqttBridge.cpp
    src/bridge/MqttBridge.h
    src/ui/ConfigPanel.cpp
    src/ui/ConfigPanel.h
    src/ui/ConfigPanel.ui
    src/ui/DeviceView.cpp
    src/ui/DeviceView.h
    src/ui/DeviceCard.cpp
    src/ui/DeviceCard.h
    src/ui/AddDeviceDialog.cpp
    src/ui/AddDeviceDialog.h
)

set_target_properties(MqttMonitor PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(MqttMonitor PRIVATE
    Qt5::Widgets
    PahoMqttCpp::paho-mqttpp3
    )

# vcpkg's PahoMqttCpp config does not propagate the dllimport define,
# which leaves mqtt::message::EMPTY_STR and similar static members unresolved.
if(MSVC)
    target_compile_definitions(MqttMonitor PRIVATE PAHO_MQTTPP_IMPORTS)
    # Treat all source files as UTF-8 (avoids GBK misparse on Chinese Windows)
    # Also suppresses C4819: non-ASCII chars in third-party Paho headers
    target_compile_options(MqttMonitor PRIVATE /utf-8 /wd4819)
    # DWM API for dark title bar
    target_link_libraries(MqttMonitor PRIVATE dwmapi)
endif()

include(GNUInstallDirs)
install(TARGETS MqttMonitor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)